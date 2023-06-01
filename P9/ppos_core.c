/*Código feito por Leonardo da Silva Camargo, GRR 20203903
Disciplina: Sistemas Operacionais
Professor: Carlos Maziero
*/

#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <signal.h>
#include <sys/time.h>


// testa sistema operacional
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

// #define DEBUG 1
#define STACKSIZE 64 * 1024 /* tamanho de pilha das threads */

task_t *currentTask_global;
task_t dispatcherTask_global;
task_t mainTask_global;
int idCounter_global, userTasks_global;
task_t *ready_task_queue;
task_t *sleeping_task_queue;
unsigned int system_time_global;


// estrutura que define um tratador de sinal
struct sigaction signal_handler_global;
int pode_preempcao = 1; int colocando_dormir = 1; // se qualquer valor desses for 0, não haverá preempção por tempo 
// estrutura de inicialização do timer
struct itimerval timer_global;


//inicializa propriedades da tarefa main
void initializeMainTask()
{
    mainTask_global.id = 0;
    mainTask_global.quantum_ticks = 0;
    mainTask_global.initial_time = systime();
    mainTask_global.initial_time_processor = systime();
    mainTask_global.activations = 0;
    task_setprio(&mainTask_global, 0);
    task_set_type(&mainTask_global, USER_TASK);


}

void ppos_init()
{
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf(stdout, 0, _IONBF, 0);
    system_time_global = 0;
    idCounter_global = 0;
    userTasks_global = 0;
    ready_task_queue = NULL;
    sleeping_task_queue = NULL;
#ifdef DEBUG
    printf("Settando current task em main\n");
#endif
    initializeMainTask();
    // coloca a main como atual
    currentTask_global = &mainTask_global;
    queue_append((queue_t **)&ready_task_queue, (queue_t *)&mainTask_global);
    userTasks_global++;
    // inicia a tarefa dispatcher
    task_init(&dispatcherTask_global, &dispatcher, NULL);
    // define dispatcher como tarefa de sistema
    task_set_type(&dispatcherTask_global, SYSTEM_TASK);
    initialize_handler();
    initialize_timer();

#ifdef DEBUG
    printf("Terminada inicializacao de sistema\n");
#endif
}

// Inicializa uma nova tarefa. Retorna um ID> 0 ou erro.
int task_init(task_t *task,               // descritor da nova tarefa
              void (*start_func)(void *), // funcao corpo da tarefa
              void *arg)                  // argumentos para a tarefa
{
    char *stack;
    #ifdef DEBUG
        printf("criando um contexto\n");
    #endif
    #ifdef DEBUG
        printf("obtendo o contexto\n");
    #endif
    getcontext(&(task->context));

    // alocando a pilha
    stack = malloc(STACKSIZE);
    if (stack)
    {
    #ifdef DEBUG
        printf("determinando os descritores do contexto\n");
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
        perror("Erro na criação da pilha: ");
        exit(1);
    }

    #ifdef DEBUG
        printf("modificando os valores do contexto\n");
        printf("salvando a funcao do ponteiro%p \n", (void *)(start_func));
    #endif
    if (arg)
        makecontext(&(task->context), (void *)(start_func), 1, arg);
    else
        makecontext(&(task->context), (void *)(start_func), 0);

    idCounter_global++;
    #ifdef DEBUG
        printf("atribuindo um ID %d\n", idCounter_global);
    #endif
    task->id = idCounter_global;

    if (idCounter_global > 1) // 0 e 1 sao reservados para main e dispatcher
    {
        // adiciona a tarefa para a fila

        queue_append((queue_t **)&ready_task_queue, (queue_t *)task);
        userTasks_global++;
        task->status = READY;
    }

    task_setprio(task, 0);
    // Caso não seja o dispatcher, é uma tarefa de usuário
    if (idCounter_global != 1)
        task_set_type(task, USER_TASK);

    task->quantum_ticks = 0;
    task->activations = 0;
    task->initial_time = systime();
    #ifdef DEBUG
        printf("criação finalizada\n");
    #endif

    return task->id;
}

// retorna o identificador da tarefa corrente (main deve ser 0 e dispatcher 1)
int task_id()
{
    return currentTask_global->id;
}

// Termina a tarefa corrente com um valor de status de encerramento e retorna para a main
void task_exit(int exit_code)
{
    // caso esteja na main, vai para o dispatcher
    if (currentTask_global == &mainTask_global)
    {
        task_switch(&dispatcherTask_global);
    }
    // caso exit na dispatcher, saia do programa

    if (currentTask_global == &dispatcherTask_global)
    {
        exit(0);
    }

    // caso seja uma tarefa normal

    currentTask_global->status = TERMINATED;
    pode_preempcao = 0;
    free_suspended_queue();
    currentTask_global->exitCode = exit_code;
    pode_preempcao = 1;
    task_switch(&dispatcherTask_global);
}

// alterna a execução para a tarefa indicada
int task_switch(task_t *task)
{
#ifdef DEBUG
    printf("solicitando troca de contexto da tarefa %d para %d\n", currentTask_global->id, task->id);
#endif
    if (task == NULL)
        return -1;
    else
    {
        task_t *aux = currentTask_global;
        currentTask_global = task;

        //acrescenta o tempo de processador da tarefa que estava executando 
        aux->final_time_processor = systime();
        aux->processor_time = aux->processor_time + (aux->final_time_processor - aux->initial_time_processor);
        //setta o tempo inicial do próximo processamento da tarefa e seu número de ativação
        currentTask_global->initial_time_processor = systime();
        currentTask_global->activations++;
        //troca para o contexto da nova tarefa
        swapcontext(&(aux->context), &(currentTask_global->context));
        return 0;
    }
}

void dispatcher()
{
#ifdef DEBUG
    printf("Entrando em Despachante\n");
#endif
    task_t *taskAux;
    
    while (userTasks_global > 0)
    {
        colocando_dormir = 0;
        wake_sleeping_tasks(); //acorda tarefas que estão dormindo e já passaram do tempo de dormir
        colocando_dormir = 1;
        taskAux = scheduler(ready_task_queue);
        // se houver alguma tarefa pronta
        if (taskAux != NULL)
        {
            // transfere o controle para a prox tarefa
            taskAux->quantum_ticks = 0;
            pode_preempcao = 0;
            task_switch(taskAux);
            pode_preempcao = 1;
            switch (taskAux->status)
            {
            case READY:
                restore_dynamic__into_static_prio(taskAux);
                break;
            case TERMINATED: // caso terminada, sair da fila

            {
                //calcula o tempo de execução da tarefa
                taskAux->final_time = systime();
                taskAux->execution_time = taskAux->final_time - taskAux->initial_time;
                //remove a tarefa da fila de prontos
                queue_remove((queue_t **)&ready_task_queue, (queue_t *)taskAux);
                //imprime tempos de execução, processador e ativação da tarefa
                printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", taskAux->id, taskAux->execution_time, taskAux->processor_time, taskAux->activations);
                //caso a tarefa não seja a main, desalocar ela
                if (taskAux != &mainTask_global)
                    free(taskAux->context.uc_stack.ss_sp);
                userTasks_global--;
            }
            break;
            default:
                break;
            }
        }
    }
    currentTask_global->final_time = systime();
    currentTask_global->execution_time = currentTask_global->final_time - currentTask_global->initial_time;    
    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", currentTask_global->id, currentTask_global->execution_time, currentTask_global->processor_time, currentTask_global->activations);

}

/********
 *@brief retorna a tarefa que deverá ser executada, em caso de fila vazia, retorna nulo
 *@param queue_task : fila de tarefas prontas
 *@return tarefa a ser executada, em caso de fila vazia, retorna nulo
 */
task_t *scheduler()
{
    if (ready_task_queue == NULL)
        return NULL;
    task_t *to_be_executed_task;
    // tarefa a ser executada é o primeiro mínimo a partir do segundo valor fila
    to_be_executed_task = first_min();
    // faz a fila de tarefas prontas apontar para a tarefa a ser executada
    // ready_task_queue = to_be_executed_task;
    // reduz a prioridade das outras tarefas que não serão executadas neste momento
    raise_priority(to_be_executed_task);

    return to_be_executed_task;
}

/****
 * @brief troca o contexto para o dispatcher
 */
void task_yield()
{

    task_switch(&dispatcherTask_global);
}

void task_setprio(task_t *task, int prio)
{
#ifdef DEBUG
    if (task)
        printf("Setando prioridade da tarefa %d como %d\n", task->id, prio);
    else
        printf("Setando prioridade da tarefa atual %d como %d\n", currentTask_global->id, prio);
#endif
    if (task)
    {
        task->priority_static = prio;
        task->priority_dynamic = prio;
    }
    else
    {
        currentTask_global->priority_static = prio;
        currentTask_global->priority_dynamic = prio;
    }
}

void task_set_type(task_t *task, short type)
{
    task->type = type;
}

int task_getprio(task_t *task)
{
    if (task)
        return task->priority_static;
    else
        return currentTask_global->priority_static;
}

/******
 * @brief retorna a primeira tarefa mínima da fila de prontos, a tarefa mínima e a tarefa com maior prioridade
 */
task_t *first_min()
{
    task_t *task_aux = ready_task_queue;
    task_t *min_task = NULL;
    int min = 100;
    for (int i = 0; i < queue_size((queue_t *)ready_task_queue); i++)
    {
        if (task_aux->priority_dynamic < min)
        {
            min_task = task_aux;
            min = task_aux->priority_dynamic;
        }
        task_aux = task_aux->next;
    }

    // printf("A tarefa mínima possui prioridade %d\n", min);
    return min_task;
}

/******
 * @brief aumenta a prioridade de todas a prioridade de todas as tarefas, com exeção da primeira. A tarefa mais prioritária é a de menor valor
 */
void raise_priority(task_t *to_be_executed_task)
{
    task_t *task_aux = to_be_executed_task;

    for (int i = 0; i < queue_size((queue_t *)ready_task_queue) - 1; i++)
    {
        task_aux = task_aux->next;
        if (task_aux->priority_dynamic > -20) // prioridade máxima é -20
            task_aux->priority_dynamic--;
    }
}

void restore_dynamic__into_static_prio(task_t *taskAux)
{
    taskAux->priority_dynamic = taskAux->priority_static;
}

// tratador do sinal
void handling(int signum)
{
    #ifdef DEBUG
    printf("Recebi o sinal %d\n", signum);
    #endif

    system_time_global++;

    //não há alterações caso seja uma tarefa de sistema ou se estiver em um processo atômico
    if (currentTask_global->status == SYSTEM_TASK || pode_preempcao == 0 || colocando_dormir == 0)
        return;
    currentTask_global->quantum_ticks++;
    if (currentTask_global->quantum_ticks > 20)
        task_yield();
}

// inicializa propriedades do timer
void initialize_timer()
{
#ifdef DEBUG
    printf("Inicializando o timer\n");
#endif
    timer_global.it_value.tv_usec = 100000;       // primeiro disparo em microsegundos
    timer_global.it_value.tv_sec = 0;        // primeiro disparo em segundos
    timer_global.it_interval.tv_usec = 1000; // disparos subsequentes em microsegundos
    timer_global.it_interval.tv_sec = 0;     // disparos subsequentes em segundos

    // arma o temporizador ITIMER_REAL
    if (setitimer(ITIMER_REAL, &timer_global, 0) < 0)
    {
        perror("Erro em setitimer: ");
        exit(1);
    }
#ifdef DEBUG
    printf("Timer inicializado\n");
#endif
}

// inicializa propriedades do handler a aponta para a função handling
void initialize_handler()
{
#ifdef DEBUG
    printf("Inicializando o handler\n");
#endif
    // registra ação para o sinal de timer SIGALRM (sinal do timer)
    signal_handler_global.sa_handler = handling;
    sigemptyset(&signal_handler_global.sa_mask);
    signal_handler_global.sa_flags = 0;
    if (sigaction(SIGALRM, &signal_handler_global, 0) < 0)
    {
        perror("Erro em sigaction: ");
        exit(1);
    }
#ifdef DEBUG
    printf("Handler inicializado\n");
#endif
}

unsigned int systime()
{
    return system_time_global;
}

//======================================Sincronização====================================================//
/**
 * @brief função que remove a tarefa atual da fila de prontos, suspende a tarefa e a adiciona na queue
 * 
 * @param queue fila em que a tarefa suspensa é adicionada 
 */
void task_suspend (task_t **queue)
{
    pode_preempcao = 0;
    task_t *taskAux;
    taskAux = currentTask_global;
    //remove a tarefa atual da fila de prontos
    if(queue)
    {
        queue_remove((queue_t **)&ready_task_queue, (queue_t *)taskAux);
    }
    //deixa o status da tarefa atual como SUSPENDED
    currentTask_global->status = SUSPENDED;
    //adiciona a tarefa atual em queue
    queue_append((queue_t **)queue,(queue_t*)currentTask_global);
    pode_preempcao = 1;
    //retorna ao dispatcher
    task_yield();
}


/**
 * @brief se a fila queue não for nula, retira a tarefa apontada por task dessa fila, ajusta o status dessa tarefa para “pronta” e insere a tarefa na fila de tarefas prontas.
 * 
 * @param task tarefa a ser colocada novamente na fila de prontos
 * @param queue fila de suspensas que deve ser retirado o elemento task 
 */
void task_resume (task_t *task, task_t **queue)
{
    pode_preempcao = 0;
    //se a fila queue não for nula, retira a tarefa apontada por task dessa fila
    if(queue == NULL)
        return;
    queue_remove((queue_t**)queue, (queue_t*)task);
    task->status = READY;
    queue_append((queue_t**)&ready_task_queue, (queue_t*)task);

    pode_preempcao = 1;

}

int task_wait(task_t *task)
{
    if(task==NULL || task->status == TERMINATED)
        return 1;
    task_suspend(&task->waiting_this_task_queue);
    return task->exitCode;

}

void free_suspended_queue()
{
    struct task_t *queueAux = currentTask_global->waiting_this_task_queue;

    //se a fila não for nula
    while (queue_size((queue_t *)currentTask_global->waiting_this_task_queue) > 0)
    {
        task_resume(queueAux, &currentTask_global->waiting_this_task_queue);
        queueAux = currentTask_global->waiting_this_task_queue;
    }
    
}

//======================================Sincronização====================================================//

/**
 * @brief faz a tarefa atual dormir em um tempo especificado e a coloca na fila de tarefas dormindo
 * 
 * @param t tempo em milisegundos que a tarefa irá dormir
 */
void task_sleep (int t)
{
    if (t <= 0)
        return;
    
    colocando_dormir = 0;
    currentTask_global->wake_up_time = systime() + t;
    task_suspend(&sleeping_task_queue);
    colocando_dormir = 1;

    

}

/**
 * @brief acorda tarefas da fila de tarefas dormindo cujo tempo para dormir já passou
 * 
 */
void wake_sleeping_tasks()
{
    task_t  *taskAux = sleeping_task_queue;
    task_t  *pass_through_queue = sleeping_task_queue;
    for (int i = 0; i < queue_size((queue_t *) sleeping_task_queue); i++)
    {
        pass_through_queue = pass_through_queue->next;

        //se já passou da hora da tarefa acordar, acorda a tarefa e retira ela da fila. Subtrai 1 do iterador pois o tamanha da fila mudou
        if (taskAux->wake_up_time <= systime())
        {
            task_resume(taskAux, &sleeping_task_queue);
            i--;
        }
        taskAux = pass_through_queue;
    }


}

//======================================Tarefas dormindo==================================================//

