CC = clang
CFLAGS += -Wno-int-to-void-pointer-cast -ggdb
LDFLAGS += -ldl -pthread

FILES = libqte.c hooks.c malloc.c
HEADERS = include/libqte.h

all: $(HEADERS) $(FILES)
	$(CC) $(CFLAGS) -fPIC -shared $(FILES) -o libqte.so $(LDFLAGS)
debug: $(HEADERS) $(FILES)
	$(CC) $(CFLAGS) -DDEBUG -fPIC -shared $(FILES) -o libqte.so $(LDFLAGS)
clean: $(HEADERS) $(FILES)
	rm libqte.so