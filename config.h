#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h> 
#include <errno.h>
#include <pthread.h>

/* UNIDADE DE TEMPO */
#define UTEMPO 1 // 1 segundo = 1 unidade tempo por enuqanto

/* nome do FIFO do controlador */
#define CONTROLADOR_FIFO "./fifos/ctrl_fifo"

/* nome do FIFO cada cliente. P %d será substituído pelo PID com sprintf */
#define CLIENTE_FIFO "./fifos/cli_%d_fifo"

/* tamanho máximo de cada palavra */
#define TAM_MAX 100

/*tamanho do buffer*/
#define TAM_BUFFER 512

/*Numero máximo de utilizadores*/
#define NUTILIZADORES 30

/*Numero maximo de veiculos*/
#define NMAXVEICULOS 10

/*Estados do cliente*/
#define LIVRE 0
#define ESPERA 1
#define VIAGEM 2

/* estrutura da mensagem correspondente a um pedido cliente -> controlador*/
typedef struct {
    pid_t pid_cliente;
    char username[TAM_MAX];
    char tipo_pergunta[TAM_MAX];
    char comando[TAM_MAX];
} pergunta_t;

/* estrutura da mensagem correspondente a uma resposta controlador -> cliente */
typedef struct {
    char responde[TAM_BUFFER];
    bool criado;
} resposta_t;

typedef struct{
    int atual;
}Time;



typedef struct Agendamento{
    int id;
    int hora;
    char local[TAM_MAX];
    int distancia;
    struct Agendamento *prox;
} Agendamento;

/* estrutura clientes*/
typedef struct  {
    int iUser;
    char nome[TAM_MAX];
    Agendamento *agenda;
    char fName[TAM_MAX];
    int estado;
} Cliente;

/* estrutura veiculos*/
typedef struct{
    int id;
    int idAgenda;
    bool ocupado;
    int iUser;
    int distanciaT;
    char local[TAM_MAX];
    float perPercorrido;
    int fd;
    pid_t pid; //necessario para enviar o controlador enviar o sinal para cancelar
} Veiculo;

typedef struct {
    Cliente *users[NUTILIZADORES];
    int proxIdAgendamento;
    int proxIdUser;
    Veiculo *carros[NMAXVEICULOS];
    int NVEICULOS;
    Time t;
    int cFifo;
    bool ligado;
    pthread_mutex_t lock;
    pthread_t tRecebe;
    pthread_t tTime;
    pthread_t tAdmin;
} ServidorData;

typedef struct{
    ServidorData *sd;
    pergunta_t pergunta;
}AuxiliaP;

typedef struct{
    int cliFifo;
    char fifoName[TAM_MAX];
} AuxiliaCliente;

typedef struct{
    ServidorData *sd;
    int idCarro;
    int iUser;

    char local[TAM_MAX];
    int distancia;
    char fName[TAM_MAX];
}AuxiliaLaunch;