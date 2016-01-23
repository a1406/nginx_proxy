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
#include "conn_srv_hash.h"

ngx_int_t conn_srv_init_module(ngx_cycle_t *cycle);
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
    conn_srv_init_module,                                  /* init module */
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
static ngx_pool_t *conn_buf_pool;
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

static int _add_list_chain(conn_buf_list *list, uint32_t size)
{
	ngx_buf_t *buf = ngx_create_temp_buf(conn_buf_pool, size);
	if (!buf)
		return (-1);
	ngx_chain_t *chain = ngx_alloc_chain_link(conn_buf_pool);
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

static int add_list_buf(conn_buf_list *list, int big)
{
	ngx_chain_t *chain;
	if (big)
	{
		chain = _del_list_chain(&free_big_buf_list);
		if (!chain)
			return _add_list_chain(list, DEFAULT_BIG_BUF_SIZE);		
	}
	else
	{
		chain = _del_list_chain(&free_normal_buf_list);
		if (!chain)
			return _add_list_chain(list, DEFAULT_NORMAL_BUF_SIZE);
		
	}
	_buf_list_pushback(list, chain);
	return (0);
}

static void move_chain(conn_buf_list *from, conn_buf_list *to)
{
	ngx_chain_t *chain = _del_list_chain(from);
	assert(chain);
	_buf_list_pushback(to, chain);
}

static int change_to_big_buf(conn_buf_list *list)
{
	ngx_chain_t *chain = list->last;
	assert(chain);
	assert(chain->buf->end - chain->buf->start == DEFAULT_NORMAL_BUF_SIZE);

	ngx_chain_t *big_chain = _del_list_chain(&free_big_buf_list);
	if (!big_chain)
		_add_list_chain(&free_big_buf_list, DEFAULT_BIG_BUF_SIZE);
	big_chain = _del_list_chain(&free_big_buf_list);
	if (!big_chain) {
		return (-1);
	}
	int size = chain->buf->last - chain->buf->pos;
	assert(size == sizeof(PROTO_HEAD));
	memcpy(big_chain->buf->pos, chain->buf->pos, size);
	big_chain->buf->last = big_chain->buf->pos + size;
	chain->buf->pos = chain->buf->last = chain->buf->start;
	
	ngx_buf_t *tmp_buf = chain->buf;
	chain->buf = big_chain->buf;
	big_chain->buf = tmp_buf;
	_buf_list_pushback(&free_normal_buf_list, big_chain);
	return (0);
}

static int del_list_buf(conn_buf_list *list)
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
	uint16_t login_seq;  //登录的seq号，登录包返回的时候比较这个seq，不一致则丢弃
	uint16_t seq;       //客户端发包的seq号，每次加1
	EXTERN_DATA ext_data;
	ngx_stream_session_t *session;
	conn_buf_list send;
	conn_buf_list recv;
} conn_node_data;

static conn_node_data *game_srv_session;
static conn_node_data *login_srv_session;
static conn_node_data client_session[UINT16_MAX + 1];
static dictht *client_player_id_hash;

static void conn_srv_close_connection(ngx_connection_t *c)
{
    conn_node_data *conn_node = c->data;
	if (!conn_node)
		goto done;
	int ret;
	do {
		ret = del_list_buf(&conn_node->recv);
	} while (ret == 0);
	do {
		ret = del_list_buf(&conn_node->send);
	} while (ret == 0);
done:
	ngx_stream_close_connection(c);
}

static void transfer_to_login_srv(conn_node_data *conn_node)
{
	move_chain(&conn_node->recv, &login_srv_session->send);
	ngx_connection_t *c = login_srv_session->session->connection;
	if (ngx_handle_write_event(c->write, 0) != NGX_OK)
		conn_srv_close_connection(c);
}
__attribute__((unused)) static void transfer_to_game_srv(conn_node_data *conn_node)
{
	move_chain(&conn_node->recv, &game_srv_session->send);
	ngx_connection_t *c = game_srv_session->session->connection;
	if (ngx_handle_write_event(c->write, 0) != NGX_OK)
		conn_srv_close_connection(c);
}

static void transfer_to_client_fd(conn_node_data *conn_node, uint32_t fd)
{
	assert(fd <= UINT16_MAX);
	conn_node_data *client = &client_session[fd];
	if (client->session)
		return;
	move_chain(&conn_node->recv, &client->send);
	ngx_connection_t *c = client->session->connection;
	if (ngx_handle_write_event(c->write, 0) != NGX_OK)
		conn_srv_close_connection(c);
}

static conn_node_data *get_client_by_playerid(uint64_t player_id)
{
	dictVal key;
	key.u64 = player_id;
	dictEntry *val = dictFind(client_player_id_hash, key);
	if (!val)
		return NULL;
	return val->val.val;
}

__attribute__((unused)) static void transfer_to_player(conn_node_data *conn_node, uint64_t player_id)
{
	conn_node_data *client = get_client_by_playerid(player_id);
	if (!client || client->session)
		return;
	move_chain(&conn_node->recv, &client->send);
	ngx_connection_t *c = client->session->connection;
	if (ngx_handle_write_event(c->write, 0) != NGX_OK)
		conn_srv_close_connection(c);
}


static EXTERN_DATA *get_extern_data(PROTO_HEAD *head)
{
	return (EXTERN_DATA *)(&head->data[htons(head->len) - sizeof(PROTO_HEAD) - sizeof(EXTERN_DATA)]);
}
static void add_extern_data(PROTO_HEAD *head, EXTERN_DATA *data)
{
	EXTERN_DATA *extern_data;
	int old_len = htons(head->len);
	extern_data = (EXTERN_DATA *)(&head->data[0] + old_len - sizeof(PROTO_HEAD));
	memcpy(extern_data, data, sizeof(EXTERN_DATA));
	head->len = htons(old_len + sizeof(EXTERN_DATA));
}

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

    int                        size;
    ssize_t                       n;
    ngx_buf_t                     *b;
    ngx_uint_t                    flags;

    for ( ;; ) {
        if (ev->write && conn_node->send.first) {
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
						del_list_buf(&conn_node->send);
                    }
                }
            }
        }
		b = conn_node->recv.last->buf;
		assert(b->pos == b->start);
		uint32_t already_read = b->last - b->pos;
		if (already_read >= sizeof(PROTO_HEAD)) {
			PROTO_HEAD *head = (PROTO_HEAD *)(b->pos);
			size = htons(head->len) + sizeof(EXTERN_DATA) - already_read;
			assert(size > 0);
		} else {
			size = sizeof(PROTO_HEAD) - already_read;
		}
//        size = b->end - b->last;
		assert(size > 0);

        if (size && c->read->ready && !c->read->delayed) {
            n = c->recv(c, b->last, size);
            if (n == NGX_AGAIN || n == 0) {
                break;
            }
            if (n > 0) {
                b->last += n;
				already_read = b->last - b->pos;
				if (already_read >= sizeof(PROTO_HEAD)) {
					PROTO_HEAD *head = (PROTO_HEAD *)(b->pos);
					assert(htons(head->len) >= already_read);
					if (htons(head->len) == already_read) {
//						add_extern_data(head, c);
						add_list_buf(&conn_node->recv, 0);						
					} else if (already_read == sizeof(PROTO_HEAD)
						&& (b->end - b->start) <  (int)(htons(head->len) + sizeof(EXTERN_DATA))) {
						if (htons(head->len) + sizeof(EXTERN_DATA) > DEFAULT_BIG_BUF_SIZE) {
							ngx_log_error(NGX_LOG_ERR, c->log, 0, "msg need size[%u] too large", htons(head->len));
							c->read->eof = 1;
							break;
						}
						change_to_big_buf(&conn_node->recv);
					}
				}
                continue;
            }

            if (n == NGX_ERROR) {
                c->read->eof = 1;
            }
        }
        break;
    }

    if (c->read->eof) {
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
	int ret = on_event_handle(ev);
	if (ret != 0)
	{
		ngx_connection_t *c = ev->data;
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: game srv disconnected", __FUNCTION__, __LINE__);
		conn_srv_close_connection(c);
		game_srv_session = NULL;
	}
}
static void conn_srv_login_srv_handler(ngx_event_t *ev)
{
	assert(login_srv_session);
	int ret = on_event_handle(ev);
	ngx_connection_t *c = ev->data;
	conn_node_data *client = login_srv_session;
	ngx_chain_t *chain;
	for (chain = client->recv.first; chain;)
	{
		size_t len = chain->buf->last - chain->buf->pos;
		if (len < sizeof(PROTO_HEAD))
			break;
		PROTO_HEAD *head = (PROTO_HEAD *)chain->buf->pos;
		if (htons(head->len) != len)
			break;
		EXTERN_DATA *ext = get_extern_data(head);
		transfer_to_client_fd(client, ext->fd);
	}
	
	if (ret != 0)
	{
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: login srv disconnected", __FUNCTION__, __LINE__);
		conn_srv_close_connection(c);
		login_srv_session = NULL;
	}
}
static void conn_srv_client_handler(ngx_event_t *ev)
{
	int ret = on_event_handle(ev);
	ngx_connection_t *c = ev->data;
	conn_node_data *client = &client_session[c->fd];
	ngx_chain_t *chain;
	for (chain = client->recv.first; chain;)
	{
		size_t len = chain->buf->last - chain->buf->pos;
		if (len < sizeof(PROTO_HEAD))
			break;
		PROTO_HEAD *head = (PROTO_HEAD *)chain->buf->pos;
		if (htons(head->len) != len)
			break;
		add_extern_data(head, &client->ext_data);
		transfer_to_login_srv(client);
	}
	
	if (ret != 0)
	{
		assert(client == c->data);
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: client[%d][%lu] disconnected", __FUNCTION__, __LINE__, c->fd, client->ext_data.player_id);
		client->session = NULL;
		conn_srv_close_connection(c);
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
	add_list_buf(&data->send, 0);
	add_list_buf(&data->recv, 0);
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
		conn_srv_close_connection(c);
		return;
	}
	game_srv_session = ngx_palloc(c->pool, sizeof(*game_srv_session));
	if (!game_srv_session)
	{
		ngx_log_error(NGX_LOG_ERR, c->log, 0, "game srv alloc failed");
		conn_srv_close_connection(c);
		return;
	}
	
	if (init_conn_node_data(game_srv_session, s) != 0)
	{
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: game srv failed", __FUNCTION__, __LINE__);
		conn_srv_close_connection(c);
		return;
	}
	game_srv_session->ext_data.fd = c->fd;
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
		conn_srv_close_connection(c);
		return;
	}
	login_srv_session = ngx_palloc(c->pool, sizeof(*login_srv_session));
	if (!login_srv_session)
	{
		ngx_log_error(NGX_LOG_ERR, c->log, 0, "login srv alloc failed");
		conn_srv_close_connection(c);
		return;
	}
	
	if (init_conn_node_data(login_srv_session, s) != 0)
	{
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: login srv failed", __FUNCTION__, __LINE__);
		conn_srv_close_connection(c);
		return;
	}
	login_srv_session->ext_data.fd = c->fd;	
	ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: login srv connected", __FUNCTION__, __LINE__);
	
	c->write->handler = conn_srv_login_srv_handler;
    c->read->handler = conn_srv_login_srv_handler;
}

static void on_client_connected(ngx_stream_session_t *s)
{
    ngx_connection_t                *c;
	c = s->connection;
	assert(c->fd <= UINT16_MAX);
	assert(client_session[c->fd].session == NULL);
	if (init_conn_node_data(&client_session[c->fd], s) != 0)
	{
		ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: client srv failed", __FUNCTION__, __LINE__);
		conn_srv_close_connection(c);		
		return;
	}
	client_session[c->fd].ext_data.fd = c->fd;		
	ngx_log_error(NGX_LOG_INFO, c->log, 0, "%s %d: client[%d] connected", __FUNCTION__, __LINE__, c->fd);
	
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
    return NGX_CONF_OK;
}

static dictType u64hash_type =
{
	dictIntHashFunction,
	NULL,
	NULL,
	NULL,
	NULL,
    NULL
};

ngx_int_t conn_srv_init_module(ngx_cycle_t *cycle)
{
	conn_buf_pool = ngx_create_pool(1024, cycle->log);
	client_player_id_hash = dictCreate(&u64hash_type);
	return NGX_OK;
}
