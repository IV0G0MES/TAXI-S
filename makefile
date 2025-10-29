CC=gcc

CFLAGS= -Wall -g 

PROGS=controlador cliente veiculo

HEADERS= config.h 

TARGETS= controlador cliente veiculo

all:$(PROGS)

controlador: controlador.c $(HEADERS)
	$(CC) $(CFLAGS) -o controlador controlador.c

cliente: cliente.c $(HEADERS)
	$(CC) $(CFLAGS) -o cliente cliente.c

veiculo: veiculo.c $(HEADERS)
	$(CC) $(CFLAGS) -o veiculo veiculo.c

clean:
	rm -f $(TARGETS)

