#include "config.h"


int s_fifo_fd, c_fifo_fd;       /* descritores de ficheiros (pipes) */

/* esta função vai atender o sinal SIGINT para terminar o servidor */
void trataSig(int i) {
    fprintf(stderr, "\nControlador a terminar (interrompido via teclado)\n\n");
    close(s_fifo_fd);
    unlink(CONTROLADOR_FIFO);
    exit(EXIT_SUCCESS);
    /* para terminar o processo */
}

int verificaUser(char *nome, struct Cliente **a){
    for(int i=0;i<NUTILIZADORES;i++){
        if(a[i]!=NULL){
            if(strcmp(nome,a[i]->nome)==0){
                printf("Utilizador ja existente\n");
                return 0;
            }
        }
    }
    struct Cliente *ptr;
    for(int i=0; i<NUTILIZADORES;i++){
        if(a[i]==NULL){
            ptr=(struct Cliente*)malloc(sizeof(struct Cliente));
            if(ptr== NULL){
                printf("Nao foi possivel criar o cliente");
                return 0;
            }
            a[i]=ptr;
            strcpy(a[i]->nome,nome);
            break;
        }
    }

    printf("Cliente criado com sucesso\n");
    return 1;
}

int main(){
    struct Cliente *users[NUTILIZADORES]={NULL};
    char comando[50];

    pergunta_t perg;/* mensagem do "tipo" pergunta */
    resposta_t resp;/* mensagem do "tipo" resposta */
    int res;
    char c_fifo_fname[50];
    int aux1;

    //verificaUser(utilizador,users);


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

    s_fifo_fd = open(CONTROLADOR_FIFO, O_RDWR);
    /* bloqueante */
    if (s_fifo_fd == -1) {
        perror("\nErro ao abrir o FIFO do servidor (RDWR/blocking)\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr,"\nFIFO aberto para READ (+WRITE) bloqueante\n");
    /* ciclo principal: read pedido -> write resposta -> repete */
    while (1) {
        res = read(s_fifo_fd, & perg, sizeof(perg));
        if (res < sizeof(perg)) {
            fprintf(stderr,"\nRecebida pergunta incompleta [bytes lidos: %d]\n", res);
            continue; /* não responde a cliente (qual cliente?) */
        }
        fprintf(stderr, "\nRecebido [%s]", perg.username);
        
        
        
        strcpy(resp.palavra, "DESCONHECIDO\n"); /* caso n encontre */
        aux1=verificaUser(perg.username,users);
        if(aux1==1){
            resp.confirmUser=true;

         /*   for(int i=0;i<NUTILIZADORES;i++){
                if(users[i]!=NULL){
                    users[i]->nome=perg.username;
                    break;
                }
            }
            for(int i=0;i<NUTILIZADORES;i++){
                printf("User %d: %s\n",i,users[i]->nome);
            }*/
        }else {
            resp.confirmUser=false;
        }
        fprintf(stderr, "\nConfirm user = [%d]\n", resp.confirmUser);
        /* ---- OBTEM FILENAME DO FIFO PARA A RESPOSTA ---- */
        sprintf(c_fifo_fname, CLIENTE_FIFO, perg.pid_cliente);
        /* ---- Abre FIFO do cliente p/ write ---- */
        c_fifo_fd = open(c_fifo_fname, O_WRONLY);
        if (c_fifo_fd == -1){
            perror("\nErro no open - Ninguem quis a resposta\n");
        }else {
            fprintf(stderr,"\nFIFO cliente aberto para WRITE\n");
            /* ---- ENVIA RESPOSTA ---- */
            res = write(c_fifo_fd, & resp, sizeof(resp));
            if (res == sizeof(resp)){
                fprintf(stderr,"\nescreveu a resposta\n");
            }else{
                perror("\nerro a escrever a resposta\n");
            }
            close(c_fifo_fd); /* FECHA LOGO O FIFO DO CLIENTE! */
            fprintf(stderr,"\nFIFO cliente fechado\n");
        }
        printf("Comando: ");
        scanf("%s",comando);
        
        if(strcmp(comando,"listar")==0){

        }else if(strcmp(comando,"utiliz")==0){

        }else if(strcmp(comando,"frota")==0){

        }else if(strcmp(comando,"cancelar")==0){//falta apanhar o id
        
        }else if(strcmp(comando,"km")==0){

        }else if(strcmp(comando,"hora")==0){

        }else if(strcmp(comando,"terminar")==0){
            break;

        }else{
            printf("Comando errado\n");
        }
    }
    /* fim do ciclo principal do servidor */
    /* em principio não chega a este ponto - sai via SIGINT */
    printf("\nVou fechar!!\n");
    close(s_fifo_fd);
    unlink(CONTROLADOR_FIFO);


    return 0;
} /* fim da função main do servidor */