#include "queue.h"
#include <stdio.h>
#include <stdlib.h>


int is_empty(queue_t **queue);

int queue_size (queue_t *queue)
{
    if(!queue)
    {
        return 0;
    }

    queue_t *queue_aux = queue;
    int counter = 1;
    while (queue_aux->next != queue)
    {
        counter++;
        queue_aux = queue_aux->next;
    }
    return counter;
    
}


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

    if(is_empty(queue)) //if this is the first element
    {
        *queue = elem;
        elem->next = elem;
        elem->prev = elem;
    }

    else //if there is at least one element
    {
        elem->next = (*queue); //points to the first element
        elem->prev = (*queue)->prev; //points to the preview last
        (*queue)->prev->next = elem;//previews last now points to element
        (*queue)->prev = elem;//element is now the last one
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