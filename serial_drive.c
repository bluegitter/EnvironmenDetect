/*
 * @Author: your name
 * @Date: 2019-12-05 19:34:15
 * @LastEditTime : 2020-03-09 23:32:57
 * @LastEditors  : Please set LastEditors
 * @Description: 串口设置
 * @FilePath: /balance-101-4g/terminal_io.c
 */
#include <string.h>
#include <sys/ioctl.h>
#include "serial_drive.h"
#include "include/zlog.h"

UartInfo gUart;//串口结构体

#ifdef LOG
extern FILE *log_fd;
#endif
//open serial
int open_serial(char *dev)
{
    int fd;
    fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY, 0);
    if (fd < 1)
    {
        logMsg(logErr, "open <%s> error %s\n", dev, strerror(errno));
        zlog_error(zc,"open <%s> error..", dev);
        return -1;
    }

    return fd;
}

//close serial
void close_serial(int pf)
{
    close(pf);
}
/**
 * @description: 设置串口参数
 * @param {fd：设备文件，databits：数据位，stopbits：停止位，parity：校验位} 
 * @return: 返回true，串口设置成功。
 */
int set_termios(int fd, struct termios *options, int databits, int stopbits, char parity)
{

    printf("\n########串口设置#########\n");
    if (tcgetattr(fd, options) != 0)
    {
        zlog_error(zc,"get serial sattr error..");
        logMsg(logErr, "SetupSerial 1");
        return (false);
    }
    options->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options->c_iflag &= ~(BRKINT | ICRNL | ISTRIP | IXON); //串口接收到0x11/0x13不忽略
    options->c_oflag &= ~OPOST;
    options->c_cflag &= ~CSIZE;
    /* No hardware flow control */
    options->c_cflag &= ~CRTSCTS;
    switch (databits)
    { /*设置数据位数*/
    case 7:
        options->c_cflag |= CS7;
        logMsg(logInfo,"数据位:7");
        break;
    case 8:
        options->c_cflag |= CS8;
        logMsg(logInfo,"数据位:8");
        break;
    default:
        logMsg(logErr, "Unsupported data size");
        zlog_error(zc,"Unsupported data size,数据位默认设置为8");
        options->c_cflag |= CS8;
        logMsg(logInfo,"数据位默认设置:8");
        return (false);
    }
    switch (parity)
    {
    case 'n':
    case 'N':
        options->c_cflag &= ~PARENB; /* Clear parity enable */
        options->c_iflag &= ~INPCK;  /* disnable parity checking */
        logMsg(logInfo,"校验位:无校验");
        break;
    case 'o':
    case 'O':
        options->c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
        options->c_iflag |= INPCK;             /* enable parity checking */
        logMsg(logInfo,"校验位:奇校验");
        break;
    case 'e':
    case 'E':
        options->c_cflag |= PARENB;  /* Enable parity */
        options->c_cflag &= ~PARODD; /* 转换为偶效验*/
        options->c_iflag |= INPCK;   /* enable parity checking */
        logMsg(logInfo,"校验位:偶校验");
        break;
    case 'S':
    case 's': /*as no parity*/
        options->c_cflag &= ~PARENB;
        options->c_cflag &= ~CSTOPB;
        break;
    default:
        zlog_error(zc,"Unsupported parity,默认校验位:无校验");
        logMsg(logErr, "Unsupported parity");
        options->c_cflag &= ~PARENB; /* Clear parity enable */
        options->c_iflag &= ~INPCK;  /* disnable parity checking */
        logMsg(logInfo,"默认校验位:无校验");
        return (false);
    }
    /* 设置停止位*/
    switch (stopbits)
    {
    case 1:
        options->c_cflag &= ~CSTOPB;
        logMsg(logInfo,"停止位:1");
        break;
    case 2:
        options->c_cflag |= CSTOPB;
        logMsg(logInfo,"停止位:2");
        break;
    default:
        logMsg(logErr, "Unsupported stop bits");
        options->c_cflag &= ~CSTOPB;
        logMsg(logInfo,"默认停止位:1");
        return (false);
    }

    /* Set input parity option */
    if (parity != 'n')
        options->c_iflag |= INPCK;
    /* Output mode */
    options->c_oflag = 0;

    /* No active terminal mode */
    options->c_lflag = 0;

    /* Overflow data can be received, but not read */
    if (tcflush(fd, TCIFLUSH) < 0)
    {
        zlog_error(zc,"tcflush failed");
        logMsg(logErr, "tcflush failed");
        return (false);
    }

    if (tcsetattr(fd, TCSANOW, options) < 0)
    {
        zlog_error(zc,"SetupSerial failed");
        logMsg(logErr, "SetupSerial failed");
        return (false);
    }
    return (true);
}

//设置串口波特率
int set_baudrate(int fd, struct termios *opt, int baudrate)
{
    /* Input baud rate */
    if (cfsetispeed(opt, baudrate) < 0)
        return false;
    /* Output baud rate */
    if (cfsetospeed(opt, baudrate) < 0)
        return false;
    /* Overflow data can be received, but not read */
    if (tcflush(fd, TCIFLUSH) < 0)
        return false;
    if (tcsetattr(fd, TCSANOW, opt) < 0)
        return false;

    return true;
}
//查找对应的波特率
int find_baudrate(int rate)
{
    int baudr;

    switch (rate)
    {
    case 600:
        baudr = B600;
        break;
    case 1200:
        baudr = B1200;
        break;
    case 4800:
        baudr = B4800;
        break;
    case 9600:
        baudr = B9600;
        break;
    case 19200:
        baudr = B19200;
        break;
    case 38400:
        baudr = B38400;
        break;
    case 57600:
        baudr = B57600;
        break;
    case 115200:
        baudr = B115200;
        break;
    case 230400:
        baudr = B230400;
        break;
    case 460800:
        baudr = B460800;
        break;
    default:
        logMsg(logInfo, "invalid baudrate, set baudrate for 115200\n");
        baudr = B9600;
        break;
    }

    return baudr;
}
/* send len bytes data in buffer */
int serial_write(int fd, const void *buf, int len, struct timeval *write_tv)
{
    int count;
    int ret;
    fd_set output;

    ret = 0;
    count = 0;
    FD_ZERO(&output);
    FD_SET(fd, &output);
    do
    { /* listen */
        ret = select(fd + 1, NULL, &output, NULL, write_tv);
        if (ret == -1)
        { /* error */
            zlog_error(zc,"select() failed!");
            logMsg(logErr, "select() failed!");
            return ERROR;
            ;
        }
        else if (ret)
        { /* write buffer */
            ret = write(fd, (BYTE *)buf + count, len);
            if (ret < 1)
            {
                zlog_error(zc, "write error..");
                logMsg(logErr, "write error %s\n", strerror(errno));
                return ERROR;
                ;
            }
            count += ret;
            len -= ret;
        }
        else
        { /* timeout */
            zlog_error(zc, "time out..");
            logMsg(logErr, "time out.\n");
            return ERROR;
        }
    } while (len > 0);
    return count;
}
/**
 * @description:发送报文 
 * @param {type} 
 * @return: 返回发送数据的长度，返回-1错误
 */
int TransData(BYTE *buff, int bufLen)
{
    int write_size,i;
    char tmp[32];
    char buf[800];
    struct timeval tv;

    memset(buf,'\0',800);
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    write_size = serial_write(gUart.fd, buff, bufLen, &tv);

    if (write_size <= 0 || write_size != bufLen)
    {
        zlog_error(zc, "Transmited data error,data size of transmission=%d", write_size);
        logMsg(logErr, "Transmited data error,data size of transmission=%d\n", write_size);
        return 0;
    }
#ifdef LOG
    if (log_fd != NULL)
    {
        struct timeval time1; //精确到微秒
        struct tm timenow;    //实例化tm结构指针
        char date[32];

        gettimeofday(&time1, NULL); //获取微秒
        time1.tv_sec = time1.tv_sec + 8 * 3600;
        localtime_r(&time1.tv_sec, &timenow); //线程安全,将秒数转化为日历，并存储在timenow结构体

        strftime(date, 32, "%H:%M:%S", &timenow);                //将时间转化为自己需要的时间格式
        sprintf(date + 8, ".%03d", (int)(time1.tv_usec / 1000)); //将微秒追加到时间后面
        fprintf(log_fd, "%s send:%d=> ", date, write_size);
    }
#endif
    printf("send:%d=> ", write_size);
    for (i = 0; i < write_size; i++)
    {
        sprintf(tmp,"%02x ",(int)buff[i]);
        strcat(buf,tmp);
        printf("%02x ", (int)buff[i]);
#ifdef LOG
        if (log_fd != NULL)
        {
            fprintf(log_fd, "%02x ", (int)buff[i]);
        }

#endif
    }
    zlog_info(zc,"send:%d=> %s", write_size,buf);
#ifdef LOG
    if (log_fd != NULL)
    {
        fprintf(log_fd, "\n");
    }
#endif
    printf("\n");
    //logMsg(logInfo,"Transmited data size:%d",write_size);
    return write_size;
}
/**
 * @description:读串口不定长数据读,假设两个数据之间最大时间间隔1ms,等待1ms后下一数据还没到来说明一帧数据读完
 * @param {type} 
 * @return: 
//  */
int serial_read(int fd, const void *data, int bufLen, int timeout_sec)
{
    int cnt = 0;
    int read_size = 0;
    struct timeval timeout;
    fd_set rfds;

    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    switch (select(fd + 1, &rfds, NULL, NULL, &timeout))
    {
    case -1:
        logMsg(logErr, "select()\n");
        return ERROR;
    case 0:
        zlog_info(zc,"recv:接收超时啦!!!\n");
        printf("recv:接收超时啦!!!\n\n");
        return ERROR;
    default:
        if (FD_ISSET(fd, &rfds)) //判断fd是否可读。
        {
            do
            {
                read_size = read(fd, (BYTE *)data + cnt, bufLen);
                if (read_size > 0)
                {
                    cnt += read_size;
                    bufLen -= read_size;
                }
                usleep(100000); //延时等待下一个数据到来
            } while (read_size > 0);
        }
    }
    return cnt;
}
/**
 * @description: 接收报文
 * @param {type} 
 * @return: 返回-1，接收错误，，，否则返回接收到的数据长度
 */
int RecvData(BYTE *buff, int bufLen)
{
    char tmp[32];
    char buf[800];
    int read_size,i;

    read_size = serial_read(gUart.fd, (BYTE *)buff, bufLen, 5);//等待5s,设置超时时间为5s,5s没收到数据时,重新请求
    if (read_size > 0)
    { //接收到数据
        printf("recv:%d<= ", read_size);
        #ifdef LOG
            if (log_fd != NULL)
            {
                struct timeval time1; //精确到微秒
                struct tm timenow;    //实例化tm结构指针
                char date[32];

                gettimeofday(&time1, NULL); //获取微秒
                time1.tv_sec = time1.tv_sec + 8 * 3600;
                localtime_r(&time1.tv_sec, &timenow);                    //线程安全,将秒数转化为日历，并存储在timenow结构体
                strftime(date, 32, "%H:%M:%S", &timenow);                //将时间转化为自己需要的时间格式
                sprintf(date + 8, ".%03d", (int)(time1.tv_usec / 1000)); //将微秒追加到时间后面
                fprintf(log_fd, "%s recv:%d<= ", date, read_size);
         }
        #endif
        for (i = 0; i < read_size; i++)
        {
            sprintf(tmp,"%02x ",(int)buff[i]);
            strcat(buf,tmp);
            printf("%02x ", (int)buff[i]);
            #ifdef LOG
                if (log_fd != NULL){
                    fprintf(log_fd, "%02x ", (int)buff[i]);
                }
                #endif
        }
        zlog_info(zc,"send:%d=> %s", read_size,buf);
        #ifdef LOG
            if (log_fd != NULL){
                fprintf(log_fd, "\n");
            }
        #endif
        printf("\n");
        return read_size;
    }
    return 0;
}

#define __u8 unsigned char
#define TGPIO_IOC_MAGIC   'T'
#define TGPIO_IOC_GETVAL  _IOR(TGPIO_IOC_MAGIC, 1, __u8)
#define TGPIO_IOC_GETSTA  _IOR(TGPIO_IOC_MAGIC, 2, __u8)
#define TGPIO_IOC_SETHIGH _IOW(TGPIO_IOC_MAGIC, 2, __u8)
#define TGPIO_IOC_SETLOW  _IOW(TGPIO_IOC_MAGIC, 3, __u8)
#define TGPIO_IOC_TOGGLE  _IOW(TGPIO_IOC_MAGIC, 4, __u8)
/**
 * @description: RS485/RS232模式切换
 * @param {state:0低电平,1:高电平(注:高电平为485模式,低电平为232模式)} 
 * @return: 
 */
int RS485_RS232_ModelSwitch()
{
    int fd=0,ret=0;
    const char *gpiodrv = "/dev/gpio_sg";
    int gpio_idx = 0;

    fd = open(gpiodrv, O_RDWR|O_NOCTTY|O_NDELAY);
    if (fd < 1){
        return false;
    }

    if(strstr(gUart.port,"RS485_1") || strstr(gUart.port,"RS485_2")){
        //logMsg(logInfo,"set RS485_1 or RS485_2");
    }
    else if(strstr(gUart.port,"RS485_3")){
        gpio_idx = 4;
        ret = ioctl(fd, TGPIO_IOC_SETHIGH, &gpio_idx);
        if(ret != 0){
            logMsg(logErr,"set rs485 model error");
            return false;
        }
    }
    else if(strstr(gUart.port,"RS485_4")){
        gpio_idx = 5;
        ret = ioctl(fd, TGPIO_IOC_SETHIGH, &gpio_idx);
        if(ret != 0){
            logMsg(logErr,"set rs485 model error");
            return false;
        }
    }
    else if(strstr(gUart.port,"RS232_1")){
        gpio_idx = 4;
        ret = ioctl(fd, TGPIO_IOC_SETLOW, &gpio_idx);
        if(ret != 0){
            logMsg(logErr,"set rs485 model error");
            return false;
        }
    }
    else if(strstr(gUart.port,"RS232_2")){
        gpio_idx = 5;
        ret = ioctl(fd, TGPIO_IOC_SETLOW, &gpio_idx);
        if(ret != 0){
            logMsg(logErr,"set rs485 model error");
            return false;
        }
    }
    else {
        return false;
        logMsg(logErr,"gUart.port error! please set correct value.");
    }
    return true;
}
/**
 * @description: 串口初始化
 * @param {type} 
 * @return: 
 */
int serial_init(int *fd, struct termios *opt)
{
    int state;
    int baudrate;
    char parity;

    //解析config.json文件,提取串口参数
    state = Uart_info_parse(&gUart,FILENAME);
    if(state == false){
        zlog_error(zc, "解析串口.json文件错误");
        logMsg(logErr,"解析串口.json文件错误");
        return false;
    }

    /* open serial */
    *fd = open_serial(gUart.dev_name);
    if (*fd < 1){
        return false;
    }

    if(strstr((const char *)gUart.parity,"Odd")){
        parity = 'o';
    }
    else if(strstr((const char *)gUart.parity,"Event")){
        parity = 'e';
    }
    else{
        parity = 'n';
    }
    //串口设置，8位数据位，1位停止位
    state = set_termios(*fd, opt, 8, 1, parity);
    if (state == false){
        logMsg(logErr, "set termios failed\n");
        return false;
    }
    //串口终端波特率设置
    logMsg(logInfo,"波特率:%d",gUart.baudRate);
    baudrate = find_baudrate(gUart.baudRate);
    state = set_baudrate(*fd, opt, baudrate);
    if (state == false){
        logMsg(logErr, "set  baudrate 115200 ");
        return false;
    }
    return true;
}

#define FILE_NAME_LEN_MAX 32
/**
 * @param {type} 
 * @return: 
 * @Description: 打开配置脚本文件
 */
char *openJsonFile(const char *fileName)
{
    FILE *pf = NULL;
    char *data = NULL;
    long lfileLen = 0;
    int rcnt = 0;

    // 打开JSON文件
    if (NULL == (pf = fopen(fileName, "r")))
    {
        return NULL;
    }

    // 读取数据
    fseek(pf, 0, SEEK_END);
    lfileLen = ftell(pf); // 获取文件长度
    if (lfileLen == 0)
    {
        zlog_error(zc,  "文件内容为空.");
        logMsg(logWarn, "文件内容为空.");
        return NULL;
    }
    fseek(pf, 0, SEEK_SET); // 移到文件开始位置

    data = (char *)malloc(lfileLen + 1); // 申请内存
    if (NULL != data)
        rcnt = fread(data, sizeof(char), lfileLen, pf); //读数据

    // 关闭json文件
    fclose(pf);

    if (rcnt < 5)
    {
        logMsg(logWarn, "Read file only %d bytes.", rcnt);
    }

    return data;
}

/**
 * @param {type} 
 * @return: 
 * @Description: 保存配置脚本文件
 */
long saveJsonFile(cJSON *json, const char *fileName)
{
    FILE *pf;
    char *data = NULL;
    long lstrLen = 0, lLen = 0;
    char filePath[FILE_NAME_LEN_MAX];

    data = cJSON_Print(json); // 格式化输出
    lstrLen = strlen(data);   // 获取长度

    sprintf(filePath, "./%s", fileName);
    if (NULL == (pf = fopen(filePath, "w")))
    {
        zlog_error(zc,  "[Error ] 写文件失败.");
        logMsg(logInfo, "[Error ] 写文件失败.");
        return -1;
    }

    lLen = fwrite(data, sizeof(char), lstrLen, pf); // 输出到文件

    fclose(pf);

    return lLen;
}
/**
 * @description: 通过解析Config.json文件获取串口参数
 * @param {type} 
 * @return: 
 */
int Uart_info_parse(UartInfo *uart,const char *fileName)
{
    char tmp[128];
    char *fd = NULL;
    cJSON *pjson = NULL;
    cJSON *item = NULL;
    cJSON *child_item=NULL;

    printf("\n########串口参数解析#########\n");
    if (NULL == (fd = openJsonFile(fileName))){ // 打开配置脚本文件,fd指向文件内容
        sprintf(tmp,"/mnt/nand/env/%s",fileName);
        if (NULL == (fd = openJsonFile(tmp))){
            zlog_error(zc, "%s文件打开错误!", tmp);
            logMsg(logErr, "%s文件打开错误!", tmp);
            return false;
        }
    }

    // 解析数据
    pjson = cJSON_Parse(fd); //解析配置脚本文件成json格式

    // 提取串口参数
    item = cJSON_GetObjectItem(pjson, "Com");
    
    child_item = cJSON_GetObjectItem(item, "DevName");
    if (NULL == child_item){
        zlog_error(zc, "提取串口设备名错误!");
        logMsg(logErr,"提取串口设备名错误!");
        return false;
    }
    strncpy(uart->dev_name, child_item->valuestring, FILE_NAME_LEN_MAX);
    logMsg(logInfo, "串口设备名: %s", uart->dev_name);
    
    child_item = cJSON_GetObjectItem(item, "Port");
    if (NULL == child_item){
        zlog_error(zc, "提取串口port错误!");
        logMsg(logErr,"提取串口port错误!");
        return false;
    }
    strncpy(uart->port, child_item->valuestring, FILE_NAME_LEN_MAX);
    logMsg(logInfo, "串口port: %s", uart->port);

    child_item = cJSON_GetObjectItem(item, "BaudRate");
    if (NULL == child_item){
        zlog_error(zc, "提取BaudRate错误!");
        logMsg(logErr,"提取BaudRate错误!");
        return false;
    }
    uart->baudRate = child_item->valueint;
    logMsg(logInfo, "串口BaudRate: %d", uart->baudRate);

    child_item = cJSON_GetObjectItem(item, "Parity");
    if (NULL == child_item){
        zlog_error(zc, "提取Parity错误!");
        logMsg(logErr,"提取Parity错误!");
        return false;
    }
    strncpy(uart->parity, child_item->valuestring, FILE_NAME_LEN_MAX>>1);
    logMsg(logInfo, "串口Parity: %s", uart->parity);

    cJSON_Delete(pjson); //删除json缓冲
    free(fd);           //释放内存空间 在openJsonFile中申请

    return true;
}