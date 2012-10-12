#Macro define
CC = gcc
CFLAGS = -fno-builtin -c
COBJ = -o

TARGET = libmcsl.so

MAINOBJ = mcsl.o mcsltool.o mcsldevice.o mcslobj.o mcsltopwin.o mcslbutton.o \
					mcslbox.o mcslinput.o mcsllabel.o mcslcanvas.o


#process

build:$(TARGET)

$(TARGET):$(MAINOBJ)
	$(CC) -fpic -shared $(COBJ) $(TARGET) $(MAINOBJ)
	
test.o:test.c
	$(CC) $(CFLAGS) $<

mcsl.o:mcsl.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
mcsltool.o:mcsltool.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
mcsldevice.o:mcsldevice.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
mcslobj.o:mcslobj.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
mcslbox.o:mcslbox.c
	$(CC) $(CFLAGS) $< $(COBJ) $@	
mcsltopwin.o:mcsltopwin.c
	$(CC) $(CFLAGS) $< $(COBJ) $@				
mcslbutton.o:mcslbutton.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
mcslinput.o:mcslinput.c
	$(CC) $(CFLAGS) $< $(COBJ) $@ 
mcsllabel.o:mcsllabel.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
mcslcanvas.o:mcslcanvas.c
	$(CC) $(CFLAGS) $< $(COBJ) $@		
	
clean:
	rm $(MAINOBJ)
	rm $(TARGET)