#include "ngx_config.h"
#include "ngx_core.h"
#include "ngx_stream.h"
#include "ngx_buf.h"
#include "ngx_array.h"
#include "ngx_hash.h"
#include "ngx_list.h"
#include <stdint.h>
#include <assert.h>
#include "include/server_proto.h"

//static void *conn_srv_create_srv_conf(ngx_conf_t *cf);
static char *conn_srv_game_srv(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *conn_srv_login_srv(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *conn_srv_client(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t  conn_srv_commands[] = {
    { ngx_string("game_srv"),
      NGX_STREAM_SRV_CONF|NGX_CONF_NOARGS,
      conn_srv_game_srv,
      NGX_STREAM_SRV_CONF_OFFSET,
      0,
      NULL },
    { ngx_string("client"),
      NGX_STREAM_SRV_CONF|NGX_CONF_NOARGS,
      conn_srv_client,
      NGX_STREAM_SRV_CONF_OFFSET,
      0,
      NULL },
    { ngx_string("login_srv"),
      NGX_STREAM_SRV_CONF|NGX_CONF_NOARGS,
      conn_srv_login_srv,
      NGX_STREAM_SRV_CONF_OFFSET,
      0,
      NULL },
	ngx_null_command	
};

static ngx_stream_module_t  conn_srv_module_ctx = {
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL, //conn_srv_create_srv_conf,      /* create server configuration */
    NULL                              /* merge server configuration */
};


ngx_module_t  conn_srv_module = {
    NGX_MODULE_V1,
    &conn_srv_module_ctx,          /* module context */
    conn_srv_commands,             /* module directives */
    NGX_STREAM_MODULE,                     /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};
/*	
static void *conn_srv_create_srv_conf(ngx_conf_t *cf)
{
    ngx_stream_proxy_srv_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_stream_proxy_srv_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->connect_timeout = NGX_CONF_UNSET_MSEC;
    conf->timeout = NGX_CONF_UNSET_MSEC;
    conf->next_upstream_timeout = NGX_CONF_UNSET_MSEC;
    conf->buffer_size = NGX_CONF_UNSET_SIZE;
    conf->upload_rate = NGX_CONF_UNSET_SIZE;
    conf->download_rate = NGX_CONF_UNSET_SIZE;
    conf->next_upstream_tries = NGX_CONF_UNSET_UINT;
    conf->next_upstream = NGX_CONF_UNSET;
    conf->proxy_protocol = NGX_CONF_UNSET;
    conf->local = NGX_CONF_UNSET_PTR;

    return conf;
}
*/
#define DEFAULT_NORMAL_BUF_SIZE (512)
#define DEFAULT_BIG_BUF_SIZE (51200)

typedef struct {
	ngx_chain_t *first;	
	ngx_chain_t *last;	
} conn_buf_list;
static conn_buf_list free_normal_buf_list;
static conn_buf_list free_big_buf_list;

static void _buf_list_pushback(conn_buf_list *list, ngx_chain_t *buf)
{
	buf->next = NULL;
	if (list->last)
	{
		assert(list->last->next == NULL);
		list->last->next = buf;
	}
	else
	{
		assert(list->first == NULL);
		list->first = buf;
	}
	list->last = buf;
}

static int _add_list_chain(ngx_pool_t *pool, conn_buf_list *list, uint32_t size)
{
	ngx_buf_t *buf = ngx_create_temp_buf(pool, size);
	if (!buf)
		return (-1);
	ngx_chain_t *chain = ngx_alloc_chain_link(pool);
	if (!chain)
		return (-10);
	chain->buf = buf;
	_buf_list_pushback(list, chain);
	return (0);
}
static ngx_chain_t *_del_list_chain(conn_buf_list *list)
{
	ngx_chain_t *buf = list->first;
	if (!buf)
		return NULL;
	list->first = buf->next;
	if (list->last == buf)
	{
		assert(buf->next == NULL);
		list->last = buf->next;
	}
	return buf;
}

static int add_list_buf(ngx_pool_t *pool, conn_buf_list *list, int big)
{
	ngx_chain_t *chain;
	if (big)
	{
		chain = _del_list_chain(&free_big_buf_list);
		if (!chain)
			return _add_list_chain(pool, list, DEFAULT_BIG_BUF_SIZE);		
	}
	else
	{
		chain = _del_list_chain(&free_normal_buf_list);
		if (!chain)
			return _add_list_chain(pool, list, DEFAULT_NORMAL_BUF_SIZE);
		
	}
	_buf_list_pushback(list, chain);
	return (0);
}

__attribute__((unused)) static int del_list_buf(conn_buf_list *list)
{
	ngx_chain_t *chain = _del_list_chain(list);
	if (!chain)
		return (-1);
	chain->buf->last = chain->buf->pos = chain->buf->start;
	if (chain->buf->end - chain->buf->start == DEFAULT_NORMAL_BUF_SIZE)
	{
		_buf_list_pushback(&free_normal_buf_list, chain);
	}
	else
	{
		assert(chain->buf->end - chain->buf->start == DEFAULT_BIG_BUF_SIZE);
		_buf_list_pushback(&free_big_buf_list, chain);		
	}
	return (0);
}

typedef struct {
	ngx_stream_session_t *session;
	conn_buf_list send;
	conn_buf_list recv;
} conn_node_data;

typedef struct {
	uint16_t login_seq;  //登录的seq号，登录包返回的时候比较这个seq，不一致则丢弃
	uint16_t seq;       //客户端发包的seq号，每次加1
	uint32_t open_id;
	uint64_t player_id;
	conn_node_data session;
} game_client;

static conn_node_data *game_srv_session;
static conn_node_data *login_srv_session;
//static ngx_stream_session_t *login_srv_session;
static game_client client_session[UINT16_MAX + 1];

static int on_event_handle(ngx_event_t *ev)
{
    ngx_connection_t             *c;
    conn_node_data *conn_node;

    c = ev->data;
    conn_node = c->data;

    if (ev->timedout) {
        if (ev->delayed) {
            ev->timedout = 0;
            ev->delayed = 0;
            if (!ev->ready) {
                if (ngx_handle_read_event(ev, 0) != NGX_OK) {
                    return -1;
                }
            }
        } else {
            ngx_connection_error(c, NGX_ETIMEDOUT, "connection timed out");
            return -10;
        }

    } else if (ev->delayed) {
        ngx_log_debug0(NGX_LOG_DEBUG_STREAM, c->log, 0,
                       "stream connection delayed");
        if (ngx_handle_read_event(ev, 0) != NGX_OK) {
			return -20;
        }

        return 0;
    }

    size_t                        size;
    ssize_t                       n;
    ngx_buf_t                     *b;
    ngx_uint_t                    flags;

    for ( ;; ) {
        if (ev->write) {
			b = conn_node->send.first->buf;
            size = b->last - b->pos;
            if (size && c->write->ready) {
                n = c->send(c, b->pos, size);
                if (n == NGX_ERROR) {
                    return -30;
                }
                if (n > 0) {
                    b->pos += n;

                    if (b->pos == b->last) {
                        b->pos = b->start;
                        b->last = b->start;
                    }
                }
            }
        }
		b = conn_node->recv.first->buf;		

        size = b->end - b->last;

        if (size && c->read->ready && !c->read->delayed) {
            n = c->recv(c, b->last, size);
            if (n == NGX_AGAIN || n == 0) {
                break;
            }
            if (n > 0) {
                b->last += n;
                continue;
            }

            if (n == NGX_ERROR) {
                c->read->eof = 1;
            }
        }
        break;
    }

    if (c->read->eof) {
//        handler = c->log->handler;
//        c->log->handler = NULL;

//		ngx_log_error(NGX_LOG_ERR, c->log, 0, "game srv disconnected");
		

//        ngx_log_error(NGX_LOG_INFO, c->log, 0,
//                      "%s disconnected"
//                      ", bytes from/to client:%O/%O"
//                      ", bytes from/to upstream:%O/%O",
//                      from_upstream ? "upstream" : "client",
//                      s->received, c->sent, u->received, pc ? pc->sent : 0);

			//      c->log->handler = handler;

        return -40;
    }

    flags = c->read->eof ? NGX_CLOSE_EVENT : 0;

    if (ngx_handle_read_event(c->read, flags) != NGX_OK) {
        return -50;
    }

    return 0;
}

static void conn_srv_game_srv_handler(ngx_event_t *ev)
{
	assert(game_srv_session);
	if (on_event_handle(ev) != 0)
	{
		ngx_connection_t *c = ev->data;
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: game srv disconnected", __FUNCTION__, __LINE__);
		ngx_stream_close_connection(c);
		game_srv_session = NULL;
	}
}
static void conn_srv_login_srv_handler(ngx_event_t *ev)
{
	assert(login_srv_session);
	if (on_event_handle(ev) != 0)
	{
		ngx_connection_t *c = ev->data;
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: login srv disconnected", __FUNCTION__, __LINE__);
		ngx_stream_close_connection(c);
		login_srv_session = NULL;
	}
}
static void conn_srv_client_handler(ngx_event_t *ev)
{
	if (on_event_handle(ev) != 0)
	{
		ngx_connection_t *c = ev->data;
		game_client *client = &client_session[c->fd];
		assert(&client->session == c->data);
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: client[%d][%lu] disconnected", __FUNCTION__, __LINE__, c->fd, client->player_id);
		client->session.session = NULL;
		ngx_stream_close_connection(c);
	}
}

static int init_conn_node_data(conn_node_data *data, ngx_stream_session_t *s)
{
    ngx_connection_t                *c;
	c = s->connection;
	c->data = data;

    if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
		ngx_log_error(NGX_LOG_ERR, c->log, 0, "%s %d: fail", __FUNCTION__, __LINE__);		
		return -1;
    }

	memset(data, 0, sizeof(*data));
	add_list_buf(c->pool, &data->send, 0);
	add_list_buf(c->pool, &data->recv, 0);
	data->session = s;
	return (0);
}

static void on_game_srv_connected(ngx_stream_session_t *s)
{
    ngx_connection_t                *c;
	c = s->connection;
	if (game_srv_session)
	{
		ngx_log_error(NGX_LOG_ERR, c->log, 0, "game srv already exist");
		ngx_stream_close_connection(c);
		return;
	}
	game_srv_session = ngx_palloc(c->pool, sizeof(*game_srv_session));
	if (!game_srv_session)
	{
		ngx_log_error(NGX_LOG_ERR, c->log, 0, "game srv alloc failed");
		ngx_stream_close_connection(c);
		return;
	}
	
	if (init_conn_node_data(game_srv_session, s) != 0)
	{
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: game srv failed", __FUNCTION__, __LINE__);
		ngx_stream_close_connection(c);
		return;
	}
	ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: game srv connected", __FUNCTION__, __LINE__);
	
	c->write->handler = conn_srv_game_srv_handler;
    c->read->handler = conn_srv_game_srv_handler;
}

static void on_login_srv_connected(ngx_stream_session_t *s)
{
    ngx_connection_t  *c;
	c = s->connection;
	if (login_srv_session)
	{
		ngx_log_error(NGX_LOG_ERR, c->log, 0, "login srv already exist");
		ngx_stream_close_connection(c);
		return;
	}
	login_srv_session = ngx_palloc(c->pool, sizeof(*login_srv_session));
	if (!login_srv_session)
	{
		ngx_log_error(NGX_LOG_ERR, c->log, 0, "login srv alloc failed");
		ngx_stream_close_connection(c);
		return;
	}
	
	if (init_conn_node_data(login_srv_session, s) != 0)
	{
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: login srv failed", __FUNCTION__, __LINE__);
		ngx_stream_close_connection(c);
		return;
	}
	ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: login srv connected", __FUNCTION__, __LINE__);
	
	c->write->handler = conn_srv_login_srv_handler;
    c->read->handler = conn_srv_login_srv_handler;
}

static void on_client_connected(ngx_stream_session_t *s)
{
    ngx_connection_t                *c;
	c = s->connection;
	assert(c->fd <= UINT16_MAX);
	assert(client_session[c->fd].session.session == NULL);
	if (init_conn_node_data(&client_session[c->fd].session, s) != 0)
	{
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: client srv failed", __FUNCTION__, __LINE__);
		ngx_stream_close_connection(c);		
		return;
	}
	static uint64_t player_id = 100;
	client_session[c->fd].player_id = player_id++;
	
	ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: client[%d][%lu] connected", __FUNCTION__, __LINE__, c->fd, client_session[c->fd].player_id);
	
	c->write->handler = conn_srv_client_handler;
    c->read->handler = conn_srv_client_handler;
}

static char *conn_srv_login_srv(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_stream_core_srv_conf_t  *cscf;
    cscf = ngx_stream_conf_get_module_srv_conf(cf, ngx_stream_core_module);
    cscf->handler = on_login_srv_connected;
    return NGX_CONF_OK;	
}
static char *conn_srv_client(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_stream_core_srv_conf_t  *cscf;
    cscf = ngx_stream_conf_get_module_srv_conf(cf, ngx_stream_core_module);
    cscf->handler = on_client_connected;
    return NGX_CONF_OK;	
}

static char *conn_srv_game_srv(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_stream_core_srv_conf_t  *cscf;
    cscf = ngx_stream_conf_get_module_srv_conf(cf, ngx_stream_core_module);
    cscf->handler = on_game_srv_connected;
/*	
    ngx_stream_proxy_srv_conf_t *pscf = conf;

    ngx_url_t                    u;
    ngx_str_t                   *value, *url;
    ngx_stream_core_srv_conf_t  *cscf;

    if (pscf->upstream) {
        return "is duplicate";
    }

    cscf = ngx_stream_conf_get_module_srv_conf(cf, ngx_stream_core_module);

    cscf->handler = ngx_stream_proxy_handler;

    value = cf->args->elts;

    url = &value[1];

    ngx_memzero(&u, sizeof(ngx_url_t));

    u.url = *url;
    u.no_resolve = 1;

    pscf->upstream = ngx_stream_upstream_add(cf, &u, 0);
    if (pscf->upstream == NULL) {
        return NGX_CONF_ERROR;
    }
*/
    return NGX_CONF_OK;
}
