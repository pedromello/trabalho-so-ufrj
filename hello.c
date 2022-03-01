#include<stdio.h>
#include<stdlib.h>
#include <unistd.h> //you need this for linux!
//#include <dos.h> //you need this for Windows!

// ---------------------------- GLOBALS -----------------------------------

#define QUEUE_MAX_SIZE 1000
#define QUANTUM 1

// ---------------------------- PROCESSO -----------------------------------

// Status para o processo
enum status {
    PRONTO,
    EM_EXECUCAO,
    BLOQUEADO,
    FINALIZADO
};

// Funcao para poder imprimir o nome do status
char* getEnumName(enum status s) {
    switch(s) {
        case PRONTO:
            return "PRONTO";
        case EM_EXECUCAO:
            return "EM EXECUCAO";
        case BLOQUEADO:
            return "BLOQUEADO";
        case FINALIZADO:
            return "FINALIZADO";
        default:
            return "UNKNOWN";
    }
}

// PID usado para a criacao do processo seguinte
int PIDs = 1;

// Estrutura do processo
struct process
{
    int pid;
    int ppid;
    int priority;
    enum status state;
    int burstTime;
    int remainingTime;

    int ioDiskTime;
    int ioTapeTime;
    int ioPrinterTime;
    
};

// Cria um processo
struct process createProcess(int priority, int burstTime, int ioDiskTime, int ioTapeTime, int ioPrinterTime) {
    struct process p;
    p.pid = PIDs++;
    p.ppid = 0;
    p.priority = priority;
    p.state = PRONTO;
    p.burstTime = burstTime;
    p.remainingTime = burstTime;
    p.ioDiskTime = ioDiskTime;
    p.ioTapeTime = ioTapeTime;
    p.ioPrinterTime = ioPrinterTime;
    return p;
}

// Cria um processo basico
struct process* createBasicProcess() {
    struct process* p = (struct process*) malloc(sizeof(struct process));

    p->pid = PIDs++;
    p->ppid = 0;
    p->priority = 1;
    p->state = PRONTO;
    p->burstTime = 4;
    p->remainingTime = 4;
    p->ioDiskTime = 2;
    p->ioTapeTime = 0;
    p->ioPrinterTime = 0;
    return p;
}

// Cria um processo filho
struct process createChildProcess(int priority, int parentId, int burstTime, int ioDiskTime, int ioTapeTime, int ioPrinterTime) {
    struct process p;
    p.pid = PIDs++;
    p.ppid = parentId;
    p.priority = priority;
    p.state = PRONTO;
    p.burstTime = burstTime;
    p.remainingTime = burstTime;
    p.ioDiskTime = ioDiskTime;
    p.ioTapeTime = ioTapeTime;
    p.ioPrinterTime = ioPrinterTime;
    return p;
}

// Muda o status do processo
void changeState(struct process *p, enum status newState)
{
    p->state = newState;
}

// "Deleta" um processo deixando ele como finalizado
void deleteProcess(struct process p)
{
    p.state = FINALIZADO;
}

// Imprime o processo
void printProcess(struct process p){
    printf("Processo \n PID: %d \n PPID: %d \n Prioridade: %d \n Estado: %s \n", p.pid, p.ppid, p.priority, getEnumName(p.state));
}

// ---------------------------- QUEUE STRUCT --------------------------------

struct queue {
    int front;
    int rear;
    int size;
    struct process array[QUEUE_MAX_SIZE];
};

struct queue* create_queue() {
    struct queue* q = (struct queue*) malloc(sizeof(struct queue));
    q->front = 0;
    q->rear = 0;
    q->size = 0;
    return q;
}

// Add a process to the end of the queue
void enqueue(struct queue* q, struct process p) {
    if (q->size == QUEUE_MAX_SIZE) {
        printf("Queue is full\n");
        return;
    }
    q->array[q->rear] = p;
    q->rear = (q->rear + 1) % QUEUE_MAX_SIZE;
    q->size++;
}

// Remove a process from the front of the queue
struct process* dequeue(struct queue* q) {
    if (q->size == 0) {
        printf("Queue is empty\n");
        return;
    }
    struct process* p = &q->array[q->front];
    q->front = (q->front + 1) % QUEUE_MAX_SIZE;
    q->size--;
    return p;
}

// is the queue empty?
int is_empty(struct queue* q) {
    return q->size == 0;
}

// is the queue full?
int is_full(struct queue* q) {
    return q->size == QUEUE_MAX_SIZE;
}

// Print the queue
void print_queue(struct queue* q) {
    printf("[");
    for (int i = 0; i < q->size; i++) {
        if(i == q->size - 1) {
            printf("%d", q->array[(q->front + i) % QUEUE_MAX_SIZE].pid);
        } else {
            printf("%d, ", q->array[(q->front + i) % QUEUE_MAX_SIZE].pid);
        }
        
    }
    printf("]\n");
}

// ---------------------------- ESCALONADOR -----------------------------------


// Estrutura do escalonador
struct scheduler {
    int quantum;
    int time;
    int timeExecutingProcess;
    struct process* currentProcess;
    struct queue* highPriorityQueue;
    struct queue* lowPriorityQueue;
    struct queue* IoDiskQueue;
    struct queue* IoTapeQueue;
    struct queue* IoPrinterQueue;
};

struct scheduler* create_scheduler() {
    struct scheduler* s = (struct scheduler*) malloc(sizeof(struct scheduler));
    s->quantum = QUANTUM;
    s->time = 0;
    s->timeExecutingProcess = 0;
    s->highPriorityQueue = create_queue();
    s->lowPriorityQueue = create_queue();
    s->IoDiskQueue = create_queue();
    s->IoTapeQueue = create_queue();
    s->IoPrinterQueue = create_queue();
    return s;
}

void getProcessFromQueue (struct scheduler* s, struct queue* q){
    s->currentProcess = dequeue(q);
    changeState(s->currentProcess, EM_EXECUCAO);
    s->timeExecutingProcess = 0;
}

void getNextProcess(struct scheduler* s){
    if(!is_empty(s->highPriorityQueue)){
        puts("Pegando processo da fila de alta prioridade");
        getProcessFromQueue(s, s->highPriorityQueue);
    } else if(!is_empty(s->lowPriorityQueue)){
        puts("Pegando processo da fila de baixa prioridade");
        getProcessFromQueue(s, s->lowPriorityQueue);
    }
}

void sendProcessToQueue(struct queue* q, struct process p, enum status newState){
    changeState(&p, newState);
    enqueue(q, p);
}

void sendCurrentProcessToQueue(struct scheduler* s, struct queue* q, struct process p, enum status newState){
    sendProcessToQueue(q, p, newState);
    s->currentProcess = NULL;
}

void currentProcessDone(struct scheduler* s){
    changeState(s->currentProcess, FINALIZADO);
    s->currentProcess = NULL;
}

void scheaduleOut(struct scheduler* s){
    if(s->currentProcess->remainingTime == 0){
        puts("Processo finalizado");
        currentProcessDone(s);
    } else if(s->currentProcess->ioDiskTime == s->timeExecutingProcess){
        puts("Processo em IO no disco");
        sendCurrentProcessToQueue(s, s->IoDiskQueue, *s->currentProcess, BLOQUEADO);
    } else if(s->currentProcess->ioTapeTime == s->timeExecutingProcess){
        puts("Processo em IO na fita");
        sendCurrentProcessToQueue(s, s->IoTapeQueue, *s->currentProcess, BLOQUEADO);
    } else if(s->currentProcess->ioPrinterTime == s->timeExecutingProcess){
        puts("Processo em IO na impressora");
        sendCurrentProcessToQueue(s, s->IoPrinterQueue, *s->currentProcess, BLOQUEADO);
    } else if(s->timeExecutingProcess == s->quantum){
        puts("Quantum finalizado");
        sendCurrentProcessToQueue(s, s->lowPriorityQueue, *s->currentProcess, PRONTO);
    }
}

void clock(struct scheduler* s) {
    s->time++;

    if(s->currentProcess){
        printProcess(*s->currentProcess);
        s->currentProcess->remainingTime--;
        s->timeExecutingProcess++;
        scheaduleOut(s);
    }
    
    // Pega o processo na fila de prontos (transformar em funcao a parte)
    if(!s->currentProcess){
        getNextProcess(s);
    }

    sleep(1);

}



int main()
{
    struct scheduler* s = create_scheduler();
    struct process* p1 = createBasicProcess();
    struct process* p2 = createBasicProcess();
    struct process* p3 = createBasicProcess();
    struct process* p4 = createBasicProcess();
    struct process* p5 = createBasicProcess();
    struct process* p6 = createBasicProcess();
    struct process* p7 = createBasicProcess();
    struct process* p8 = createBasicProcess();
    struct process* p9 = createBasicProcess();
    struct process* p10 = createBasicProcess();

    enqueue(s->highPriorityQueue, *p1);
    enqueue(s->highPriorityQueue, *p2);
    enqueue(s->highPriorityQueue, *p3);
    enqueue(s->highPriorityQueue, *p4);
    enqueue(s->highPriorityQueue, *p5);
    enqueue(s->highPriorityQueue, *p6);
    enqueue(s->highPriorityQueue, *p7);
    enqueue(s->highPriorityQueue, *p8);
    enqueue(s->highPriorityQueue, *p9);
    enqueue(s->highPriorityQueue, *p10);

    while (!is_empty(s->highPriorityQueue))
    {
        clock(s);
        puts("High Priority Queue:");
        print_queue(s->highPriorityQueue);
        puts("Low Priority Queue:");
        print_queue(s->lowPriorityQueue);
        puts("IoDisk Queue:");
        print_queue(s->IoDiskQueue);
        puts("IoTape Queue:");
        print_queue(s->IoTapeQueue);
        puts("IoPrinter Queue:");
        print_queue(s->IoPrinterQueue);
        printf("\n");

    }
    

    return 0;
}