CC = mpicxx
CFLAGS = -fopenmp
LIBS = -lm

NVCC = nvcc
CUDAFLAGS = -gencode arch=compute_61,code=sm_61
CUDALIBS = -L/usr/local/cuda/lib -L/usr/local/cuda/lib64 -lcudart

SRCS = main.c util.c
OBJS = $(SRCS:.c=.o)
CUOBJS = cuda.o

TARGET = proximity

all: $(TARGET)

$(TARGET): $(OBJS) $(CUOBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(CUDALIBS) $(LIBS)
	rm *.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

cuda.o: cuda.cu
	$(NVCC) -I./Common $(CUDAFLAGS) -c $< -o $@ 

clean:
	rm -f *.o $(TARGET) *.btr output.txt

run:
	mpiexec -np 2 ./$(TARGET) $(args)

runOn2:
	mpiexec -np 2 -machinefile ips.txt -map-by node ./$(TARGET) $(args)
