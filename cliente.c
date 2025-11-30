#include "config.h"
#include <stdio.h>
#include <string.h>

int s_fifo_fd, c_fifo_fd;       /* descritores de ficheiros (pipes) */
char c_fifo_fname[25];          /* nome do FIFO deste cliente */


/* esta função vai atender o sinal SIGINT para terminar o servidor */
void trataSig(int i) {
    fprintf(stderr, "Cliente a terminar (interrompido via teclado)\n\n");

    close(c_fifo_fd);
    close(s_fifo_fd);
    unlink(c_fifo_fname);
    exit(EXIT_SUCCESS);
    /* para terminar o processo */
}

int main(int argc, char *argv[]){
    pergunta_t perg;/* mensagem do "tipo" pergunta */
    resposta_t resp;/* mensagem do "tipo" resposta */
    int read_res;
    char comando[50];
    fflush(stdout);
    if(argc!=2){
        printf("Precisa de indicar um nome de utilizador!!!\n");
        close(c_fifo_fd);
        close(s_fifo_fd);
        unlink(c_fifo_fname);
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT,trataSig) == SIG_ERR) {
        perror("Nao foi possivel configurar o sinal SIGINT\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Sinal SIGINT configurado\n");

    //Verificação controlador ativo
    s_fifo_fd = open(CONTROLADOR_FIFO, O_WRONLY); /* bloqueante */
    if (s_fifo_fd == -1) {
        fprintf(stderr, "O servidor não está a correr\n");
        unlink(c_fifo_fname);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr,"\nFIFO do servidor aberto WRITE / BLOCKING\n");

    //Abre fifo para comunicar com controlador
     /* cria o FIFO do cliente */
    perg.pid_cliente = getpid();
    sprintf(c_fifo_fname, CLIENTE_FIFO, perg.pid_cliente);
    if (mkfifo(c_fifo_fname, 0777) == -1) {
        perror("\nmkfifo FIFO cliente deu erro\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr,"\nFIFO do cliente criado\n");

    c_fifo_fd = open(c_fifo_fname, O_RDWR); /* bloqueante */
    if (c_fifo_fd == -1) {
        perror("\nErro nao abrir o FIFO do cliente\n");
        close(s_fifo_fd);
        unlink(c_fifo_fname);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "\nFIFO do cliente aberto para READ (+Write) BLOCK\n");

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
        printf("Erro de comunicacao com o servidor");
        close(c_fifo_fd);
        close(s_fifo_fd);
        unlink(c_fifo_fname);
        exit(EXIT_FAILURE);
    }
    
    
    while(1){
        printf("Comando: ");
        if(fgets(comando,sizeof(comando),stdin)==NULL)
            break;
        comando[strcspn(comando, "\n")] = '\0';

        perg.pid_cliente=getpid();
        strcpy(perg.username,argv[1]);
        strcpy(perg.tipo_pergunta,"COMANDO");
        strcpy(perg.comando,comando);
        write(s_fifo_fd,& perg,sizeof(perg));
        if(strcmp(comando,"consultar")==0){
            while(1){
                read_res=read(c_fifo_fd, &resp, sizeof(resp));
                if(read_res!=sizeof(resp)){
                    fprintf(stderr,"Erro na rececao de dados/n");
                    break;
                }
                if(strcmp(resp.responde,"FIM")==0){
                    break;
                }
                printf("%s\n",resp.responde);
            }
        }
        else{
            read(c_fifo_fd, &resp, sizeof(resp));
            printf("%s\n",resp.responde);
        }
    }
    printf("\nVou fechar!!\n");
    close(c_fifo_fd);
    close(s_fifo_fd);
    unlink(c_fifo_fname);
    return 0;
}