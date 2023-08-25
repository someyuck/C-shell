main:
	gcc -g main.c prompt.c parse_input.c warp.c peek.c proclore.c system_commands.c -Wall -fsanitize=address,null