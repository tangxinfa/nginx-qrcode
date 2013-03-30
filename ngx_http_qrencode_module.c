/*
 * Copyright 2012 Alex Chamberlain
 */

/* Nginx Includes */
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <png.h>
#include <qrencode.h>

/**
 * Types
 */

/* Main Configuration */
typedef struct {
} ngx_http_qrencode_main_conf_t;

/* Location Configuration */
typedef struct {
    ngx_uint_t                version;
    ngx_uint_t                level;
    ngx_uint_t                size;
    ngx_uint_t                margin;
    ngx_http_complex_value_t* data;
} ngx_http_qrencode_loc_conf_t;

/**
 * Public Interface
 */

/* Module definition. */
// Forward definitions - variables
static ngx_http_module_t ngx_http_qrencode_module_ctx;
static ngx_command_t ngx_http_qrencode_commands[];

// Forward definitions - functions
static ngx_int_t ngx_http_qrencode_init_worker(ngx_cycle_t* cycle);

ngx_module_t ngx_http_qrencode_module = {
    NGX_MODULE_V1,
    &ngx_http_qrencode_module_ctx,
    ngx_http_qrencode_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    ngx_http_qrencode_init_worker,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};

/* Module context. */
// Forward declarations - functions
static void* ngx_http_qrencode_create_main_conf(ngx_conf_t* directive);
static void* ngx_http_qrencode_create_loc_conf(ngx_conf_t* directive);
static char* ngx_http_qrencode_merge_loc_conf(ngx_conf_t* directive, void* parent, void* child);

static ngx_http_module_t ngx_http_qrencode_module_ctx = {
    NULL, /* preconfiguration */
    NULL, /* postconfiguration */
    ngx_http_qrencode_create_main_conf,
    NULL, /* init main configuration */
    NULL, /* create server configuration */
    NULL, /* init server configuration */
    ngx_http_qrencode_create_loc_conf,
    ngx_http_qrencode_merge_loc_conf
};

/* Array specifying how to handle configuration directives. */
// Forward declarations - functions
char * ngx_http_config_qrencode(ngx_conf_t *cf, ngx_command_t *cmd, void *dummy);

static ngx_command_t ngx_http_qrencode_commands[] = {
    {
        ngx_string("qrencode"),
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
        ngx_http_config_qrencode,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    {
        ngx_string("qrencode_version"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_qrencode_loc_conf_t, version),
        NULL
    },
    {
        ngx_string("qrencode_level"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_qrencode_loc_conf_t, level),
        NULL
    },
    {
        ngx_string("qrencode_size"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_qrencode_loc_conf_t, size),
        NULL
    },
    {
        ngx_string("qrencode_margin"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_qrencode_loc_conf_t, margin),
        NULL
    },
    {
        ngx_string("qrencode_data"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_http_set_complex_value_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_qrencode_loc_conf_t, data),
        NULL
    },
    ngx_null_command
};


static ngx_int_t ngx_http_qrencode_handler(ngx_http_request_t* request);

static ngx_int_t ngx_http_qrencode_init_worker(ngx_cycle_t* cycle) {
    return NGX_OK;
}

/* Parse the 'qrencode' directive. */
char * ngx_http_config_qrencode(ngx_conf_t *cf, ngx_command_t *cmd, void *void_conf) {
    ngx_http_core_loc_conf_t *core_conf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    core_conf-> handler = ngx_http_qrencode_handler;

    return NGX_CONF_OK;
}

static void *ngx_http_qrencode_create_main_conf(ngx_conf_t *cf) {
    ngx_http_qrencode_main_conf_t  *qrencode_main_conf;

    qrencode_main_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_qrencode_main_conf_t));
    if (qrencode_main_conf == NULL) {
        return NULL;
    }

    return qrencode_main_conf;
}

static void* ngx_http_qrencode_create_loc_conf(ngx_conf_t* directive) {
    ngx_http_qrencode_loc_conf_t* qrencode_conf;

    qrencode_conf = ngx_pcalloc(directive->pool, sizeof(ngx_http_qrencode_loc_conf_t));
    if (qrencode_conf == NULL) {
        ngx_conf_log_error(NGX_LOG_EMERG, directive, 0, "Failed to allocate memory for qrencode Location Config.");
        return NGX_CONF_ERROR;
    }
    qrencode_conf->version = NGX_CONF_UNSET_UINT;
    qrencode_conf->level =  NGX_CONF_UNSET_UINT;
    qrencode_conf->size =  NGX_CONF_UNSET_UINT;
    qrencode_conf->margin =  NGX_CONF_UNSET_UINT;
    
    return qrencode_conf;
}

static char* ngx_http_qrencode_merge_loc_conf(ngx_conf_t* cf, void* void_parent, void* void_child) {
    ngx_http_qrencode_loc_conf_t *prev = void_parent;
    ngx_http_qrencode_loc_conf_t *conf = void_child;
    
    ngx_conf_merge_uint_value(conf->version, prev->version, 5);
    ngx_conf_merge_uint_value(conf->level, prev->level, QR_ECLEVEL_H);
    ngx_conf_merge_uint_value(conf->size, prev->size, 3);
    ngx_conf_merge_uint_value(conf->margin, prev->margin, 0);
    if(conf->data == NULL){
        conf->data = prev->data;
    }

    return NGX_CONF_OK;
}

typedef struct qrcode_png_data_t
{
    u_char* buffer;
    size_t size;
    ngx_http_request_t* request;
} qrcode_png_data;

void qrcode_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
    qrcode_png_data* p = (qrcode_png_data*)png_get_io_ptr(png_ptr);
    size_t nsize = p->size + length;
    if(p->buffer){
        u_char* buffer = ngx_pcalloc(p->request->pool, nsize);
        if(buffer){
            memcpy(buffer, p->buffer, p->size);
        }
        p->buffer = buffer;
    }else{
        p->buffer = ngx_pcalloc(p->request->pool, nsize);
    }
    if(! p->buffer){
        ngx_log_error(NGX_LOG_ERR, p->request->connection->log, 0, "Failed to allocate png write buffer");
        return;
    }

    /* copy new bytes to end of buffer */
    memcpy(p->buffer + p->size, data, length);
    p->size += length;
}

static ngx_int_t qrcode_draw(qrcode_png_data* data, QRcode *qrcode, int margin, int size, ngx_http_request_t* request)
{
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned char *row, *p, *q;
    int x, y, xx, yy, bit;
    int realwidth;

    realwidth = (qrcode->width + margin * 2) * size;
    row = (unsigned char *)ngx_pcalloc(request->pool, (realwidth + 7) / 8);
    if (row == NULL) {
        ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "Failed to allocate png row memory");
        return NGX_ERROR;
    }
    
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "Failed to initialize png writer struct");
        return NGX_ERROR;
    }
    
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "Failed to initialize png info struct");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return NGX_ERROR;
    }
    
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "Failed to write png image");
        return NGX_ERROR;
    }

    png_set_write_fn(png_ptr, data, qrcode_write, NULL);
    png_set_IHDR(png_ptr, info_ptr,
                 realwidth, realwidth,
                 1,
                 PNG_COLOR_TYPE_GRAY,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);
    
    /* top margin */
    memset(row, 0xff, (realwidth + 7) / 8);
    for (y = 0; y < margin * size; y++) {
        png_write_row(png_ptr, row);
    }
    
    /* data */
    p = qrcode->data;
    for (y = 0; y < qrcode->width; y++) {
        bit = 7;
        memset(row, 0xff, (realwidth + 7) / 8);
        q = row;
        q += margin * size / 8;
        bit = 7 - (margin * size % 8);
        for (x = 0; x < qrcode->width; x++) {
            for (xx = 0; xx < size; xx++) {
                *q ^= (*p & 1) << bit;
                bit--;
                if (bit < 0) {
                    q++;
                    bit = 7;
                }
            }
            p++;
        }
        for (yy = 0; yy < size; yy++) {
            png_write_row(png_ptr, row);
        }
    }
    /* bottom margin */
    memset(row, 0xff, (realwidth + 7) / 8);
    for (y = 0; y < margin * size; y++) {
        png_write_row(png_ptr, row);
    }
    
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    return NGX_OK;
}

static ngx_int_t ngx_http_qrencode_handler(ngx_http_request_t* request) {
    ngx_http_qrencode_loc_conf_t* qrencode_conf = ngx_http_get_module_loc_conf(request, ngx_http_qrencode_module);

    ngx_str_t qrencode_data;
    if(NGX_OK != ngx_http_complex_value(request, qrencode_conf->data, &qrencode_data)){
        ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "Failed retrieve qrencode_data config value");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    
    char* data;
    if(qrencode_data.data && qrencode_data.len > 0){
        data = ngx_pcalloc(request->pool, qrencode_data.len + 1);
        memcpy(data, qrencode_data.data, qrencode_data.len);
    }else if(request->headers_in.referer){
        data = ngx_pcalloc(request->pool, request->headers_in.referer->value.len + 1);        
        memcpy(data, request->headers_in.referer->value.data, request->headers_in.referer->value.len);
    }else{
        ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "No data to qrencode");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;        
    }

    ngx_log_error(NGX_LOG_DEBUG, request->connection->log, 0, "qrencode config, vesion:%d, level:%d, size:%d, margin:%d, data:%s", qrencode_conf->version, qrencode_conf->level, qrencode_conf->size, qrencode_conf->margin, data);

    QRcode* code = QRcode_encodeString8bit(data, qrencode_conf->version, qrencode_conf->level);
    if(code == NULL) {
        ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "Failed to encode address.");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    qrcode_png_data* draw_data = ngx_pcalloc(request->pool, sizeof(qrcode_png_data));
    draw_data->request = request;
    if(draw_data == NULL){
        ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "Failed to allocate png data buffer");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    if(NGX_OK != qrcode_draw(draw_data, code, qrencode_conf->margin, qrencode_conf->size, request)){
        QRcode_free(code);
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    QRcode_free(code);
  
    request->headers_out.status = NGX_HTTP_OK;
    request->headers_out.content_length_n = draw_data->size;
    ngx_str_set(&request->headers_out.content_type, "image/png");
    ngx_http_send_header(request);

    /* Allocate space for the response buffer */
    ngx_buf_t* buffer;
    buffer = ngx_pcalloc(request->pool, sizeof(ngx_buf_t));
    if (buffer == NULL) {
        ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "Failed to allocate response buffer");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    /* Set up the buffer chain */
    buffer->pos = draw_data->buffer;
    buffer->last = draw_data->buffer + draw_data->size; // Don't write NULL
    buffer->memory = 1;
    buffer->last_buf = 1;
    ngx_chain_t out;
    out.buf = buffer;
    out.next = NULL;

    /* Serve the Chunk */
    return ngx_http_output_filter(request, &out);
}
