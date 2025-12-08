#include "config.h"

int fdCliente = -1;

void trataSinal(int i){
    //controlador manda cancelar 
    if(fdCliente != -1){
        resposta_t msg;
        strcpy(msg.responde,"[VEICULO] Viagem canelado pelo admin");
        msg.criado=false;
        write(fdCliente, &msg, sizeof(msg));
        close(fdCliente);
    }
    exit(EXIT_SUCCESS);
}

//veiculo <local> <distancia> <fifoCliente>
int main(int agrc, char *argv[]){
    if(agrc != 4){
        printf("[VEICULO] ERRO: A faltar argumentos\n");
        return -1;
    }

    char fifo_cliente[TAM_MAX];
    char local[TAM_MAX];
    int distancia = atoi(argv[2]);
    strcpy(local, argv[1]);
    local[TAM_MAX-1]='\0';
    strcpy(fifo_cliente, argv[3]);
    fifo_cliente[TAM_MAX-1]='\0';
    
    
    //abre o fifo do cliente
    fdCliente = open(fifo_cliente, O_WRONLY);
    if(fdCliente == -1){
        printf("[VEICULO]ERRO ao tentar abrir o FIFO do cliente");
        return -1;
    }
    printf("[VEICULO]Consegui abrir o fifo do cliente!!!!!\n");
    
    //sinal SIGURS1
    if(signal(SIGUSR1, trataSinal) == SIG_ERR){
        perror("Erro no SIGURS1\n");
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, SIG_IGN); // ingora ctrl+c


    resposta_t msg;
    //INICIO DA VIAGEM
    snprintf(msg.responde, TAM_MAX, "[VEICULO]Inicio da viagem para %s",local);
    msg.criado=true;
    write(fdCliente, &msg, sizeof(msg));

    int kms = 0;
    int lastReport = 0;
    while(kms < distancia){
        sleep(UTEMPO);  
        kms++;

        int distPerc = (kms*100) / distancia;
        while(distPerc >= lastReport + 10){
            lastReport +=10;
            printf("[VEICULO]Percorri %d\n", lastReport);
            fflush(stdout);
        }        
    }
    
    snprintf(msg.responde, TAM_MAX, "[VEICULO]Terminei a viagem");
    write(fdCliente, &msg, sizeof(msg));

    printf("[VEICULO]Terminei a viagem\n"); 
    fflush(stdout);
    close(fdCliente);
    return 0;
}