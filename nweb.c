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


#include <time.h>

#include "config.h"

#define BUFSIZE 8096
#define ERROR      42
#define LOG        44
#define FORBIDDEN 403
#define NOTFOUND  404

#ifndef SIGCLD
#   define SIGCLD SIGCHLD
#endif

// https://stackoverflow.com/questions/2673207/c-c-url-decode-library#14530993
static void urldecode2(char *dst, const char *src)
{
  char a, b;
  while (*src) {
    if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a'-'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a'-'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      *dst++ = 16*a+b;
      src+=3;
    } else if (*src == '+') {
      *dst++ = ' ';
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst++ = '\0';
}

static void logger(int type, const char *s1, const char *s2, int socket_fd)
{
  int written;
  char logbuffer[BUFSIZE*2];

  switch (type) {
  case ERROR:
    (void)sprintf(logbuffer,"[ERROR]: %s:%s Errno=%d exiting pid=%d",s1, s2, errno,getpid());
    break;
  case FORBIDDEN:
    written = write(socket_fd, "HTTP/1.1 403 Forbidden\nContent-Length: 185\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\nThe requested URL, file type or operation is not allowed.\n</body></html>\n",271);
    (void)sprintf(logbuffer,"[FORBIDDEN]: %s:%s",s1, s2);
    break;
  case NOTFOUND:
    written = write(socket_fd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found.\n</body></html>\n",224);
    (void)sprintf(logbuffer,"[NOT FOUND]: %s:%s",s1, s2);
    break;
  case LOG:
    (void)sprintf(logbuffer,"[INFO]: %s:%s:%d",s1, s2,socket_fd);
    break;
  }
  char time_buf[26];
  time_t now;
  time(&now);
  ctime_r(&now, time_buf);
  time_buf[24] = 0;
  time_buf[25] = 0;
  (void)fprintf(stdout, "%s %s\n", time_buf, logbuffer);
  if(type == ERROR || type == NOTFOUND || type == FORBIDDEN) exit(3);
}

/* this is a child web server process, so we can exit on errors */
static void web(int fd, int hit)
{
  int j, file_fd, buflen, written;
  long i, ret, len;
  const char * fstr;
  static char buffer[BUFSIZE+1]; /* static so zero filled */

  ret =read(fd,buffer,BUFSIZE);   /* read Web request in one go */
  if(ret == 0 || ret == -1) {  /* read failure stop now */
    logger(FORBIDDEN,"failed to read browser request","",fd);
  }
  if(ret > 0 && ret < BUFSIZE)  /* return code is valid chars */
    buffer[ret]=0;    /* terminate the buffer */
  else buffer[0]=0;
  for(i=0;i<ret;i++)  /* remove CF and LF characters */
    if(buffer[i] == '\r' || buffer[i] == '\n')
      buffer[i]='*';
  logger(LOG,"request",buffer,hit);
  if( strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4) ) {
    logger(FORBIDDEN,"Only simple GET operation supported",buffer,fd);
  }
  for(i=4;i<BUFSIZE;i++) { /* null terminate after the second space to ignore extra stuff */
    if(buffer[i] == ' ') { /* string is "GET URL " +lots of other stuff */
      buffer[i] = 0;
      break;
    }
  }
  urldecode2(&buffer[5],&buffer[5]);
  if(buffer[5] == '/') { /* check for illegal absolute directory path use */
    logger(FORBIDDEN,"Absolute directory (/) path names not supported",buffer,fd);
  }
  for(j=0;j<i-1;j++) {   /* check for illegal parent directory use .. */
    if(buffer[j] == '.' && buffer[j+1] == '.') {
      logger(FORBIDDEN,"Parent directory (..) path names not supported",buffer,fd);
    }
  }
  if( !strncmp(&buffer[0],"GET /iftop\0",6) || !strncmp(&buffer[0],"get /iftop\0",6) ) {
	  //TODO: serve request
	  logger(LOG,"SEND",&buffer[5],hit);
	  
	  char *dummy_payload = "{\"traffic\":[]}";
	  len = strlen(dummy_payload);
	  
	  (void)sprintf(buffer,"HTTP/1.1 200 OK\nServer: iftop/%s\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", IFTOP_VERSION, len, "application/json"); // Header + a blank line
	  logger(LOG,"Header",buffer,hit);
	  written = write(fd,buffer,strlen(buffer));
	  
	  written = write(fd, dummy_payload, len);

/*	  len = (long)lseek(file_fd, (off_t)0, SEEK_END); // lseek to the file end to find the length 
			(void)lseek(file_fd, (off_t)0, SEEK_SET); // lseek back to the file start ready for reading

	  // send file in 8KB block - last block may be smaller
	  while (  (ret = read(file_fd, buffer, BUFSIZE)) > 0 ) {
		(void)write(fd,buffer,ret);
	  } */
  } else {
	logger(NOTFOUND, "failed to serve request",&buffer[4],fd);
  }

  sleep(1);  /* allow socket to drain before signalling the socket is closed */
  close(fd);
  exit(1);
}

int init_web(int port)
{
  int pid, listenfd, socketfd, hit;
  socklen_t length;
  static struct sockaddr_in cli_addr; /* static = initialised to zeros */
  static struct sockaddr_in serv_addr; /* static = initialised to zeros */

  signal(SIGCLD, SIG_IGN); /* ignore child death */
  signal(SIGHUP, SIG_IGN); /* ignore terminal hangups */
  close(STDIN_FILENO);
  close(STDERR_FILENO);
  setpgrp();    /* break away from process group */

  logger(LOG,"nweb starting", "pid", getpid());
  /* setup the network socket */
  if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0)
    logger(ERROR, "system call","socket",0);
  
  if(port < 0 || port >60000)
    logger(ERROR,"Invalid port number (try 1->60000)","port",port);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);
  if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0)
    logger(ERROR,"system call","bind",0);
  if( listen(listenfd,64) <0)
    logger(ERROR,"system call","listen",0);
  for(hit=1; ;hit++) {
    length = sizeof(cli_addr);
    if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0)
      logger(ERROR,"system call","accept",0);
    if((pid = fork()) < 0) {
      logger(ERROR,"system call","fork",0);
    }
    else {
      if(pid == 0) {   /* child */
        (void)close(listenfd);
        web(socketfd,hit); /* never returns */
      } else {   /* parent */
        (void)close(socketfd);
      }
    }
  }
}
