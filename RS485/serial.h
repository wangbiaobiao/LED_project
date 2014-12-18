#ifndef __SERIAL_H__
#define __SERIAL_H__
#include "common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define SERIAL_RETRY 20
int  serial_open(const char *dev, unsigned int speed);
void serial_close(int fd);


boolean serial_read (int fd, void *buf, int read_len,int* has_read_len);

boolean serial_write(int fd, const void *buf, int count);


#ifdef __cplusplus
};
#endif

#endif /* __SERIAL_H__ */

