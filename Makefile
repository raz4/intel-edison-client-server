#!/bin/sh
.SILENT:

default:
	gcc -lmraa -lm -std=c11 -D_XOPEN_SOURCE=600 -o lab4_1 prog1.c
	gcc -lssl -lcrypto -lmraa -lm -pthread -std=c11 -g -D_XOPEN_SOURCE=600 -o lab4_3 prog3.c
	gcc -lmraa -lm -pthread -std=c11 -g -D_XOPEN_SOURCE=600 -o lab4_2 prog2.c

dist:
	tar -cvf lab4-704666892.tar.gz prog1.c prog2.c prog3.c README Makefile lab4_1.log lab4_2.log lab4_3.log

clean:
	-rm lab4_1
	-rm lab4_2
	-rm lab4_3

