#ifndef RDB_RDBO_H
#define RDB_RDBO_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct redisContext redisContext;

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

bool rdbo_exists       (redisContext *_db, const char _id[], bool *_exists);
bool rdbo_delete       (redisContext *_db, const char _id[]);
bool rdbo_ls_children  (redisContext *_db, const char _id[], FILE *_o_fp);
bool rdbo_add_children (redisContext *_db, const char _id[], const char _child_id[]);
bool rdbo_del_children (redisContext *_db, const char _id[], const char _child_id[]);
bool rdbo_is_owned_by  (redisContext *_db, const char _id[], const char _parent_id[], bool *_o);
bool rdbo_get_owner    (redisContext *_db, const char _id[], char **_o_m);

#endif
