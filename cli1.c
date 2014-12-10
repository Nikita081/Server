#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include "backstatic.h"

int main()
{
   char MAT_PATH[] = "pthreadfile.txt";
    key_t key,key_1;
    int num_client = 0;
    int semid = 0;
    int fd;
    int msqid = 0;
    int num_msg=0;
    long int pid = 0;
    long int t_id = 0;
    int length1 = 0;
    struct mymsgbuf2 short_mess = {0};
    struct sembuf mybuf;

    if((key = ftok(KEY,0))<0)
    {
	printf("can't create key for q\n");
	exit(-1);
    }
    
    if((key_1 = ftok(KEY,1))<0)
    {
	printf("can't create key for q\n");
	exit(-1);
    }
    
    if((msqid = msgget(key,0666 | IPC_CREAT))<0)
    {
	printf("can't get msqid\n");
	exit(-1);
    }
    
    if((semid =semget(key_1,1,0666 | IPC_CREAT))<0)
    {
	printf("cann't get sem\n");
	exit(-1);
    }

        
    pid = getpid();

    //формираем риветственное сообщение , шлем в нем свой пид
    short_mess.type = SERVER_TYPE;
    short_mess.shortmsgtype = HELLO;
    short_mess.info = pid;
    t_id = pid + INT_MAX;
    
    length1 = sizeof(struct mymsgbuf2) - sizeof(long int);
    
    if(msgsnd(msqid,&short_mess,length1,0)<0)
    {
	printf("pid %ld can't send hello\n",pid);
        exit(-1);
    }

    struct mymsgbuf1 short_mess1 = {0};
    
    length1 = sizeof(struct mymsgbuf1) - sizeof(long);
    
    // получаем информацию о количестве сообщений для считывания, размере матрицы, и количестве строк 
    msgrcv(msqid,&short_mess1,length1,t_id,0);

    int size =  short_mess1.finfo.size; 
    int num_msg_get = short_mess1.finfo.info; 
    int block_size =   short_mess1.finfo.block_size;
    int i,j,m,k,l;
    int *ans = malloc(block_size*size*sizeof(int));
    char* str_res = (char*)calloc(size*block_size*(sizeof(int)+4)+200,sizeof(char));
    int real_max = (int)((double)MAX/(double)sizeof(int));
    
    if(block_size*size<=real_max)
	num_msg = 1;

    else if ((int)(block_size*size%real_max)==0)
	num_msg=(int)((double)block_size*size/(double)real_max);

    else 
	num_msg=(int)((double)block_size*size/(double)real_max)+1;

    int *ptr = (int*)calloc(real_max,sizeof(int));
    int *ptr1 = (int*)calloc((size*block_size-real_max*(num_msg-1)),sizeof(int));
    int *task = (int*)calloc(block_size*size+size*size,sizeof(int));

    // получаем данные для подсчета
    client_receive_informatioin(msqid,real_max,size,block_size,task,pid,num_msg_get);
    
    //умножаем
    int *matrix_2 = task + size*block_size;
    int tmp=0;

    for(i = 0; i < block_size; i++)
    {
	m = i * size;

	for(j = 0; j < size; j++)
	{
	    l = 0;

	    for(k = 0; k < size; k++)
	    {
		tmp += task[m + k] * matrix_2[j + l];
	        l += size;
	    }
            ans[m + j] = tmp;
	    tmp = 0;
	}
    }
    //проверяем возможность доступаа к файлу
    mybuf.sem_op = -1;
    mybuf.sem_num = 0;
    mybuf.sem_flg = 0;
    
    if(semop(semid,&mybuf,1)<0)
    {
	printf("can't wait for condition\n");
	exit(-1);
    } 
    // открываем лог файл
    if((fd = open(LOG_PATH,O_WRONLY))<0)
    {
	printf("can't open log file\n");
	exit(-1);
    }
    
    // формируем данные для записив файл
    char buf[2*sizeof(int)];
    
    strncat(str_res,"I am client with pid ",22);
    sprintf(buf,"%ld ",pid);
    strncat (str_res,buf,10);
    strncat (str_res,"has done my job and got next result:\n\n",40);
     
    for(i=0;i<size*block_size;i++)
    {
	sprintf(buf,"%d%c",ans[i],' ');
	strncat (str_res,buf,sizeof(int)+3);
    }
    strncat(str_res," \n\n",5);

    if(lseek(fd,0,SEEK_END)<0)
    {
	perror("lseek: ");
	printf("seek error\n");
	exit(-1);
    }
    //записываем
    my_1st_write(str_res,fd);
    close(fd);
    //снимаем блокировку с доступа к файлу
    mybuf.sem_op = 1;
    mybuf.sem_num = 0;
    mybuf.sem_flg = 0;
    
    if(semop(semid,&mybuf,1)<0)
    {
	perror("semop:");
	printf("can't wait for condition\n");
	exit(-1);
    }
    //отправляем сообщение, что работы готова и количество сообщений
    struct mymsgbuf2 short_mess2 = {0};

    short_mess2.type = t_id;
    short_mess2.shortmsgtype = WORKDONE;
    short_mess2.info = num_msg;

    length1 = sizeof(struct mymsgbuf2) - sizeof(long);

    if(msgsnd(msqid,&short_mess2,length1,0)<0)
    {
	printf("pid %ld can't send workdone\n",pid);
	exit(-1);
    }

    // шлем результат
    send_information_function(msqid,pid,ans,real_max, size*block_size,RES);
    
    free(ans);
    free(str_res);
    free(ptr);
    
    //пытаемся получить прощальное сообщение
    struct mymsgbuf2 short_mess3 = {0};
       
    length1 = sizeof(struct mymsgbuf2) - sizeof(long int);
    
    msgrcv(msqid,&short_mess3,length1,pid,0);
    
    if(short_mess3.shortmsgtype = BYE)
    {
	return 0; 
    }
    else 
    {
	printf("no such type of msg, wrong end\n ");
	exit(-1);
    }
}
