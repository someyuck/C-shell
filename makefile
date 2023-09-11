main:
	gcc -g main.c prompt.c parse_input.c warp.c peek.c proclore.c system_commands.c pastevents.c utils.c seek.c redirect.c pipeline.c -Wall -fsanitize=address,null