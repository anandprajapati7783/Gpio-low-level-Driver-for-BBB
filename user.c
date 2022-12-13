#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int main()
{
        int fd;
	fd=open("/dev/gpio_device", O_RDWR);
		if(fd<0)
			printf("faioled to open file\n");

write(fd,"1",1);
sleep(2);
write(fd,"0",1);
}
