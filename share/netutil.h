#ifndef __OPENPKMN_SHARE_NET_UTIL__
#define __OPENPKMN_SHARE_NET_UTIL__ 1

#include <sys/types.h>

extern ssize_t recv_all(int s, void * buf, size_t len,
			int flags);
extern ssize_t send_all(int s, const void *buf,
			size_t len, int flags);

int init_socket(int port);

int init_connection(char * addr, int port);

#endif
