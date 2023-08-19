main:
	gcc -g main.c prompt.c parse_input.c warp.c -Wall -fsanitize=address,null