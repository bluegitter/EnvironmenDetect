/*
 * @Author: your name
 * @Date: 2020-02-27 19:14:18
 * @LastEditTime : 2020-03-09 22:55:01
 * @LastEditors  : Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /Env/mosquitto_client.h
 */
#ifndef _MOSQUITTO_CLIENT__H
#define _MOSQUITTO_CLIENT__H

#include "gtypedef.h"
#include "include/mosquitto.h"
#include "modbus_master.h"

#define MQTT_CONFIG_FILE_NAME "mqttConfig.json"//配置文件名
#define BODY_NUMBER_MAX 30//body数组成员个数最大为30
#define DEVICE_NUMBER_MAX 30//最大30台设备

#ifdef __cplusplus
extern "C"
{
#endif

//模型body结构体
typedef struct {
    char    name[20];
	char	type[8];
	char	unit[12];
	char	deadzone[8];
	char	ratio[8];
	char	isReport[8];
	char	userdefine[8];
} Model_body;
//模型结构体
typedef struct{
    char token[12];
    char name[32];//模型名
    ushort body_size;//模型body数组成员个数
    Model_body body[BODY_NUMBER_MAX];//模型body数组成员总数
}Model;

//注册设备/获取数据body结构体
typedef struct	    
{
	char	port[16];
	char	addr[16];
	char	model_name[32];
	char	desc[16];
	
}Register_body;
//register结构体
typedef struct 
{
    char dev_guid[DEVICE_NUMBER_MAX][64];
    char token[12];
    ushort reg_ok_dev_num;//注册成功设备数
    Register_body body[BODY_NUMBER_MAX];
    ushort body_size;//register body数组成员个数,即注册的设备数量
}Register;
//parameter_body结构体
typedef struct 
{
    char name[32];
    char val[16];
    char type[8];
    
}Parameter_body;
//parameter结构体
typedef struct 
{
    char token[12];
    ushort body_size;
    Parameter_body body[BODY_NUMBER_MAX];
}Parameter;


//大结构体
typedef struct 
{   
    Model model;//模型
    Register reg;//注册
    Parameter par;//定值
}Config_struc;
extern Config_struc configHandle;//配置参数结构体
//注册模型
#define Topic_Set_Model "Env/set/request/database/model"
#define Topic_Set_Model_Return "database/set/response/Env/model"  

//删除模型
#define Topic_Delete_Model "Env/action/request/database/deletemodel"
#define Topic_Delete_Model_Return "database/action/response/Env/deletemodel"

//查询注册的模型名称
#define Topic_Get_Model "wuyise/get/request/database/modelschema"
#define Topic_Get_Model_Return "database/get/response/wuyise/modelschema"

//注册设备
#define Topic_Dev_Register "Env/set/request/database/register"
#define Topic_Dev_Register_Return "+/set/response/+/register"

//查询设备
#define Topic_Dev_Get "Env/get/request/database/register"
#define Topic_Dev_Get_Return "database/get/response/Env/register"
//删除设备
#define Topic_Dev_Delete "Env/action/request/database/unregister"
#define Topic_Dev_Delete_Return "database/action/response/Env/unregister"

//读取设备GUID
#define Topic_DEVICE_GUID "Env/get/request/database/guid"
#define Topic_DEVICE_GUID_Return "+/get/response/+/guid"

//数据主动上传
#define Topic_DATA_SEND "Env/notify/event/database"
#define Topic_DATA_SEND_Return "database/notify/response/Env/+/+"

//根据设备读取实时数据
#define Topic_DATA_READ "Env/get/request/database/realtime"
#define Topic_DATA_READ_Return "database/get/response/Env/realtime"

void *mosquitto_client_thread(void *args);
void my_message_callback_recvData(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
int Database_returnResult(struct mosquitto *mosq, const struct mosquitto_message *message);
int Mqtt_config_parse(Config_struc *config,const char *fileName);
int model_json_parse(cJSON *pjson,Config_struc *config);
int register_json_parse(cJSON *pjson,Config_struc *config);
int parameter_json_parse(cJSON *pjson,Config_struc *config);
void Config_model(Config_struc *config);
void Model_delete(Config_struc *config);
void Model_get(Config_struc *config);
void Register_dev(Config_struc *config);
void delete_dev(Config_struc *config);
void Get_dev_guid(Config_struc *config);
int Handle_Topic_GetGUID(struct mosquitto *mosq, const struct mosquitto_message *message,Config_struc *config);
void Realtime_data_report(Config_struc *config,Sensor_data gdata,uchar dev_index);
void My_mosq_publish(const char *topic,const char *buf,ushort len);

#ifdef __cplusplus
}
#endif

#endif