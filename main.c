
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#include "serial_drive.h"
#include "gtypedef.h"
#include "modbus_master.h"
#include "mosquitto_client.h"
#include "include/zlog.h"

#ifdef LOG
    FILE *log_fd;
#endif

LinkedList gHeadNode = NULL;//链表头节点
sem_t g_sem;//信号量,用于同步线程

/* Short option names */
static const char g_shortopts [] = "v";

zlog_category_t *zc; //zlog变量分类

//信号处理函数
static void signal_handle(int signo)
{
    printf("pressed ctrl+c.\n");
    #ifdef LOG 
        if(log_fd != NULL)
        {
            fclose(log_fd);//记录报文文件描述词
        }
    #endif
    exit(0);
}
int main(int argc, char **argv) 
{
    int rc;
	struct termios opt;
    int flag = 0,ch;
    LinkedList element;
    VariableHandle *var_struc;
	pthread_t t1;

    //短选项参数
    while ((ch = getopt(argc, argv, g_shortopts)) != -1)
    {
        switch (ch) 
        {
            case 'v':
            printf("EnvironmentDetect version \"v1.00_20200305\" \n");
            exit(0);
        }
    }
    rc = zlog_init("zlog.conf");
    if (rc) {
        rc = zlog_init("/mnt/nand/env/zlog.conf");
        if (rc) {
            printf("zlog init failed\n");
            printf("rc=%d\n",rc);
            return -1;
        }
    }
    zc = zlog_get_category("my_cat");
    if (!zc) {
        printf("zlog get cat fail\n");
        zlog_fini();
        return -2;
    }
    //信号处理
    signal(SIGINT,signal_handle);
    //信号量初始化
    flag = sem_init(&g_sem, 0, 0);
    if (flag == -1) {
        printf("sem_init failed \n");
        zlog_error(zc,"sem_init failed");
        return -1;
    }
    //创建线程
	pthread_create(&t1, NULL, mosquitto_client_thread, NULL);  //创建mosquitto客户端应用线程
    sleep(1);
    //串口初始化
    flag=serial_init(&gUart.fd,&opt);
    if(flag==false) {
         printf("serial inital failed.\n");
         zlog_error(zc,"serial inital failed.");
         return -1;
    }
    //modbus配置初始化
    gHeadNode = modbus_config_init(&modbus_argu,FILENAME);
    if(gHeadNode == NULL) {
         printf("modbus config init failed!\n");
         zlog_error(zc,"modbus config init failed!");
         return -1;
    }
    #ifdef LOG
        time_t now;
        struct tm timenow;
        char date[32];
        time(&now);  //得到时间秒数
        now = now + 8*3600;
        localtime_r(&now, &timenow);  //线程安全,将秒数转化为日历，并存储在timenow结构体
        strftime(date,32,"master-%Y-%m-%d,%H:%M:%S.txt",&timenow);
        log_fd=fopen(date,"w");//创建记录报文文件
    #endif
    
    //分配内存
    m_Rxd.buf = (BYTE *)malloc(MAX_RECV_BUFF_LEN);
    m_Txd.buf = (BYTE *)malloc(MAX_SEND_BUFF_LEN);
    //内存初始化
    memset(m_Rxd.buf, 0, sizeof(BYTE) * MAX_RECV_BUFF_LEN);
    memset(m_Txd.buf, 0, sizeof(BYTE) * MAX_SEND_BUFF_LEN);

    element = gHeadNode;//获取链表头节点
    while(1)
    {
        if(element->next != NULL){
            element = element->next;
            var_struc = (VariableHandle *)element->data;
            switch (var_struc->func)//判断功能码
            {
            case 0x01://读线圈状态
                Master_ReadCoilState_Request(m_Txd.buf,*var_struc,modbus_argu);
                break;
            case 0x03://读保持寄存器
                Master_ReadHoldingReg_Request(m_Txd.buf,*var_struc,modbus_argu);
                break;
            case 0x04://读输入寄存器
                Master_ReadHoldingReg_Request(m_Txd.buf,*var_struc,modbus_argu);
                break;
            default:
                printf("无法识别配置文件中的功能码,功能码:%d",(int)var_struc->func);
                zlog_error(zc,"无法识别配置文件中的功能码,功能码:%d",(int)var_struc->func);
                break;
            }
            //查询接收
            if((m_Rxd.len = RecvData(m_Rxd.buf, MAX_RECV_BUFF_LEN)) > 0)
            {
                Master_handle_recvFrame(m_Rxd.buf,m_Rxd.len,m_Txd.buf,var_struc);//处理接收数据  
                usleep(10000);//延时10ms再召唤
            }
            
        }
        else{//遍历完一次链表,重新赋值表头
            Sensor_data_process(&gdata,gHeadNode);
            element = gHeadNode;
            sem_post(&g_sem);

        }   
    }

    pthread_join (t1, NULL);//等待线程t1结束
    My_linkedList_destroy(gHeadNode);//深度销毁链表
    close_serial(gUart.fd);
    zlog_fini();
    return 0;
}