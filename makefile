CC=gcc

CFLAGS= -Wall -g 

PROGS=pasta controlador cliente veiculo

HEADERS= config.h 

TARGETS= controlador cliente veiculo

PASTA="fifos"

all:$(PROGS)

pasta: 
	mkdir -p $(PASTA)

controlador: controlador.c $(HEADERS)
	$(CC) $(CFLAGS) -o controlador controlador.c

cliente: cliente.c $(HEADERS)
	$(CC) $(CFLAGS) -o cliente cliente.c

veiculo: veiculo.c $(HEADERS)
	$(CC) $(CFLAGS) -o veiculo veiculo.c

clean:
	rm -f $(TARGETS)

