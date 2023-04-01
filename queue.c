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
    return 0;
}


// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: 0 se sucesso, <0 se ocorreu algum erro
int queue_remove (queue_t **queue, queue_t *elem)
{
    if (!queue) //verify queue existence
    {
        fprintf(stderr, "queue does not exist");
        return -1;
    }

    if(is_empty(queue)) //verify if queue is empty
    {
        fprintf(stderr, "queue is empty");
        return -1;
    }

    if (!elem) //verify elem existence
    {
        fprintf(stderr, "element does not exist");
        return -1;
    }

    if(!(elem->next || elem->prev)) //verify elem isolation
    {
        fprintf(stderr, "element is isolated");
        return -1;
    }

    queue_t *queue_aux = (*queue);

    if(elem == *(queue) && queue_size(*queue) == 1) //removing the last element
    {
        (*queue)->next = NULL;
        (*queue)->prev = NULL;
        (*queue) = NULL;
        queue = NULL;
        return 0;
    }
     do
    {
        if (queue_aux == elem)
        {

            elem->next->prev = elem->prev;
            elem->prev->next = elem->next;
            if(elem == (*queue))
            {
                (*queue) = (*queue)->next;
            }            
            return 0;
        }
        queue_aux = queue_aux->next;
    }
    while (queue_aux != (*queue)); //wandering in all queue to search for the element
   

    //if it's not on the queue, return error
    fprintf(stderr, "element out of queue");
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