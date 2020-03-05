/*
 * @Author: your name
 * @Date: 2019-12-19 23:18:38
 * @LastEditTime : 2020-02-25 20:18:04
 * @LastEditors  : Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /hebi-modbus/serial_drive.h
 */
/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-08-14 00:52:19
 * @LastEditTime: 2019-12-19 23:29:45
 * @LastEditors: Please set LastEditors
 */
#ifndef _SERIAL_DRIVE_H_
#define _SERIAL_DRIVE_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>

#include "gtypedef.h"
#include "syslog.h"
#include "include/cJSON.h"

#define FILENAME "modbusConfig.json"

//串口结构体
typedef struct uart_info{
    int   fd;
    char  dev_name[32];
    char  port[16];
    int baudRate;
    uchar dataBit;
    uchar stopBit;
    char parity[16];
} UartInfo;
extern UartInfo gUart;

int  open_serial(char *dev);
void close_serial(int pf);
int set_termios(int fd, struct termios *options, int databits, int stopbits, char parity);
int set_baudrate(int fd, struct termios *opt, int baudrate);
int find_baudrate(int rate);
int serial_read(int fd, const void *data, int bufLen,int timeout_sec);
int TransData(uchar *buff, int bufLen);
int serial_write(int fd, const void *buf, int len, struct timeval *write_tv);
int RecvData(uchar *buff,int bufLen);
int RS485_RS232_ModelSwitch();
int serial_init(int *fd,struct termios *opt);
char *openJsonFile(const char *fileName);
long saveJsonFile(cJSON *json, const char *fileName);
int Uart_info_parse(UartInfo *uart,const char *fileName);
#endif