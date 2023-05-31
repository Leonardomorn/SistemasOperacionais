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
        fprintf(stderr, "queue does not exist, can't append\n");
        return -1;
    }

    if (!elem) //verify elem existence
    {
        fprintf(stderr, "element does not exist\n");
        return -1;
    }

    if(elem->next || elem->prev) //verify elem isolation
    {
        fprintf(stderr, "element is not isolated\n");
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
    return 0;
}

/**
 * @brief Remove the requested element on the queue
 * 
 * @param queue to be reduzed
 * @param elem element to be removed
 * @return 0 in success, -1 otherwise
 */
int queue_remove (queue_t **queue, queue_t *elem)
{
    if (!queue) //verify queue existence
    {
        fprintf(stderr, "queue does not exist\n");
        return -1;
    }

    if(is_empty(queue)) //verify if queue is empty
    {
        fprintf(stderr, "queue is empty\n");
        return -1;
    }

    if (!elem) //verify elem existence
    {
        fprintf(stderr, "element does not exist\n");
        return -1;
    }

    if(!(elem->next || elem->prev)) //verify elem isolation
    {
        fprintf(stderr, "element is isolated\n");
        return -1;
    }


    if(elem == *(queue) && queue_size(*queue) == 1) //removing the last element
    {
        (*queue)->next = NULL;
        (*queue)->prev = NULL;
        (*queue) = NULL;
        return 0;
    }

    queue_t *queue_aux = (*queue);
    do
    {
        if (queue_aux == elem)
        {

            elem->next->prev = elem->prev;
            elem->prev->next = elem->next;

            if(elem == (*queue)) //if is the first element of the queue
            {
                (*queue) = (*queue)->next;
            }

            //isolate element
            elem->next = NULL;
            elem->prev = NULL;            
            return 0;
        }
        queue_aux = queue_aux->next;
    }
    while (queue_aux != (*queue)); //wandering in all queue to search for the element
   

    //if it's not on the queue, return error
    fprintf(stderr, "element out of queue\n");
    return -1;
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

void queue_print (char *name, queue_t *queue, void print_elem (void*) )
{
    printf("%s :", name); 
    if(!queue)
    {
        printf("[]\n");
        return;
    }

    queue_t *queue_aux = queue;


    printf("[");
    do
    {
        print_elem(queue_aux);
        printf(" ");
        queue_aux = queue_aux->next;
    } while (queue_aux->next != queue);

    print_elem(queue_aux);
        
    printf("]\n");


}
