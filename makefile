all:
	gcc -Wall -Wextra -Werror mini_serv.c -o server &&\
	gcc -Wall -Wextra -Werror client_Read.c -o client_Read &&\
	gcc -Wall -Wextra -Werror client_Write.c -o client_Write

clean:
	rm server client_Read client_Write