# Makefile
# default:
# 	gcc server/main.c server/router.c server/video.c server/hashtable.c server/utils.c server/parse.c server/prime.c server/alccalc.c libflate/flate.c -o rcw -g -O0 -lm -lssl -lcrypto -D_DEFAULT_SOURCE -Wall -Wextra -pedantic -std=c99
CC=gcc
SERVER=server/main.c server/router.c server/video.c server/hashtable.c server/utils.c server/parse.c server/prime.c server/alccalc.c #server/users.c
LIBFLATE=server/libflate/flate.c
LIBBCRYPT=-Iserver/libbcrypt -Lserver/libbcrypt -l:bcrypt.a
CFLAGS=-g -O0 -lm -lssl -lcrypto -D_DEFAULT_SOURCE -Wall -Wextra -pedantic -std=c99
rcw:
	$(CC) $(SERVER) $(LIBFLATE) $(LIBBCRYPT) $(CFLAGS) -o rcw
#
# clean: rm -f rcw
