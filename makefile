CC = cc
AR = ar
CFLAG = -I./include
CPATH = ./src/
OPATH = ./build/
APATH = ./bin/
SOURCE = $(wildcard $(CPATH)*.c)
OBJECTS = $(patsubst $(CPATH)%.c, %.o, $(SOURCE))

vpath %.a lib
vpath %.o build
vpath %.c src

$(APATH)libFlyUtil.a: $(OBJECTS)
	$(AR) -rc $@ $(patsubst %.o, $(OPATH)%.o, $^)
$(OBJECTS):%.o:%.c
	$(CC) -c $(CFLAG) $< -o $(OPATH)$@

.PHONY: clear

clear:
	-rm $(OPATH)*.o $(APATH)*.a 
