/*
 * @Author: your name
 * @Date: 2019-12-18 18:55:13
 * @LastEditTime : 2020-03-04 18:27:16
 * @LastEditors  : Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /modbus-slave/modbus-master.h
 */
#ifndef _MODBUS_MASTER__H
#define _MODBUS_MASTER__H

#include"linked_list.h"
#include "gtypedef.h"

//宏定义缓冲区长度
#define MAX_SEND_BUFF_LEN 256 //发送缓冲长度
#define MAX_RECV_BUFF_LEN 256 //接收缓冲长度
//地址宏定义
#define RADIO_BROAD_ADDR 0x00 //广播地址
#define SLAVE_ADDR 0x01 //从站地址
//功能码
#define READ_COIL_STATE 0x01  //读线圈状态
#define READ_HOLDING_REG 0x03 //读保持寄存器
#define READ_INPUT_REG 0x04  //读输入寄存器
#define WRITE_SINGLE_REG 0x06  //写单个寄存器
#define WRITE_MULTIPLE_REG 0x10//写多个寄存器

#ifndef PACKED
#define PACKED __attribute__((packed)) //不对齐，结构体的长度，就是各个成员变量长度的和
#endif

//收发结构体
typedef struct 
{
    uchar *buf;
    ushort  len; 
} FRAME_STRUC;
extern FRAME_STRUC m_Rxd;
extern FRAME_STRUC m_Txd; 
//变量结构体
typedef struct
{
    char name[20]; //变量名
    char type[20]; //数据类型
    ushort  startAddr; //起始地址
    ushort  endAddr; //结束地址
    ushort number; //数量
    uchar func;
    void *dataPtr; //指向数据指针
}  VariableHandle;

//modubs报文的主要参数
typedef struct 
{
    uchar  slaveAddr;
    uchar  broadcastAddr;
}MODBUS_ARGU_STRU;
extern MODBUS_ARGU_STRU modbus_argu;
//保存转换后的传感器数据
typedef struct 
{
    float line_tmp[10][6];
    float tmp[10];
    float hum[10];
    uchar smokeAlm[10];
    ushort lev[10];
}Sensor_data;
extern Sensor_data gdata;

int Master_handle_recvFrame(uchar *recv_buff,ushort recv_len,uchar *send_buff,VariableHandle *varStruc);
int Master_ReadCoilState_Request(uchar *send_buff,VariableHandle varStruc,MODBUS_ARGU_STRU modb);
int Master_ReadHoldingReg_Request(uchar *send_buff,VariableHandle varStruc,MODBUS_ARGU_STRU modb);
int Master_ReadCoilState_RespProc(uchar *recv_buff,VariableHandle *varStruc);
int Master_ReadHoldingReg_RespProc(uchar *recv_buff,VariableHandle *varStruc);
void Master_ErrorHandling(uchar *recv_buff);
ushort crc16(uchar *buf, uint dataLen);//查找法(效率高)
ushort Modbus_CRC16(uchar *puchMsg, uint dataLen);//计算法
void Sensor_data_process(Sensor_data *data,LinkedList headNode);
LinkedList modbus_config_init(MODBUS_ARGU_STRU *modPara,const char *fileName);
void My_linkedList_destroy(LinkedList list);
#endif