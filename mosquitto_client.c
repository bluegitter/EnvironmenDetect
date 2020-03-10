/*
 * @Author: your name
 * @Date: 2020-02-20 18:01:53
 * @LastEditTime : 2020-03-09 22:53:57
 * @LastEditors  : Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /EnvironmentDetec/mosquittoClient.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <semaphore.h>

#include "include/cJSON.h"
#include "mosquitto_client.h"
#include "serial_drive.h"
#include "syslog.h"
#include "include/zlog.h"

//#define HOST "192.168.1.101"
//#define HOST "localhost"
#define PORT  1883
#define KEEP_ALIVE 6

static char broker_ip[32];
struct mosquitto *gMosq = NULL;//mosquitto句柄
uchar  gMqttStateMachine;//流程状态机
Config_struc configHandle;//app配置参数结构体
extern sem_t g_sem;//信号量,用于同步线程
/**
 * @description: 日志回调函数
 * @param {type} 
 * @return: 
 */
void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    /* Pring all log messages regardless of level. */
    printf("%s\n", str);
}
/**
 * @description: 连接状态回调函数
 * @param {type} 
 * @return: 
 */
void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    if(!result){
        /* Subscribe to broker information topics on successful connect. */
        mosquitto_subscribe(mosq, NULL, Topic_Set_Model_Return, 2);
		mosquitto_subscribe(mosq, NULL, Topic_Delete_Model_Return, 2);
		mosquitto_subscribe(mosq, NULL, Topic_Get_Model_Return, 2);
		mosquitto_subscribe(mosq, NULL, Topic_Dev_Register_Return, 2);
		mosquitto_subscribe(mosq, NULL, Topic_Dev_Delete_Return, 2);
        mosquitto_subscribe(mosq, NULL, Topic_Dev_Get_Return, 2);
        mosquitto_subscribe(mosq, NULL, Topic_DEVICE_GUID_Return, 2);
        mosquitto_subscribe(mosq, NULL, Topic_DATA_SEND_Return, 2);
    }
    else{
        fprintf(stderr, "Connect failed\n");
        zlog_error(zc,"Connect failed");
    }
}
/**
 * @description: 连接断开回调函数
 * @param {type} 
 * @return: 
 */
void my_disconnect_callback(struct mosquitto *mosq, void *obj, int result)
{
   logMsg(logErr,"mosquitto client disconnect!!!!!!!!");
    zlog_error(zc,"mosquitto client disconnect!!!!!!!!");
    mosquitto_connect(gMosq, broker_ip, PORT, KEEP_ALIVE);//断开重连
    
}
/**
 * @description: 订阅回调
 * @param {type} 
 * @return: 
 */
void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    int i;
    printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for(i=1; i<qos_count; i++){
        printf(", %d", granted_qos[i]);
    }
    printf("\n");
}
// /**
//  * @description: 发布回调函数
//  * @param {type} 
//  * @return: 
//  */
// void my_publish_callback(struct mosquitto *mosq, void *obj, int mid)
// {

// }
/**
 * @description: mqtt接收消息回调函数
 * @param {type} 
 * @return: 
 */
void my_message_callback_recvData(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    
	printf("Get MQTT messageLen : %d\r\n",message->payloadlen);
	printf("Top : %s\r\n",message->topic);

     if(message->payloadlen)
	{
        if(strstr((char*)message->topic,"modelschema")!=NULL)	//查询注册的模型名称				
		{
            logMsg(logInfo,"payload message:%s \n",(char*)message->payload);
			return;
			
		}
        //判断mqtt主题
		else if(strstr((char*)message->topic,"unregister")!= NULL)		//取消设备注册			
		{
			
			if(Database_returnResult(mosq,message)==0)
			{
		      	logMsg(logInfo,"+++++Device unregister success+++++! \n");
				gMqttStateMachine |=0x01;
			    return;
			}
			
		}
        else if(strstr((char*)message->topic,"model")!=NULL)	//模型注册					
		{
			
			if(Database_returnResult(mosq,message)==0)
			{
			    logMsg(logInfo,"model register success!!!!!!! \n");
			    gMqttStateMachine |=0x02;
			    return;
			}
			
		}
        else if(strstr((char*)message->topic,"register")!=NULL)	// 设备注册					
		{
			
			if(Database_returnResult(mosq,message)==0)
			{
			    logMsg(logInfo,"Device register success!!!!!!!!\n");
				gMqttStateMachine |=0x04;
			    return;
			}
			
		}
        else if(strstr((char*)message->topic,"guid")!=NULL)	//获取设备 guid				
		{
			
			if(Handle_Topic_GetGUID(mosq,message,&configHandle) == 0)
			{
				gMqttStateMachine |=0x08;
			    return;
			}
			
		}
		else if(strstr((char*)message->topic,"notify")!=NULL)	// 	
		{
			
			if(Database_returnResult(mosq,message)==0)
			{
				logMsg(logInfo,"Data  send success!!!!!!!! \n");
			    return;
			}
		}
		else
		{
		    logMsg(logErr,"unknow  topic! \n");
            zlog_warn(zc,"unknow  topic!");
             
		}
    }
    
}
/**
 * @description: 读取设备GUID
 * @param {type} 
 * @return: 
 */
int Handle_Topic_GetGUID(struct mosquitto *mosq, const struct mosquitto_message *message,Config_struc *config)
{
	int i;
	cJSON* root;
	cJSON* element;
	cJSON* body_index;
	cJSON* element1;
	
	//主题
    logMsg(logInfo,"get guid topic:%s \n",(char *)message->topic);
    logMsg(logInfo,"get guid total message:%s \n",(char *)message->payload);
    
    root = cJSON_Parse(message->payload);
    for(element = (root != NULL) ? root->child:NULL; element != NULL; element = element->next)
    {
        if(element->type == cJSON_String)
        {
            if(strcmp(element->string,"token")==0)
            {
                continue;
            }

            if(strcmp(element->string,"timestamp")==0)
            {
                continue;
            }
        }

        if(element->type == cJSON_Array)
        {
            config->reg.reg_ok_dev_num = cJSON_GetArraySize(element);
            logMsg(logInfo,"注册设备成功数量 = %d",config->reg.reg_ok_dev_num);
 
            for(i = 0; i < config->reg.reg_ok_dev_num; i++)
            {
                body_index = cJSON_GetArrayItem(element, i);
                for(element1 = (body_index != NULL) ? body_index->child:NULL; element1 != NULL; element1 = element1->next)
                {
                    if(strcmp(element1->string,"dev")==0)
                    {
                        if(strlen(element1->valuestring))
                        {
                            strcpy(config->reg.dev_guid[i],element1->valuestring);
                            logMsg(logInfo,"guid_%02d:%s",i,config->reg.dev_guid[i]);
                        }
                        else
                        {
                            memset(config->reg.dev_guid[i],'\0',sizeof(config->reg.dev_guid[i]));
                        }
                    }
                    else if(strcmp(element1->string,"guid")==0)
                    {
                        if(strlen(element1->valuestring))
                        {
                            
                        }
                    }
                }
            }
        }
    }
    cJSON_Delete(root);
    return 0;
}
/**
 * @description: 数据中心返回结果
 * @param {type} 
 * @return: 
 */ 
int Database_returnResult(struct mosquitto *mosq, const struct mosquitto_message *message)
{
	cJSON* root;
	cJSON* element;

    logMsg(logInfo,"payload message:%s",(char*)message->payload);
    root = cJSON_Parse((const char*)message->payload);
    for(element = (root != NULL) ? root->child:NULL; element != NULL; element = element->next)
    {
        if(element->type == cJSON_String)
        {
            if(strcmp(element->string,"token")==0)
            {
                continue;
            }

            if(strcmp(element->string,"timestamp")==0)
            {
                continue;
            }
        
            if(strcmp(element->string,"status")==0)
            {
                //操作成功
                if(strcmp(element->valuestring,"OK")==0)
                {

                    return 0;
                }
                else
                {
                    //操作失败
                    logMsg(logErr,"oprate error! \n");
                    zlog_error(zc,"oprate error!");
                    return -1;
                }
            }
        }
    }
    
    cJSON_Delete(root);
    return -1;
}
/**
 * @description: 解析mqttConfig.json文件
 * @param {type} 
 * @return: 
 */
int Mqtt_config_parse(Config_struc *config,const char *fileName)
{
    char tmp[128];
    char *file_data = NULL;
    int state;
    cJSON *pjson = NULL;
    cJSON *item = NULL;
    
    printf("\n########mqtt配置文件解析#########\n");
    if (NULL == (file_data = openJsonFile((char *)fileName))){ // 打开配置脚本文件,fd指向文件内容}
        sprintf(tmp,"/mnt/nand/env/%s",fileName);
        if (NULL == (file_data = openJsonFile(tmp))){
            zlog_error(zc, "%s文件打开错误!", tmp);
            logMsg(logErr, "%s文件打开错误!", tmp);
            return false;
        }
    }
    // 解析数据
    pjson = cJSON_Parse(file_data); //解析配置脚本文件成json格式
     // 进入模型对象
    item = cJSON_GetObjectItem(pjson, "broker_ip");
    if (NULL == item){
        zlog_error(zc,"提取broker_ip错误!");
        logMsg(logErr,"提取broker_ip错误!");
        return false;
    }
    strncpy(broker_ip, item->valuestring, 32);
    logMsg(logInfo, "broker_ip: %s", broker_ip);
    //模型解析
    state = model_json_parse(pjson,config);
    if(state == false){
        zlog_error(zc,"解析模型错误");
        logMsg(logErr,"解析模型错误");
        return false;
    }
    //register解析
    state = register_json_parse(pjson,config);
    if(state == false){
        zlog_error(zc,"解析register错误");
        logMsg(logErr,"解析register错误");
        return false;
    }
    //parameter解析
    state = parameter_json_parse(pjson,config);
    if(state == false){
        zlog_error(zc,"解析parameter错误");
        logMsg(logErr,"解析parameter错误");
        return false;
    }

    cJSON_Delete(pjson); //删除json缓冲
    free(file_data);     //释放内存空间 在openJsonFile中申请
    return true;
}
/**
 * @description: 模型解析
 * @param {type} 
 * @return: 
 */
int model_json_parse(cJSON *pjson,Config_struc *config)
{
    ushort i;
    cJSON *item = NULL;
    cJSON *child_item=NULL;
    cJSON *arr_item=NULL;
    cJSON *arr_object=NULL;
    
    printf("\n++++++++++模型解析+++++++++++\n");
    // 进入模型对象
    item = cJSON_GetObjectItem(pjson, "model");

    child_item = cJSON_GetObjectItem(item, "token");
    if (NULL == child_item){
        zlog_error(zc,"提取model_token错误!");
        logMsg(logErr,"提取model_token错误!");
        return false;
    }
    strncpy(config->model.token, child_item->valuestring, 12);
    logMsg(logInfo, "model_token: %s", config->model.token);
        
    child_item = cJSON_GetObjectItem(item, "model_name");
    if (NULL == child_item){
        zlog_error(zc,"提取model_name错误!");
        logMsg(logErr,"提取model_name错误!");
        return false;
    }
    strncpy(config->model.name, child_item->valuestring, 32);
    logMsg(logInfo, "model_name: %s", config->model.name);

    //解析body
    child_item = cJSON_GetObjectItem(item, "body");
    if(cJSON_IsArray(child_item) == 0){ 
        zlog_error(zc,"body is not array!");
        logMsg(logErr,"body is not array!"); 
        return false;
    }

    config->model.body_size = cJSON_GetArraySize(child_item);
    if(config->model.body_size<1){//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!警告,如果数组为空,则结束程序
        zlog_error(zc,"model_body没有元素!");
        logMsg(logErr,"model_body没有元素!");
        return false;
    }
    for(i=0;i < config->model.body_size;i++){
        printf("=============================\n");
        arr_item = cJSON_GetArrayItem(child_item,i);
        //提取变量名
        arr_object = cJSON_GetObjectItem(arr_item, "name");
        if (NULL != arr_object){
        
            strncpy(config->model.body[i].name, arr_object->valuestring, 20);
            logMsg(logInfo, "变量名：%s",config->model.body[i].name);
        }
        //提取数据类型
        arr_object = cJSON_GetObjectItem(arr_item, "type");
        if (NULL != arr_object){
            strncpy(config->model.body[i].type, arr_object->valuestring, 8);
            logMsg(logInfo, "数据类型：%s",config->model.body[i].type);
        }
        //提取单位
        arr_object = cJSON_GetObjectItem(arr_item, "unit");
        if (NULL != arr_object){
            strncpy(config->model.body[i].unit, arr_object->valuestring, 8);
            logMsg(logInfo, "单位：%s",config->model.body[i].unit);
        }
        //deadzone
        arr_object = cJSON_GetObjectItem(arr_item, "deadzone");
        if (NULL != arr_object){
            strncpy(config->model.body[i].deadzone, arr_object->valuestring, 8);
            logMsg(logInfo, "deadzone:%s",config->model.body[i].deadzone);
        }
        //ratio
        arr_object = cJSON_GetObjectItem(arr_item, "ratio");
        if (NULL != arr_object){
            strncpy(config->model.body[i].ratio, arr_object->valuestring, 8);
            logMsg(logInfo, "ratio：%s",config->model.body[i].ratio);
        }
        //isReport
        arr_object = cJSON_GetObjectItem(arr_item, "isReport");
        if (NULL != arr_object){
            strncpy(config->model.body[i].isReport, arr_object->valuestring, 8);
            logMsg(logInfo, "isReport：%s",config->model.body[i].isReport);
        }
        //userdefine
        arr_object = cJSON_GetObjectItem(arr_item, "userdefine");
        if (NULL != arr_object){
            strncpy(config->model.body[i].userdefine, arr_object->valuestring, 8);
            logMsg(logInfo, "userdefine：%s",config->model.body[i].userdefine);
        }
    }
    return true;
}
/**
 * @description: register解析
 * @param {type} 
 * @return: 
 */
int  register_json_parse(cJSON *pjson,Config_struc *config)
{
    ushort i;
    cJSON *item = NULL;
    cJSON *child_item=NULL;
    cJSON *arr_item=NULL;
    cJSON *arr_object=NULL;
    printf("\n+++++++++register解析++++++++++\n");

    // 进入模型对象
    item = cJSON_GetObjectItem(pjson, "register");
    
    child_item = cJSON_GetObjectItem(item, "token");
    if (NULL == child_item){
        zlog_error(zc,"提取register_token错误!");
        logMsg(logErr,"提取register_token错误!");
        return false;
    }
    strncpy(config->reg.token, child_item->valuestring, 12);
    logMsg(logInfo, "register_token: %s", config->reg.token);

    //解析数组
    child_item = cJSON_GetObjectItem(item, "body");
    if(cJSON_IsArray(child_item) == 0){ 
        zlog_error(zc,"body is not array!");
        logMsg(logErr,"body is not array!"); 
        return false;
    }

    config->reg.body_size = cJSON_GetArraySize(child_item);
    if(config->reg.body_size<1){//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!警告,如果数组为空,则结束程序
        zlog_error(zc,"配置文件中register_body数组内没有元素!");
        logMsg(logErr,"配置文件中register_body数组内没有元素!");
        return false;
    }
    for(i=0;i < config->reg.body_size;i++){
         printf("=============================\n");
        arr_item = cJSON_GetArrayItem(child_item,i);
        //提取模型名
        arr_object = cJSON_GetObjectItem(arr_item, "model");
        if (NULL != arr_object){
           
            strncpy(config->reg.body[i].model_name, arr_object->valuestring, 32);
            logMsg(logInfo, "model name:%s",config->reg.body[i].model_name);
        }
        //提取端口
        arr_object = cJSON_GetObjectItem(arr_item, "port");
        if (NULL != arr_object){
            strncpy(config->reg.body[i].port, arr_object->valuestring, 16);
            logMsg(logInfo, "port：%s",config->reg.body[i].port);
        }
        //提取addr
        arr_object = cJSON_GetObjectItem(arr_item, "addr");
        if (NULL != arr_object){
            strncpy(config->reg.body[i].addr, arr_object->valuestring, 16);
            logMsg(logInfo, "addr：%s",config->reg.body[i].addr);
        }
        //desc
        arr_object = cJSON_GetObjectItem(arr_item, "desc");
        if (NULL != arr_object){
            strncpy(config->reg.body[i].desc, arr_object->valuestring, 16);
            logMsg(logInfo, "desc:%s",config->reg.body[i].desc);
        }
    }
    return true;
}
/**
 * @description: parameter解析
 * @param {type} 
 * @return: 
 */
int  parameter_json_parse(cJSON *pjson,Config_struc *config)
{
    ushort i;
    cJSON *item = NULL;
    cJSON *child_item=NULL;
    cJSON *arr_item=NULL;
    cJSON *arr_object=NULL;
    
    printf("\n++++++++++parameter解析+++++++++++\n");
    // 进入模型对象
    item = cJSON_GetObjectItem(pjson, "parameter");
    
    child_item = cJSON_GetObjectItem(item, "token");
    if (NULL == child_item){
        zlog_error(zc,"提取parameter_token错误!");
        logMsg(logErr,"提取parameter_token错误!");
        return false;
    }
    strncpy(config->par.token, child_item->valuestring, 12);
    logMsg(logInfo, "parameter_token: %s", config->par.token);

    //解析数组
    child_item = cJSON_GetObjectItem(item, "body");
    if(cJSON_IsArray(child_item) == 0){ 
        zlog_error(zc,"body is not array!");
        logMsg(logErr,"body is not array!"); 
        return false;
    }

    config->par.body_size = cJSON_GetArraySize(child_item);
    if(config->par.body_size<1){//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!警告,如果数组为空,则结束程序
         zlog_error(zc,"配置文件中parameter_token数组内没有元素!");
        logMsg(logErr,"配置文件中parameter_token数组内没有元素!");
        return false;
    }
    for(i=0;i < config->par.body_size;i++){
         printf("=============================\n");
        arr_item = cJSON_GetArrayItem(child_item,i);
        //提取变量名
        arr_object = cJSON_GetObjectItem(arr_item, "name");
        if (NULL != arr_object){
           
            strncpy(config->par.body[i].name, arr_object->valuestring, 32);
            logMsg(logInfo, "变量名：%s",config->par.body[i].name);
        }
        //提取定值
        arr_object = cJSON_GetObjectItem(arr_item, "val");
        if (NULL != arr_object){
            strncpy(config->par.body[i].val, arr_object->valuestring, 8);
            logMsg(logInfo, "val：%s",config->par.body[i].val);
        }
        //提取定值类型
        arr_object = cJSON_GetObjectItem(arr_item, "dataType");
        if (NULL != arr_object){
            strncpy(config->par.body[i].type, arr_object->valuestring, 8);
            logMsg(logInfo, "dataType：%s",config->par.body[i].type);
        }
    }
    return true;
}
/**
 * @description: 获取json格式timestamp
 * @param {type} 
 * @return: 
 */
void Get_current_timestamp(char* timestamp)
{
	char tmp[32], ms[8];
	struct timeval tv;
	struct tm *now;
	
    //获取当前时间
	gettimeofday(&tv, NULL);
	//转化为当地时间
	now = localtime(&tv.tv_sec);
	//将时间格式化
	strftime(tmp, sizeof(tmp), "%Y-%m-%dT%H:%M:%S",now);
	snprintf(ms, 32, ".%03ld", tv.tv_usec / 1000);
	strncat(tmp, ms, strlen(ms));
	strncat(tmp, "+0800", strlen("+0800"));

	//printf("time : %s\r\n",tmp); 

	if(timestamp){
		strcpy(timestamp,tmp);
	}
}
/**
 * @description: 模型注册
 * @param {type} 
 * @return: 
 */
void Config_model(Config_struc *config)
{
    ushort i;
	cJSON* root;
	cJSON* body;
	char* pMsg = NULL;
	char timestamp[32];
	cJSON* body_root;

    //创建json对象
	root = cJSON_CreateObject();
    //填充token
	cJSON_AddItemToObject(root,"token",cJSON_CreateString(config->model.token));
	memset(timestamp,'\0',sizeof(timestamp));
	Get_current_timestamp(timestamp);
    //填充时间戳
	cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(timestamp));
    //填充模型名称
	cJSON_AddItemToObject(root, "model", cJSON_CreateString(config->model.name));
    //添加数组到json对象
	body = cJSON_AddArrayToObject(root, "body");
    for(i=0;i<config->model.body_size;i++){
        //创建数组中子对象
        body_root = cJSON_CreateObject();
        cJSON_AddItemToObject(body_root, "name", cJSON_CreateString(config->model.body[i].name));
        //类型
        cJSON_AddItemToObject(body_root, "type", cJSON_CreateString(config->model.body[i].type));
        //单位
        cJSON_AddItemToObject(body_root, "unit", cJSON_CreateString(config->model.body[i].unit));
        cJSON_AddItemToObject(body_root, "deadzone", cJSON_CreateString(config->model.body[i].deadzone));
        cJSON_AddItemToObject(body_root, "ratio", cJSON_CreateString(config->model.body[i].ratio));
        cJSON_AddItemToObject(body_root, "isReport", cJSON_CreateString(config->model.body[i].isReport));
        cJSON_AddItemToObject(body_root, "userdefine", cJSON_CreateString(config->model.body[i].userdefine));
        //将子对象添加到数组中
        cJSON_AddItemToArray(body, body_root);
    }
	pMsg = cJSON_Print(root);
    printf("publish topic:%s\n",Topic_Set_Model);
	logMsg(logInfo,"config model Json=%s! \n",pMsg);
    //发布消息
	My_mosq_publish(Topic_Set_Model,pMsg,strlen(pMsg));

	free(pMsg);
	cJSON_Delete(root);
}
/**
 * @description: 模型删除
 * @param {type} 
 * @return: 
 */
void Model_delete(Config_struc *config)
{
	cJSON* root;
	cJSON* body;
	char* pMsg = NULL;
	char timestamp[32];

    //创建json对象
	root = cJSON_CreateObject();
    //填充token
	cJSON_AddItemToObject(root,"token",cJSON_CreateString(config->model.token));
	memset(timestamp,'\0',sizeof(timestamp));
	Get_current_timestamp(timestamp);
    //填充时间戳
	cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(timestamp));

    //添加数组到json对象
	body = cJSON_AddArrayToObject(root, "body");
    cJSON_AddItemToArray(body,cJSON_CreateString(config->model.name));
    pMsg = cJSON_Print(root);
    printf("publish topic:%s\n",Topic_Dev_Delete);
	logMsg(logInfo,"delete model Json=%s! \n",pMsg);
    //发布消息
	My_mosq_publish(Topic_Delete_Model,pMsg,strlen(pMsg));

	free(pMsg);
	cJSON_Delete(root);
}
/**
 * @description: 模型查询
 * @param {type} 
 * @return: 
 */
void Model_get(Config_struc *config)
{
	cJSON* root;
	//cJSON* body;
	char* pMsg = NULL;
	char timestamp[32];

    //创建json对象
	root = cJSON_CreateObject();
    //填充token
	cJSON_AddItemToObject(root,"token",cJSON_CreateString(config->model.token));
	memset(timestamp,'\0',sizeof(timestamp));
	Get_current_timestamp(timestamp);
    //填充时间戳
	cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(timestamp));

    //添加数组到json对象
	 cJSON_AddArrayToObject(root, "body");
    //cJSON_AddItemToArray(body,cJSON_CreateString(config->tol.model_name));
    pMsg = cJSON_Print(root);
    printf("publish topic:%s\n",Topic_Get_Model);
	logMsg(logInfo,"delete model Json=%s \n",pMsg);
    //发布消息
	My_mosq_publish(Topic_Get_Model,pMsg,strlen(pMsg));

	free(pMsg);
	cJSON_Delete(root);
}
/**
 * @description: app/dev注册
 * @param {type} 
 * @return: 
 */
void Register_dev(Config_struc *config)
{
    ushort i;
	cJSON* root;
	cJSON* body;
	char* pMsg = NULL;
	char timestamp[32];
	cJSON* body_root;

    //创建json对象
	root = cJSON_CreateObject();
    //填充token
	cJSON_AddItemToObject(root,"token",cJSON_CreateString(config->reg.token));
	memset(timestamp,'\0',sizeof(timestamp));
	Get_current_timestamp(timestamp);
    //填充时间戳
	cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(timestamp));
 
    //添加数组到json对象
	body = cJSON_AddArrayToObject(root, "body");
    for(i=0;i<config->reg.body_size;i++){
        //创建数组中子对象
        body_root = cJSON_CreateObject();
        cJSON_AddItemToObject(body_root, "model", cJSON_CreateString(config->reg.body[i].model_name));
        //port
        cJSON_AddItemToObject(body_root, "port", cJSON_CreateString(config->reg.body[i].port));
        //地址
        cJSON_AddItemToObject(body_root, "addr", cJSON_CreateString(config->reg.body[i].addr));
        //desc
        cJSON_AddItemToObject(body_root, "desc", cJSON_CreateString(config->reg.body[i].desc));
        //将子对象添加到数组中
        cJSON_AddItemToArray(body, body_root);
    }
	pMsg = cJSON_Print(root);
    printf("publish topic:%s\n",Topic_Dev_Register);
	logMsg(logInfo,"register model Json=%s \n",pMsg);
    //发布消息
	My_mosq_publish(Topic_Dev_Register,pMsg,strlen(pMsg));

	free(pMsg);
	cJSON_Delete(root);
}
/**
 * @description: 删除 dev/app注册
 * @param {type} 
 * @return: 
 */
void delete_dev(Config_struc *config)
{
    ushort i;
	cJSON* root;
	cJSON* body;
	char* pMsg = NULL;
	char timestamp[32];
	cJSON* body_root;

    //创建json对象
	root = cJSON_CreateObject();
    //填充token
	cJSON_AddItemToObject(root,"token",cJSON_CreateString(config->reg.token));
	memset(timestamp,'\0',sizeof(timestamp));
	Get_current_timestamp(timestamp);
    //填充时间戳
	cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(timestamp));
 
    //添加数组到json对象
	body = cJSON_AddArrayToObject(root, "body");
    for(i=0;i<config->reg.body_size;i++){
        //创建数组中子对象
        body_root = cJSON_CreateObject();
        cJSON_AddItemToObject(body_root, "model", cJSON_CreateString(config->reg.body[i].model_name));
        //port
        cJSON_AddItemToObject(body_root, "port", cJSON_CreateString(config->reg.body[i].port));
        //地址
        cJSON_AddItemToObject(body_root, "addr", cJSON_CreateString(config->reg.body[i].addr));
        //desc
        cJSON_AddItemToObject(body_root, "desc", cJSON_CreateString(config->reg.body[i].desc));
        //将子对象添加到数组中
        cJSON_AddItemToArray(body, body_root);
    }
	pMsg = cJSON_Print(root);
    printf("publish topic:%s\n",Topic_Dev_Delete);
	logMsg(logInfo,"register model Json=%s! \n",pMsg);
    //发布消息
	My_mosq_publish(Topic_Dev_Delete,pMsg,strlen(pMsg));

	free(pMsg);
	cJSON_Delete(root);
}
/**
 * @description: 获取注册关系
 * @param {type} 
 * @return: 
 */
void Get_register_relation(Config_struc *config)
{
	cJSON* root;
	char* pMsg = NULL;
	char timestamp[32];

    //创建json对象
	root = cJSON_CreateObject();
    //填充token
	cJSON_AddItemToObject(root,"token",cJSON_CreateString(config->reg.token));
	memset(timestamp,'\0',sizeof(timestamp));
	Get_current_timestamp(timestamp);
    //填充时间戳
	cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(timestamp));
 
    //添加数组到json对象
	cJSON_AddArrayToObject(root, "body");
    
	pMsg = cJSON_Print(root);
    printf("publish topic:%s\n",Topic_Dev_Delete);
	logMsg(logInfo,"register model Json=%s! \n",pMsg);
    //发布消息
	My_mosq_publish(Topic_Dev_Delete,pMsg,strlen(pMsg));

	free(pMsg);
	cJSON_Delete(root);
}
/**
 * @description: 获取guid
 * @param {type} 
 * @return: 
 */
void Get_dev_guid(Config_struc *config)
{
    ushort i;
	cJSON* root;
	cJSON* body;
	char* pMsg = NULL;
	char timestamp[32];
	cJSON* body_root;

    //创建json对象
	root = cJSON_CreateObject();
    //填充token
	cJSON_AddItemToObject(root,"token",cJSON_CreateString(config->reg.token));
	memset(timestamp,'\0',sizeof(timestamp));
	Get_current_timestamp(timestamp);
    //填充时间戳
	cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(timestamp));
 
    //添加数组到json对象
	body = cJSON_AddArrayToObject(root, "body");
    for(i=0;i<config->reg.body_size;i++){
        //创建数组中子对象
        body_root = cJSON_CreateObject();
        cJSON_AddItemToObject(body_root, "model", cJSON_CreateString(config->reg.body[i].model_name));
        //port
        cJSON_AddItemToObject(body_root, "port", cJSON_CreateString(config->reg.body[i].port));
        //地址
        cJSON_AddItemToObject(body_root, "addr", cJSON_CreateString(config->reg.body[i].addr));
        //desc
        cJSON_AddItemToObject(body_root, "desc", cJSON_CreateString(config->reg.body[i].desc));
        //将子对象添加到数组中
        cJSON_AddItemToArray(body, body_root);
    }
	pMsg = cJSON_Print(root);
    printf("publish topic:%s\n",Topic_DEVICE_GUID);
	logMsg(logInfo,"register model Json=%s! \n",pMsg);
    //发布消息
	My_mosq_publish(Topic_DEVICE_GUID,pMsg,strlen(pMsg));

	free(pMsg);
	cJSON_Delete(root);
}
/**
 * @description: 设置定值
 * @param {type} 
 * @return: 
 */
void Set_parameter(Config_struc *config)
{
    ushort i;
	cJSON* root;
	cJSON* body;
	char* pMsg = NULL;
	char timestamp[32];
	cJSON* body_root;

    //创建json对象
	root = cJSON_CreateObject();
    //填充token
	cJSON_AddItemToObject(root,"token",cJSON_CreateString(config->par.token));
	memset(timestamp,'\0',sizeof(timestamp));
	Get_current_timestamp(timestamp);
    //填充时间戳
	cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(timestamp));
 
    //添加数组到json对象
	body = cJSON_AddArrayToObject(root, "body");
    for(i=0;i<config->par.body_size;i++){
        // //创建数组中子对象
         body_root = cJSON_CreateObject();
        // cJSON_AddItemToObject(body_root, "model", cJSON_CreateString(config->par.body[i].model_name));
        // //port
        // cJSON_AddItemToObject(body_root, "port", cJSON_CreateString(config->regbd[i].port));
        // //地址
        // cJSON_AddItemToObject(body_root, "addr", cJSON_CreateString(config->regbd[i].addr));
        // //desc
        // cJSON_AddItemToObject(body_root, "desc", cJSON_CreateString(config->regbd[i].desc));
        // //将子对象添加到数组中
         cJSON_AddItemToArray(body, body_root);
    }
	pMsg = cJSON_Print(root);
    printf("publish topic:%s\n",Topic_DEVICE_GUID);
	logMsg(logInfo,"register model Json=%s! \n",pMsg);
    //发布消息
	My_mosq_publish(Topic_DEVICE_GUID,pMsg,strlen(pMsg));

	free(pMsg);
	cJSON_Delete(root);
}
/**
 * @description: 删除定值
 * @param {type} 
 * @return: 
 */
/**
 * @description: 获取定值
 * @param {type} 
 * @return: 
 */
/**
 * @description: 传感器实时数据主动上送
 * @param {dev_index:设备索引} 
 * @return: 
 */
void Realtime_data_report(Config_struc *config,Sensor_data gdata,uchar dev_index)
{
    ushort i;
	cJSON* root;
	cJSON* body;
	char* pMsg = NULL;
    char message[32];
    char topic[128];
	char timestamp[32];
	cJSON* body_root;

    memset(message,'\0',32);
    //创建json对象
	root = cJSON_CreateObject();
    //填充token
	cJSON_AddItemToObject(root,"token",cJSON_CreateString(config->model.token));
	memset(timestamp,'\0',sizeof(timestamp));
	Get_current_timestamp(timestamp);
    //填充时间戳
	cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(timestamp));
    //填充data_low
    cJSON_AddItemToObject(root,"data_row",cJSON_CreateString("single"));
    //添加数组到json对象
	body = cJSON_AddArrayToObject(root, "body");
    for(i=0;i<config->model.body_size;i++){
        //创建数组中子对象
        body_root = cJSON_CreateObject();
        cJSON_AddItemToObject(body_root, "name", cJSON_CreateString(config->model.body[i].name));
        //var
        if(i<6)
            sprintf(message,"%.1f",gdata.line_tmp[dev_index][i]);
        else if(i<7)
            sprintf(message,"%.1f",gdata.tmp[dev_index]);
        else if(i<8)
            sprintf(message,"%.1f",gdata.hum[dev_index]);
        else if(i<9)
            sprintf(message,"%d",gdata.smokeAlm[dev_index]);
        else if(i<10)
            sprintf(message,"%d",gdata.lev[dev_index]);
        else 
            logMsg(logErr,"model body size error!");

        cJSON_AddItemToObject(body_root, "val", cJSON_CreateString(message));
        //地址
        cJSON_AddItemToObject(body_root, "quality", cJSON_CreateString("0"));
        //desc
        cJSON_AddItemToObject(body_root, "timestamp", cJSON_CreateString(timestamp));
        //将子对象添加到数组中
        cJSON_AddItemToArray(body, body_root);
    }
	pMsg = cJSON_Print(root);
    memset(topic,'\0',128);
    sprintf(topic,"%s/%s/%s",Topic_DATA_SEND,config->model.name,config->reg.dev_guid[dev_index]);
       printf("publish topic:%s\n",topic);
	logMsg(logInfo,"send data Json=%s! \n",pMsg);
    //发布消息
	My_mosq_publish(topic,pMsg,strlen(pMsg));
} 
/**
 * @description: 根据设备获取实时数据
 * @param {type} 
 * @return: 
 */
void Get_realtime_Data(Config_struc *config)
{
    //ushort i;
	cJSON* root;
	cJSON* body;
	char* pMsg = NULL;
	char timestamp[32];
	cJSON* body_root;

    //创建json对象
	root = cJSON_CreateObject();
    //填充token
	cJSON_AddItemToObject(root,"token",cJSON_CreateString(config->model.token));
	memset(timestamp,'\0',sizeof(timestamp));
	Get_current_timestamp(timestamp);
    //填充时间戳
	cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(timestamp));
 
    //添加数组到json对象
	body = cJSON_AddArrayToObject(root, "body");
    //for(i=0;i<config->tol.register_body_size;i++){
        //创建数组中子对象
        body_root = cJSON_CreateObject();
        cJSON_AddItemToObject(body_root, "dev", cJSON_CreateString(config->reg.dev_guid[0]));
        //port
        cJSON_AddItemToObject(body_root, "totalcall", cJSON_CreateString("1"));
        //数组
        cJSON_AddArrayToObject(body_root, "body");
        //将子对象添加到数组中
        cJSON_AddItemToArray(body, body_root);
    //}
	pMsg = cJSON_Print(root);
    printf("publish topic:%s\n",Topic_DATA_READ);
	logMsg(logInfo,"read realtime data Json=%s \n",pMsg);
    //发布消息
	My_mosq_publish(Topic_DATA_READ,pMsg,strlen(pMsg));

	free(pMsg);
	cJSON_Delete(root);
}
/**
 * @description: 发布MQTT消息
 * @param {type} 
 * @return: 
 */
void My_mosq_publish(const char *topic,const char *buf,ushort len)
{
	int retval = -1;
	int sendcount =0 ;
	
	retval = mosquitto_publish(gMosq,NULL,topic,len,buf,2,0);
	//printf("retval :%d \r\n",retval);
	if(retval == MOSQ_ERR_NO_CONN)
	{
		while(1)
		{
			sleep(1);
			if(mosquitto_connect(gMosq, broker_ip, PORT, KEEP_ALIVE))
			{
				logMsg(logErr,"Mqtt unable connect mosquitto service!");
				//continue;
			}
            
			sendcount++;
			if(sendcount>=3)
				break;
		}
	}
}
/**
 * @description: app与数据中心交互主程序
 * @param {type} 
 * @return: 
 */
void *mosquitto_client_thread(void *args)
{
    bool session = true;
    int cnt,state;

    //mqtt配置文件初始化
    state=Mqtt_config_parse(&configHandle,MQTT_CONFIG_FILE_NAME);
    if(state == false){
        zlog_error(zc,"Mqtt_config_parse return false!");
        printf("Mqtt_config_parse return false!");
        exit(-1);
    }
    //libmosquitto 库初始化
    mosquitto_lib_init();
    //创建mosquitto客户端
    gMosq = mosquitto_new(NULL,session,NULL);
    if(!gMosq){
        zlog_error(zc,"create mosquitto client failed..");
        logMsg(logErr,"create mosquitto client failed..\n");
        mosquitto_lib_cleanup();
        return NULL;
    }

    //设置回调函数，需要时可使用
    //mosquitto_log_callback_set(gMosq, my_log_callback);
    mosquitto_connect_callback_set(gMosq, my_connect_callback);//连接回调
    mosquitto_disconnect_callback_set(gMosq, my_disconnect_callback);//断开回调
    mosquitto_message_callback_set(gMosq, my_message_callback_recvData);//订阅到消息回调
    //mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
    sleep(2);	
    //连接服务器
    while(1)
    {
	    if(mosquitto_connect(gMosq, broker_ip, PORT, KEEP_ALIVE)){
            zlog_error(zc,"failed! Unable to connect service.");
	        logMsg(logErr, "failed! Unable to connect service.\n");
	        continue;
            sleep(1);
	    }
        printf("\nmosquitto client connect susscced!\n");
		break;
    }
    //开启一个线程，在线程里不停的调用 mosquitto_loop() 来处理网络信息
    int loop = mosquitto_loop_start(gMosq); 
    if(loop != MOSQ_ERR_SUCCESS)
    {
        zlog_error(zc,"mosquitto loop start error!");
        logMsg(logErr,"mosquitto loop start error!\n");
        return NULL;
    }
    cnt=0;
    gMqttStateMachine = 0; //将mqtt状态机置0
	while(!(gMqttStateMachine&0x02))
	{
        printf("\n=================注册模型================\n");
		Config_model(&configHandle); //注册模型
		usleep(100000);	//delay100ms
		cnt ++;
        if(!(gMqttStateMachine&0x02)){
            zlog_error(zc,"模型注册失败,失败次数 = %d,gMqttStateMachine = %02x!",cnt,gMqttStateMachine);
            logMsg(logWarn,"模型注册失败,失败次数 = %d,gMqttStateMachine = %02x!",cnt,gMqttStateMachine);
        }
	}
    cnt= 0;
    while(!(gMqttStateMachine&0x04))
    {
        printf("\n=================注册设备================\n");
        Register_dev(&configHandle); //注册设备
		usleep(100000);	//delay100ms
		cnt ++;
        if(!(gMqttStateMachine&0x04)){
            zlog_error(zc,"设备注册失败,失败次数 = %d,gMqttStateMachine = %02x!",cnt,gMqttStateMachine);
            logMsg(logWarn,"设备注册失败,失败次数 = %d,gMqttStateMachine = %02x!",cnt,gMqttStateMachine);
        }
    }
    cnt = 0;
    while(!(gMqttStateMachine&0x08))
    {
        printf("\n================获取GUID================\n");
        Get_dev_guid(&configHandle); //获取GUID
		usleep(100000);	//delay100ms
		cnt ++;
        if(!(gMqttStateMachine&0x08)){
             zlog_error(zc,"获取GUID失败,失败次数 = %d,gMqttStateMachine = %02x!",cnt,gMqttStateMachine);
            logMsg(logWarn,"获取GUID失败,失败次数 = %d,gMqttStateMachine = %02x!",cnt,gMqttStateMachine);
        }
    }
    while(1){
        sem_wait(&g_sem);//获取信号量
        printf("\n================上报实时数据================\n");
        for(cnt = 0;cnt<configHandle.reg.reg_ok_dev_num;cnt++){
            printf("编号_%02d\n",cnt);
            Realtime_data_report(&configHandle,gdata,cnt);//实时数据主动上报
            usleep(1000);
        }
    }
    
    mosquitto_destroy(gMosq);
    mosquitto_lib_cleanup();

    return NULL;
}