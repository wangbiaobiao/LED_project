#ifndef  __SEMAPHORE__
#define  __SEMAPHORE__

#include "common.h"

extern int rs485_sem_id;
extern int network_send_sem_id;
extern int network_recieve_sem_id;

boolean semaphore_init();

boolean set_semvalue(void);

void del_semvalue(void);

int get_sem_val();

boolean rs485_semaphore_p(void);

boolean rs485_semaphore_v(void);

boolean network_send_semaphore_p(void);

boolean network_send_semaphore_v(void);

boolean network_recieve_semaphore_p(void);

boolean network_recieve_semaphore_v(void);


#endif

