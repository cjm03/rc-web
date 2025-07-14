# Makefile
# default:
# 	gcc server/main.c server/router.c server/video.c server/hashtable.c server/utils.c server/parse.c server/prime.c server/alccalc.c libflate/flate.c -o rcw -g -O0 -lm -lssl -lcrypto -D_DEFAULT_SOURCE -Wall -Wextra -pedantic -std=c99
CC=gcc
SERVER=server/main.c server/router.c server/video.c server/hashtable.c server/utils.c server/parse.c server/prime.c server/alccalc.c
LIBFLATE=server/libflate/flate.c
CFLAGS=-g -O0 -lm -lssl -lcrypto -D_DEFAULT_SOURCE -Wall -Wextra -pedantic -std=c99
rcw:
	$(CC) $(SERVER) $(LIBFLATE) -o rcw $(CFLAGS)
#
# clean: rm -f rcw
