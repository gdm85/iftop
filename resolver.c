/*
 * resolver.c:
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>

#include "ns_hash.h"
#include "iftop.h"

#include "threadprof.h"



#define RESOLVE_QUEUE_LENGTH 20

struct in_addr resolve_queue[RESOLVE_QUEUE_LENGTH];

pthread_cond_t resolver_queue_cond;
pthread_mutex_t resolver_queue_mutex;

hash_type* ns_hash;

int head;
int tail;


/* 
 * We have a choice of resolver methods. Real computers have gethostbyaddr_r,
 * which is reentrant and therefore thread safe. Other machines don't, and so
 * we can use non-reentrant gethostbyaddr and have only one resolver thread.
 * Alternatively, we can use the MIT ares asynchronous DNS library to do this.
 */

#if defined(USE_GETHOSTBYADDR_R)
/**
 * Implementation of do_resolve for platforms with working gethostbyaddr_r
 *
 * Some implementations of libc choose to implement gethostbyaddr_r as
 * a non thread-safe wrapper to gethostbyaddr.  An interesting choice...
 */
char* do_resolve(struct in_addr * addr) {
    struct hostent hostbuf, *hp;
    size_t hstbuflen = 1024;
    char *tmphstbuf;
    int res;
    int herr;
    char * ret = NULL;

    /* Allocate buffer, remember to free it to avoid memory leakage.  */            
    tmphstbuf = xmalloc (hstbuflen);

    while ((res = gethostbyaddr_r ((char*)addr, sizeof(struct in_addr), AF_INET,
                                   &hostbuf, tmphstbuf, hstbuflen,
                                   &hp, &herr)) == ERANGE) {
        /* Enlarge the buffer.  */
        hstbuflen *= 2;
        tmphstbuf = realloc (tmphstbuf, hstbuflen);
      }

    /*  Check for errors.  */
    if (res || hp == NULL) {
        /* failed */
        /* Leave the unresolved IP in the hash */
    }
    else {
        ret = xstrdup(hp->h_name);

    }
    xfree(tmphstbuf);
    return ret;
}

#elif defined(USE_GETHOSTBYADDR)

/**
 * Implementation using gethostbyname. Since this is nonreentrant, we have to
 * wrap it in a mutex, losing all benefit of multithreaded resolution.
 */
char *do_resolve(struct in_addr *addr) {
    static pthread_mutex_t ghba_mtx = PTHREAD_MUTEX_INITIALIZER;
    char *s = NULL;
    struct hostent *he;
    pthread_mutex_lock(&ghba_mtx);
    he = gethostbyaddr((char*)addr, sizeof *addr, AF_INET);
    if (he)
        s = xstrdup(he->h_name);
    pthread_mutex_unlock(&ghba_mtx);
    return s;
}


#elif defined(USE_LIBRESOLV)

#include <arpa/nameser.h>
#include <resolv.h>

/**
 * libresolv implementation 
 * resolver functions may not be thread safe
 */
char* do_resolve(struct in_addr * addr) {
  char msg[PACKETSZ];
  char s[35];
  int l;
  unsigned char* a;
  char * ret = NULL;

  a = (unsigned char*)addr;

  snprintf(s, 35, "%d.%d.%d.%d.in-addr.arpa.",a[3], a[2], a[1], a[0]);

  l = res_search(s, C_IN, T_PTR, msg, PACKETSZ);
  if(l != -1) {
    ns_msg nsmsg;
    ns_rr rr;
    if(ns_initparse(msg, l, &nsmsg) != -1) {
      int c;
      int i;
      c = ns_msg_count(nsmsg, ns_s_an);
      for(i = 0; i < c; i++) {
        if(ns_parserr(&nsmsg, ns_s_an, i, &rr) == 0){
          if(ns_rr_type(rr) == T_PTR) {
            char buf[256];
            ns_name_uncompress(msg, msg + l, ns_rr_rdata(rr), buf, 256);
            ret = xstrdup(buf);
          }
        }
      }
    }
  }
  return ret;
}

#elif defined(USE_ARES)

/**
 * ares implementation
 */

#include <sys/time.h>
#include <ares.h>
#include <arpa/nameser.h>

/* callback function for ares */
struct ares_callback_comm {
    struct in_addr *addr;
    int result;
    char *name;
};

static void do_resolve_ares_callback(void *arg, int status, unsigned char *abuf, int alen) {
    struct hostent *he;
    struct ares_callback_comm *C;
    C = (struct ares_callback_comm*)arg;

    if (status == ARES_SUCCESS) {
        C->result = 1;
        ares_parse_ptr_reply(abuf, alen, C->addr, sizeof *C->addr, AF_INET, &he);
        C->name = xstrdup(he->h_name);;
        ares_free_hostent(he);
    } else {
        C->result = -1;
    }
}

char *do_resolve(struct in_addr * addr) {
    struct ares_callback_comm C;
    char s[35];
    unsigned char *a;
    ares_channel *chan;
    static pthread_mutex_t ares_init_mtx = PTHREAD_MUTEX_INITIALIZER;
    static pthread_key_t ares_key;
    static int gotkey;

    /* Make sure we have an ARES channel for this thread. */
    pthread_mutex_lock(&ares_init_mtx);
    if (!gotkey) {
        pthread_key_create(&ares_key, NULL);
        gotkey = 1;
        
    }
    pthread_mutex_unlock(&ares_init_mtx);
    
    chan = pthread_getspecific(ares_key);
    if (!chan) {
        chan = xmalloc(sizeof *chan);
        pthread_setspecific(ares_key, chan);
        if (ares_init(chan) != ARES_SUCCESS) return NULL;
    }
    
    a = (unsigned char*)addr;
    sprintf(s, "%d.%d.%d.%d.in-addr.arpa.", a[3], a[2], a[1], a[0]);
    
    C.result = 0;
    C.addr = addr;
    ares_query(*chan, s, C_IN, T_PTR, do_resolve_ares_callback, &C);
    while (C.result == 0) {
        int n;
        fd_set readfds, writefds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        n = ares_fds(*chan, &readfds, &writefds);
        ares_timeout(*chan, NULL, &tv);
        select(n, &readfds, &writefds, NULL, &tv);
        ares_process(*chan, &readfds, &writefds);
    }

    /* At this stage, the query should be complete. */
    switch (C.result) {
        case -1:
        case 0:     /* shouldn't happen */
            return NULL;

        default:
            return C.name;
    }
}

#else

#   warning No name resolution method specified; name resolution will not work

char *do_resolve(struct in_addr *addr) {
    return NULL;
}

#endif

void resolver_worker(void* ptr) {
/*    int thread_number = *(int*)ptr;*/
    pthread_mutex_lock(&resolver_queue_mutex);
    sethostent(1);
    while(1) {
        /* Wait until we are told that an address has been added to the 
         * queue. */
        pthread_cond_wait(&resolver_queue_cond, &resolver_queue_mutex);

        /* Keep resolving until the queue is empty */
        while(head != tail) {
            char * hostname;
            struct in_addr addr = resolve_queue[tail];

            /* mutex always locked at this point */

            tail = (tail + 1) % RESOLVE_QUEUE_LENGTH;

            pthread_mutex_unlock(&resolver_queue_mutex);

            hostname = do_resolve(&addr);

            /*
             * Store the result in ns_hash
             */
            pthread_mutex_lock(&resolver_queue_mutex);

            if(hostname != NULL) {
                char* old;
                if(hash_find(ns_hash, &addr, (void**)&old) == HASH_STATUS_OK) {
                    hash_delete(ns_hash, &addr);
                    xfree(old);
                }
                hash_insert(ns_hash, &addr, (void*)hostname);
            }

        }
    }
}

void resolver_initialise() {
    int* n;
    int i;
    pthread_t thread;
    head = tail = 0;

    ns_hash = ns_hash_create();
    
    pthread_mutex_init(&resolver_queue_mutex, NULL);
    pthread_cond_init(&resolver_queue_cond, NULL);

    for(i = 0; i < 2; i++) {
        n = (int*)xmalloc(sizeof *n);
        *n = i;
        pthread_create(&thread, NULL, (void*)&resolver_worker, (void*)n);
    }

}

void resolve(struct in_addr* addr, char* result, int buflen) {
    char* hostname;
    int added = 0;

    pthread_mutex_lock(&resolver_queue_mutex);

    if(hash_find(ns_hash, addr, (void**)&hostname) == HASH_STATUS_OK) {
        /* Found => already resolved, or on the queue */
    }
    else {
        hostname = strdup(inet_ntoa(*addr));
        hash_insert(ns_hash, addr, hostname);

        if(((head + 1) % RESOLVE_QUEUE_LENGTH) == tail) {
            /* queue full */
        }
        else {
            resolve_queue[head] = *addr;
            head = (head + 1) % RESOLVE_QUEUE_LENGTH;
            added = 1;
        }
    }
    pthread_mutex_unlock(&resolver_queue_mutex);

    if(added == 1) {
        pthread_cond_signal(&resolver_queue_cond);
    }

    if(result != NULL && buflen > 1) {
        strncpy(result, hostname, buflen - 1);
        result[buflen - 1] = '\0';
    }
}
