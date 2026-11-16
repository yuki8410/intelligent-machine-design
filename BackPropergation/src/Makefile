target = backprop
src = backprop.c main.c
obj = $(src:.c=.o)
CFLAGS = -Wall -O3 -g
LIBS = -lm
CC = gcc

.PHONY: all clean
all: $(target)
$(target): $(obj)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)


%.o: %.c
	$(CC) $(CFLAGS) -c $<



clean:
	rm -f $(target)
	rm -f *.o
	rm -f *.log
	rm -f *.csv
	rm -f *.png

run: $(target)
	./$(target) >>/dev/null 2>&1

rmse:
	.venv/bin/python3 rmse.py


show3d:
	.venv/bin/python3 draw3d.py graph.csv

correct:
	.venv/bin/python3 draw3d.py correct.csv
