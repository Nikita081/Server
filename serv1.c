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
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <errno.h>
#include "backstatic.h"

int size;
int msqid;
int step;
int num_client;

int allocate_matrix(int size)
{
    int *matrix = (int*)malloc(size*size*sizeof(int));
    return (int)matrix;
}

void* thread_client(void* pargs)
{
    struct matrix_args *args = (struct matrix_args *) pargs;
    int i, j, k, l=0, m;
    int *matrix_1 = args->matrix_1;
    int *matrix_2 = args->matrix_2;
    int *ans = args->ans;
    int size = args->size;
    int begin = args->begin;
    int end = args->end;
    long int client_pid = args -> client_pid;
    long int t_id = (long int)args -> client_pid + INT_MAX;
    int length2;
    int real_max = (int)((double)MAX/(double)sizeof(int));
    int *task;

    if((end-begin)==step)
	task = (int*)calloc(size*step*sizeof(int)+size*size*sizeof(int),1);

    else
	task = (int*)calloc((size-step*(num_client-1))*size*sizeof(int) + size*size*sizeof(int),1);

    for(i=begin;i<end;i++)
    {
	m=i*size;
	
	for(j=0;j<size;j++)
	{
		task[j+l] = matrix_1[m+j];
	}
	l+=size;
    }

    for (i=0;i<size;i++)
    {
	for(j=0;j<size;j++)
	{
		task[step*size+i*size+j] = matrix_2[i*size + j];
	}
    }

   int tmp1 = 0;
   int num_msg;
   int current = size*step+size*size;
   int block_size =end-begin;


    if(current<=real_max)
	num_msg = 1;

    else if ((int)(current%real_max)==0)
	num_msg=(int)((double)current/(double)real_max);

    else 
	num_msg=(int)((double)current/(double)real_max)+1;


    struct mymsgbuf1 short_mess1 = {0};
    int length1 = sizeof(struct mymsgbuf1) - sizeof(long);

    short_mess1.type = t_id;
    short_mess1.finfo.shortmsgtype = HELLO;
    short_mess1.finfo.info =num_msg;
    short_mess1.finfo.size =size;
    short_mess1.finfo.block_size = block_size;

    if( msgsnd(msqid, &short_mess1,length1, 0) <0)
    {
	printf("can't send hello to client with pid %ld\n",client_pid);
	perror("msgsnd: ");
	exit(-1);
    }

    if(current<real_max)
	 length2 = 2*sizeof(char) + current*sizeof(int);

    else
	 length2 = 2*sizeof(char)+ (real_max+1)*sizeof(int);

    struct mymsgbuf  data;

    if(current<real_max)
    {
	data.type = client_pid;
	data.shortmsgtype = DOWORK;

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
		data.shortmsgtype = DOWORK;


		if(msgsnd(msqid,&data,length2,0)<0)
		{
			printf("can't send msg\n");
			exit(-1);
		}

		free(block_task);
	}
    }
    free(task);

    struct mymsgbuf2 short_mess={0};

    length1 = sizeof(struct mymsgbuf2) - sizeof(long);

    if(msgrcv(msqid,&short_mess,length1,t_id,0)<0)
    {
	printf("can't receive msg\n");
	exit(-1);
    }
   
    if(short_mess.shortmsgtype != WORKDONE)
    {
	printf("unknown type of msg\n");
	exit(-1);
    }

    num_msg = short_mess.info;

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
   
    struct mymsgbuf2 short_mess3={0};
   
    short_mess3.type = client_pid;
    short_mess3.shortmsgtype = BYE;
    short_mess3.info = size;
    length1 = sizeof(struct mymsgbuf2) - sizeof(long int);
   
    if(msgsnd(msqid,&short_mess3,length1,0)<0)
    {
	printf("can't send msg\n");
	exit(-1);
    }

    pthread_exit(NULL);
 
}


int main (int argc, char** argv)
{
    char MAT_PATH[] = "pthreadfile.txt";
    key_t key,key_1;
    int semid = 0;
    int length1;
    int tmp = 0;
    int fd;
    pthread_attr_t attr;
    struct mymsgbuf2 short_mess = {0};
    struct sembuf mybuf;

    if((key = ftok(KEY,0))<0)
    {
	printf("can't create key for q\n");
	exit(-1);
    }
    if((key_1 = ftok(KEY,1))<0)
    {
	printf("can't create key for sem\n");
	exit(-1);
    }

    if((msqid = msgget(key,0666 | IPC_CREAT | IPC_EXCL ))<0)
    {
	if(errno == EEXIST)
	{
		msgctl(msqid,IPC_RMID,NULL);
		msqid = msgget(key,0666);
	}
	else 
	{
	printf("can't get msqid\n");
	exit(-1);
	}
    }
    
    if((semid =semget(key_1,1,0666 | IPC_CREAT))<0)
    {
	printf("cann't get sem\n");
	exit(-1);
    }
    mybuf.sem_op = 1;
    mybuf.sem_num = 0;
    mybuf.sem_flg = 0;
    
    if(semop(semid,&mybuf,1)<0)
    {
	printf("can't wait for condition\n");
	exit(-1);
    }

    if((fd = open(LOG_PATH,O_RDONLY))<0)
    {
	printf("can't open log for num\n");
	exit(-1);
    }
    
    int num_thr;
    char *num = (char*)calloc(sizeof(int)+1,sizeof(char));
    num = (char*)my_1st_read(fd);
    sscanf(num,"%d",&num_thr);
    close(fd);
    free(num);
    
    if((fd = open(MAT_PATH,O_RDONLY))<0)
    {
	printf("can't open matrix file\n");
	exit(-1);
    }
    
    int *ssize = (int*)calloc(1,sizeof(int));
    
    ssize = (int*)my_read(fd,sizeof(int),(char*)ssize);
    size = ssize[0];
    
    int *matrix = (int*)calloc(2*size*size,sizeof(int));
    
    matrix = (int*)my_read(fd,0,(char*)(matrix));
    close(fd);
    free(ssize);
    
    printf("read end\n");
    
    int *matrix_1 = matrix;
    int *matrix_2 = matrix + size*size;
    int *ans = (int*) allocate_matrix(size);
    int i,j;

    if(pthread_attr_init(&attr)!=0)
    {
	printf("can't pthr attr init\n");
	exit(-1);
    }
    
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
    pthread_t thread[num_thr];
    
    step = (int)((double)size/(double)num_thr);
    
    int k_pos=0;
    
    struct matrix_args ARG[num_thr];
    
    int id;
    
    while(num_client != num_thr)
    { 
	length1 = sizeof(struct mymsgbuf2) - sizeof(long int);	

	if(msgrcv(msqid,&short_mess,length1,SERVER_TYPE,0)<0)
	{
		perror("msgrcv: ");
		printf("can't receive hello from client\n");
		exit(-1);	
	}

	ARG[num_client].matrix_1 = matrix_1;
	ARG[num_client].matrix_2 = matrix_2;
	ARG[num_client].ans = ans;
	ARG[num_client].size = size;
	ARG[num_client].begin = k_pos;
	k_pos+=step;
	ARG[num_client].end = (num_client==num_thr - 1) ? size : k_pos;
	ARG[num_client].client_pid = short_mess.info;

	id = pthread_create(&thread[num_client],&attr,thread_client,(void*)&ARG[num_client]);
	if(id>0)
	{
	    printf("can't create treade %d\n",num_client);
	    exit(-1);
	}
	num_client++;
    }
     
    pthread_attr_destroy(&attr);
    
    for(i=0;i<num_thr;i++)
{
	if(pthread_join(thread[i],NULL)>0)
	{
		printf("thread mistake %d\n",i);
		exit(-1);
	}
	printf("join %d\n",i);
}	
	printf("all thread join\n");
	printf("server start write to log file\n");
    

    if((fd = open(LOG_PATH,O_WRONLY))<0)
    {
	printf("can't open log\n");
	exit(-1);
    }
     
    char buf[2*sizeof(int)];
    char* str_res = (char*)calloc(size*size*(sizeof(int)+4)+2,sizeof(char));
     
    for(i=0;i<size*size;i++)
    {
	sprintf(buf,"%d%c",ans[i],' ');
	strncat (str_res,buf,sizeof(int)+3);
    }

    strncat(str_res," \n\n",5);

    if(lseek(fd,0,SEEK_END)<0)
    {
	printf("seek error\n");
	exit(-1);
    }

    my_1st_write(str_res,fd);
    close(fd);
   
    printf("write all!!!!\n");
   
    if(msgctl(msqid,IPC_RMID,0)<0)
    {
	printf("can't delet que\n");
	exit(-1);
    }
    if(semctl(semid,IPC_RMID,0)<0)
    {
	printf("can't delete sem\n");
	exit(-1);
    }
    return 0;
}
