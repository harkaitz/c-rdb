#include "rdbo.h"
#include "rdb-m.h"
#include "rdb-cmd.h"
#include "rdb-keys.h"
#include "rdb-validators.h"
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <hiredis/hiredis.h>

bool rdbo_exists(redisContext *_db, const char _id[], bool *_exists) {
    int         res  = 0;
    bool        ret  = false;
    const char *key1 = rdb_data_key     (_id, RDB_K_STORE); if (!key1/*err*/) return false;
    const char *key2 = rdb_children_key (_id, RDB_K_STORE); if (!key2/*err*/) return false;
    const char *key3 = rdb_parents_key  (_id, RDB_K_STORE); if (!key3/*err*/) return false;
    redisReply *rep1 = NULL;
    redisReply *rep2 = NULL;
    redisReply *rep3 = NULL;
    res = rdb_cmd(_db, &rep1, rdb_is_integer, "EXISTS", key1, NULL);
    if (!res/*err*/) goto cleanup;
    res = rdb_cmd(_db, &rep2, rdb_is_integer, "EXISTS", key2, NULL);
    if (!res/*err*/) goto cleanup;
    res = rdb_cmd(_db, &rep3, rdb_is_integer, "EXISTS", key3, NULL);
    if (!res/*err*/) goto cleanup;
    if (_exists) {
        *_exists = rep1->integer || rep2->integer || rep3->integer;
    }
    ret = true;
 cleanup:
    if(rep1) freeReplyObject(rep1);
    if(rep2) freeReplyObject(rep2);
    if(rep3) freeReplyObject(rep3);
    return ret;
}

bool rdbo_delete(redisContext *_db, const char _id[]) {
    redisReply *reply = NULL;
    const char *key_c = rdb_children_key(_id, RDB_K_STORE); if (!key_c/*err*/) return false;
    const char *key_p = rdb_parents_key (_id, RDB_K_STORE); if (!key_p/*err*/) return false;
    const char *key_d = rdb_data_key    (_id, RDB_K_STORE); if (!key_d/*err*/) return false;
    if (rdb_cmd(_db, &reply, rdb_is_string_array, "SMEMBERS", key_c, NULL)) {
        rdb_cmd_ignr(_db, "DEL", key_c, NULL);
        for (size_t i=0; i<reply->elements; i++) {
            rdbo_delete(_db, reply->element[i]->str);
        }
    }
    if (rdb_cmd(_db, &reply, rdb_is_string_array, "SMEMBERS", key_p, NULL)) {
        rdb_cmd_ignr(_db, "DEL", key_p, NULL);
        for (size_t i=0; i<reply->elements; i++) {
            rdb_k key_s; const char *s = reply->element[i]->str;
            rdb_cmd_ignr(_db, "SREM", rdb_children_key(s, &key_s), _id, NULL);
        }
    }
    rdb_cmd_ignr(_db, "DEL", key_d, NULL);
    freeReplyObject(reply);
    return true;
}

bool rdbo_ls_children (redisContext *_db, const char _id[], FILE *_o_fp) {
    int         res   = 0;
    redisReply *reply = NULL;
    const char *key_c = rdb_children_key(_id, RDB_K_STORE);
    if (!key_c/*err*/) return false;
    res = rdb_cmd(_db, &reply, rdb_is_string_array, "SMEMBERS", key_c, NULL);
    if (!res/*err*/) return false;
    for (size_t i=0; _o_fp && i<reply->elements; i++) {
        fprintf(_o_fp, "%s\n", reply->element[i]->str);
    }
    freeReplyObject(reply);
    return true;
}

bool rdbo_add_children (redisContext *_db, const char _id[], const char _child_id[]) {
    int         res   = 0;
    const char *key_p = rdb_parents_key (_child_id, RDB_K_STORE); if (!key_p/*err*/) return false;
    const char *key_c = rdb_children_key(_id      , RDB_K_STORE); if (!key_c/*err*/) return false;
    res = rdb_cmd_ignr(_db, "SADD", key_p, _id, NULL);
    if (!res/*err*/) return false;
    res = rdb_cmd_ignr(_db, "SADD", key_c, _child_id, NULL);
    if (!res/*err*/) return false;
    return true;
}

bool rdbo_del_children (redisContext *_db, const char _id[], const char _child_id[]) {
    int         res   = 0;
    const char *key_p = rdb_parents_key (_child_id, RDB_K_STORE); if (!key_p/*err*/) return false;
    const char *key_c = rdb_children_key(_id      , RDB_K_STORE); if (!key_c/*err*/) return false;
    res = rdb_cmd_ignr(_db, "SREM", key_p, _id, NULL);
    if (!res/*err*/) return false;
    res = rdb_cmd_ignr(_db, "SREM", key_c, _child_id, NULL);
    if (!res/*err*/) return false;
    return true;
}

bool rdbo_is_owned_by (redisContext *_db, const char _id[], const char _parent_id[], bool *_o) {
    bool        ret   = false;
    int         res   = 0;
    redisReply *reply = NULL;
    const char *key_p = rdb_parents_key (_id, RDB_K_STORE); if (!key_p/*err*/) return false;
    *_o = false;
    res = rdb_cmd(_db, &reply, rdb_is_integer, "SISMEMBER", key_p, _parent_id, NULL);
    if (!res/*err*/) goto cleanup;
    if (reply->integer) {
        *_o = true;
        ret = true;
        goto cleanup;
    }
    res = rdb_cmd(_db, &reply, rdb_is_string_array, "SMEMBERS", key_p, NULL);
    if (!res/*err*/) goto cleanup;
    for (int i=0; i<reply->elements && !*_o; i++) {
        res = rdbo_is_owned_by(_db, reply->element[i]->str, _parent_id, _o);
        if (!res/*err*/) goto cleanup;
    }
    ret = true;
 cleanup:
    if (reply) freeReplyObject(reply);
    return ret;
}

bool rdbo_get_owner (redisContext *_db, const char _id[], char **_o_m) {
    rdb_k       key_s;
    bool        ret      = false;
    int         res      = 0;
    redisReply *reply    = NULL;
    const char *key      = rdb_parents_key (_id, &key_s); if (!key/*err*/) return false;
    *_o_m = NULL;
    res = rdb_cmd(_db, &reply, rdb_is_string_array, "SMEMBERS", key, NULL);
    if (!res/*err*/) goto cleanup;
    for (int i=0; i<reply->elements; i++) {
        redisReply *e = reply->element[i];
        if (!strcmp(e->str, _id)) continue;
        *_o_m = e->str;
        e->str = NULL;
        break;
    }
    if (*_o_m) {
        ret = true;
        goto cleanup;
    }
    for (int i=0; !*_o_m && i<reply->elements; i++) {
        if (!strcmp(reply->element[i]->str, _id)) continue;
        res = rdbo_get_owner(_db, reply->element[i]->str, _o_m);
        if (!res/*err*/) goto cleanup;
    }
    ret = true;
 cleanup:
    if (reply) freeReplyObject(reply);
    return ret;
}
