
PGCONFIG=pg_config
CC=gcc
OUT=lotest
SRC=main.c utils.c

LIBS=`${PGCONFIG} --libs`
INCLUDEDIR=`${PGCONFIG} --includedir`
LIBDIR=`${PGCONFIG} --libdir`

main: main.c
	$(CC) -I$(INCLUDEDIR) $(LIBS) $(CFLAGS) -L$(LIBDIR) -lpq -o $(OUT) $(SRC)

clean:
	rm -rf $(OBJ)

