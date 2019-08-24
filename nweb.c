#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <ctype.h>

#include <stdarg.h>
#include <time.h>

#include <pwd.h>
#include <grp.h>
#include "config.h"
#include "hash.h"
#include "ui_common.h"

void *xmalloc(size_t n);
void *xcalloc(size_t n, size_t m);
void *xrealloc(void *w, size_t n);
char *xstrdup(const char *s);
void xfree(void *v);

extern hash_type* history;

#define BUFSIZE 1024

#ifndef SIGCLD
#   define SIGCLD SIGCHLD
#endif

extern time_t first_timestamp;

static void logger(const char *fmt, ...) {
  int written, error;
  char time_buf[26];
  char *output;
  time_t now;
  
  va_list args;

  va_start(args, fmt);

  time(&now);
  ctime_r(&now, time_buf);
  time_buf[24] = 0;
  time_buf[25] = 0;
  /* prefix log output with timestamp */
  (void)fprintf(stderr, "[%s] ", time_buf);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
}

static char *generate_payload(long int *len) {
	hash_node_type* n = NULL;
	char *payload;
	long int p;

	*len = BUFSIZE;
	payload = (char *)xmalloc(*len);
	strcpy(payload, "{\"version\":\"" PACKAGE_VERSION "\",\"started\":\"");
	p = strlen(payload);

  // add timestamp for the 'started at' time
  char time_buf[26];
  ctime_r(&first_timestamp, time_buf);
  time_buf[24] = 0;  time_buf[25] = 0;

  strcpy(payload + p, time_buf);
  p = strlen(payload);

  // add history array begin
  strcpy(payload + p, "\",\"history\":[");
  p = strlen(payload);

    while(hash_next_item(history, &n) == HASH_STATUS_OK) {
        history_type* d = (history_type*)n->rec;
        addr_pair ap;

        ap = *(addr_pair*)n->key;
        
        int expected = strlen("\n{\"source\":\"") + HOSTNAME_LENGTH + strlen("\", \"destination\":\"") + HOSTNAME_LENGTH 
						// for the port specification
						+ 2 + strlen("65536")*2
						+ strlen("\", \"received\":") + strlen("65536") + strlen(", \"sent\":") + strlen("65536")
						+ strlen(", \"protocol\":") + strlen("65536")
						+ strlen("},");

		if (*len - p < expected) {
			*len += expected;
			payload = (char *)xrealloc(payload, *len);
		}
		strcpy(payload + p, "\n{\"source\":\"");
		p += strlen("\n{\"source\":\"");

		sprint_host_raw(payload + p, ap.af, &(ap.src6), ap.src_port, ap.protocol);
		p = strlen(payload);
		
		strcpy(payload + p, "\", \"destination\":\"");
		p += strlen("\", \"destination\":\"");
        sprint_host_raw(payload + p, ap.af, &(ap.dst6), ap.dst_port, ap.protocol);
        p = strlen(payload);
        
        strcpy(payload + p, "\", \"received\":");
        p += strlen("\", \"received\":");
        
        sprintf(payload + p, "%ld", d->total_recv);
        p = strlen(payload);

        strcpy(payload + p, ", \"sent\":");
        p += strlen(", \"sent\":");

        sprintf(payload + p, "%ld", d->total_sent);
        p = strlen(payload);

        strcpy(payload + p, ", \"protocol\":");
        p += strlen(", \"protocol\":");

        sprintf(payload + p, "%d", ap.protocol);
        p = strlen(payload);
        
        strcpy(payload + p, "},");
        p += strlen("},");
    }

    // remove the last comma
    char *pos = strrchr(payload, '}');
    if (pos) {
		pos[1] = ' ';
	}

	if (*len - p < 4) {
		*len += 4;
		payload = (char *)xrealloc(payload, *len);
	}
	payload[p++] = '\n';
	payload[p++] = ']';
	payload[p++] = '}';
	payload[p] = 0;
	
	*len = p;	
	return payload;
}

/* this is a child web server process, it will exit once the request has been served */
static int handle_request(int fd) {
  int j, file_fd, buflen, written;
  long i, ret, len;
  const char * fstr;
  static char buffer[BUFSIZE+1], out_buffer[BUFSIZE+1]; /* static so zero filled */

  ret = read(fd,buffer,BUFSIZE);   /* read request in one go */
  if (ret == 0 || ret == -1) {  /* read failure stop now */
    logger("failed to read browser request (pid=%d): %s", getpid(), strerror(errno));
    return 1;
  }
  if (ret > 0 && ret < BUFSIZE)  /* return code is valid chars */
    buffer[ret]=0;    /* terminate the buffer */
  else {
	logger("invalid request size (size=%d) (pid=%d)", ret, getpid());
	buffer[0]=0;
	ret = 0;
  }

  for(i=4;i<ret;i++) { /* null terminate after the second space to ignore extra stuff */
    if (buffer[i] == ' ') { /* string is "GET URL " +lots of other stuff */
      buffer[i] = 0;
      break;
    }
  }

  if (!strncmp(buffer, "GET /iftop/history", 19) || !strncmp(buffer, "get /iftop/history", 19) ) {
	  char *payload;
	  payload = generate_payload(&len);
	  
	  (void)sprintf(out_buffer,"HTTP/1.1 200 OK\nServer: iftop/%s\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", PACKAGE_VERSION, len, "application/json"); // Header + a blank line
	  written = write(fd, out_buffer, strlen(out_buffer));
	  
	  written = write(fd, payload, len);
	  
	  xfree(payload);
	  
	  logger("\"%s\" 200 %d", buffer, len);
  
	  return (written != len);
  }
  
  if (!strncmp(buffer, "GET /iftop/version", 19) || !strncmp(buffer, "get /iftop/version", 19) ) {
	  char *payload = "{\"version\":\"" PACKAGE_VERSION "\"}";
	  len = strlen(payload);
	  
	  (void)sprintf(out_buffer,"HTTP/1.1 200 OK\nServer: iftop/%s\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", PACKAGE_VERSION, len, "application/json"); // Header + a blank line
	  written = write(fd, out_buffer, strlen(out_buffer));
	  
	  written = write(fd, payload, len);
	  
	  logger("\"%s\" 200 %d", buffer, len);
  
	  return (written != len);
  }

  (void)sprintf(out_buffer, "HTTP/1.1 404 Not Found\nerver: iftop/%s\nContent-Length: 2\nConnection: close\nContent-Type: %s\n\n{}\n", PACKAGE_VERSION, "application/json");
  written = write(fd, out_buffer, strlen(out_buffer));
  
  logger("\"%s\" 404 %d", buffer, 2);

  return 1;
}

int drop_root(const char *run_as_user, const char *run_as_group) {
    struct passwd *pwd;
    struct group* g;

    if (NULL == (pwd = getpwnam(run_as_user))) {
		if (errno) {
			logger("getpwnam: %s", strerror(errno));
			return 1;
		}
        logger("drop_root: user '%s' not found", run_as_user);
        return 2;
    }

    if (NULL == (g = getgrnam(run_as_group))) {
		if (errno) {
			logger("getgrnam: %s", strerror(errno));
			return 3;
		}
        logger("drop_root: group '%s' not found", run_as_group);
        return 4;
    }

	/*  Drop superuser privileges in correct order */
	if (setgid(g->gr_gid) == -1) {
		logger("setgid: %s", strerror(errno));
		return 5;
	}
	if (setuid(pwd->pw_uid) == -1) {
		logger("setuid: %s", strerror(errno));
		return 6;
	}

	logger("dropped privileges to %s:%s", run_as_user, run_as_group);
	return 0;
}

int init_web(int port) {
  int pid, listenfd, socketfd, hit, exit_code;
  socklen_t length;
  static struct sockaddr_in cli_addr; /* static = initialised to zeros */
  static struct sockaddr_in serv_addr; /* static = initialised to zeros */

  signal(SIGCLD, SIG_IGN); /* ignore child death */
  signal(SIGHUP, SIG_IGN); /* ignore terminal hangups */
  close(STDIN_FILENO);
  setpgrp();    /* break away from process group */

  /* setup the network socket */
  if ((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0) {
    logger("socket: %s", strerror(errno));
    return 1;
  }
  
  logger("REST webservice starting on port %d", port);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);
  if (bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0) {
    logger("bind: %s", strerror(errno));
    return 3;
  }

  if (strcmp(options.http_run_as_user, "")) {
	  int result = drop_root(options.http_run_as_user, options.http_run_as_group);
	  if (result) {
		  return result;
	  }
  }

  if (listen(listenfd,64) <0) {
    logger("listen: %s", strerror(errno));
    return 4;
  }

  for(hit=1; ;hit++) {
    length = sizeof(cli_addr);
    if ((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0) {
      logger("accept: %s", strerror(errno));
      return 5;
    }

    if ((pid = fork()) < 0) {
      logger("fork: %s", strerror(errno));
      return 6;
    }

    if (pid == 0) {   /* child */
      (void)close(listenfd);
      exit_code = handle_request(socketfd);

      sleep(1);  /* allow socket to drain before signalling the socket is closed */
      close(socketfd);

      return exit_code;
    }

    /* parent */
    (void)close(socketfd);
  }

  /* never reached */
  return 42;
}
