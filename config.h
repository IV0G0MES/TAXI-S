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

/* nome do FIFO do controlador */
#define CONTROLADOR_FIFO "/tmp/ctrl_fifo"

/* nome do FIFO cada cliente. P %d será substituído pelo PID com sprintf */
#define CLIENTE_FIFO "/tmp/cli_%d_fifo"

/* tamanho máximo de cada palavra */
#define TAM_MAX 50

/*Numero máximo de utilizadores*/
#define NUTILIZADORES 30

/*Numero maximo de veiculos*/
#define NVEICULOS 10;

/* estrutura da mensagem correspondente a um pedido cliente -> controlador*/
typedef struct {
    pid_t pid_cliente;
    char username[TAM_MAX];
} pergunta_t;

/* estrutura da mensagem correspondente a uma resposta controlador -> cliente */
typedef struct {
    bool confirmUser;
    char palavra[TAM_MAX];
} resposta_t;

/* estrutura clientes*/
struct Cliente {
    char nome[50];
} ;