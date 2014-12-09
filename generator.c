#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#define MAT "pthreadfile.txt"
int main(int argc, char** argv)
{
    if(argc != 2)
    {
	printf("wrong number of arguments\n");
    }

    int size = atoi(argv[1]);
    int *matrix = (int*)calloc(2*size*size, sizeof(int));
    int i = 0;
    int k = 0;
    
    for(i=0;i<2*size;i++)
    {
	for(k=0;k<size;k++)
	{
	    matrix[i * size + k] =0+rand()%2;
	}
    }
    
    int fd=0;
    
    if((fd= open(MAT, O_WRONLY | O_CREAT | O_TRUNC, 0777))<0)
    {    
	printf("can't open file\n");
	exit(-1);
    }
    
    if(write(fd, &size,sizeof(int)) != sizeof(int))
    {
	printf("can't write size\n");
	exit(-1);
    }
    
    int size_mess=0;
    if((size_mess = write(fd, matrix,2*size*size*sizeof(int))) != 2*size*size*sizeof(int))

	printf("wrote   %d\n", size_mess);

    if(close(fd) < 0)
    {
	printf("can't close file\n");
	exit(-1);
    }
    free(matrix);
    return 0;
}
