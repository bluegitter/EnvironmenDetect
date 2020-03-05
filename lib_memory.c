/*
 * @Author: your name
 * @Date: 2020-02-24 22:52:57
 * @LastEditTime : 2020-02-24 23:10:29
 * @LastEditors  : Please set LastEditors
 * @Description: 内存管理API
 * @FilePath: /EnvironmentDetec/lib_memory.c
 */

#include "lib_memory.h"

static void noMemoryAvailableHandler(void)
{
    printf("no memory of available!");
}

/**
 * @description: 分配大小为size个字节的内存
 * @param {type} 
 * @return: 
 */
void* Memory_malloc(size_t size)
{
    void* memory = malloc(size);

    if (memory == NULL)
        noMemoryAvailableHandler();

    return memory;
}
/**
 * @description: 分配nmemb个大小为size内存
 * @param {type} 
 * @return: 
 */
void* Memory_calloc(size_t nmemb, size_t size)
{
    void* memory = calloc(nmemb, size);

    if (memory == NULL)
        noMemoryAvailableHandler();

    return memory;
}
/**
 * @description: 将ptr指向的内存重新分配为size个字节大小
 * @param {type} 
 * @return: 
 */
void * Memory_realloc(void *ptr, size_t size)
{
    void* memory = realloc(ptr, size);

    if (memory == NULL)
        noMemoryAvailableHandler();

    return memory;
}
/**
 * @description: 释放内存
 * @param {type} 
 * @return: 
 */
void Memory_free(void* memb)
{
    free(memb);
}

