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
#define HELLO 1
#define DOWORK 2
#define WORKDONE 3
#define BYE 4
#define RES 5

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

char* my_read(int fd, int size_of_rd_elem,char* string);

char* my_1st_read(int fd);

void my_1st_write (char* message, int fd_to);

void* server_receive_informatioin(int msqid,int real_max,int size, int begin,int block_size,int* ans, long int client_pid,int num_msg);

void* client_receive_informatioin(int msqid,int real_max,int size,int block_size,int* task, long int pid,int num_msg_get);

void* send_information_function(int msqid, long int client_pid, int *task, int real_max, int current,int signal);
