#include "rdb.h"
#include <types/long_ss.h>
#include <str/str2num.h>
#include <hiredis/hiredis.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

bool rdb_create(redisContext **_db, const char *opts[]) {
    int           res         = 0;
    const char   *redis_host  = "127.0.0.1";
    long          redis_port  = 6379;
    redisContext *redis_ctx   = NULL;
    
    for (const char **opt = opts; opts && *opt; opt+=2) {
        switch(str2num(*opt, strcasecmp, "redis_host", 1, "redis_port", 2, NULL)) {
        case 1:
            redis_host = *(opts+1);
            break;
        case 2:
            res = long_parse(&redis_port, *(opts+1), NULL);
            if (!res/*err*/) goto cleanup_invalid_port;
            break;
        }
    }

    redis_ctx = redisConnect(redis_host, redis_port);
    if (!redis_ctx/*err*/) goto cleanup_errno;
    if (redis_ctx->errstr && redis_ctx->err/*err*/) goto cleanup_redis_error;
    *_db = redis_ctx;
    return true;
 cleanup_errno:
    syslog(LOG_ERR, "%s", strerror(errno));
    goto cleanup;
 cleanup_invalid_port:
    syslog(LOG_ERR, "Invalid 'redis_port' value.");
    goto cleanup;
 cleanup_redis_error:
    syslog(LOG_ERR, "Redis: %s.", redis_ctx->errstr);
    goto cleanup;
 cleanup:
    if (redis_ctx) redisFree(redis_ctx);
    return false;
}

void rdb_destroy(redisContext *_db) {
    redisFree(_db);
}

void rdb_reset(redisReply **_ret_opt) {
    if (_ret_opt && *_ret_opt) {
        freeReplyObject(*_ret_opt);
        *_ret_opt = NULL;
    }
}

bool rdb_cmd_c(redisContext *_db, redisReply **_opt_r, rdb_c *_cmd) {
    bool        ret = false;
    redisReply *res = NULL;
    const char *err = NULL;
    rdb_reset(_opt_r);
    res = redisCommandArgv(_db, _cmd->argc, _cmd->argv, _cmd->argvlen_opt);
    if (!res/*err*/) goto cleanup_redis_error;
    if (res->type == REDIS_REPLY_ERROR/*err*/) goto cleanup_error_reply;
    if (_cmd->validator_opt && !_cmd->validator_opt(res, &err)/*err*/) goto cleanup_invalid_response;
    if (_opt_r) {
        *_opt_r = res;
        res = NULL;
    }
    ret = true;
    goto cleanup;
  cleanup_redis_error:
    syslog(LOG_ERR, "Redis: %s: %s", _cmd->argv[0], _db->errstr);
    goto cleanup;
  cleanup_error_reply:
    syslog(LOG_ERR, "Redis: %s: Reply: %s", _cmd->argv[0], (res->str)?res->str:"Unknown error.");
    goto cleanup;
  cleanup_invalid_response:
    syslog(LOG_ERR, "Redis: %s: Invalid response: %s", _cmd->argv[0], err);
    goto cleanup;
  cleanup:
    if (res) freeReplyObject(res);
    return ret;
}

bool rdb_cmd_v(redisContext *_db, redisReply **_opt_r, redis_verify_f _opt_v, const char _cmd[], va_list _va) {
    rdb_c c = {0};
    c.validator_opt = _opt_v;
    c.argv[0] = _cmd;
    while ((c.argv[++c.argc] = va_arg(_va, const char*))) { }
    return rdb_cmd_c(_db, _opt_r, &c);
}

bool rdb_cmd(redisContext *_db, redisReply **_opt_r, redis_verify_f _opt_v, const char _cmd[], ...) {
    va_list va;
    va_start(va, _cmd);
    bool ret = rdb_cmd_v(_db, _opt_r, _opt_v, _cmd, va);
    va_end(va);
    return ret;
}

bool rdb_cmd_ignr(redisContext *_db, const char _cmd[], ...) {
    va_list va;
    va_start(va, _cmd);
    bool ret = rdb_cmd_v(_db, NULL, NULL, _cmd, va);
    va_end(va);
    return ret;
}

bool rdb_is_integer(redisReply *_r, const char **_opt_errmsg) {
    if (_r->type == REDIS_REPLY_INTEGER) {
        return true;
    } else {
        if (_opt_errmsg) *_opt_errmsg = "Not an integer";
        return false;
    }
}

bool rdb_is_string(redisReply *_r, const char **_opt_errmsg) {
    if (_r->type == REDIS_REPLY_STRING) {
        return true;
    } else {
        if (_opt_errmsg) *_opt_errmsg = "Not a string";
        return false;
    }
}

bool rdb_is_string_or_nil(redisReply *_r, const char **_opt_errmsg) {
    if (_r->type == REDIS_REPLY_STRING || _r->type == REDIS_REPLY_NIL) {
        return true;
    } else {
        if (_opt_errmsg) *_opt_errmsg = "Not nil or string";
        return false;
    }
}

bool rdb_is_string_array(redisReply *_r, const char **_opt_errmsg) {
    if (_r->type != REDIS_REPLY_ARRAY) {
        if (_opt_errmsg) *_opt_errmsg = "Not an array";
        return false;
    }
    for (size_t i=0; i<_r->elements; i++) {
        if (_r->element[i]->type != REDIS_REPLY_STRING) {
            if (_opt_errmsg) *_opt_errmsg = "Array element not a string";
            return false;
        }
    }
    return true;
}
