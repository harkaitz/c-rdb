#include "rdbs.h"
#include "rdbo.h"
#include "rdb-keys.h"
#include "rdb-m.h"
#include <stdlib.h>
#include <syslog.h>
#include <string.h>

bool rdbs_ls (redisContext *_db, FILE *_fp) {
    const char *user_key  = rdb_user_key(RDB_K_STORE);
    if (!user_key/*err*/) return false;
    return rdbo_ls_children(_db, user_key, _fp);
}

bool rdbs_we_are_authorized (redisContext *_db, const char _id[], rdb_k *_o_opt_user_key) {
    bool        res       = false;
    bool        is_owned  = false;
    bool        exists    = false;
    const char *user_key  = rdb_user_key((_o_opt_user_key)?_o_opt_user_key:RDB_K_STORE);
    if (!user_key/*err*/) return false;
    res = rdbo_is_owned_by(_db, _id, user_key, &is_owned);
    if (!res/*err*/) return false;
    res = rdbo_exists(_db, _id, &exists);
    if (!res/*err*/) return false;
    if (is_owned == false && exists == true) {
        syslog(LOG_ERR, "Unauthorized.");
        return false;
    }
    return true;
}

bool rdbs_delete (redisContext *_db, const char _id[]) {
    if (!rdbs_we_are_authorized(_db, _id, NULL)) return false;
    if (!rdbo_delete(_db, _id))                  return false;
    return true;
}

bool rdbs_add_m (redisContext *_db, const char _id[], size_t _dsz, const void *_d) {
    rdb_k user_key;
    if (!rdbs_we_are_authorized(_db, _id, &user_key)) return false;
    if (!rdb_add_m(_db, _id, _dsz, _d))               return false;
    if (!rdbo_add_children(_db, user_key.s, _id))     return false;
    return true;
}

bool rdbs_get_m (redisContext *_db, const char _id[], bool *_found, size_t *_dsz, void **_d) {
    bool    retval = false;
    size_t  dsz    = 0;
    void   *d      = NULL;
    bool    found  = false;
    if (!rdbs_we_are_authorized(_db, _id, NULL)) goto cleanup;
    if (!rdb_get_m(_db, _id, &found, &dsz, &d))  goto cleanup;
    if (_found) { *_found = found; }
    if (_d)     { *_d = d; d = NULL; }
    if (_dsz)   { *_dsz = dsz; }
    retval = true;
 cleanup:
    if (d) free(d);
    return retval;
}

bool rdbs_get_m_cp (redisContext *_db, const char _id[], bool *_found, size_t _dsz, void *_d) {
    bool    retval = false;
    size_t  dsz    = 0;
    void   *d      = NULL;
    if (!rdbs_we_are_authorized(_db, _id, NULL)) goto cleanup;
    if (!rdbs_get_m(_db, _id, _found, &dsz, &d)) goto cleanup;
    if (!d) { retval = true; goto cleanup; }
    if (dsz > _dsz) { if (_found) *_found = false; goto cleanup; }
    memset(_d, 0, _dsz);
    memcpy(_d, d,  dsz);
    retval = true;
 cleanup:
    if (d) free(d);
    return retval;
}
