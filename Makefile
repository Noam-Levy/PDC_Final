CC = mpicc
CFLAGS = -fopenmp
LIBS = -lm

SRCS = main.c util.c point.c
OBJS = $(SRCS:.c=.o)

TARGET = proximity

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	rm *.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET) *.btr output.txt

run:
	mpiexec -np 2 ./$(TARGET) $(args)

runOn2:
	mpiexec -np 2 -machinefile ips.txt -map-by node ./$(TARGET) $(args)

