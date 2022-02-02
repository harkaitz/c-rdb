#include "rdb-keys.h"
#include <sys/authorization.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

const char *
rdb_children_key(const char *_id, rdb_k *_store) {
    if (_id && _id[0] && strlen(_id)<512) {
        sprintf(_store->s, "RDB:CHILDREN:%s", _id);
        return _store->s;
    } else {
        syslog(LOG_ERR, "rdb: Invalid ID: %s", (_id)?_id:"(NULL)");
        return NULL;
    }
}

const char *
rdb_data_key(const char *_id, rdb_k *_store) {
    if (_id && _id[0] && strlen(_id)<512) {
        sprintf(_store->s, "RDB:DATA:%s", _id);
        return _store->s;
    } else {
        syslog(LOG_ERR, "rdb: Invalid ID: %s", (_id)?_id:"(NULL)");
        return NULL;
    }
}

const char *
rdb_parents_key(const char *_id, rdb_k *_store) {
    if (_id && _id[0] && strlen(_id)<512) {
        sprintf(_store->s, "RDB:PARENTS:%s", _id);
        return _store->s;
    } else {
        syslog(LOG_ERR, "rdb: Invalid ID: %s", (_id)?_id:"(NULL)");
        return NULL;
    }
}

const char *
rdb_user_key(rdb_k *_store) {
    const char *username  = authorization_get_username();
    if (username) {
        sprintf(_store->s, "user:%s", username);
        return _store->s;
    } else {
        syslog(LOG_ERR, "Unauthorized: Not logged in.");
        return NULL;
    }
}

const char *
rdb_key(const char *_prefix, uuid_t _uuid, rdb_k *_store) {
    size_t _prefixsz = (_prefix)?strlen(_prefix):0;
    if (_uuid && !uuid_is_null(_uuid)) {
        if (_prefixsz) memcpy(_store->s, _prefix, _prefixsz+1);
        uuid_unparse_lower(_uuid, _store->s+_prefixsz);
        return _store->s;
    } else {
        syslog(LOG_ERR, "rdb: Null uuid id.");
        return NULL;
    }
}
