#ifndef _QUEUE_THREAD_SAFE_H_
#define _QUEUE_THREAD_SAFE_H_

#include <pthread.h>

#include "unordered_circular_doubly_list_with_base.h"

typedef struct queue_thread_safe {
    unordered_circular_doubly_list_with_base_t *elements;
    pthread_mutex_t mutex;
} queue_thread_safe_t;

queue_thread_safe_t *queue_create_thread_safe();
void queue_insert_thread_safe(void * element, queue_thread_safe_t *queue);
void *queue_remove_thread_safe(queue_thread_safe_t *queue);
void *queue_query_first_thread_safe(queue_thread_safe_t *queue);
void _destroy_thread_safe(queue_thread_safe_t **queue);
#define queue_destroy_thread_safe(queue) _destroy_thread_safe((queue_thread_safe_t **)(&(queue)))
uint32_t queue_get_number_of_elements_thread_safe(queue_thread_safe_t *queue);
void queue_print_int_thread_safe(queue_thread_safe_t *queue);

#endif /* _QUEUE_THREAD_SAFE_H_ */