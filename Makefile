TRAGET := pcm2aac

CC := gcc
CFLAG := -I./include -lm

all : $(TRAGET) 

$(TRAGET) : main.c lib/libfdk-aac.a
	$(CC) $^ $(CFLAG) -o $@

clean : 
	rm -rf $(TRAGET) *.aac
.PHONY := clean

