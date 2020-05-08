a.out: 			main.o command.o
						gcc -g main.o command.o

main.o: 		main.c command.h
						gcc -c main.c
command.o: 	command.c command.h
						gcc -c command.c

clean:
						rm a.out main.o command.o
