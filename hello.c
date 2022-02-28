#include<stdio.h>

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
};

// Cria um processo
struct process createProcess(int priority)
{
    struct process p;
    p.pid = PIDs++;
    p.ppid = 0;
    p.priority = priority;
    p.state = PRONTO;

    return p;
}

// Cria um processo filho
struct process createProcessWithParent(int priority, int parentId)
{
    struct process p;
    p.pid = PIDs++;
    p.ppid = parentId;
    p.priority = priority;
    p.state = PRONTO;
    return p;
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

int main()
{
    struct process p;
    p = createProcess(1);    
    printProcess(p);
    return 0;
}