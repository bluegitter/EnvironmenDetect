/*
 * @Author: your name
 * @Date: 2020-02-24 22:53:10
 * @LastEditTime : 2020-03-03 17:59:39
 * @LastEditors  : Please set LastEditors
 * @Description: 单向链表API
 * @FilePath: /EnvironmentDetec/linked_list.h
 */

#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "lib_memory.h"
/**
 * \defgroup LINKED_LIST LinkedList data type definition and handling functions
 */

struct sLinkedList {
	void* data;
	struct sLinkedList* next;
};
/**
 * \brief Reference to a linked list or to a linked list element.
 */
typedef struct sLinkedList* LinkedList;

/**
 * \brief Create a new LinkedList object
 *
 * \return the newly created LinkedList instance
 */
LinkedList LinkedList_create();
/**
 * \brief Delete a LinkedList object
 *
 * This function destroy the LinkedList object. It will free all data structures used by the LinkedList
 * instance. It will call free for all elements of the linked list. This function should only be used if
 * simple objects (like dynamically allocated strings) are stored in the linked list.
 *
 * \param self the LinkedList instance
 */
void LinkedList_destroy(LinkedList self);


typedef void (*LinkedListValueDeleteFunction) (void*);

/**
 * \brief Delete a LinkedList object
 *
 * This function destroy the LinkedList object. It will free all data structures used by the LinkedList
 * instance. It will call a user provided function for each data element. This user provided function is
 * responsible to properly free the data element.
 *
 * \param self the LinkedList instance
 * \param valueDeleteFunction a function that is called for each data element of the LinkedList with the pointer
 *         to the linked list data element.
 */
void LinkedList_destroyDeep(LinkedList self, LinkedListValueDeleteFunction valueDeleteFunction);

/**
 * \brief Delete a LinkedList object without freeing the element data
 *
 * This function should be used statically allocated data objects are stored in the LinkedList instance.
 * Other use cases would be if the data elements in the list should not be deleted.
 *
 * \param self the LinkedList instance
 */
void LinkedList_destroyStatic(LinkedList self);

/**
 * \brief Add a new element to the list
 *
 * This function will add a new data element to the list. The new element will the last element in the
 * list.
 *
 * \param self the LinkedList instance
 * \param data data to append to the LinkedList instance
 */
void LinkedList_add(LinkedList self, void* data);

/**
 * \brief Removed the specified element from the list
 *
 * \param self the LinkedList instance
 * \param data data to remove from the LinkedList instance
 */
bool LinkedList_remove(LinkedList self, void* data);

/**
 * \brief Get the list element specified by index (starting with 0).
 *
 * \param self the LinkedList instance
 * \param index index of the requested element.
 */
LinkedList LinkedList_get(LinkedList self, int index);

/**
 * \brief Get the next element in the list (iterator).
 *
 * \param self the LinkedList instance
 */
LinkedList LinkedList_getNext(LinkedList self);

/**
 * \brief Get the last element in the list.
 *
 * \param listElement the LinkedList instance
 */
LinkedList LinkedList_getLastElement(LinkedList self);

/**
 * \brief Insert a new element int the list
 *
 * \param listElement the LinkedList instance
 */
LinkedList LinkedList_insertAfter(LinkedList listElement, void* data);

/**
 * \brief Get the size of the list
 *
 * \param self the LinkedList instance
 *
 * \return number of data elements stored in the list
 */
int LinkedList_size(LinkedList self);

void* LinkedList_getData(LinkedList self);

#ifdef __cplusplus
}
#endif

#endif /* LINKED_LIST_H_ */
