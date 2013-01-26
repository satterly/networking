/*
 * tcp-client.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
    perror("inet_pton failed");
  }

  if (connect(sockfd, (struct sockaddr *) &remote_addr, sizeof(remote_addr)) < 0) {
    perror("Connect failed");
    exit(1);
  } else {
    printf("Connected to %s:%d\n", remote_host, remote_port);
  }

  char *buf = "this is a message\n\n";
  int bytes = 0;
  bytes = write(sockfd, buf, strlen(buf));
  if (bytes != strlen(buf)) {
    perror("write failed");
  } else {
    printf("sent %d bytes ok!\n", bytes);
  }

  char readbuf[1024];

  int len = read(sockfd, readbuf, sizeof(readbuf));
  readbuf[len] = '\0';
  printf("read %d bytes = %s\n", len, readbuf);

  if (close(sockfd) < 0) {
    perror("socket close failed");
  } else {
    printf("socket close OK\n");
  }
}
