/*
 * @Author: your name
 * @Date: 2020-02-24 22:52:42
 * @LastEditTime : 2020-03-03 17:59:32
 * @LastEditors  : Please set LastEditors
 * @Description: 单向链表API
 * @FilePath: /EnvironmentDetec/linked_list.c
 */

#include "linked_list.h"

/**
 * @description: 获取链表最后一个元素
 * @param {type} 
 * @return: 
 */
LinkedList LinkedList_getLastElement(LinkedList list)
{
    while (list->next != NULL) {
        list = list->next;
    }
    return list;
}
/**
 * @description: 创建链表
 * @param {type} 
 * @return: 
 */
LinkedList LinkedList_create()
{
    LinkedList newList;

    newList = (LinkedList) GLOBAL_MALLOC(sizeof(struct sLinkedList)); 
    newList->data = NULL;
    newList->next = NULL;

    return newList;
}

/**
 * Destroy list (free). Also frees element data with helper function.
 */
void LinkedList_destroyDeep(LinkedList list, LinkedListValueDeleteFunction valueDeleteFunction)
{
    LinkedList nextElement = list;
    LinkedList currentElement;

    do {
        currentElement = nextElement;
        nextElement = currentElement->next;

        if (currentElement->data != NULL)
            valueDeleteFunction(currentElement->data);

        GLOBAL_FREEMEM(currentElement);
    }
    while (nextElement != NULL);
}
/**
 * @description: 销毁链表(链表数据一并销毁)
 * @param {type} 
 * @return: 
 */
void LinkedList_destroy(LinkedList list)
{
    LinkedList_destroyDeep(list, Memory_free);
}
/**
 * @description: 销毁链表但不释放元素数据
 * @param {type} 
 * @return: 
 */
void LinkedList_destroyStatic(LinkedList list)
{
    LinkedList nextElement = list;
    LinkedList currentElement;

    do {
        currentElement = nextElement;
        nextElement = currentElement->next;
        GLOBAL_FREEMEM(currentElement);
    }
    while (nextElement != NULL);
}
/**
 * @description: 查询链表元素大小
 * @param {type} 
 * @return: 
 */
int LinkedList_size(LinkedList list)
{
    LinkedList nextElement = list;
    int size = 0;

    while (nextElement->next != NULL) {
        nextElement = nextElement->next;
        size++;
    }

    return size;
}
/**
 * @description: 在链表最后新增一个元素
 * @param {type} 
 * @return: 
 */
void LinkedList_add(LinkedList list, void* data)
{
    LinkedList newElement = LinkedList_create();

    newElement->data = data;

    LinkedList listEnd = LinkedList_getLastElement(list);

    listEnd->next = newElement;
}
/**
 * @description: 移除当前链表元素
 * @param {type} 
 * @return: 
 */
bool LinkedList_remove(LinkedList list, void* data)
{
    LinkedList lastElement = list;

    LinkedList currentElement = list->next;

    while (currentElement != NULL) {
        if (currentElement->data == data) {
            lastElement->next = currentElement->next;
            GLOBAL_FREEMEM(currentElement);
            return true;
        }

        lastElement = currentElement;
        currentElement = currentElement->next;
    }

    return false;
}
/**
 * @description: 在当前链表元素插入新元素
 * @param {type} 
 * @return: 
 */
LinkedList LinkedList_insertAfter(LinkedList list, void* data)
{
    LinkedList originalNextElement = LinkedList_getNext(list);

    LinkedList newElement = LinkedList_create();

    newElement->data = data;
    newElement->next = originalNextElement;

    list->next = newElement;

    return newElement;
}
/**
 * @description: 获取元素指针
 * @param {type} 
 * @return: 
 */
LinkedList LinkedList_getNext(LinkedList list)
{
    return list->next;
}
/**
 * @description: 按元素索引号查询元素,不存在返回NULL
 * @param {type} 
 * @return: 
 */
LinkedList LinkedList_get(LinkedList list, int index)
{
    LinkedList element = LinkedList_getNext(list);

    int i = 0;

    while (i < index) {
        element = LinkedList_getNext(element);

        if (element == NULL)
            return NULL;

        i++;
    }

    return element;
}
/**
 * @description: 获取链表元素
 * @param {type} 
 * @return: 
 */
void* LinkedList_getData(LinkedList self)
{
    return self->data;
}

