#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ht.h"

/*
 * implementation of a convinient greedy ht_thread-safe line reading function to
 * avoid slow byte-wise reading from filedescriptors - which is important for
 * high-performance situations.
 */
ssize_t ht_readline_ev(int fd, void *buf, size_t buflen, ht_event_t ev_extra);
#define READLINE_MAXLEN 1024
static ht_key_t  readline_key;
static ht_once_t readline_once_ctrl = HT_ONCE_INIT;

typedef struct {
    int   rl_cnt;
    char *rl_bufptr;
    char  rl_buf[READLINE_MAXLEN];
} readline_buf;

static void readline_buf_destroy(void *vp)
{
    free(vp);
    return;
}

static void readline_init(void *vp)
{
    ht_key_create(&readline_key, readline_buf_destroy);
    return;
}

ssize_t ht_readline(int fd, void *buf, size_t buflen)
{
    return ht_readline_ev(fd, buf, buflen, NULL);
}

ssize_t ht_readline_ev(int fd, void *buf, size_t buflen, ht_event_t ev_extra)
{
    size_t n;
    ssize_t rc;
    char c = '\0', *cp;
    readline_buf *rl;

    ht_once(&readline_once_ctrl, readline_init, NULL);
    if ((rl = (readline_buf *)ht_key_getdata(readline_key)) == NULL) {
        rl = (readline_buf *)malloc(sizeof(readline_buf));
        rl->rl_cnt = 0;
        rl->rl_bufptr = NULL;
        ht_key_setdata(readline_key, rl);
    }
    cp = (char *)buf;
    for (n = 1; n < buflen; n++) {

        /* fetch one character (but read more) */
        rc = 1;
        if (rl->rl_cnt <= 0) {
            if ((rl->rl_cnt = ht_read_ev(fd, rl->rl_buf, READLINE_MAXLEN, ev_extra)) < 0)
                rc = -1;
            else if (rl->rl_cnt == 0)
                rc = 0;
            else
                rl->rl_bufptr = rl->rl_buf;
        }
        if (rc == 1) {
            rl->rl_cnt--;
            c = *rl->rl_bufptr++;
        }

        /* act on fetched character */
        if (rc == 1) {
            if (c == '\r') {
                n--;
                continue;
            }
            *cp++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0) {
            if (n == 1)
                return 0;
            else
                break;
        }
        else
            return -1;
    }
    *cp = NUL;
    return n;
}

