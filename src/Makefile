STD=-std=c18
WARN=-Wall -W
CFLAGS=$(STD) $(WARN) -g

OBJ=mem.o lex.o ast.o obj.o main.o

SILO_NAME=silo

all: $(SILO_NAME)

$(SILO_NAME): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(SILO_NAME) *.o