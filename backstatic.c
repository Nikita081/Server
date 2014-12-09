#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX 8100
#define NUM_OF_SEM 1
#define SERVER_TYPE 1
#define LOG_PATH "log.txt"
#define INT_MAX 0x78888888
#define KEY "/bin/sh"
#define KEYY "/bin/bash"
#define HELLO 1
#define DOWORK 2
#define WORKDONE 3
#define BYE 4
#define RES 5
#define BUF 4096




struct mymsgbuf 
{
    long type;
    char shortmsgtype;
    int info[2025];
    char wtf;
};

struct mymsgbuf2 
{
    long type;
    char shortmsgtype;
    int info;
    char wtf;
};

struct mymsgbuf1 
{
    long type;
    struct {
    char shortmsgtype;
    int info;
    int size;
    int block_size;
    }finfo;
};


struct matrix_args
{
    int *matrix_1;
   int *matrix_2;
   int *ans;
   int size;
   int begin;
   int end;
   long int client_pid;
};



char* my_read(int fd, int size_of_rd_elem,char* string)
{
    if(size_of_rd_elem == 0)
    {
	int pos = 0;
	int size_mess = read(fd, string, BUF);

	if(size_mess < 0)

            printf("can't read\n");

	while(size_mess > 0)
        {
            pos += size_mess;
            string = (char*)realloc(string, pos + BUF * sizeof(char));
            size_mess = read(fd, string + pos, BUF);
	}
    }
    else
    {
        int pos = 0;
        while(pos != size_of_rd_elem)
        {
            int num = read(fd, string, size_of_rd_elem);
            if(num < 0)
            {
                printf("can't  read\n");
                exit(-1);
            }
            pos += num;
        }
    }
    return string;
}

char* my_1st_read(int fd)
{
   int BSIZE=1024;
   int size_mes=0;
    char* buffer = (char*)malloc(BSIZE*sizeof(char));
    char* str_result = NULL;
    while((size_mes = read(fd,buffer,BSIZE))>0)
    { 
	if (str_result == NULL) 
	{
	    str_result = (char*)calloc(size_mes+sizeof(char),1);
	}
	else 
	{
	    str_result = realloc(str_result,(strlen(str_result)+1)*sizeof(char)+size_mes);
	}
	strncat(str_result,buffer,size_mes);
    }
    if (size_mes<0)
    {
	perror("can't read\n");
	exit(-1);
    }
	free(buffer);
    return str_result;
} 

void my_1st_write (char* message, int fd_to)
{	int error = 0;
	int count = 0;
	int string_len = 0;
	string_len = strlen (message);
	while ((error = write(fd_to, message + count, string_len - count)) != 0)
	{
	   count += error;
	   if (string_len == count)
	     break;
	if (error < 0){
        printf("Error with write\n");
           break;	}
	}
	
	
}
