#define _GNU_SOURCE
#include "config.h"

ServidorData sd;
int c_fifo_fd;       /* descritores de ficheiros (pipes) */

//pthread_mutex_lock lock = PTHREAD_MUTEX_INITIALIZER;
/* esta função vai atender o sinal SIGINT para terminar o servidor */
void trataSig(int i) {
    fprintf(stderr, "\nControlador a terminar (interrompido via teclado)\n\n");
    close(sd.cFifo);
    unlink(CONTROLADOR_FIFO);
    exit(EXIT_SUCCESS);
    /* para terminar o processo */
}

void preencheVeiculos(ServidorData *sd){
    for(int i=0;i<NVEICULOS; i++){
        Veiculo *novo_veiculo=(Veiculo*)malloc(sizeof(Veiculo));
            if(novo_veiculo== NULL){
                printf("Nao foi possivel criar o veiculo");
                exit(1);
            }
            novo_veiculo->id=i;
            novo_veiculo->ocupado=false;
            novo_veiculo->distancia=0;
            strcpy(novo_veiculo->local,"");
            sd->carros[i]=novo_veiculo;
    }
    printf("Vetor veiculos preenchido");
}

void listaCarros(ServidorData *sd){
    pthread_mutex_lock(&sd->lock);
    printf("\n----- Frota-----\n");
    for(int i=0; i<NVEICULOS; i++){
        printf("Veiculo %d:\nId: %d\nOcupado: %d\nLocal: %s\nDistancia: %d\n",i+1 ,sd->carros[i]->id,sd->carros[i]->ocupado,sd->carros[i]->local,sd->carros[i]->distancia);
    }
    pthread_mutex_unlock(&sd->lock);
}

int verificaUser(char *nome,Cliente **a){
    for(int i=0;i<NUTILIZADORES;i++){
        if(a[i]!=NULL){
            if(strcmp(nome,a[i]->nome)==0){
                printf("Utilizador ja existente\n");
                return 1;
            }
        }
    }
    Cliente *ptr;
    for(int i=0; i<NUTILIZADORES;i++){
        if(a[i]==NULL){
            ptr=(Cliente*)malloc(sizeof(Cliente));
            if(ptr== NULL){
                printf("Nao foi possivel criar o cliente");
                return 2;
            }
            a[i]=ptr;
            strcpy(a[i]->nome,nome);
            printf("Cliente criado com sucesso\n");
            return 0;
        }
    }

    printf("Nao ha espaco para mais clientes\n");
    return 3;
}

void lista_users(ServidorData *sd){
    for (int i=0;i<30;i++){
        if(sd->users[i]!=NULL){
            printf("Utilizador N %d\nNome: %s\nNome do fifo: %s\n",i,sd->users[i]->nome,sd->users[i]->fName);
        }
        else{
            break;
        }
    }
}

void remove_users(ServidorData *sd,char* nome){
    for(int i=0;i<NUTILIZADORES;i++){
        if(sd->users[i]!=NULL){
            if(strcmp(sd->users[i]->nome,nome)==0){
                sd->users[i]=NULL;
                break;
            }
        }
    }
}

int agenda(ServidorData *sd, char* nome,int hora,char *local, int distancia){
    Agendamento *nova_agenda=malloc(sizeof(Agendamento));
    nova_agenda->hora=hora;
    strcpy(nova_agenda->local,local);
    nova_agenda->distancia=distancia;
    nova_agenda->prox=NULL;

    
    for(int i=0;i<NUTILIZADORES;i++){
        if(sd->users[i]==NULL){
            continue;
        }
        if(strcmp(sd->users[i]->nome,nome)==0){
            if(sd->users[i]->agenda==NULL){
                sd->users[i]->agenda=nova_agenda;
            }
            else{
                Agendamento *agendamento_atual=sd->users[i]->agenda;
                while (agendamento_atual->prox!=NULL){
                    agendamento_atual=agendamento_atual->prox;
                }
                agendamento_atual->prox=nova_agenda;
            }
            return 0;
        }
    }
    printf("Cliente nao encontrado");
    return 1;
}

void *trataPedido(void *arg){
    AuxiliaP *aux=(AuxiliaP *) arg;
    ServidorData *sd=aux->sd;
    pergunta_t pergunta=aux->pergunta; 
    free(aux);

    resposta_t resposta;
    int cFifoD;
    char ClienteFifo[50];
    sprintf(ClienteFifo,CLIENTE_FIFO,pergunta.pid_cliente);

    if(strcmp(pergunta.tipo_pergunta,"LOGIN")==0){
        int verifica;
        verifica=verificaUser(pergunta.username,sd->users);
        if(verifica==0){
            strcpy(resposta.responde,"Cliente criado com sucesso\n");
            resposta.criado=1;
        }else if(verifica==1){
            strcpy(resposta.responde,"Cliente ja existe pelo que nao foi criado\n");
            resposta.criado=0;
        }else if(verifica==2){
            strcpy(resposta.responde,"Criacao de cliente deu erro\n");
            resposta.criado=0;
        }else{
            strcpy(resposta.responde,"Cliente nao criado\nMaximo de clientes atingido\n");
            resposta.criado=0;
        }
    }else{
        if(strncmp(pergunta.comando,"agendar ",8)==0){
            int hora;
            char local[50];
            int distancia;
            if(sscanf(pergunta.comando+8,"%d %s %d",&hora,local,&distancia)==3){
                printf("Hora: %d\nLocal: %s\nDistancia: %d\n",hora,local,distancia);

                fflush(stdout);

                if(agenda(sd,pergunta.username,hora,local,distancia)==0){
                    strcpy(resposta.responde,"Agendamento efetuado com sucesso\n");
                }else{
                    strcpy(resposta.responde,"Agendamento nao efetuado, erro no servidor\n");
                }
            }else{
                strcpy(resposta.responde,"Sintaxe do comando agendar incorreta\n");
            }

        }else if(strcmp(pergunta.comando,"consultar")==0){
            printf("Estou na thread\n");
        }else if(strncmp(pergunta.comando,"cancelar ",9)){

        }else if(strncmp(pergunta.comando,"entrar ",7)){

        }else if(strcmp(pergunta.comando,"sair")==0){

        }else if(strcmp(pergunta.comando,"terminar")==0){

        }else{
            strcpy(resposta.responde,"Comando Errado\n");
        }
    }


    /*if(strcmp(comando,"agendar")==0){ //falta apanhar hora,local, distancia
            
        }else if(strcmp(comando,"consultar")==0){

        }else if(strcmp(comando,"cancelar")==0){//falta apanhar id

        }else if(strcmp(comando,"entrar")==0){//falta apanhar destino
        
        }else if(strcmp(comando,"sair")==0){

        }else if(strcmp(comando,"terminar")==0){
            break;

        }else{
            printf("Comando errado\n");
        }
            */



        //FALTA PASSO 3- ENVIAR RESPOSTA PARA O CLIENTE
        cFifoD=open(ClienteFifo,O_WRONLY);
        write(cFifoD,&resposta,sizeof(resposta));
    
}

void *atendeCliente(void *arg){
    ServidorData *sd = (ServidorData *) arg;
    pergunta_t pergunta;
    int res;

    fprintf(stderr, "[Thread] a iniciar escuta do FIFO\n");

    sd->cFifo = open(CONTROLADOR_FIFO, O_RDWR);
    if(sd->cFifo == -1){
        perror("[Thread] Erro ao abrir FIFO\n");
        pthread_exit(NULL);
    }

    while(sd->ligado){  //QUANDO DESLIGAR SAI DO WHILE
        res = read(sd->cFifo, &pergunta, sizeof(pergunta));

        if(res!=sizeof(pergunta)){
            fprintf(stderr,"Erro na rececao de dados/n");
        }
        else{
            AuxiliaP *nova_tarefa=malloc(sizeof(AuxiliaP));
            
            nova_tarefa->sd=sd;
            nova_tarefa->pergunta=pergunta;

            pthread_t tPedido;
            if(pthread_create(&tPedido, NULL, trataPedido, nova_tarefa) != 0){
                perror("[ERRO] Não foi possivel criar Pedido");
                free(nova_tarefa);
            }

        }

        
        /*if(res > 0){
            if(res < sizeof(pergunta)){
                
                fprintf(stderr, "\nLongin recebido: %s", pergunta.username);

                pthread_mutex_lock(&sd->lock); // bloqueia 

                x = verificaUser(pergunta.username, sd->users);

                if(x == 1){
                    resp.confirmUser = true;
                    sprintf(c_fifo_fname, CLIENTE_FIFO, pergunta.pid_cliente);
                }

                pthread_mutex_unlock(&sd->lock); // desbloqueia
            }
        }*/
    }
}

int main(){
    //ServidorData sd;
    
    char comando[50];
    int res;

    sd.ligado=true;

    preencheVeiculos(&sd);

    printf("\nControlador Iniciado\n");
    if (signal(SIGINT,trataSig) == SIG_ERR) {
        perror("\nNao foi possivel configurar o sinal SIGINT\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "\nSinal SIGINT configurado");
    res = mkfifo(CONTROLADOR_FIFO, 0777);
    if (res == -1) {
        perror("\nmkfifo do FIFO do servidor deu erro");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "\nFIFO servidor criado");
    /* prepara FIFO do servidor */
    /* abertura read+write -> evita o comportamento de ficar */
    /* bloqueado no open. a execução prossegue e as*/
    /* operações read/write (neste caso APENAS READ)*/
    /* continuam bloqueantes (mais fácil de gerir)*/

    sd.cFifo = open(CONTROLADOR_FIFO, O_RDWR);
    /* bloqueante */
    if (sd.cFifo == -1) {
        perror("\nErro ao abrir o FIFO do servidor (RDWR/blocking)\n");
        exit(EXIT_FAILURE);
    }
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags);

    printf("\nControlador aberto...\n");
    
    if(pthread_create(&sd.tRecebe, NULL, &atendeCliente, &sd) != 0){
        exit(EXIT_FAILURE);
    }

    while (sd.ligado) {
        printf("Comando: ");
        if(fgets(comando,sizeof(comando),stdin)==NULL)
            break;
        comando[strcspn(comando, "\n")] = '\0';
        printf("\n.%s.\n",comando);
        if(strcmp(comando, "listar") == 0){
            printf("------listar-------\n");
            lista_users(&sd);
            printf("-------------------\n");

        }else if(strcmp(comando,"utiliz")==0){
            printf("Estou no utiliz\n");
        }else if(strcmp(comando,"frota")==0){
            listaCarros(&sd);
        }else if(strncmp(comando,"cancelar ",9)==0){//falta apanhar o id
            printf("Estou no cancelar\n");
        }else if(strcmp(comando,"km")==0){
            printf("Estou no km\n");
        }else if(strcmp(comando,"hora")==0){
            printf("Estou na hora\n");
        }else if(strcmp(comando,"terminar")==0){
            sd.ligado=false;
            printf("Estou no sair\n");
           break;
        }else {
            printf("Comando errado\n");
        }
    }
        usleep(1000000);

    /* fim do ciclo principal do servidor */
    /* em principio não chega a este ponto - sai via SIGINT */
    printf("\nVou fechar!!\n");
    
    close(sd.cFifo);
    unlink(CONTROLADOR_FIFO);
    return 0;
} /* fim da função main do servidor */