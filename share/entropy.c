#include "entropy.h"

unsigned int genrand(int range, int start)
{
	unsigned int raw;
	int fd;
	fd=open("/dev/urandom",O_RDONLY);
	read(fd,&raw,sizeof raw);
	close(fd);
	return (raw % range) + start;
}	
