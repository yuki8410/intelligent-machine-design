target = rga 
src =  main.c rga.c MT.c
obj = $(src:.c=.o)
CFLAGS = -Wall -O3 -g -std=c2x
LIBS = -lm
CC = gcc

.PHONY: all clean
all: $(target)
$(target): $(obj)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)



data: $(data)
$(data): $(obj)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f $(target)
	rm -f *.o
	rm -f *.log
	rm -f *.csv
	rm -f *.png

