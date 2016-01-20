#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_stream.h>
#include <stdint.h>
#include <assert.h>
#include "ngx_array.h"

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
typedef struct {
	uint16_t login_seq;  //登录的seq号，登录包返回的时候比较这个seq，不一致则丢弃
	uint16_t seq;       //客户端发包的seq号，每次加1
	uint32_t open_id;
	uint64_t player_id;
	ngx_stream_session_t *session;
} game_client;

static ngx_stream_session_t *game_srv_session;
static ngx_stream_session_t *login_srv_session;
//static ngx_stream_session_t *login_srv_session;
static game_client client_session[UINT16_MAX + 1];

static int on_event_handle(ngx_event_t *ev)
{
    ngx_connection_t             *c;
    ngx_stream_session_t         *s;

    c = ev->data;
    s = c->data;

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
//    ngx_log_handler_pt            handler;

    for ( ;; ) {
        if (ev->write) {
			b = &s->upstream->upstream_buf;
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
		b = &s->upstream->downstream_buf;

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
		assert(client->session == c->data);
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: client[%d][%lu] disconnected", __FUNCTION__, __LINE__, c->fd, client->player_id);
		client_session[c->fd].session = NULL;
		ngx_stream_close_connection(c);
	}
}

static int init_server_session(ngx_stream_session_t *s)
{
    ngx_connection_t                *c;
    ngx_stream_upstream_t           *u;	
	c = s->connection;

    if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
		ngx_log_error(NGX_LOG_ERR, c->log, 0, "%s %d: fail", __FUNCTION__, __LINE__);		
		return -1;
    }
	
    u = ngx_pcalloc(c->pool, sizeof(ngx_stream_upstream_t));
    if (u == NULL) {
		ngx_log_error(NGX_LOG_ERR, c->log, 0, "%s %d: fail", __FUNCTION__, __LINE__);		
        return -10;
    }

    s->upstream = u;
    u->peer.log = c->log;
    u->peer.log_error = NGX_ERROR_ERR;
    u->peer.start_time = ngx_current_msec;
    u->start_sec = ngx_time();

    u_char *p1, *p2;	
    p1 = ngx_pnalloc(c->pool, 1024);
    if (p1 == NULL) {
		ngx_log_error(NGX_LOG_ERR, c->log, 0, "%s %d: fail", __FUNCTION__, __LINE__);		
        return -20;
    }
    u->downstream_buf.start = p1;
    u->downstream_buf.end = p1 + 1024;
    u->downstream_buf.pos = p1;
    u->downstream_buf.last = p1;

    p2 = ngx_pnalloc(c->pool, 1024);
    if (p2 == NULL) {
		ngx_log_error(NGX_LOG_ERR, c->log, 0, "%s %d: fail", __FUNCTION__, __LINE__);		
        return -30;
    }
    u->upstream_buf.start = p2;
    u->upstream_buf.end = p2 + 1024;
    u->upstream_buf.pos = p2;
    u->upstream_buf.last = p2;
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
	if (init_server_session(s) != 0)
	{
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: game srv failed", __FUNCTION__, __LINE__);
		ngx_stream_close_connection(c);
		return;
	}
	ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: game srv connected", __FUNCTION__, __LINE__);
	game_srv_session = s;
	
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
	if (init_server_session(s) != 0)
	{
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: login srv failed", __FUNCTION__, __LINE__);
		ngx_stream_close_connection(c);		
		return;
	}
	ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: login srv connected", __FUNCTION__, __LINE__);
	login_srv_session = s;
	
	c->write->handler = conn_srv_login_srv_handler;
    c->read->handler = conn_srv_login_srv_handler;
}

static void on_client_connected(ngx_stream_session_t *s)
{
    ngx_connection_t                *c;
	c = s->connection;
	assert(c->fd <= UINT16_MAX);
	assert(client_session[c->fd].session == NULL);
	client_session[c->fd].session = s;
	static uint64_t player_id = 100;
	client_session[c->fd].player_id = player_id++;

	if (init_server_session(s) != 0)
	{
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: client srv failed", __FUNCTION__, __LINE__);
		ngx_stream_close_connection(c);		
		return;
	}
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
