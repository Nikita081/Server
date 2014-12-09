#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#define LOG_PATH "log.txt"

int main(int argc, char** argv)
{
    int i = 0;
    int fork_res = 0;
    char path_log[] = "pthreadfile.txt";
    int num_client = atoi(argv[1]);
    int fd=0;
    fork_res = fork(); 
    if (fork_res == -1) 
    {
	printf("fork_error \n");
	exit(-1);
    } 

    if((fd = open(LOG_PATH,O_WRONLY | O_CREAT | O_TRUNC , 0666))<0)
    {
	perror("open:");
	printf("can't open log\n");
	exit(-1);
    }
    my_1st_write(argv[1],fd);
    close(fd);
    if(fork_res == 0)
    {
	execlp("./server.out", "./server.out", NULL);
	printf("Exec error");
	exit(-1);
    }
    sleep(4);

    if(fork_res>0)
    {
	for(i = 0; i < num_client; i++)
	{
	    fork_res = fork();
	    if(fork_res == 0) 
	    {
		execlp("./client.out", "./client.out", NULL);
		printf("Exec error %d \n",i);
		exit(-1);
	    } 
	}
    }
    return 0;
}
