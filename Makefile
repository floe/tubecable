#NAME=displaylink
#EXTRA_CFLAGS+=-Wall

#obj-m := ${NAME}.o

#all: dldemo
#	make -C /usr/src/linux SUBDIRS=`pwd` modules

#clean:
#	-rm -f ${NAME}.mod.c ${NAME}.mod.o ${NAME}.o ${NAME}.ko .${NAME}.* modules.order Module.symvers

tubecable_demo: tubecable_demo.c tubecable.c
	g++ -ggdb -Wall -o $@ $^ -lusb 

