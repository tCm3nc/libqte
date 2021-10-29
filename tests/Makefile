CC = clang
# tests are compiled in debug mode.
CFLAGS += -Wno-int-to-void-pointer-cast -ggdb -DDEBUG
LDFLAGS += -ldl -pthread

FILES = double_free.c test_memalign.c
HEADERS = ../

all: $(HEADERS) $(FILES)
	$(foreach var, $(FILES), $(CC) $(CFLAGS) -fPIC $(var) -I$(HEADERS) -o $(var:.c=) $(LDFLAGS);)
clean: $(FILES)
	rm $(FILES:.c=)

