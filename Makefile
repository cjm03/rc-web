# Makefile
default:
	gcc server/main.c server/router.c server/video.c server/hashtable.c server/utils.c server/parse.c -o rcw -Wall -Wextra -pedantic -std=c99
# CC=gcc
# CFLAGS=-Wall -Wextra -O2
#
# server: server/main.c
# 	$(CC) $(CFLAGS) -o rcw server/main.c
#
# clean: rm -f rcw
