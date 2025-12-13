#include "config.h"
#include <stdio.h>
#include <string.h>       

/* esta função vai atender o sinal SIGINT para terminar o servidor */
void trataSig(int i) {
    fprintf(stderr, "[CLIENTE] Para terminar a sua sessao utilize o comando terminar\n");
}

int main(int argc, char *argv[]){
    pergunta_t perg;/* mensagem do "tipo" pergunta */
    resposta_t resp;/* mensagem do "tipo" resposta */
    int s_fifo_fd, c_fifo_fd;       /* descritores de ficheiros (pipes) */
    char c_fifo_fname[25];          /* nome do FIFO deste cliente */
    int read_res;
    char comando[50];

    //fflush(stdout);
    if(argc!=2){
        printf("[CLIENTE] Precisa de indicar um nome de utilizador!!!\n");
        exit(EXIT_FAILURE);
    }
    
    if (signal(SIGINT,trataSig) == SIG_ERR) {
        perror("[CLIENTE] Nao foi possivel configurar o sinal SIGINT\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "[CLIENTE] Sinal SIGINT configurado\n");

    //Verificação controlador ativo
    s_fifo_fd = open(CONTROLADOR_FIFO, O_WRONLY);
    if (s_fifo_fd == -1) {
        fprintf(stderr, "[CLIENTE] O servidor não está a correr\n");
        unlink(c_fifo_fname);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr,"\n[CLIENTE] FIFO do servidor aberto WRITE / BLOCKING\n");

    //Abre fifo para comunicar com controlador
     /* cria o FIFO do cliente */
    perg.pid_cliente = getpid();
    sprintf(c_fifo_fname, CLIENTE_FIFO, perg.pid_cliente);
    if (mkfifo(c_fifo_fname, 0777) == -1) {
        perror("\n[CLIENTE] mkfifo FIFO cliente deu erro\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr,"\n[CLIENTE] FIFO do cliente criado\n");

    c_fifo_fd = open(c_fifo_fname, O_RDWR); /* bloqueante */
    if (c_fifo_fd == -1) {
        perror("\n[CLIENTE] Erro nao abrir o FIFO do cliente\n");
        close(s_fifo_fd);
        unlink(c_fifo_fname);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "\n[CLIENTE] FIFO do cliente aberto para READ (+Write) BLOCK\n");

    //Verificação utilizador
    perg.pid_cliente=getpid();
    strcpy(perg.username,argv[1]);
    strcpy(perg.tipo_pergunta,"LOGIN");
    write(s_fifo_fd,& perg,sizeof(perg));
    read_res=read (c_fifo_fd, &resp, sizeof(resp));
        
    if(read_res==sizeof(resp)){
        printf("%s",resp.responde);
        if(resp.criado==0){
            close(c_fifo_fd);
            close(s_fifo_fd);
            unlink(c_fifo_fname);
            exit(EXIT_FAILURE);
        }
    }else{
        printf("[CLIENTE] Erro de comunicacao com o servidor");
        close(c_fifo_fd);
        close(s_fifo_fd);
        unlink(c_fifo_fname);
        exit(EXIT_FAILURE);
    }
    
    //Variaveis Select
    fd_set readFd;
    int max;
    int nfd;
    printf("[CLIENTE] Comando: ");
    fflush(stdout);

    while(1){

        FD_ZERO(&readFd);
        FD_SET(STDIN_FILENO,&readFd);
        FD_SET(c_fifo_fd,&readFd);
        if(STDIN_FILENO>c_fifo_fd){
            max=STDIN_FILENO+1;
        }else{
            max=c_fifo_fd+1;
        }

        nfd=select(max,&readFd,NULL,NULL,NULL);
        if(nfd==-1){
            if(errno==EINTR){
                continue;
            }
            perror("[CLIENTE] Erro no select\n");
            close(c_fifo_fd);
            close(s_fifo_fd);
            unlink(c_fifo_fname);
            exit(EXIT_FAILURE);
        }
        if(FD_ISSET(STDIN_FILENO,&readFd)){
            if(fgets(comando,sizeof(comando),stdin)==NULL){
                break;
            }
            comando[strcspn(comando, "\n")] = '\0';
            if(strlen(comando) > 0){
                perg.pid_cliente=getpid();
                strcpy(perg.username,argv[1]);
                strcpy(perg.tipo_pergunta,"COMANDO");
                strcpy(perg.comando,comando);
                write(s_fifo_fd, &perg,sizeof(perg));
            }
        }
        if(FD_ISSET(c_fifo_fd,&readFd)){
            resposta_t resp;
            read_res=read(c_fifo_fd, &resp, sizeof(resp));
            if(read_res > 0){
                if(strcmp(resp.responde, "INICIO")==0){
                    while(1){
                        read_res=read(c_fifo_fd, &resp, sizeof(resp));
                        if(read_res!=sizeof(resp)){
                            fprintf(stderr,"[CLIENTE] Erro na rececao de dados\n");
                            break;
                        }
                        if(strcmp(resp.responde,"FIM")==0){
                            break;
                        }
                        printf("%s\n",resp.responde);
                    }
                }else{
                    if(strcmp(resp.responde, "ENCERRAR") == 0){
                        printf("\n\n[CLIENTE] O servidor encerrou a sessão. A sair...\n");
                        break;
                    }else if(strcmp(resp.responde, "ELIMINADO") == 0){
                        printf("[CLIENTE]A terminar sessao...\n");
                        break;
                    }
                    printf("%s\n",resp.responde);
                }
            }else if( read_res == 0){
                break;
            }else{
                perror("[CLIENTE] Erro ao ler o fifo");
                break;
            }
        }
        printf("[CLIENTE] Comando: ");
        fflush(stdout);
    }
    printf("\n[CLIENTE]Sessao a encerrar!\n");
    close(c_fifo_fd);
    close(s_fifo_fd);
    unlink(c_fifo_fname);
    return 0;
}