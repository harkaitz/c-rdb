#include "../rdb/rdb.h"
#include <security/pam_appl.h>
#include <security/pam_appl_weblogin.h>
#include <str/str2num.h>
#include <sys/authorization.h>
#include <io/fcopy.h>
#include <libgen.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define COPYRIGHT_LINE \
    "Donate bitcoin: 1C1ZzDje7vHhF23mxqfcACE8QD4nqxywiV" "\n" \
    "Copyright (c) 2022 Harkaitz Agirre, harkaitz.aguirre@gmail.com" "\n" \
    ""

const char help[] =
    "Usage: %s [-s PAMSERVICE][-l] ..."                            "\n"
    ""                                                             "\n"
    "This program helps testing RDBS module. The program will"     "\n"
    "authenticate with PAMSERVICE. You can force a login with -l." "\n"
    ""                                                             "\n"
    "... ls            : List your objects."                       "\n"
    "... add ID [DATA] : Add an object."                           "\n"
    "... get ID        : Get object."                              "\n"
    "... del ID        : Delete object."                           "\n"
    ""                                                             "\n"
    COPYRIGHT_LINE;

int main (int _argc, char *_argv[]) {
    _argv[0] = basename(_argv[0]);

    int             opt,res;
    char           *pamservice   = "weblogin";
    bool            force_login  = false;
    pam_handle_t   *pamh         = NULL;
    pam_talker      talk         = { 0 };
    redisContext   *db           = NULL;
    void           *data         = NULL;
    size_t          datasz       = 0;
    FILE           *datafp       = NULL;
    bool            datafree     = false;
    int             retval       = 1;
    char           *a1,*a2,*a3;
    bool            b1;
    
    /* Print help. */
    if (_argc == 1 || !strcmp(_argv[1], "--help") || !strcmp(_argv[1], "-h")) {
        fprintf(stderr, help, _argv[0]);
        goto cleanup;
    }

    /* Initialize logging. */
    openlog(_argv[0], LOG_PERROR, 0);

    /* Get options. */
    while ((opt = getopt (_argc, _argv, "s:l")) != -1) {
        switch (opt) {
        case 's': pamservice = optarg; break;
        case 'l': force_login = true; break;
        case '?':
        default:
            return 1;
        }
    }

    /* Initialize PAM conversation. */
    res = pam_start_talker(pamservice, NULL, &talk, &pamh);

    /* Load the cookie. */
    if (force_login) {
        res = pam_weblogin_delete_cookie(pamh);
    } else {
        res = pam_weblogin_try_loading_cookie(pamh);
    }
    if (!res/*err*/) goto cleanup;

    /* Authenticate. */
    res = pam_authenticate(pamh, 0);
    if (res != PAM_SUCCESS/*err*/) goto cleanup_pam_auth;
    res = pam_weblogin_try_saving_cookie(pamh);
    if (!res/*err*/) goto cleanup;
    if (optind == _argc) {
        retval = 0;
        goto cleanup;
    }
    res = authorization_open_pam(pamh);
    if (!res/*err*/) goto cleanup;
    
    
    /* Connect to redis. */
    res = rdb_create(&db, NULL);
    if (!res/*err*/) goto cleanup;

    /* Get command and argument count. */
    opt = optind;
    a1 = (opt<_argc)?_argv[opt++]:NULL;
    a2 = (opt<_argc)?_argv[opt++]:NULL;
    a3 = (opt<_argc)?_argv[opt++]:NULL;
    switch(str2num(a1, strcasecmp, "ls", 1, "add", 2, "get", 3, "del", 4, NULL)) {
    case 1:
        res = rdbs_ls(db, stdout);
        if (!res/*err*/) goto cleanup;
        break;
    case 2:
        if (!a2/*err*/) goto cleanup_invalid_arguments;
        if (a3) {
            data     = a3;
            datasz   = strlen(a3);
            datafree = false;
            ((char*)data)[datasz++] = '\n';
        } else {
            datafp = open_memstream((char**)&data, &datasz);
            if (!datafp/*err*/) goto cleanup_errno;
            res = fcopy_ff(datafp, stdin, 0, NULL);
            if (res<0/*err*/) goto cleanup_errno;
            res = fflush(datafp);
            if (res==EOF/*err*/) goto cleanup_errno;
        }
        res = rdbs_add_m(db, a2, datasz, data);
        if (!res/*err*/) goto cleanup;
        break;
    case 3:
        if (!a2/*err*/) goto cleanup_invalid_arguments;
        res = rdbs_get_m(db, a2, &b1, &datasz, &data);
        if (!res/*err*/) goto cleanup;
        if (!b1/*err*/) goto cleanup_not_found;
        fwrite(data, 1, datasz, stdout);
        break;
    case 4:
        if (!a2/*err*/) goto cleanup_invalid_arguments;
        res = rdbs_delete(db, a2);
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
 cleanup_pam_auth:
    syslog(LOG_ERR, "pam: %s", pam_strerror(pamh, res));
    goto cleanup;
 cleanup:
    openlog(_argv[0], 0, 0);
    if (data && datafree) free(data);
    if (datafp)           fclose(datafp);
    if (db)               rdb_destroy(db);
    if (pamh)             pam_close_session(pamh, PAM_SILENT);
    return retval;
}
