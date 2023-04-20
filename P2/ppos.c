#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>

#define STACKSIZE 64*1024	/* tamanho de pilha das threads */

task_t *currentTask_global;
task_t mainTask_global;
int idCounter_global;

void ppos_init ()
{
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0);
    idCounter_global = 0;

    #ifdef DEBUG
    printf ("Settando current task em main\n") ;
    #endif
    currentTask_global = &mainTask_global;

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
   }
   else
   {
      perror ("Erro na criação da pilha: ") ;
      exit (1) ;
   }

    #ifdef DEBUG
    printf ("modificando os valores do contexto\n") ;
    #endif
    makecontext (&(task->context), (void*)(start_func),1, arg) ;


    idCounter_global++;
    #ifdef DEBUG
    printf ("atribuindo um ID %d\n", idCounter_global) ;
    #endif
    task->id = idCounter_global;

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
    if(currentTask_global == &mainTask_global)
    {
        perror("task exit in main");
        exit(1);
    }

    setcontext(&(mainTask_global.context));
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

    swapcontext(&(aux->context), &(task->context));
    return 0;
    }
    return -1;
}