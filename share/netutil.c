#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <assert.h>
#include <netdb.h>

#include "netutil.h"

ssize_t send_all(int s, const void *buf, size_t len, int flags)
{
  int sent;
  unsigned int total = 0;
  while(total < len)
    {
      if((sent = send(s, (char *)buf + total,
		      len - total, flags)) != -1)
	{
	  total += sent;
	}
      else
	{
	  return -1;
	}
    }
  return total;
}

ssize_t recv_all(int s, void * buf, size_t len, int flags)
{
  unsigned int total = 0;
  int received;
  while(total < len)
    {
      fprintf(stderr, "begin receive loop\n");
      if((received = recv(s, (char *)buf + total,
			  len - total, flags)) > 0)
	{
	  
	  total += received;
	  fprintf(stderr, "received %i out of %i\n", (int)total, (int)len);
	}
      else if(received == 0)
	{
	  perror("recv_all");
	  return -1;
	}
      else
	{ 
	  perror("recv_all");
	  return -1;
	}
    }
  fprintf(stderr, "received all\n");
  return total;
}

int init_socket(int port)
{
  struct sockaddr_in my_addr;    /* my address information */
  const int yes = 1;
  int sockfd;
  if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror("socket");
      return -1;
    }

  /* set socket reusable */
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
		 &yes, sizeof(int)) == -1)
    {
      perror("setsockopt");
      return -1;
    }

  
  my_addr.sin_family = AF_INET;         /* host byte order */
  my_addr.sin_port = htons(port);       /* short, network byte order */
  my_addr.sin_addr.s_addr = INADDR_ANY; /* automatically fill with my IP */
  memset(&(my_addr.sin_zero), '\0', 8); /* zero the rest of the struct */

  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))
      == -1)
    {
      perror("bind");
      return -1;
    }

  if (listen(sockfd, 10) == -1) 
    {
      perror("listen");
      return -1;
    }
  return sockfd;
}

int init_connection(char * addr, int port)
{
  struct hostent *he;
  struct sockaddr_in their_addr;

  int sock;
   
  if ((he = gethostbyname(addr)) == NULL)
    { /* get the host info */ 
      herror("gethostbyname");
      return -1;
    }

  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror("socket");
      return -1;
    }

  their_addr.sin_family = AF_INET;    /* host byte order */ 
  their_addr.sin_port = htons(port);  /* short, network byte order */
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);
  memset(&(their_addr.sin_zero), '\0', 8);  /* zero the rest of the struct */

  if(connect(sock, (struct sockaddr *)&their_addr,
	     sizeof(struct sockaddr)) == -1)
    {
      perror("connect");
      return -1;
    }

  return sock;
}
