#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_stream.h>

static void *conn_srv_create_srv_conf(ngx_conf_t *cf);
static char *conn_srv_game_srv(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t  conn_srv_commands[] = {
    { ngx_string("game_srv"),
      NGX_STREAM_SRV_CONF|NGX_CONF_TAKE1,
      conn_srv_game_srv,
      NGX_STREAM_SRV_CONF_OFFSET,
      0,
      NULL },
	ngx_null_command	
};

static ngx_stream_module_t  conn_srv_module_ctx = {
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    conn_srv_create_srv_conf,      /* create server configuration */
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

static void *conn_srv_create_srv_conf(ngx_conf_t *cf)
{
	return NULL;
/*	
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
*/    
}


static char *conn_srv_game_srv(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
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
