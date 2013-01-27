/*
 * tcpclient-nonblock.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <poll.h>
#include <errno.h>

// socket()
//     set non-blocking
// connect()
//     select() or poll()
// write()
// read()
// close()

/*
       EINPROGRESS
              The  socket  is  nonblocking and the connection cannot be completed immediately.  It is possible to
              select(2) or poll(2) for completion by selecting the socket for writing.  After select(2) indicates
              writability, use getsockopt(2) to read the SO_ERROR option at level SOL_SOCKET to determine whether
              connect() completed successfully (SO_ERROR is zero) or unsuccessfully (SO_ERROR is one of the usual
              error codes listed here, explaining the reason for the failure).
 */

int
main() {

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("Could not open socket");
    exit(1);
  } else {
    printf("Socket created!\n");
  }

  char *remote_host = "127.0.0.1";
  int remote_port = 8649;

  struct sockaddr_in remote_addr = {};
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_port = htons(remote_port);

  if (inet_aton(remote_host, &remote_addr.sin_addr) <= 0) {
    perror("inet_aton failed");
  }

  // set to non-blocking
  long flags = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

  int n;
  if ((n = connect(sockfd, (struct sockaddr *) &remote_addr, sizeof(remote_addr))) < 0) {
    if (errno != EINPROGRESS) {
      perror("Connect failed");
      exit(1);
    } else {
      printf("Connect to %s:%d in progress...\n", remote_host, remote_port);
    }
  }

  int error = 0;
  if (n == 0) {
    printf("Connected to %s:%d\n", remote_host, remote_port);
  } else {

    struct pollfd fds[1] = {};
    fds[0].fd = sockfd;
    fds[0].events = POLLIN | POLLOUT;

    int timeout = 5*1000; /* 5 seconds */

    if ((n = poll(fds, 1, timeout)) == 0) {
      errno = ETIMEDOUT; /* if returning */
      perror("poll()");
      close(sockfd);
      exit(1); 
    } else if (n < 0) {
      perror("poll()");
      close(sockfd);
      exit(1);
    } 

    int len = sizeof(error);
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
      perror("getsockopt()");
      close(sockfd);
      exit(1);
    }

    if (error) {
      errno = error;
      perror("connect()");
      close(sockfd);
      exit(1);
    }

    if (fds[0].revents & POLLIN) {
      printf("socket ready to read\n");
      char readbuf[1024];

      int len = read(sockfd, readbuf, sizeof(readbuf));
      readbuf[len] = '\0';
      printf("read %d bytes = %s\n", len, readbuf);
    
    } else {
      fprintf(stderr, "socket not ready to read\n");
    }

    if (fds[0].revents & POLLOUT) {
      printf("socket ready to write!\n");
      fcntl(sockfd, F_SETFL, flags);
    
      char *buf = "this is a message\n\n";
      int bytes = 0;
      bytes = write(sockfd, buf, strlen(buf));
      if (bytes != strlen(buf)) {
        perror("write failed");
      } else {
        printf("sent %d bytes ok!\n", bytes);
      }
    } else {
      fprintf(stderr, "socket not ready to write\n");
    }
  }

  if (close(sockfd) < 0) {
    perror("socket close failed");
  } else {
    printf("socket close OK\n");
  }
}
