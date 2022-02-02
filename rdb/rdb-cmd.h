#ifndef RDB_CMD_H
#define RDB_CMD_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct redisReply    redisReply;
typedef struct redisContext  redisContext;
typedef bool (*redis_verify_f) (redisReply *_r, const char **_errmsg);

typedef struct rdb_c {
    redis_verify_f validator_opt;
    int            argc;
    const char    *argv[100];
    size_t        *argvlen_opt;
    size_t         argvlen_opt_d[100];
} rdb_c;

bool rdb_create  (redisContext **_db, const char *opts[]);
void rdb_destroy (redisContext  *_db);
void rdb_reset  (redisReply **_ret_opt);

bool rdb_cmd_c    (redisContext *_db, redisReply **_opt_r, rdb_c *_cmd);
bool rdb_cmd_v    (redisContext *_db, redisReply **_opt_r, redis_verify_f _opt_v, const char _cmd[], va_list _va);
bool rdb_cmd      (redisContext *_db, redisReply **_opt_r, redis_verify_f _opt_v, const char _cmd[], ...);
bool rdb_cmd_ignr (redisContext *_db, const char _cmd[], ...);

#endif
