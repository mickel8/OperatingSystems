all: static shared

static: static_lib main_static start_static
shared: shared_lib main_shared start_shared


static_lib: 
	gcc -c array_lib.c
	ar rcs libarray_lib.a array_lib.o

shared_lib:
	gcc -fPIC -c array_lib.c 
	gcc -shared -Wl,-soname,libarray_lib.so.1 -o libarray_lib.so.1.0.1 array_lib.o -lc 
	ln -s libarray_lib.so.1.0.1 libarray_lib.so.1
	ln -s libarray_lib.so.1 libarray_lib.so
	
main_static:
	gcc -c main.c
	gcc main.o libarray_lib.a -o main

main_shared:
	gcc main.c -larray_lib -L ./ -o main

start_static: 
	echo "TEST BIBLIOTEKI STATYCZNEJ" >> raport2.txt
	echo "\n* TABLICA ALOKOWANA STATYCZNIE [1000]x[1001] *\n" >> raport2.txt
	echo "Parametry dla fill delete fill_and_delete find => 1000 1000 1000 341 \n" >> raport2.txt
	
	./main 1000 1000 static fill 1000 delete 1000 fill 1000 find 341 delete 1 fill_and_delete 1000
	
	echo "\n\n* TABLICA ALOKOWANA DYNAMICZNIE  [1000]x[1001]*\n" >> raport2.txt
	echo "Parametry dla fill delete fill_and_delete find => 1000 999 999 341 \n" >> raport2.txt
	./main 1000 1000 dynamic add 1000 delete 999 add 999 find 341 delete 1 fill_and_delete 1000

start_shared:
	echo "\n\n\nTEST BIBLIOTEKI WSPÓŁDZIELONEJ" >> raport2.txt
	echo "\n* TABLICA ALOKOWANA STATYCZNIE [1000]x[1001] *\n" >> raport2.txt
	echo "Parametry dla fill delete fill_and_delete find => 1000 1000 1000 341 \n" >> raport2.txt
	
	export LD_LIBRARY_PATH="."; \
	./main 1000 1000 static fill 1000 delete 1000 fill 1000 find 341 delete 1 fill_and_delete 1000
	
	echo "\n\n* TABLICA ALOKOWANA DYNAMICZNIE  [1000]x[1001]*\n" >> raport2.txt
	echo "Parametry dla fill delete fill_and_delete find => 1000 999 999 341 \n" >> raport2.txt

	export LD_LIBRARY_PATH="."; \
	./main 1000 1000 dynamic add 1000 delete 999 add 999 find 341 delete 1 fill_and_delete 1000	


clean:
	rm *.o *.a main *.so

