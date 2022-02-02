#ifndef RDB_M_H
#define RDB_M_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct redisContext redisContext;

bool rdb_add_m    (redisContext *_db, const char _id[], size_t _dsz, const void *_d);
bool rdb_get_m    (redisContext *_db, const char _id[], bool *_found, size_t *_dsz, void **_d);
bool rdb_get_m_cp (redisContext *_db, const char _id[], bool *_found, size_t _dsz, void *_d);

#endif
