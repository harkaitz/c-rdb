#ifndef RDB_VALIDATORS_H
#define RDB_VALIDATORS_H

#include <stdbool.h>

typedef struct redisReply redisReply;

bool rdb_is_integer       (redisReply *_r, const char **_opt_errmsg);
bool rdb_is_string        (redisReply *_r, const char **_opt_errmsg);
bool rdb_is_string_or_nil (redisReply *_r, const char **_opt_errmsg);
bool rdb_is_string_array  (redisReply *_r, const char **_opt_errmsg);

#endif
