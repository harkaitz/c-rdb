#include "rdb-validators.h"
#include <hiredis/hiredis.h>

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
