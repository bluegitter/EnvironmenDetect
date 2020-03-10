/*
 * @Author: your name
 * @Date: 2019-12-18 18:55:03
 * @LastEditTime : 2020-03-09 23:55:23
 * @LastEditors  : Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /hebi-modbus/modbus_master.c
 */
//----------------------------------------------------------------------------//
//此代码只支持作为Modbus主站设备的Modbus RTU模式
//
//支持的功能码：
//0x01 读线圈状态
//0x03 读保持寄存器（读多个保持寄存器的值）
//0x04 读输入寄存器 (读多个输入寄存器的值)
//0x06 写单个寄存器（写入一个寄存器的值）
//0x10 写多个寄存器（写入多个寄存器的值）
//
//异常吗码的功能码:funCode+0x80
//支持的异常码：MAX_SEND_BUFF_LEN
//0x01 非法功能码
//0x02 非法数据地址（起始地址不在有效范围内）
//0x03 非法数据值（在起始地址的基础上，数量是不合法的）
//----------------------------------------------------------------------------//
#include <stdio.h>
#include <string.h>

#include "modbus_master.h"
#include "serial_drive.h"
#include "include/cJSON.h"
#include "syslog.h"
#include "include/zlog.h"

/* 变量定义 ------------------------------------------------------------------*/
// uchar Modbus_Send_Buff[MAX_SEND_BUFF_LEN]; //发送数据缓冲区
// uchar Modbus_Rcv_Buff[MAX_RECV_BUFF_LEN]; //接收数据缓冲区


FRAME_STRUC m_Txd={NULL,0}; //发送数据结构体
FRAME_STRUC m_Rxd={NULL,0}; //接收数据结构体

MODBUS_ARGU_STRU modbus_argu={0,0};//保存modbus从机地址和广播地址
Sensor_data gdata;
//----------------------------------------------------------------------------//
//函数功能：Modbus处理报文,帧错误则丢弃
//入口参数：无
//出口参数：无
//最后修改：
//备注：
//----------------------------------------------------------------------------//
int Master_handle_recvFrame(uchar *recv_buff,ushort recv_len,uchar *send_buff,VariableHandle *varStruc)
{
    uchar Modbus_CRC_Rcv_Hi; //接收到的ModbusCRC校验码高字节
    uchar Modbus_CRC_Rcv_Lo; //接收到的ModbusCRC校验码低字节
    ushort Modbus_CRC_Rcv;   //接收到的ModbusCRC校验码
    ushort Modbus_CRC_Cal;   //根据接收到的数据计算出来的CRC值
    
    //----------------------------------------------------------//
    //开始解析
    //----------------------------------------------------------//
    if(recv_buff == NULL ||  send_buff == NULL || recv_len < 5) //如果接收到的一帧的字节数大于4 首先确保帧的长度在正常范围
    {
        logMsg(logErr,"recv_buff is NULL or send_buff is NULL or recv_len lower 5");
        return ERROR;
    }

    Modbus_CRC_Rcv_Lo = recv_buff[recv_len - 2];//接收到的ModbusCRC校验码低字节
    Modbus_CRC_Rcv_Hi = recv_buff[recv_len - 1];//接收到的ModbusCRC校验码高字节
    Modbus_CRC_Rcv = MAKEWORD(Modbus_CRC_Rcv_Lo,Modbus_CRC_Rcv_Hi);//接收到的ModbusCRC校验码（16位）
    Modbus_CRC_Cal = crc16(recv_buff, recv_len - 2);//根据接收到的数据计算CRC值
    
    if(Modbus_CRC_Cal == Modbus_CRC_Rcv)//如果计算的CRC值与接收的CRC值相等
    {
        if(SLAVE_ADDR == recv_buff[0]) //如果是本机地址
        {
            switch(recv_buff[1]) //用switch分支语句来确定功能
            {
            case READ_COIL_STATE://读线圈状态
                Master_ReadCoilState_RespProc(recv_buff,varStruc);//从站相应读线圈状态处理
                break;
            case READ_HOLDING_REG:   //读保持寄存器                                   
                Master_ReadHoldingReg_RespProc(recv_buff,varStruc); //从站响应读保持寄存器处理
                break;     
            case READ_INPUT_REG: //读输入性寄存器
                Master_ReadHoldingReg_RespProc(recv_buff,varStruc); //从站响应读保持寄存器处理
                break;   
            default:
                if(recv_buff[1] & 0x80){//从机返回的异常功能码
                    zlog_error(zc,"function = %02x",recv_buff[1]);
                    logMsg(logErr,"function = %02x",recv_buff[1]);
                    Master_ErrorHandling(recv_buff);//异常功能码处理
                }
                else{
                    zlog_error(zc,"function = %02x",recv_buff[1]);
                    logMsg(logErr,"无法识别从机功能码!");
                    return ERROR;
                }    
            }
        }
        else
        {
            zlog_error(zc,"slave address error!");
            logMsg(logErr,"slave address error!");
            return ERROR;
        }   
    }
    else
    {
        zlog_error(zc,"crc16 error!");
        logMsg(logErr,"crc16 error!");
        return ERROR;
    } 

    return OK;
}
/**
 * @description: 主站发起读线圈状态请求
 * @param {type} 
 * @return: 
 */
int Master_ReadCoilState_Request(uchar *send_buff,VariableHandle varStruc,MODBUS_ARGU_STRU modb)
{
    ushort loc_send_len = 0;
    ushort mod_crc16;

    if(send_buff == NULL){
        logMsg(logErr,"send_buff pointer is NULL");
        return ERROR;
    }

    send_buff[loc_send_len++] = modb.slaveAddr;
    send_buff[loc_send_len++] = varStruc.func;

    send_buff[loc_send_len++] = HIBYTE(varStruc.startAddr);//起始地址高8位
    send_buff[loc_send_len++] = LOBYTE(varStruc.startAddr);//起始地址低8位

    send_buff[loc_send_len++] = HIBYTE(varStruc.number);//数据个数
    send_buff[loc_send_len++] = LOBYTE(varStruc.number);

    mod_crc16 = crc16(send_buff,loc_send_len);
    send_buff[loc_send_len++] = LOBYTE(mod_crc16);//crc16低8位
    send_buff[loc_send_len++] = HIBYTE(mod_crc16);//crc16高8位

    //******************************
     TransData(send_buff,loc_send_len);
    //******************************
    return OK;
}
/**
 * @description: 主站发起读保持寄存器
 * @param {type} 
 * @return: 
 */
int Master_ReadHoldingReg_Request(uchar *send_buff,VariableHandle varStruc,MODBUS_ARGU_STRU modb)
{
    ushort loc_send_len=0;
    ushort mod_crc16;

    if(send_buff == NULL)
    {
        logMsg(logErr,"send_buff pointer is NULL");
        return ERROR;
    }

    send_buff[loc_send_len++] = modb.slaveAddr;//从站地址
    send_buff[loc_send_len++] = varStruc.func;//功能码
    
    send_buff[loc_send_len++] = HIBYTE(varStruc.startAddr);//起始地址高8位
    send_buff[loc_send_len++] = LOBYTE(varStruc.startAddr);//起始地址低8位
    
    send_buff[loc_send_len++] = HIBYTE(varStruc.number);//数据个数
    send_buff[loc_send_len++] = LOBYTE(varStruc.number);

    mod_crc16 = crc16(send_buff,loc_send_len);
    send_buff[loc_send_len++] = LOBYTE(mod_crc16);//crc16低8位
    send_buff[loc_send_len++] = HIBYTE(mod_crc16);//crc16高8位

    //*send_len = loc_send_len;//保存报文长度

    //******************************
     TransData(send_buff,loc_send_len);
    //******************************
    return OK;
}
/**
 * @description: 从站响应读线圈状态处理
 * @param {type} 
 * @return: 
 */
int Master_ReadCoilState_RespProc(uchar *recv_buff,VariableHandle *varStruc)
{
    uchar byte_cnt; //数据字节数量
    uchar *ptr;
    ushort i, j,len=0;     //临时变量
    ushort tmp_num;

    printf("\n++++++++++++++++++++++++\n");
    logMsg(logInfo,"变量名:%s",varStruc->name);
    logMsg(logInfo,"数据类型:%s",varStruc->type);
    logMsg(logInfo,"起始地址:%04x",varStruc->startAddr);
    logMsg(logInfo,"数量:%d",varStruc->number);
    
    if(recv_buff == NULL || varStruc ==NULL){
        logMsg(logErr,"recv_buff or varStruc pointer is NULL");
        return ERROR;
    }

    ptr = (uchar *)varStruc->dataPtr;
    tmp_num = varStruc->number;//获取数量
    byte_cnt = recv_buff[2];//数据字节数
      
    for(i = 0; i < byte_cnt;i++)//读取寄存器的数据
    {
        if(tmp_num > 8){//线圈数量大于8
            for(j=0;j < 8;j++)
            {
                ptr[len++] = (recv_buff[3+i] & (0x01<<j)); //获取每一位状态
                printf("%s_%d:%d\r",varStruc->name,len+1,ptr[len]);
            }
            tmp_num -= 8;
        }
        else
        {
            for(j=0;j < tmp_num;j++)
            {
                ptr[len++] = (recv_buff[3+i] & (0x01<<j)); //获取每一位状态
                printf("%s_%d:%d\r",varStruc->name,len+1,ptr[len]);
            }
        } 
    }
    printf("\n");
    return OK;
}
/**
 * @description: 从站响应读保持寄存器处理
 * @param {type} 
 * @return: 
 */
int Master_ReadHoldingReg_RespProc(uchar *recv_buff,VariableHandle *varStruc)
{
    uchar byte_cnt; //数据字节数量
    ushort i, j=0,len=0;     //临时变量
    short *ptr;

    if(recv_buff == NULL || varStruc == NULL){
        logMsg(logErr,"error! pointer to NULL.");
        return ERROR;
    }

    ptr = (short*)varStruc->dataPtr;
    byte_cnt = recv_buff[2];//数据字节数
    
    printf("\n++++++++++++++++++++++++\n");
    logMsg(logInfo,"变量名:%s",varStruc->name);
    logMsg(logInfo,"数据类型:%s",varStruc->type);
    logMsg(logInfo,"起始地址:%04x",varStruc->startAddr);
    logMsg(logInfo,"数量:%d",varStruc->number);

    if(byte_cnt != 2*varStruc->number) {
        logMsg(logErr,"number of reading hold register error_%s",varStruc->name);
        return ERROR;
    }
    
    for(i = 0, j = 3; i < varStruc->number; i++, j += 2){//读取寄存器的数据
        ptr[len++] = MAKEWORD(recv_buff[j+1],recv_buff[j]);
         printf("%s_%d:%04x\r",varStruc->name,len+1,ptr[len]);
    }           

    printf("\n");
    return OK;
}
/**
 * @description: 错误码处理
 * @param {type} 
 * @return: 
 */
void Master_ErrorHandling(uchar *recv_buff)
{
    uchar excep_code; //异常码

    excep_code = recv_buff[2];
    switch(excep_code)
    {
    case 0x01:
        zlog_info(zc,"[ERROR]:从机无法识别功能码!");
        logMsg(logErr,"从机无法识别功能码!");
        break;
    case 0x02:
        zlog_info(zc,"[ERROR]:从机识别到错误寄存器地址!");
        logMsg(logErr,"从机识别到错误寄存器地址!");
        break;

    case 0x03:
        zlog_info(zc,"[ERROR]:从机识别到错误数据数量!");
        logMsg(logErr,"从机识别到错误数据数量!");
        break;
    default: 
        zlog_info(zc,"[ERROR]:Unknown exception code,exception code = %02x",excep_code);
        logMsg(logErr,"Unknown exception code,exception code = %02x",excep_code);
    }
}

//crc16校验码计算方法:计算法和查找法

/***********************************
 * 查找法
 ***********************************/
const ushort crc_16_tab[] = {
  0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
  0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
  0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
  0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
  0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
  0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
  0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
  0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
  0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
  0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
  0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
  0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
  0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
  0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
  0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
  0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
  0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
  0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
  0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
  0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
  0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
  0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
  0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
  0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
  0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
  0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
  0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
  0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
  0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
  0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
  0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
  0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};
/**
 * @description: 计算modbus-crc16校验
 * @param {type} 
 * @return: 
 */
ushort crc16(uchar *buf, uint dataLen)
{
    ushort crc = 0xffff;

    if ( buf == NULL ) 
        return 0;
            
    while (dataLen--)
        crc = (ushort)(crc >> 8) ^ crc_16_tab[(crc ^ *buf++) & 0x00ff];
    return crc;
}
/***********************************
 * 计算法
 ***********************************/
/**
 * @description: 返回高低位翻转的16位数据,直接附加到modbus报文中
 * @param {type} 
 * @return: 
 */
ushort Modbus_CRC16(uchar *puchMsg, uint dataLen)
{
    ushort CRC_Cal = 0xFFFF;//初始值
    uchar CRC_High, CRC_Low;
    uint j;
    uchar i;
    
    if ( puchMsg == NULL ) 
        return 0;
            
    for(j = 0; j < dataLen; j++)
    {
        CRC_Cal = CRC_Cal ^ *puchMsg++;
        
        for (i = 0; i < 8; i++)
        {
            if((CRC_Cal & 0x0001) == 0x0001)
            {
                CRC_Cal = CRC_Cal >> 1;
                CRC_Cal = CRC_Cal ^ 0xA001;
            }
            else
            {
                CRC_Cal = CRC_Cal >> 1;
            }
        }
    }
    CRC_High = (uchar)(CRC_Cal >> 8);
    CRC_Low = (uchar)(CRC_Cal & 0x00FF);
    
    return (CRC_Low << 8 | CRC_High);
    //return CRC_Cal;

}
#define ITEM_LEN_MAX 20
/**
 * @description: 解析modbus.json文件,提取参数
 * @param {type} 
 * @return: 返回链表头节点,根据头结点遍历整个链表
 */
LinkedList modbus_config_init(MODBUS_ARGU_STRU *modPara,const char *fileName)
{
    char tmp[128];
    LinkedList headNode;
    LinkedList element;
    uint reg_addr;
    VariableHandle *var_struc;
    char *file_data = NULL;
    uchar arraySize,i;
    cJSON *pjson = NULL;
    cJSON *item = NULL;
    cJSON *child_item=NULL;
    cJSON *arr_item=NULL;
    cJSON *arr_object=NULL;

    printf("\n########modbus配置文件解析########\n");
    if (NULL == (file_data = openJsonFile((char *)fileName))){ // 打开配置脚本文件,fd指向文件内容
        sprintf(tmp,"/mnt/nand/env/%s",fileName);
        if (NULL == (file_data = openJsonFile(tmp))){
            zlog_error(zc, "%s文件打开错误!", tmp);
            logMsg(logErr, "%s文件打开错误!", tmp);
            return NULL;
        }
    }
    // 解析数据
    pjson = cJSON_Parse(file_data); 
    // 提取从机地址
    item = cJSON_GetObjectItem(pjson, "ModbusPara");
    child_item = cJSON_GetObjectItem(item, "SlaveAddr");
    if(child_item == NULL){
        zlog_error(zc,"提取从机地址错误!");
        logMsg(logErr,"提取从机地址错误!");
        return NULL;
    }
    modPara->slaveAddr = child_item->valueint;
    logMsg(logInfo, "从机地址: %02x[十六进制]", modPara->slaveAddr);

    //提取广播地址
    child_item = cJSON_GetObjectItem(item, "BroadcastAddr");
    if(child_item == NULL){
        zlog_error(zc,"提取广播地址错误!");
        logMsg(logErr,"提取广播地址错误!");
        return false;
    }
    modPara->broadcastAddr = child_item->valueint;
    logMsg(logInfo, "广播地址: %02x[十六进制]", modPara->broadcastAddr);
    //解析数组
    child_item = cJSON_GetObjectItem(item, "Variable_Read");
    if(cJSON_IsArray(child_item) == 0){ 
        zlog_error(zc,"Variable is not array!");
        logMsg(logErr,"Variable is not array!"); 
        return false;
    }
    arraySize = cJSON_GetArraySize(child_item);
    if(arraySize > 0){
       element = headNode =  LinkedList_create();//创建表头链表,表头链表数据为空
    }
    else{//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!警告,如果数组为空,则结束程序
          zlog_error(zc,"配置文件中variable_Read数组内没有元素!");
        logMsg(logErr,"配置文件中variable_Read数组内没有元素!");
        return false;
    }

    for(i=0;i<arraySize;i++){
        printf("=============================\n");
        //在链表最后面增加新元素
        LinkedList_add(element,GLOBAL_MALLOC(sizeof(VariableHandle)));
        element = element->next;
        arr_item = cJSON_GetArrayItem(child_item,i);
        var_struc = (VariableHandle *)element->data;
        
        //提取变量名
        arr_object = cJSON_GetObjectItem(arr_item, "Name");
        if (NULL != arr_object){
           
            strncpy(var_struc->name, arr_object->valuestring, ITEM_LEN_MAX);
            logMsg(logInfo, "变量名：%s",var_struc->name);
        }
        //提取数据类型
        arr_object = cJSON_GetObjectItem(arr_item, "Type");
        if (NULL != arr_object){
            strncpy(var_struc->type, arr_object->valuestring, ITEM_LEN_MAX);
            logMsg(logInfo, "数据类型：%s",var_struc->type);
        }
        //提取起始地址
        arr_object = cJSON_GetObjectItem(arr_item, "StartAddr");
        if (NULL != arr_object){
            sscanf(arr_object->valuestring,"%x",&reg_addr);
            var_struc->startAddr = reg_addr;
            logMsg(logInfo, "起始地址：%04x[十六进制]",var_struc->startAddr);
        }
        //提取结束地址
        arr_object = cJSON_GetObjectItem(arr_item, "EndAddr");
        if (NULL != arr_object){
            sscanf(arr_object->valuestring,"%x",&reg_addr);
            var_struc->endAddr = reg_addr;
            logMsg(logInfo, "结束地址：%04x[十六进制]",var_struc->endAddr);
        }
        //提取数量
        arr_object = cJSON_GetObjectItem(arr_item, "Number");
        if (NULL != arr_object){
            var_struc->number = arr_object->valueint;
            logMsg(logInfo, "数量：%d",var_struc->number);
        }
        //提取功能码
        arr_object = cJSON_GetObjectItem(arr_item, "Func");
        if (NULL != arr_object){
            var_struc->func = arr_object->valueint;
            logMsg(logInfo, "功能码：%d",var_struc->func);
        }
        if(strstr(var_struc->type,"bit") || strstr(var_struc->type,"char")){
            var_struc->dataPtr = GLOBAL_MALLOC(var_struc->number);
        }
        else if(strstr(var_struc->type,"short")){
            var_struc->dataPtr = GLOBAL_MALLOC(2*var_struc->number);
        }
        else if(strstr(var_struc->type,"int")){
            var_struc->dataPtr = GLOBAL_MALLOC(4*var_struc->number);
        }
        else if(strstr(var_struc->type,"float")){
            var_struc->dataPtr = GLOBAL_MALLOC(4*var_struc->number);
        }
        else{
            logMsg(logErr,"data type error! supportting data type for bit char short int float.");
            return NULL;
        }
    }
    printf("链表大小:%d\n\n",LinkedList_size(headNode));

    cJSON_Delete(pjson); //删除json缓冲
    free(file_data);     //释放内存空间 在openJsonFile中申请

    return headNode;
}

////////////////////////////////////////////////
/**
 * @description: 销毁链表(特殊情况)
 * @param {type} 
 * @return: 
 */
void My_linkedList_destroy(LinkedList list)
{
    LinkedList nextElement = list;
    LinkedList currentElement;
    VariableHandle *var;

    do {
        currentElement = nextElement;
        nextElement = currentElement->next;

        if (currentElement->data != NULL){
            var = (VariableHandle*)currentElement->data;
            if(var->dataPtr != NULL){
                Memory_free(var->dataPtr);
            }
            Memory_free(currentElement->data);
        }
            
        GLOBAL_FREEMEM(currentElement);
    }
    while (nextElement != NULL);
}
/**
 * @description: 处理链表中的数据
 * @param {type} 
 * @return: 
 */
void Sensor_data_process(Sensor_data *data,LinkedList headNode)
{
    uchar len;
    ushort i,j,*p;
    LinkedList element;
    element = headNode;//获取链表头节点
    VariableHandle *var;
    
    while(element->next != NULL){//遍历链表
        element = element->next;
        if(element->data != NULL){//判断数据是否为空
            printf("==============================\n");
            len=0;
            var = (VariableHandle *)element->data;
            p = (ushort *)var->dataPtr;
            
            if(strstr(var->name,"TmpEnvHum")) {//温湿度
                for(i=0;i<10;i++){
                    data->tmp[i] = (float)(p[len++])*0.1;//温度
                    data->hum[i] = (float)(p[len++])*0.1;//湿度
                    printf("温湿度传感器_%02d号_温度:%.1f\n",i+1,data->tmp[i]);
                    printf("温湿度传感器_%02d号_湿度:%.1f\n",i+1,data->hum[i]);
                }
            }
            else if (strstr(var->name,"Tmp")){//温度
                for(i=0;i<10;i++){//注册十台设备
                    for(j=0;j<6;j++){//每台6个温度值
                        data->line_tmp[i][j] = (float)(p[len++])*0.1-20;//提取温度值
                        printf("温度传感器_%02d号_温度:%.1f\n",len,data->line_tmp[i][j]);
                    }
                }
            }

            else if(strstr(var->name,"Smoke")){//烟雾状态
                for(i=0;i<var->number;i++){
                    data->smokeAlm[i] = (p[len++] == 0)?0:1;
                    printf("%02d号_烟雾状态:%d\n",i+1,data->smokeAlm[i]);
                }
            }
            else if(strstr(var->name,"Lev")){//水位
                for(i=0;i<var->number;i++){
                    data->lev[i] = p[len++];
                    printf("%02d号_水位:%d\n",i+1,(int)data->lev[i]);
                }
            }
            else {
                printf("错误的传感器变量名\n");
            }
        }
    }
}