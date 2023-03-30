#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

int queue_append (queue_t **queue, queue_t *elem)
{
    if (!queue) //verifica se a fila existe
    {
        fprintf(stderr, "A fila solicitada não existe")
        return 0;
    }
}