#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h> 
#include <errno.h>
#include <pthread.h>

#define UTEMPO 1 // 1 segundo = 1 unidade tempo por enuqanto
/* nome do FIFO do controlador */
#define CONTROLADOR_FIFO "./fifos/ctrl_fifo"

/* nome do FIFO cada cliente. P %d será substituído pelo PID com sprintf */
#define CLIENTE_FIFO "./fifos/cli_%d_fifo"

/* tamanho máximo de cada palavra */
#define TAM_MAX 100

/*Numero máximo de utilizadores*/
#define NUTILIZADORES 30

/*Numero maximo de veiculos*/
#define NVEICULOS 10

/* estrutura da mensagem correspondente a um pedido cliente -> controlador*/
typedef struct {
    pid_t pid_cliente;
    char username[TAM_MAX];
    char tipo_pergunta[25];
    char comando[25];
} pergunta_t;

/* estrutura da mensagem correspondente a uma resposta controlador -> cliente */
typedef struct {
    char responde[TAM_MAX];
    bool criado;
} resposta_t;

typedef struct{
    int atual;
}Time;

typedef struct Agendamento{
    int id;
    int hora;
    char local[50];
    int distancia;
    struct Agendamento *prox;
} Agendamento;

/* estrutura clientes*/
typedef struct  {
    char nome[50];
    Agendamento *agenda;
    char fName[50];
} Cliente;

/* estrutura veiculos*/
typedef struct{
    int id;
    bool ocupado;
    int distanciaT;
    char local[50];
    float perPercorrido;
    int percorrido;
} Veiculo;

typedef struct {
    Cliente *users[NUTILIZADORES];
    int proxIdAgendamento;
    Veiculo *carros[NVEICULOS];
    Time t;
    int cFifo;
    bool ligado;
    pthread_mutex_t lock;
    pthread_t tRecebe;
    pthread_t tTime;
} ServidorData;

typedef struct{
    ServidorData *sd;
    pergunta_t pergunta;
}AuxiliaP;

typedef struct{
    int cliFifo;
    char fifoName[50];
} AuxiliaCliente;