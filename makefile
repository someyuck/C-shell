main:
	gcc -g main.c prompt.c parse_input.c -Wall -Werror -fsanitize=address,null