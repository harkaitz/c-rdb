
#include "../rdb/rdb.h"
#include <str/str2num.h>
#include <io/fcopy.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <uuid/uuid.h>
#include <libgen.h>

#define COPYRIGHT_LINE \
    "Donate bitcoin: 1C1ZzDje7vHhF23mxqfcACE8QD4nqxywiV" "\n" \
    "Copyright (c) 2022 Harkaitz Agirre, harkaitz.aguirre@gmail.com" "\n" \
    ""

const char help[] =
    "Usage: %s  ..."                                                        "\n"
    ""                                                                      "\n"
    "This program helps handing objects in a Redis database."               "\n"
    ""                                                                      "\n"
    "... own-ls  PARENT          : List object belonging to PARENT."        "\n"
    "... own-add PARENT ID       : Link data to PARENT."                    "\n"
    "... own-del PARENT ID       : Unlink data from PARENT."                "\n"
    "... is-owned ID PARENT      : Check ID is owned by parent (recursive)" "\n"
    "... owner    ID             : Get owner."                              "\n"
    ""                                                                      "\n"
    "... add ID [DATA]   : Add data to a key and print ID."                 "\n"
    "... get ID          : Get data."                                       "\n"
    "... del ID          : Delete data from a key."                         "\n"
    ""                                                                      "\n"
    ""
    
    COPYRIGHT_LINE;

int main (int _argc, char *_argv[]) {
    _argv[0] = basename(_argv[0]);

    int           res          = 1;
    int           retval       = 1;
    void         *data         = NULL;
    size_t        datasz       = 0;
    bool          data_free    = true;
    FILE         *data_fp      = NULL;
    redisContext *db           = NULL;
    int           opt          = 1;
    bool          b1;
    char         *a1,*a2,*a3;
    
    
    /* Print help. */
    if (_argc == 1 || !strcmp(_argv[1], "--help") || !strcmp(_argv[1], "-h")) {
        fprintf(stderr, help, _argv[0]);
        goto cleanup;
    }
    openlog(_argv[0], LOG_PERROR, 0);

    /* Connect to Redis. */
    res = rdb_create(&db, NULL);
    if (!res/*err*/) goto cleanup;

    /* Get command and argument count. */
    a1 = (opt<_argc)?_argv[opt++]:NULL;
    a2 = (opt<_argc)?_argv[opt++]:NULL;
    a3 = (opt<_argc)?_argv[opt++]:NULL;
    switch(str2num(a1, strcasecmp,
                   "own-ls",   1,
                   "own-add",  2,
                   "own-del",  3,
                   "is-owned", 4,
                   "owner",    5,
                   "add",      6,
                   "get",      7,
                   "del",      8,
                   NULL)) {
    case 1:
        if (!a2/*err*/) goto cleanup_invalid_arguments;
        res = rdbo_ls_children(db, a2, stdout);
        if (!res/*err*/) goto cleanup;
        break;
    case 2:
        if (!a2||!a3/*err*/) goto cleanup_invalid_arguments;
        res = rdbo_add_children(db, a2, a3);
        if (!res/*err*/) goto cleanup;
        break;
    case 3:
        if (!a2||!a3/*err*/) goto cleanup_invalid_arguments;
        res = rdbo_del_children(db, a2, a3);
        if (!res/*err*/) goto cleanup;
        break;
    case 4:
        if (!a2||!a3/*err*/) goto cleanup_invalid_arguments;
        res = rdbo_is_owned_by(db, a2, a3, &b1);
        if (!res/*err*/) goto cleanup;
        retval = (b1)?0:2;
        break;
    case 5:
        if (!a2/*err*/) goto cleanup_invalid_arguments;
        res = rdbo_get_owner(db, a2, (char**) &data);
        if (!res/*err*/) goto cleanup;
        if (data) {
            printf("%s\n", (char*)data);
        }
        break;
    case 6:
        if (!a2/*err*/) goto cleanup_invalid_arguments;
        if (a3) {
            data      = a3;
            datasz    = strlen(a3);
            data_free = false;
            ((char*)data)[datasz++] = '\n';
        } else {
            data_fp = open_memstream((char**)&data, &datasz);
            if (!data_fp/*err*/) goto cleanup_errno;
            res = fcopy_ff(data_fp, stdin, 0, NULL);
            if (res<0/*err*/) goto cleanup_errno;
            res = fflush(data_fp);
            if (res==EOF/*err*/) goto cleanup_errno;
        }
        res = rdb_add_m(db, a2, datasz, data);
        if (!res/*err*/) goto cleanup;
        break;
    case 7:
        if (!a2/*err*/) goto cleanup_invalid_arguments;
        res = rdb_get_m(db, a2, &b1, &datasz, &data);
        if (!res/*err*/) goto cleanup;
        if (!b1/*err*/) goto cleanup_not_found;
        fwrite(data, 1, datasz, stdout);
        break;
    case 8:
        if (!a2/*err*/) goto cleanup_invalid_arguments;
        res = rdbo_delete(db, a2);
        if (!res/*err*/) goto cleanup;
        break;
    default:
        goto cleanup_invalid_command;
    }

    /* Success return. */
    retval = (retval==1)?0:retval;
    goto cleanup;
 cleanup_invalid_command:
    syslog(LOG_ERR, "Invalid command: %s", a1);
    goto cleanup;
 cleanup_invalid_arguments:
    syslog(LOG_ERR, "Invalid argument count.");
    goto cleanup;
 cleanup_errno:
    syslog(LOG_ERR, "%s", strerror(errno));
    goto cleanup;
 cleanup_not_found:
    syslog(LOG_ERR, "Not found.");
    goto cleanup;
 cleanup:
    openlog(_argv[0], 0, 0);
    if (data && data_free) free(data);
    if (data_fp)           fclose(data_fp);
    if (db)                rdb_destroy(db);
    return retval;
}
