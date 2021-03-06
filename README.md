<!--
 * @Author: your name
 * @Date: 2020-03-03 02:19:38
 * @LastEditTime : 2020-03-09 23:46:34
 * @LastEditors  : Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /EnvironmentDetec/README.md
 -->
<center><font face="黑体" size=20>环境检测模块设计文档</font></center>
[TOP]

# 1 引言
    
## 1.1目的

    描述环境检测模块设计思路、逻辑实现，用于项目组成员交流。

## 1.2 设计思路
    环境检测程序划分为两个线程处理,子线程和主线程;主线程负责实现modbus规约,召唤子站的传感器数据,召唤完传感器数据后,释放同步线程信号量;子线程负责实现mqtt协议,实现与数据中心交互,当获取到同步线程信号量时,将传感器数据上送到数据中心.

# 2 环境说明

## 2.1 硬件平台

   zhixin融合终端

## 2.2 通信点表

    温度 60
    温湿度 10
    烟感 10
    水位 10

    序号	寄存器号	名称	描述
    1	0000H	1号温度值	实际值=上传值*0.1-20（℃）
    2	0001H	2号温度值	实际值=上传值*0.1-20（℃）
    3	0002H	3号温度值	实际值=上传值*0.1-20（℃）
    4	0003H	4号温度值	实际值=上传值*0.1-20（℃）
    5	0004H	5号温度值	实际值=上传值*0.1-20（℃）
    6	0005H	6号温度值	实际值=上传值*0.1-20（℃）
    7	0006H	7号温度值	实际值=上传值*0.1-20（℃）
    8	0007H	8号温度值	实际值=上传值*0.1-20（℃）
    9	0008H	9号温度值	实际值=上传值*0.1-20（℃）
    10	0009H	10号温度值	实际值=上传值*0.1-20（℃）
    11	000AH	11号温度值	实际值=上传值*0.1-20（℃）
    12	000BH	12号温度值	实际值=上传值*0.1-20（℃）
    13	000CH	13号温度值	实际值=上传值*0.1-20（℃）
    14	000DH	14号温度值	实际值=上传值*0.1-20（℃）
    15	000EH	15号温度值	实际值=上传值*0.1-20（℃）
    16	000FH	16号温度值	实际值=上传值*0.1-20（℃）
    17	0010H	17号温度值	实际值=上传值*0.1-20（℃）
    18	0011H	18号温度值	实际值=上传值*0.1-20（℃）
    19	0012H	19号温度值	实际值=上传值*0.1-20（℃）
    20	0013H	20号温度值	实际值=上传值*0.1-20（℃）
    21	0014H	21号温度值	实际值=上传值*0.1-20（℃）
    22	0015H	22号温度值	实际值=上传值*0.1-20（℃）
    23	0016H	23号温度值	实际值=上传值*0.1-20（℃）
    24	0017H	24号温度值	实际值=上传值*0.1-20（℃）
    25	0018H	25号温度值	实际值=上传值*0.1-20（℃）
    26	0019H	26号温度值	实际值=上传值*0.1-20（℃）
    27	001AH	27号温度值	实际值=上传值*0.1-20（℃）
    28	001BH	28号温度值	实际值=上传值*0.1-20（℃）
    29	001CH	29号温度值	实际值=上传值*0.1-20（℃）
    30	001DH	30号温度值	实际值=上传值*0.1-20（℃）
    31	001EH	31号温度值	实际值=上传值*0.1-20（℃）
    32	001FH	32号温度值	实际值=上传值*0.1-20（℃）
    33	0020H	33号温度值	实际值=上传值*0.1-20（℃）
    34	0021H	34号温度值	实际值=上传值*0.1-20（℃）
    35	0022H	35号温度值	实际值=上传值*0.1-20（℃）
    36	0023H	36号温度值	实际值=上传值*0.1-20（℃）
    37	0024H	37号温度值	实际值=上传值*0.1-20（℃）
    38	0025H	38号温度值	实际值=上传值*0.1-20（℃）
    39	0026H	39号温度值	实际值=上传值*0.1-20（℃）
    40	0027H	40号温度值	实际值=上传值*0.1-20（℃）
    41	0028H	41号温度值	实际值=上传值*0.1-20（℃）
    42	0029H	42号温度值	实际值=上传值*0.1-20（℃）
    43	002AH	43号温度值	实际值=上传值*0.1-20（℃）
    44	002BH	44号温度值	实际值=上传值*0.1-20（℃）
    45	002CH	45号温度值	实际值=上传值*0.1-20（℃）
    46	002DH	46号温度值	实际值=上传值*0.1-20（℃）
    47	002EH	47号温度值	实际值=上传值*0.1-20（℃）
    48	002FH	48号温度值	实际值=上传值*0.1-20（℃）
    49	0030H	49号温度值	实际值=上传值*0.1-20（℃）
    50	0031H	50号温度值	实际值=上传值*0.1-20（℃）
    51	0032H	51号温度值	实际值=上传值*0.1-20（℃）
    52	0033H	52号温度值	实际值=上传值*0.1-20（℃）
    53	0034H	53号温度值	实际值=上传值*0.1-20（℃）
    54	0035H	54号温度值	实际值=上传值*0.1-20（℃）
    55	0036H	55号温度值	实际值=上传值*0.1-20（℃）
    56	0037H	56号温度值	实际值=上传值*0.1-20（℃）
    57	0038H	57号温度值	实际值=上传值*0.1-20（℃）
    58	0039H	58号温度值	实际值=上传值*0.1-20（℃）
    59	003AH	59号温度值	实际值=上传值*0.1-20（℃）
    60	003BH	60号温度值	实际值=上传值*0.1-20（℃）
    71	01E0H	1号温湿度温度值	实际值=上传值*0.1（℃）
    72	01E1H	1号温湿度湿度值	实际值=上传值*0.1（%）
    73	01E2H	2号温湿度温度值	实际值=上传值*0.1（℃）
    74	01E3H	2号温湿度湿度值	实际值=上传值*0.1（%）
    75	01E4H	3号温湿度温度值	实际值=上传值*0.1（℃）
    76	01E5H	3号温湿度湿度值	实际值=上传值*0.1（%）
    77	01E6H	4号温湿度温度值	实际值=上传值*0.1（℃）
    78	01E7H	4号温湿度湿度值	实际值=上传值*0.1（%）
    79	01E8H	5号温湿度温度值	实际值=上传值*0.1（℃）
    80	01E9H	5号温湿度湿度值	实际值=上传值*0.1（%）
    91	01EAH	6号温湿度温度值	实际值=上传值*0.1（℃）
    92	01EBH	6号温湿度湿度值	实际值=上传值*0.1（%）
    93	01ECH	7号温湿度温度值	实际值=上传值*0.1（℃）
    94	01EDH	7号温湿度湿度值	实际值=上传值*0.1（%）
    95	01EEH	8号温湿度温度值	实际值=上传值*0.1（℃）
    96	01EFH	8号温湿度湿度值	实际值=上传值*0.1（%）
    97	01F0H	9号温湿度温度值	实际值=上传值*0.1（℃）
    98	01F1H	9号温湿度湿度值	实际值=上传值*0.1（%）
    99	01F2H	10号温湿度温度值	实际值=上传值*0.1（℃）
    100	01F3H	10号温湿度湿度值	实际值=上传值*0.1（%）
    101	01F4H	1号烟雾值	0-正常；非0-报警；
    102	01F5H	2号烟雾值	0-正常；非0-报警；
    103	01F6H	3号烟雾值	0-正常；非0-报警；
    104	01F7H	4号烟雾值	0-正常；非0-报警；
    105	01F8H	5号烟雾值	0-正常；非0-报警；
    106	01F9H	6号烟雾值	0-正常；非0-报警；
    107	01FAH	7号烟雾值	0-正常；非0-报警；
    108	01FBH	8号烟雾值	0-正常；非0-报警；
    109	01FCH	9号烟雾值	0-正常；非0-报警；
    110	01FDH	10号烟雾值	0-正常；非0-报警；
    111	01FEH	1号液位值	实际值=上传值*1（mm）
    112	01FFH	2号液位值	实际值=上传值*1（mm）
    113	0200H	3号液位值	实际值=上传值*1（mm）
    114	0201H	4号液位值	实际值=上传值*1（mm）
    115	0202H	5号液位值	实际值=上传值*1（mm）
    116	0203H	6号液位值	实际值=上传值*1（mm）
    117	0204H	7号液位值	实际值=上传值*1（mm）
    118	0205H	8号液位值	实际值=上传值*1（mm）
    119	0206H	9号液位值	实际值=上传值*1（mm）
    120	0207H	10号液位值	实际值=上传值*1（mm）

## 2.3 模块要求
 
   (1作为modbus主站召唤从站传感器数据。

   (2)将召唤上来的传感器数据上送到数据中心

   
# 3 功能规划

## 3.1 串口驱动

    serial_drive.c 

## 3.2 modbus主站

    modbus_master.c

## 3.3 modbus配置文件

    modbusConfig.json

## 3.4 内存分配

    lib_memmoty.c

## 3.5 单向链表管理

    linked_list.c
    
## 3.6 mqtt协议

    mosquitto_client.c

## 3.7 mqtt配置文件

    mqttConfig.json
# 4 配置文件说明(#注释说明)
## 4.1 modbus配置文件说明
    {
        "Tag": "modbus参数配置", 
        "Com":{ #串口配置
            "DevName": "/dev/ttySZ3",  #串口设备号
            "Port": "RS485_1",    #智芯融合终端串口通信协议(RS485_1/RS485_2/RS485_3/RS485_4,RS232_1/RS232_2)
            "BaudRate": 9600, #波特率 (支持1200/4800/9600/115200/230400/460800)
            "DataBit":8,    #数据位(支持7/8)
            "StopBit":1,    #停止位(支持1//2)
            "Parity":"Odd"  #校验位(支持Odd/Event/None)   
        },
        "ModbusPara":{ #modbus配置
            "BroadcastAddr":0, #广播地址(十进制)
            "SlaveAddr": 128, #从机地址(十进制)
            "Variable_Read":[ #读寄存器(支持功能码01/03/04)
                {
                    "Name":"Tmp",  #变量名(选填)
                    "Unit":"°C",  #单位(选填)
                    "Type":"ushort", #保存的数据格式
                    "StartAddr":"0000", #起始地址
                    "EndAddr":"003B", #结束地址(选填)
                    "Number":60, #数量
                    "Func":4 #选择的功能码
                },
                {
                    "Name":"TmpEnvHum",
                    "Unit":"°C/%RH",
                    "Type":"ushort",
                    "StartAddr":"01E0",
                    "EndAddr":"01F3",
                    "Number":20,
                    "Func":4
                },
                {
                    "Name":"Smoke",
                    "Unit":"ON/OFF",
                    "Type":"ushort",
                    "StartAddr":"01F4",
                    "EndAddr":"01FD",
                    "Number":10,
                    "Func":4
                },
                {
                    "Name":"Lev",
                    "Unit":"mm",
                    "Type":"ushort",
                    "StartAddr":"01FE",
                    "EndAddr":"0207",
                    "Number":10,
                    "Func":4
                }
            ],
            "Variable_Write":[ #写寄存器(支持功能码06/10)

            ]
        }
    }
# 版本管理
    1.版本:v1.00 时间:2020/03/05 描述:实现modbus主站召唤从站的传感器数据,并将数据上送到数据中心
    
    