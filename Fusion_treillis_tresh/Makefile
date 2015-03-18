### Makefile pour fusion_phon_N_treillis => gregory.senay@univ-avignon.fr

### Compilateur et Options 
CC=g++
ICC=icc
CFLAGS=-W
LDFLAGS=
DLIBS=

###

PROG = fusion_phon_N_treillis

all: $(PROG)
	$(CC) $(CFLAGS) -o $(PROG) $(PROG).cpp

icc:
	$(ICC) $(CFLAGS) -o $(PROG).icc $(PROG).cpp

test:
	./fusion_phon_N_treillis test.phos test.treil toto.out
