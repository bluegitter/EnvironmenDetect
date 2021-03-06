#CC = gcc
CC = arm-linux-gnueabihf-gcc
INCLUDE = -I ./include
LIBS   +=  -lssl -lcrypto -lmosquitto -lpthread -lcares -lzlog
#LIBS   += -Bstatic -lzlog -Bdynamic
LIB_PATH += -L /home/fashion/zhixin_item/lib 
LIB_PATH += -L ./lib
CFLAGS = -Wall 
OBJS   = main.o modbus_master.o serial_drive.o cJSON.o linked_list.o lib_memory.o mosquitto_client.o
TARGET = Env 

all:$(TARGET)

$(TARGET):$(OBJS) 
	$(CC) -o $@ $^ $(LIBS) $(LIB_PATH)
 
 %.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE) #-DLOG 

.PHONY: all clean cleanobject
cleanobject:
	rm -rf *.o
clean: 
	rm -rf *.exe *.o Env

