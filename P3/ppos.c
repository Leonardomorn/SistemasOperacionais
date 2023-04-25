#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"




//#define DEBUG 1
#define STACKSIZE 64*1024	/* tamanho de pilha das threads */

task_t *currentTask_global;
task_t dispatcherTask_global;
task_t mainTask_global;
int idCounter_global, userTasks_global;

//PARTE DA FILA DE TASKS - INICIO

task_t * ready_task_queue;

//PARTE DA FILA DE TASKS - FINAL

void ppos_init ()
{
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0);
    idCounter_global = 0;
    userTasks_global = 0;
    ready_task_queue = NULL;


    #ifdef DEBUG
    printf ("Settando current task em main\n") ;
    #endif
    //coloca a main como atual
    currentTask_global = &mainTask_global;

    //inicia a tarefa dispatcher
    task_init(&dispatcherTask_global, &dispatcher,NULL);
    
    #ifdef DEBUG
    printf ("Terminada inicializacao de sistema\n");
    #endif

    
}


// Inicializa uma nova tarefa. Retorna um ID> 0 ou erro.
int task_init (task_t *task,			// descritor da nova tarefa
               void  (*start_func)(void *),	// funcao corpo da tarefa
               void   *arg) 			// argumentos para a tarefa
{
    char *stack ;
    #ifdef DEBUG
    printf ("criando um contexto\n") ;
    #endif
    #ifdef DEBUG
    printf ("obtendo o contexto\n") ;
    #endif
    getcontext (&(task->context)) ;

    //alocando a pilha
    stack = malloc (STACKSIZE) ;
    if (stack)
    {
        #ifdef DEBUG
        printf ("determinando os descritores do contexto\n") ;
        #endif
        task->context.uc_stack.ss_sp = stack;
        task->context.uc_stack.ss_size = STACKSIZE;
        task->context.uc_stack.ss_flags = 0;
        task->context.uc_link = 0;
        task->next = NULL;
        task->prev = NULL;
   }
   else
   {
      perror ("Erro na criação da pilha: ") ;
      exit (1) ;
   }

    #ifdef DEBUG
    printf ("modificando os valores do contexto\n") ;
    printf("salvando a funcao do ponteiro%p \n", (void*)(start_func));
    #endif
    if(arg)
        makecontext (&(task->context), (void*)(start_func),1, arg) ;
    else
        makecontext (&(task->context), (void*)(start_func),0) ;

    idCounter_global++;
    #ifdef DEBUG
    printf ("atribuindo um ID %d\n", idCounter_global) ;
    #endif
    task->id = idCounter_global;

    if(idCounter_global > 1) //0 e 1 sao reservados para main e dispatcher
    {
        //adiciona a tarefa para a fila

        queue_append((queue_t **) &ready_task_queue, (queue_t *) task);
        userTasks_global++;
        task->status = READY;
    }

    #ifdef DEBUG
    printf ("criação finalizada\n") ;
    #endif
    



    return task->id; 
}

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id ()
{
    return currentTask_global->id;
}

// Termina a tarefa corrente com um valor de status de encerramento e retorna para a main
void task_exit (int exit_code) 
{
    //caso esteja na main, vai para o dispatcher
    if(currentTask_global == &mainTask_global)
    {
        task_switch(&dispatcherTask_global);
    }
    //caso exit na dispatcher, saia do programa

    if(currentTask_global == &dispatcherTask_global)
    {
        exit(exit_code);
    }

    //caso seja uma tarefa normal
    currentTask_global->status = TERMINATED;
    task_switch(&dispatcherTask_global);
}

// alterna a execução para a tarefa indicada
int task_switch (task_t *task)
{
    #ifdef DEBUG
    printf ("solicitando troca de contexto\n") ;
    #endif
    if (task)
    {
    task_t *aux = currentTask_global;
    currentTask_global = task;


    swapcontext(&(aux->context), &(currentTask_global->context));
    return 0;
    }
    return -1;
}


void dispatcher ()
{
    #ifdef DEBUG
    printf ("Entrando em Despachante\n") ;
    #endif
    task_t *taskAux;
    while(userTasks_global > 0 )
    {
        taskAux = scheduler(ready_task_queue);
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
                    queue_remove((queue_t **) &ready_task_queue, (queue_t *) taskAux);
                    userTasks_global--;
                    
                }
                break;
            default:
                break;
            }
        }
    }
}

/*
*@brief retorna a tarefa que deverá ser executada, em caso de fila vazia, retorna nulo
*@param queue_task : fila de tarefas prontas
*/
task_t * scheduler ()
{
    if (ready_task_queue == NULL)
        return NULL;
    return ready_task_queue;
}

/****
 * @brief gira a lista de tarefas prontas e troca o contexto para o dispatcher
*/
void task_yield()
{
    if(ready_task_queue)
    {
    ready_task_queue = (task_t *) ready_task_queue->next;
    }  
    task_switch(&dispatcherTask_global);

}