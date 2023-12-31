CC = gcc
CFLAGS = -g -Wall -fsanitize=address,null
MAINFILES = src/prompt.c src/parse_input.c src/warp.c src/peek.c src/proclore.c src/system_commands.c src/pastevents.c src/utils.c src/seek.c src/redirect.c src/pipeline.c src/activities.c src/ping.c src/fgbg.c src/neonate.c src/iman.c

main:
	$(CC) $(CFLAGS) src/main.c $(MAINFILES)