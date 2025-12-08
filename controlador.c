#define _GNU_SOURCE
#include "config.h"
//ENTREGA ATÉ AO DIA 13 DE DEZEMBRO
//SINAL
void trataSig(int i) {
    printf("Para terminar o Controlador utilize o comando terminar\n");
    fflush(stdout);
}

//INICIALIZAÇÕES
int getVA(ServidorData *sd){
    char *va=getenv("NVEICULOS");
    if (va != NULL) {
        if (atoi(va) < 0) {
            fprintf(stderr, "Erro - A variavel de ambiente NVEICULOS apresenta um valor invalido\n");
            return -1;
        } else if(atoi(va) > 10){
            sd->NVEICULOS=10;
            fprintf(stderr, "A variavel de ambiente NVEICULOS apresenta um valor superior ao máximo permitido, valor alterado para 10\n");
        }
        else {
            sd->NVEICULOS = atoi(va);
        }
    } else {
        fprintf(stderr, "Erro - A variavel de ambiente necessaria NVEICULOS nao se encontra definida\n");
        return -1;
    }
    printf("Variavel de ambiente recebida: NVEICULOS - %d",sd->NVEICULOS);
}

void preencheVeiculos(ServidorData *sd){
    for(int i=0;i<sd->NVEICULOS; i++){
        Veiculo *novo_veiculo=(Veiculo*)malloc(sizeof(Veiculo));
            if(novo_veiculo== NULL){
                printf("Nao foi possivel criar o veiculo");
                exit(1);
            }
            novo_veiculo->id=i;
            novo_veiculo->ocupado=false;
            novo_veiculo->distanciaT=0;
            strcpy(novo_veiculo->local,"");
            novo_veiculo->perPercorrido=0.0;
            sd->carros[i]=novo_veiculo;
    }
    printf("Vetor veiculos preenchido");
}

void preencheClientes(ServidorData *sd){
    for(int i=0;i<NUTILIZADORES; i++){
        sd->users[i]=NULL;
    }
}

void preencheSd(ServidorData *sd){
    sd->ligado=true;
    sd->proxIdAgendamento=1;
    pthread_mutex_init(&sd->lock, NULL);
    getVA(sd);
    preencheVeiculos(sd);
    preencheClientes(sd);
    sd->t.atual=0;
}

//FUNÇOES DO CLIENTE
int verificaUser(ServidorData *sd,char *nome,char *nomeFifo){
    pthread_mutex_lock(&sd->lock);
    for(int i=0;i<NUTILIZADORES;i++){
        if(sd->users[i]!=NULL){
            if(strcmp(sd->users[i]->nome,nome)==0){
                printf("Utilizador ja existente\n");
                pthread_mutex_unlock(&sd->lock);
                return 1;
            }
        }
    }
    Cliente *ptr;
    for(int i=0; i<NUTILIZADORES;i++){
        if(sd->users[i]==NULL){
            ptr=(Cliente*)malloc(sizeof(Cliente));
            if(ptr== NULL){
                printf("Nao foi possivel criar o cliente");
                pthread_mutex_unlock(&sd->lock);
                return 2;
            }
            strcpy(ptr->nome,nome);
            ptr->agenda=NULL;
            strcpy(ptr->fName,nomeFifo);
            sd->users[i]=ptr;
            printf("Cliente criado com sucesso\n");
            pthread_mutex_unlock(&sd->lock);
            return 0;
        }
    }
    printf("Nao ha espaco para mais clientes\n");
    pthread_mutex_unlock(&sd->lock);
    return 3;
}

int agenda(ServidorData *sd, char* nome,int hora,char *local, int distancia){
    Agendamento *nova_agenda=malloc(sizeof(Agendamento));
    nova_agenda->id=sd->proxIdAgendamento;
    nova_agenda->hora=hora;
    strcpy(nova_agenda->local,local);
    nova_agenda->distancia=distancia;
    nova_agenda->prox=NULL;
    sd->proxIdAgendamento++;
    
    pthread_mutex_lock(&sd->lock);

    for(int i=0;i<NUTILIZADORES;i++){
        if(sd->users[i]!=NULL && strcmp(sd->users[i]->nome,nome)==0){
            if(sd->users[i]->agenda==NULL){
                sd->users[i]->agenda=nova_agenda;
                sd->users[i]->estado=ESPERA;
            }
            else{
                Agendamento *agendamento_atual=sd->users[i]->agenda;
                while (agendamento_atual->prox!=NULL){
                    agendamento_atual=agendamento_atual->prox;
                }
                agendamento_atual->prox=nova_agenda;
                sd->users[i]->estado=ESPERA;
            }
            pthread_mutex_unlock(&sd->lock);
            return 0;
        }
    }
    free(nova_agenda);
    printf("Cliente nao encontrado");
    pthread_mutex_unlock(&sd->lock);
    return 1;
}

int cancelar(ServidorData *sd,char*nome,int id){
    pthread_mutex_lock(&sd->lock);
    for(int i=0;i<NUTILIZADORES;i++){
        if(sd->users[i]!=NULL && strcmp(sd->users[i]->nome,nome)==0){
            if(id==0){
                Agendamento *aux=sd->users[i]->agenda;
                while(aux!=NULL){
                    Agendamento *aux2 = aux->prox;
                    free(aux);
                    aux = aux2;
                }
                sd->users[i]->agenda=NULL;
                pthread_mutex_unlock(&sd->lock);
                return 2;
            }else{
                Agendamento *atual = sd->users[i]->agenda;
                Agendamento *anterior = NULL;
                while (atual!=NULL){
                    if(atual->id==id){
                        if(anterior==NULL){
                            sd->users[i]->agenda=NULL;
                        }else{
                            anterior->prox=atual->prox;
                        }
                        free(atual);
                        pthread_mutex_unlock(&sd->lock);
                        return 0;
                    }
                    anterior=atual;
                    atual=atual->prox;
                }
                pthread_mutex_unlock(&sd->lock);
                return 1;
            }
        }
    }
    pthread_mutex_unlock(&sd->lock);
    return 3;
}

void lista_agenda(ServidorData *sd,char *nome,int cFifoD){
    resposta_t resposta;
    pthread_mutex_lock(&sd->lock);
    for(int i=0;i<NUTILIZADORES;i++){
        if(sd->users[i]!=NULL && strcmp(sd->users[i]->nome,nome)==0){
            if(sd->users[i]->agenda!=NULL){
                Agendamento *agenda_cliente=sd->users[i]->agenda;
                while (agenda_cliente!=NULL){
                    snprintf(resposta.responde,sizeof(resposta.responde),"Id Agendamento: %d\nHora: %d\nLocal: %s\nDistancia: %d\n\n",agenda_cliente->id,agenda_cliente->hora,agenda_cliente->local,agenda_cliente->distancia);
                    write(cFifoD,&resposta,sizeof(resposta));
                    agenda_cliente=agenda_cliente->prox;
                }
            }else{
                strcpy(resposta.responde,"O utilizador nao tem nada agendado\n");
            }
            strcpy(resposta.responde,"FIM");
            write(cFifoD,&resposta,sizeof(resposta));
            pthread_mutex_unlock(&sd->lock);
            return;
        }
    }
    strcpy(resposta.responde,"Utilizador nao encontrado\n");
    write(cFifoD,&resposta,sizeof(resposta));
    strcpy(resposta.responde,"FIM");
    write(cFifoD,&resposta,sizeof(resposta));
    pthread_mutex_unlock(&sd->lock);
}

//FUNÇOES DO CONTROLADOR
int remove_users(ServidorData *sd,char* nome){
    pthread_mutex_lock(&sd->lock);
    for(int i=0;i<NUTILIZADORES;i++){
        if(sd->users[i]!=NULL && strcmp(sd->users[i]->nome,nome)==0){
            Agendamento *aux = sd->users[i]->agenda;
            while(aux != NULL){
                Agendamento *prox = aux->prox;
                free(aux);
                aux = prox;
            }
            free(sd->users[i]);
            sd->users[i]=NULL;
            pthread_mutex_unlock(&sd->lock);
            return 0;
        }
    }
    pthread_mutex_unlock(&sd->lock);
    return 1;
}

int cancelar_admin(ServidorData *sd, int id) {
    pthread_mutex_lock(&sd->lock);
    if (id == 0) {
        for (int i = 0; i < NUTILIZADORES; i++) {
            if (sd->users[i] != NULL) {
                Agendamento *aux = sd->users[i]->agenda;
                while (aux != NULL) {
                    Agendamento *aux2 = aux->prox;
                    free(aux);
                    aux = aux2;
                }
                sd->users[i]->agenda = NULL;
            }
        }
        pthread_mutex_unlock(&sd->lock);
        return 2;
    }
    for (int i = 0; i < NUTILIZADORES; i++) {
        if (sd->users[i] != NULL) {
            Agendamento *atual = sd->users[i]->agenda;
            Agendamento *anterior = NULL;
            while (atual != NULL) {
                if (atual->id == id) {
                    if (anterior == NULL) {
                        sd->users[i]->agenda = atual->prox;
                    } else {
                        anterior->prox = atual->prox;
                    }
                    free(atual);
                    pthread_mutex_unlock(&sd->lock);
                    return 0;
                }
                anterior = atual;
                atual = atual->prox;
            }
        }
    }
    pthread_mutex_unlock(&sd->lock);
    return 1;
}

void lista_users(ServidorData *sd){
    pthread_mutex_lock(&sd->lock);
    for (int i=0;i<NUTILIZADORES;i++){
        if(sd->users[i]!=NULL){
            printf("Utilizador N %d\nNome: %s\nEstado do Cliente: %s\nNome do fifo: %s\n",i,sd->users[i]->nome,sd->users[i]->estado,sd->users[i]->fName);
        }
    }
    pthread_mutex_unlock(&sd->lock);
}

void listaCarros(ServidorData *sd){
    pthread_mutex_lock(&sd->lock);
    printf("\n----- Frota-----\n");
    for(int i=0; i<sd->NVEICULOS; i++){
        printf("Veiculo %d:\nId: %d\nOcupado: %d\nLocal: %s\nDistancia: %d\n",i+1 ,sd->carros[i]->id,sd->carros[i]->ocupado,sd->carros[i]->local,sd->carros[i]->distanciaT);
    }
    pthread_mutex_unlock(&sd->lock);
}

void lista_agenda_admin(ServidorData *sd){
    pthread_mutex_lock(&sd->lock);

    for(int i=0;i<NUTILIZADORES;i++){
        if(sd->users[i]!=NULL){
            if(sd->users[i]->agenda!=NULL){
                printf("Nome do Utilizador: %s\n",sd->users[i]->nome);
                Agendamento *agenda_cliente=sd->users[i]->agenda;
                while (agenda_cliente!=NULL){
                    printf("Id Agendamento: %d\nHora: %d\nLocal: %s\nDistancia: %d\n\n",agenda_cliente->id,agenda_cliente->hora,agenda_cliente->local,agenda_cliente->distancia);
                    agenda_cliente=agenda_cliente->prox;
                }
            }
            fflush(stdout);
        }
    }
    pthread_mutex_unlock(&sd->lock);
}

int checkAvailability(ServidorData *sd){
    for(int i=0;i<sd->NVEICULOS;i++){
        if(sd->carros[i]->ocupado==false){
            return sd->carros[i]->id;
        }
    }
    return -1;
}

void *lauchCar(void *arg){
    AuxiliaLaunch *aux=(AuxiliaLaunch *) arg;
    ServidorData *sd=aux->sd;
    int idCarro=aux->idCarro; 
    int iUser=aux->iUser;

    pthread_mutex_lock(&sd->lock);

    char local[50];
    strcpy(local,aux->local);
    int distancia=aux->distancia;
    char fName[50];
    strcpy(fName,aux->fName);
    free(aux);
    
    pthread_mutex_unlock(&sd->lock);

    int fd[2];
    //pipes: 1-escrita  0-leitura
    if(pipe(fd)==-1){
        perror("Erro ao criar o canal de comunicacao com o veiculo");
        return NULL;
    }

    pid_t pid = fork();
    if(pid == -1){
        perror("Erro a iniciar o veiculo"); // erro a criar processo
        return NULL;
    }
    if(pid==0){//filho
        close(fd[0]);
        close(STDOUT_FILENO);
        dup(fd[1]);
        close(fd[1]);

        char distanciaStr[50];
        sprintf(distanciaStr,"%d",distancia);

        execlp("./veiculo","veiculo",local,distanciaStr,fName,NULL);
        perror("Ocorreu um erro na execucao do veiculo\n");
        exit(EXIT_FAILURE);
    }
    else{//pai
        close(fd[1]);//fecha escrita
        int flags = fcntl(fd[0], F_GETFL, 0); //obter as flags que o pipe tem
        fcntl(fd[0], F_SETFL, flags | O_NONBLOCK); //pegar nas flags antigas e adicionar o nonblock

        pthread_mutex_lock(&sd->lock);

        sd->carros[idCarro]->fd=fd[0];
        sd->carros[idCarro]->pid=pid;
        sd->carros[idCarro]->ocupado=true;
        sd->carros[idCarro]->perPercorrido=0;
        sd->carros[idCarro]->iUser=iUser;
        strcpy(sd->carros[idCarro]->local, local);
        sd->users[iUser]->estado=VIAGEM;

        pthread_mutex_unlock(&sd->lock);

        printf("[CONTROLADOR] Veículo lançado com PID %d\n", pid);
    }
    return NULL;
}

void checkAgendas(ServidorData *sd){
    pthread_mutex_lock(&sd->lock);
    for(int i=0; i<NUTILIZADORES;i++){
        if(sd->users[i]!=NULL){
            Agendamento *agenda=sd->users[i]->agenda;
            Agendamento *anterior=NULL;
            while(agenda!=NULL){
                if(agenda->hora<=sd->t.atual){
                    int id_carro=checkAvailability(sd);
                    if(id_carro!=-1){
                        AuxiliaLaunch *launch=malloc(sizeof(AuxiliaLaunch));
                        if(launch==NULL){
                            perror("Erro a criar o lauch auxiliar\n");
                            anterior=agenda;
                            agenda=agenda->prox;
                            continue;
                        }
                        launch->idCarro=id_carro;
                        launch->iUser=i;
                        launch->sd=sd;
                        strcpy(launch->local, agenda->local);
                        launch->distancia = agenda->distancia;
                        strcpy(launch->fName, sd->users[i]->fName);
                        pthread_t tCarro;
                        if(pthread_create(&tCarro, NULL, lauchCar,launch) != 0){
                            perror("[ERRO] Não foi possivel lançar o carro");
                            free(launch);
                            anterior = agenda;
                            agenda = agenda->prox;
                        }else{
                            Agendamento *temp = agenda;
                            if(anterior == NULL){
                                sd->users[i]->agenda = agenda->prox;
                            } else {
                                anterior->prox = agenda->prox;
                            }
                            agenda = agenda->prox;
                            free(temp);
                            continue;
                        }
                    }
                    else{
                        anterior = agenda;
                        agenda = agenda->prox;
                        printf("Não existem carros disponiveis, assim que possivel lanco um carro");
                    }
                }else{
                    anterior=agenda;
                    agenda=agenda->prox;
                }
            }
        }
    }
    pthread_mutex_unlock(&sd->lock);
}

void readFrota(ServidorData *sd){
    char feedbackV[100];
    int n;
    int percentagem;
    for(int i=0;i<sd->NVEICULOS;i++){
        if (sd->carros[i]->ocupado) {
            while(true){ //ler TUDO o q tem o pipe
                n = read(sd->carros[i]->fd, feedbackV, sizeof(feedbackV)-1); //SE NAO LER NAO BLOQUEIA PORQUE FOI ABERTO COM NONBLOCK
                if (n > 0) {
                    feedbackV[n] = '\0';
                    printf("[VEICULO] Carro ID %d diz: %s\n", sd->carros[i]->id, feedbackV);
                    if(sscanf(feedbackV,"Percorri %d",&percentagem)==1){
                        sd->carros[i]->perPercorrido=percentagem;
                    }else if(strstr(feedbackV, "Terminei ")!=NULL){
                        printf("O veiculo ficou livre\n");
                        break;
                    }
                }
                else{
                    if (n == 0) {
                        int iUser=sd->carros[i]->iUser;
                        if(sd->users[iUser]!=NULL){
                            if(sd->users[iUser]->agenda==NULL){
                                sd->users[iUser]->estado=LIVRE;
                            }else{
                                sd->users[iUser]->estado=ESPERA;
                            }
                        }
                        sd->carros[i]->ocupado = false;
                        close(sd->carros[i]->fd);
                    }
                    else if (errno != EAGAIN) { //SE FOR EAGAIN É SINAL QUE O CARRO NAO MANDOU NADA
                            perror("Erro no pipe");
                    }
                    break;
                }
            }
        }
    }
}

void trataPedido(ServidorData *sd,pergunta_t pergunta){
    resposta_t resposta;
    int cFifoD;
    char ClienteFifo[50];
    sprintf(ClienteFifo,CLIENTE_FIFO,pergunta.pid_cliente);

    if(strcmp(pergunta.tipo_pergunta,"LOGIN")==0){
        int verifica;
        verifica=verificaUser(sd,pergunta.username,ClienteFifo);
        if(verifica==0){
            strcpy(resposta.responde,"[CONTROLADOR]Cliente criado com sucesso\n");
            resposta.criado=1;
        }else if(verifica==1){
            strcpy(resposta.responde,"[CONTROLADOR]Cliente ja existe pelo que nao foi criado\n");
            resposta.criado=0;
        }else if(verifica==2){
            strcpy(resposta.responde,"[CONTROLADOR]Criacao de cliente deu erro\n");
            resposta.criado=0;
        }else{
            strcpy(resposta.responde,"[CONTROLADOR]Cliente nao criado\nMaximo de clientes atingido\n");
            resposta.criado=0;
        }
    }else{
        if(strncmp(pergunta.comando,"agendar ",8)==0){
            int hora;
            char local[50];
            int distancia;
            if(sscanf(pergunta.comando+8,"%d %s %d",&hora,local,&distancia)==3){
                if(agenda(sd,pergunta.username,hora,local,distancia)==0){
                    strcpy(resposta.responde,"[CONTROLADOR]Agendamento efetuado com sucesso\n");
                }else{
                    strcpy(resposta.responde,"[CONTROLADOR]Agendamento nao efetuado, erro no servidor\n");
                }
            }else{
                strcpy(resposta.responde,"[CONTROLADOR]Sintaxe do comando agendar incorreta\n");
            }

        }else if(strcmp(pergunta.comando,"consultar")==0){
            cFifoD=open(ClienteFifo,O_WRONLY);
            if(cFifoD!=-1){
                lista_agenda(sd,pergunta.username,cFifoD);
                close(cFifoD);
            }
            return;

        }else if(strncmp(pergunta.comando,"cancelar ",9)==0){
            int id;
            if(sscanf(pergunta.comando+9,"%d",&id)==1){
                int cancela=cancelar(sd,pergunta.username,id);
                if(cancela==0){
                    strcpy(resposta.responde,"[CONTROLADOR]Agenda com o id indicado eliminada com sucesso\n");
                }else if(cancela==1){
                    strcpy(resposta.responde,"[CONTROLADOR]Nao existe nenhum agendamento com esse Id\n");
                }else if(cancela==2){
                    strcpy(resposta.responde,"[CONTROLADOR]Agenda eliminada\n");
                }
                else{
                    strcpy(resposta.responde,"[CONTROLADOR]Utilizador nao encontrado\n");
                }
            }else{
                strcpy(resposta.responde,"[CONTROLADOR]Sintaxe do comando cancelar incorreta\n");
            }
        }else if(strcmp(pergunta.comando,"terminar")==0){
            int removido;
            removido=remove_users(sd,pergunta.username);
            if(removido==0){
                strcpy(resposta.responde,"ELIMINADO");
            }else{
                strcpy(resposta.responde,"[CONTROLADOR]Erro a eliminar o utilizador\n");
            }
        }else{
            strcpy(resposta.responde,"[CONTROLADOR]Comando Errado\n");
        }
    }

    cFifoD = open(ClienteFifo, O_WRONLY);
    
    if(cFifoD != -1){
        write(cFifoD, &resposta, sizeof(resposta));
        close(cFifoD);
    } else {
        perror("Erro ao abrir FIFO do cliente para resposta");
    }
}



//TEMPO
void *tempo(void *arg){
    ServidorData *sd=(ServidorData *) arg;
    while(sd->ligado){
        sd->t.atual++;
        readFrota(sd);
        checkAgendas(sd);

        //Limpeza de veiculos "mortos"
        while (true) {
            pid_t pidRemover = waitpid(-1, NULL, WNOHANG);
            if(pidRemover<=0){
                break;
            }
        }
        sleep(UTEMPO);
    }
    return NULL;
}

//FUNÇOES THREADS
void *atendeCliente(void *arg){
    ServidorData *sd = (ServidorData *) arg;
    pergunta_t pergunta;
    int res;

    fprintf(stderr, "[Thread] a iniciar escuta do FIFO\n");

    while(sd->ligado){  //QUANDO DESLIGAR SAI DO WHILE
        res = read(sd->cFifo, &pergunta, sizeof(pergunta));

        if(res!=sizeof(pergunta)){
            fprintf(stderr,"Erro na rececao de dados/n");
        }
        else{
            trataPedido(sd,pergunta);
        }
    }
    return NULL;
}

void *cAdmin(void *arg){
    ServidorData *sd = (ServidorData *) arg;
    char comando[50];

    while (sd->ligado) {
        printf("Comando: ");
        if(fgets(comando,sizeof(comando),stdin)==NULL)
            break;
        comando[strcspn(comando, "\n")] = '\0';
        if(strcmp(comando, "listar") == 0){
            printf("------listar-------\n");

            lista_agenda_admin(sd);
            printf("-------------------\n");

        }else if(strcmp(comando,"utiliz")==0){
            lista_users(sd);
        }else if(strcmp(comando,"frota")==0){
            listaCarros(sd);
        }else if(strncmp(comando,"cancelar ",9)==0){
            int id;
            if(sscanf(comando+9,"%d",&id)==1){
                int cancela=cancelar_admin(sd,id);
                if(cancela==0){
                    printf("Agenda com o id indicado eliminada com sucesso\n");
                }else if(cancela==1){
                    printf("Nao existe nenhum agendamento com esse Id\n");
                }else{
                    printf("Agenda eliminada\n");
                }
            }else{
                printf("Sintaxe do comando cancelar incorreta\n");
            }
            
        }else if(strcmp(comando,"km")==0){
            printf("Estou no km\n");
        }else if(strcmp(comando,"hora")==0){
            printf("Estou na hora\n");
        }else if(strcmp(comando,"terminar")==0){
            sd->ligado=false;
           break;
        }else {
            printf("Comando errado\n");
        }
    }

}

int main(){
    ServidorData sd;
    int res;

    preencheSd(&sd);

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
    

    sd.cFifo = open(CONTROLADOR_FIFO, O_RDWR);
    if (sd.cFifo == -1) {
        perror("\nErro ao abrir o FIFO do servidor (RDWR/blocking)\n");
        exit(EXIT_FAILURE);
    }
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags);

    printf("\nControlador aberto...\n");
    

    //THREADS
    if(pthread_create(&sd.tRecebe, NULL, &atendeCliente, &sd) != 0){
        exit(EXIT_FAILURE);
    }

    if(pthread_create(&sd.tTime, NULL, &tempo, &sd) != 0){
        exit(EXIT_FAILURE);
    }

    if(pthread_create(&sd.tAdmin,NULL,&cAdmin,&sd)!=0){
        exit(EXIT_FAILURE);
    }

    pthread_join(sd.tAdmin, NULL);
    printf("\nControlador a encerrar!\n");
    
    close(sd.cFifo);
    unlink(CONTROLADOR_FIFO);
    return 0;
}