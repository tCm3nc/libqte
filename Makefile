CC = clang
CFLAGS += -Wno-int-to-void-pointer-cast -ggdb
LDFLAGS += -ldl -pthread

FILES = libqte.c
HEADERS = libqte.h

all: $(HEADERS) $(FILES)
	$(CC) $(CFLAGS) -fPIC -shared $(FILES) -o libqte.so $(LDFLAGS)