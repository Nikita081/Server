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

void* send_information_function(int msqid,long int client_pid, int* task, int real_max, int current,int signal)
{
   int tmp1=0;
   int i;
   int length2 = 0;
   if(current<real_max)
	 length2 = 2*sizeof(char) + current*sizeof(int);

    else
	 length2 = 2*sizeof(char)+ (real_max+1)*sizeof(int);

    struct mymsgbuf  data;

    if(current<real_max)
    {
	data.type = client_pid;
	data.shortmsgtype = signal;

	for(i=0;i<current;i++)
	    data.info[i] = task[i];
	
	if(msgsnd(msqid,&data,length2,0)<0)
	{
		printf("can't send msg\n");
		exit(-1);
	}
    }

    else
    {
	int *block_task;
	
	while(tmp1 != current)
	{

		if((current-tmp1)>real_max)
		{
			block_task = (int*)calloc(real_max,sizeof(int));

			for(i=0;i<real_max;i++)
			    data.info[i] = task[i+tmp1];

			tmp1+=real_max; 
		}

		else
		{
			block_task = (int*)calloc((current-tmp1),sizeof(int));

			for(i=0;i<(current-tmp1);i++)

			    data.info[i] = task[i+tmp1];

			tmp1=current;
		}

		data.type = client_pid;
		data.shortmsgtype = signal;


		if(msgsnd(msqid,&data,length2,0)<0)
		{
			printf("can't send msg\n");
			exit(-1);
		}

		free(block_task);
	}
    }
   return;
}

void* server_receive_informatioin(int msqid,int real_max,int size, int begin,int block_size,int* ans, long int client_pid,int num_msg)
{
    int length2 = 0;
    int m,i,j;
    struct mymsgbuf long_mess;

    length2 = 2*sizeof(char) + (real_max+2)*sizeof(int);

   for (i=0;i<num_msg;i++)
   {
	m=size*begin + i*real_max;

	if(msgrcv(msqid,&long_mess,length2,client_pid,0)<0)
	{
	    printf("can't receive msg\n");
	    exit(-1);
	} 

	if(((block_size*size))>real_max*(i+1))
	{
	    for(j=0;j<real_max;j++)
	    {
		ans[j+m] = long_mess.info[j];
	    }
	} 

	else 
	{
	    for(j=0;j<((block_size*size)-real_max*i);j++)
	    {
		ans[j+m] = long_mess.info[j];
	    }
	}
    }
  return;
}

void* client_receive_informatioin(int msqid,int real_max,int size,int block_size,int* task, long int pid,int num_msg_get)
{
   int length2 = 0;
    int m,i,j;
    struct mymsgbuf long_mess;
    length2 = 2*sizeof(char) + (real_max+1)*sizeof(int);
    
    for(i=0;i<num_msg_get;i++)
    {
	m=i*real_max;

	if(msgrcv(msqid,&long_mess,length2,pid,0)<0)
	{
		perror("msgsnd: ");
		printf("pid %ld can't receive data\n",pid);
 		exit(-1);
	}

	if((size*size+block_size*size)>real_max*(i+1))
	{
	    for(j=0;j<real_max;j++)
	    {
		task[j+m] = long_mess.info[j];
	    } 
	} 

	else 
	{
	    for(j=0;j<((size*size+block_size*size)-real_max*i);j++)
	    {
		task[j+m] = long_mess.info[j];
	    }
	}
    }
}	
