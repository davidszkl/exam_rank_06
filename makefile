all:
	gcc -Wall -Wextra -Werror mini_serv.c -o server &&\
	gcc -Wall -Wextra -Werror client.c -o client &&\
	./server