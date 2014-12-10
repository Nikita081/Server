#!/bin/bash
gcc -c backstatic.c
ar rcs libstatic.a backstatic.o
gcc serv1.c -lpthread -L. -lstatic -o server.out 
gcc cli1.c -L. -lstatic -o client.out 
gcc manager.c -L. -lstatic -o manager.out 
gcc generator.c -o gen.out
