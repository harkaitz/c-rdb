#ifndef RDB_RDBS_H
#define RDB_RDBS_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct redisContext redisContext;
typedef struct rdb_k        rdb_k;

bool rdbs_ls (redisContext *_db, FILE *_fp);

bool rdbs_we_are_authorized (redisContext *_db, const char _id[], rdb_k *_o_opt_user_key);
bool rdbs_delete            (redisContext *_db, const char _id[]);
bool rdbs_add_m             (redisContext *_db, const char _id[], size_t _dsz, const void *_d);

bool rdbs_get_m    (redisContext *_db, const char _id[], bool *_found, size_t *_dsz, void **_d);
bool rdbs_get_m_cp (redisContext *_db, const char _id[], bool *_found, size_t  _dsz, void  *_d);

#endif
