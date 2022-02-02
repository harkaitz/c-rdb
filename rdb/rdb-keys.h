#ifndef RDB_KEYS_H
#define RDB_KEYS_H

#include <stdbool.h>
#include <uuid/uuid.h>

#define RDB_K_STORE alloca(sizeof(rdb_k))

typedef struct rdb_k {
    char s[512+64];
} rdb_k;

const char *rdb_children_key (const char *_id, rdb_k *_store);
const char *rdb_data_key     (const char *_id, rdb_k *_store);
const char *rdb_parents_key  (const char *_id, rdb_k *_store);
const char *rdb_user_key     (rdb_k *_store);
const char *rdb_key (const char *_prefix, uuid_t _uuid, rdb_k *_store);

#endif
