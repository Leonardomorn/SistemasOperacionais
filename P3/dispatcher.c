#include "dispatcher.h"


void dispatcher (queue_t* queue_task, int userTasksQuantity)
{
    task_t *taskAux;
    while(userTasksQuantity > 0 )
    {
        taskAux = scheduler(queue_task);
        //se houver alguma tarefa pronta
        if (taskAux)
        {
            //transfere o controle para a prox tarefa
            task_switch(taskAux);
            switch (taskAux->status)
            {
            case READY:
                
                break;
            case TERMINATED://caso terminada, sair da fila

                {
                    queue_remove((queue_t **) &queue_task, (queue_t *) &taskAux);
                }
                break;
            default:
                break;
            }
        }
    }
}

/*
*@brief joga a tarefa a ser executada no final da fila e a retorna para ser execucao
*@param queue_task : fila de tarefas prontas
*@return ponteiro para a tarefa a ser executada
*/
int * scheduler (queue_t* queue_ReadyTask)
{
    if (queue_ReadyTask == NULL)
        return NULL;
    queue_ReadyTask = queue_ReadyTask->next;
    return queue_ReadyTask->prev;
}
