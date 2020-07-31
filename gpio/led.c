
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char buf[80];

	char i = 0;
	int fd = -1;
	memset(buf, 0, 80);

	printf("GPIO SET : %s\n", argv[1]);
	fd = open ("/dev/gpioled", O_RDWR);
	if (fd ==-1){
		printf("fail to open dev/gpioled\n");
	}

	write(fd, argv[1], strlen(argv[1]), NULL);

	read(fd, buf, strlen(argv[1]), NULL);
	printf("Read data : %s\n",buf);

	close(fd);

	return 0;
}
