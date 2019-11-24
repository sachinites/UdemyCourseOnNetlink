rm *.o
rm *exe
gcc -g -c nlrt.c -o nlrt.o
gcc -g -c userspace.c -o userspace.o
gcc -g userspace.o nlrt.o -o userspace.exe -lpthread
