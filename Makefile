#Macro define
CC = gcc
CFLAGS = -fno-builtin -c
COBJ = -o
LD = ld
LDLIB = -lX11 -lXi -lXpm -lm -lmcsl


TARGET = b

#process

build:$(TARGET)

$(TARGET):test.o
	$(CC) $(LDLIB) test.o $(COBJ) $(TARGET)
	
test.o:test.c
	$(CC) $(CFLAGS) $<

	
clean:
	rm test.o
	rm $(TARGET)