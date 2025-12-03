#include "config.h"

int fdCliente = -1;

int trataSinal(int i){
    if(i == SIGUSR1){
        //controlador manda cancelar 
        if(fdCliente != -1){
            resposta_t msg;
            strcpy(msg.responde,"[VEICULO] Viagem canelado pelo admin");
            msg.criado=false;
            write(fdCliente, &msg, sizeof(msg));
            close(fdCliente);
        }
    }
    printf("CANCELADO\n");
    fflush(stdout);
    exit(EXIT_SUCCESS);
}

//veiculo <local> <distancia> <fifoCliente>
int main(int agrc, char *argv[]){
    if(agrc != 4){
        printf("[VEICULO] ERRO: A faltar argumentos\n");
        return -1;
    }

    char fifo_cliente[50];
    char local[50];
    int distancia = atoi(argv[2]);
    strcpy(local, argv[1]);
    strcpy(fifo_cliente, argv[3]);
    
    
    //abre o fifo do cliente
    int fdCli = open(fifo_cliente, O_WRONLY);
    if(fdCli == -1){
        printf("[VEICULO] ERRO ao tentar abrir o FIFO do cliente");
        return -1;
    }
    printf("Consegui abrir o fifo do cliente!!!!!\n");
    
    //sinal SIGURS1
    if(signal(SIGUSR1, trataSinal) == SIG_ERR){
        perror("Erro no SIGURS1\n");
        return -1;
    }
    signal(SIGINT, SIG_IGN); // ingora ctrl+c


    resposta_t msg;
    //INICIO DA VIAGEM
    snprintf(msg.responde, TAM_MAX, "O veiculo a iniciar viagem apartir do local '%s'", local);
    msg.criado=true;
    write(fdCli, &msg, sizeof(msg));

    //printf("INICIO\n");
    //fflush(stdout);
    int kms = 0;
    int lastReport = 0;
    while(kms < distancia){
        sleep(1); // 1 segundo = 1 unidade tempo por enuqanto 
        kms++;

        int distPerc = (kms*100) / distancia;
        while(distPerc >= lastReport + 10){
            lastReport +=10;
            printf("PERCORREU %d\n", lastReport);
            //fflush(stdout);
        }        
    }
    //Logica da viagem em si
    
    
    //FIM DA VIAGEM
    //printf("FIM\n");
    //fflush(stdout);
    
    snprintf(msg.responde, TAM_MAX, "O veiculo chegou ao local '%s'. Viagem completa", local);
    write(fdCli, &msg, sizeof(msg));
    close(fdCli);
    return 0;

}