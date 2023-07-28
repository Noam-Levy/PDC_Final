build:
	mpicxx -fopenmp -c main.c -o main.o
	mpicxx -fopenmp -c util.c -o util.o
	nvcc -I./Common  -gencode arch=compute_61,code=sm_61 -c cuda.cu -o cuda.o -lm
	mpicxx -fopenmp -o proximity  main.o util.o cuda.o  -L/usr/local/cuda/lib -L/usr/local/cuda/lib64 -lcudart

clean:
	rm -f *.o proximity *.btr output.txt

run:
	mpiexec -np 2 ./proximity


# CC = mpicc
# CFLAGS = -fopenmp
# LIBS = -lm

# SRCS = main.c util.c point.c
# OBJS = $(SRCS:.c=.o)

# TARGET = proximity

# all: $(TARGET)

# $(TARGET): $(OBJS)
# 	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
# 	rm *.o

# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# clean:
# 	rm -f *.o $(TARGET) *.btr output.txt

# run:
# 	mpiexec -np 2 ./$(TARGET) $(args)

# runOn2:
# 	mpiexec -np 2 -machinefile ips.txt -map-by node ./$(TARGET) $(args)

