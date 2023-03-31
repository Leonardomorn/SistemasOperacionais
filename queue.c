#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

int queue_append (queue_t **queue, queue_t *elem)
{
    if (!queue) //verify queue existence
    {
        fprintf(stderr, "queue does not exist");
        return -1;
    }

    if (!elem) //verify elem existence
    {
        fprintf(stderr, "element does not exist");
        return -1;
    }

    if(elem->next || elem->prev) //verify elem isolation
    {
        fprintf(stderr, "element is not isolated");
        return -1;
    }

    if(is_empty(queue)) //caso seja o primeiro elemento da fila
    {
        queue = elem;
        elem->next = elem;
        elem->prev = elem;
    }

    else //caso haja pelo menos um elemento
    {
        elem->next = (*queue);
        elem->prev = (*queue)->prev;
    }

}

/**
 * @brief verify if queue is empty
 * 
 * @param queue : queue type 
 * @return 0 if it is not empty, 1 if empty
 **/
int is_empty(queue_t **queue)
{
    if(*queue)
        return 0;
    return 1;
}