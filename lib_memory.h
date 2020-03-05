/*
 * @Author: your name
 * @Date: 2020-02-24 22:53:10
 * @LastEditTime : 2020-02-25 20:16:15
 * @LastEditors  : Please set LastEditors
 * @Description: 内存管理API
 * @FilePath: /EnvironmentDetec/lib_memory.h
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#define CALLOC(nmemb, size) Memory_calloc(nmemb, size)
#define MALLOC(size)        Memory_malloc(size)
#define REALLOC(oldptr, size)   Memory_realloc(oldptr, size)
#define FREEMEM(ptr)        Memory_free(ptr)

#define GLOBAL_CALLOC(nmemb, size) Memory_calloc(nmemb, size)
#define GLOBAL_MALLOC(size)        Memory_malloc(size)
#define GLOBAL_REALLOC(oldptr, size)   Memory_realloc(oldptr, size)
#define GLOBAL_FREEMEM(ptr)        Memory_free(ptr)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

void* Memory_malloc(size_t size);

void* Memory_calloc(size_t nmemb, size_t size);

void* Memory_realloc(void *ptr, size_t size);

void Memory_free(void* memb);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_H_ */
