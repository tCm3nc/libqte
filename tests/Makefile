CC = clang
# tests are compiled in debug mode.
CFLAGS += -Wno-int-to-void-pointer-cast -ggdb -DDEBUG
LDFLAGS += -ldl -pthread

FILES = test_double_free.c test_memalign.c test_malloc.c
HEADERS =../include/libqte.h

all: $(HEADERS) $(FILES)
	$(foreach var, $(FILES), $(CC) $(CFLAGS) -fPIC $(var) ../libqte.so -o $(var:.c=) $(LDFLAGS);)
clean: $(FILES)
	rm $(FILES:.c=)

