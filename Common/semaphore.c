#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "semaphore.h"
#include <errno.h>

int rs485_sem_id = -1;
int network_send_sem_id = -1;
int network_recieve_sem_id = -1;

union semun { int val; struct semid_ds *buf; unsigned short int *array; struct seminfo *__buf; };
boolean semaphore_init()
{
	rs485_sem_id = semget((key_t)5564, 1, 0666 | IPC_CREAT);
	if(rs485_sem_id == -1)
		return FALSE;
	printf("create rs485_sem_id success \n");

	network_send_sem_id = semget((key_t)6664, 1, 0666 | IPC_CREAT);
	if(network_send_sem_id == -1)
		return FALSE;
	printf("create network_send_sem_id success \n");

	network_recieve_sem_id = semget((key_t)8864, 1, 0666 | IPC_CREAT);
	if(network_recieve_sem_id == -1)
		return FALSE;
	printf("create network_recieve_sem_id success \n");
	return TRUE;			
}

boolean set_semvalue(void)
{
    union semun sem_union;
    sem_union.val = 1;
    if(semctl(rs485_sem_id, 0, SETVAL, sem_union) == -1)
    {
    	printf("rs485_sem_id:%s\n", strerror(errno));
		return FALSE;
	}
	
    if(semctl(network_send_sem_id, 0, SETVAL, sem_union) == -1)
    {
    	printf("rs485_sem_id:%s\n", strerror(errno));
		return FALSE;	
    }
	if(semctl(network_recieve_sem_id, 0, SETVAL, sem_union) == -1)
	{
    	printf("rs485_sem_id:%s\n", strerror(errno));
		return FALSE;	
	}
	return TRUE;
}

void del_semvalue(void)
{
    union semun sem_union;

    if(semctl(rs485_sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete rs485_sem_id/n");    
	
    if(semctl(network_send_sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete network_sem_id/n"); 

	if(semctl(network_recieve_sem_id, 0, IPC_RMID, sem_union) == -1)
		fprintf(stderr, "Failed to delete network_sem_id/n"); 

}

int get_rs485_sem_val()
{
	return semctl(rs485_sem_id,0,GETVAL,0);
}


boolean rs485_semaphore_p(void)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    if(semop(rs485_sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "rs485_semaphore_p failed/n");
        return FALSE;
    }
    return TRUE;
}

boolean rs485_semaphore_v(void)
{
//    if(get_rs485_sem_val == 0)
//      {
        struct sembuf sem_b;

   	 sem_b.sem_num = 0;
	 sem_b.sem_op = 1;
	 sem_b.sem_flg = SEM_UNDO;
         if(semop(rs485_sem_id, &sem_b, 1) == -1)
         {
            fprintf(stderr, "rs485_semaphore_v failed/n");
            return FALSE;
         }
         return TRUE;
//    }
//   return FALSE;
}

boolean network_send_semaphore_p(void)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    if(semop(network_send_sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "network_send_semaphore_p failed/n");
        return FALSE;
    }
    return TRUE;

}

boolean network_send_semaphore_v(void)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;
    if(semop(network_send_sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "network_send_semaphore_v failed/n");
        return FALSE;
    }
    return TRUE;

}

boolean network_recieve_semaphore_p(void)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    if(semop(network_recieve_sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "network_recieve_semaphore_p failed/n");
        return FALSE;
    }
    return TRUE;

}

boolean network_recieve_semaphore_v(void)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;
    if(semop(network_recieve_sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "network_recieve_semaphore_v failed/n");
        return FALSE;
    }
    return TRUE;

}



