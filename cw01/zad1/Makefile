all: static shared

static: static_lib
shared: shared_lib 


static_lib: 
	gcc -c array_lib.c
	ar rcs libarray_lib.a array_lib.o


shared_lib:
	gcc -fPIC -c array_lib.c 
	gcc -shared -Wl,-soname,libarray_lib.so.1 -o libarray_lib.so.1.0.1 array_lib.o -lc 
	ln -s libarray_lib.so.1.0.1 libarray_lib.so.1
	ln -s libarray_lib.so.1 libarray_lib.so

clean:
	rm *.o 

