#include "rdb-m.h"
#include "rdb-cmd.h"
#include "rdb-keys.h"
#include "rdb-validators.h"
#include <hiredis/hiredis.h>
#include <string.h>
#include <syslog.h>

bool rdb_add_m(redisContext *_db, const char _id[], size_t _dsz, const void *_d) {
    rdb_k       key_s = {0};
    rdb_c       cmd   = {0};
    cmd.argc    = 3;
    cmd.argv[0] = "SET";
    cmd.argv[1] = rdb_data_key(_id, &key_s);
    if (!cmd.argv[1]/*err*/) return false;
    cmd.argv[2] = _d;
    cmd.argvlen_opt_d[0] = strlen(cmd.argv[0]);
    cmd.argvlen_opt_d[1] = strlen(cmd.argv[1]);
    cmd.argvlen_opt_d[2] = _dsz;
    cmd.argvlen_opt = cmd.argvlen_opt_d;
    return rdb_cmd_c(_db, NULL, &cmd);
}

bool rdb_get_m(redisContext *_db, const char _id[], bool *_found, size_t *_dsz, void **_d) {
    redisReply *reply  = NULL;
    rdb_k       key_s  = {0};
    const char *key    = rdb_data_key(_id, &key_s);
    int         res    = 0;
    bool        retval = false;
    if (key) {
        res = rdb_cmd(_db, &reply, rdb_is_string_or_nil, "GET", key, NULL);
        if (!res/*err*/) goto cleanup;
    }
    if (!key || reply->type == REDIS_REPLY_NIL) {
        if (_dsz) { *_dsz   = 0; }
        if (_d)   { *_d = 0;     }
        if (!_found/*err*/) goto cleanup_not_found;
        *_found = false;
    } else {
        if (_dsz)   { *_dsz   = reply->len; }
        if (_found) { *_found = true;       }
        if (_d)     { *_d = reply->str; reply->str = NULL; }
    }
    retval = true;
    goto cleanup;
 cleanup_not_found:
    syslog(LOG_ERR, "%s: Not found.", key_s.s);
    goto cleanup;
 cleanup:
    if (reply) freeReplyObject(reply);
    return retval;
}

bool rdb_get_m_cp(redisContext *_db, const char _id[], bool *_found, size_t _dsz, void *_d) {
    void  *d   = NULL;
    size_t dsz = 0;
    if (!rdb_get_m(_db, _id, _found, &dsz, &d)) return false;
    if (!d) return true;
    if (dsz > _dsz) {
        if (_found) *_found = false;
        free(d);
        return true;
    } else {
        memset(_d, 0, _dsz);
        memcpy(_d, d,  dsz);
        free(d);
        return true;
    }
}
