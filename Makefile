CFLAGS += -O2 -Wall 
CFLAGS += -I/usr/include/freetype2
LIBS += -lfreetype

font2c: font2c.c options.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f font2c
