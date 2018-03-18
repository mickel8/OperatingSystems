all: static shared dynamic

static: static_lib main_static start_static
shared: shared_lib main_shared start_shared
dynamic: dynamic_lib main_dynamic start_dynamic

static_lib: 
	gcc -c array_lib.c
	ar rcs libarray_lib.a array_lib.o

shared_lib:
	gcc -fPIC -c array_lib.c 
	gcc -shared -Wl,-soname,libarray_lib.so.1 -o libarray_lib.so.1.0.1 array_lib.o -lc 
	ln -s libarray_lib.so.1.0.1 libarray_lib.so.1
	ln -s libarray_lib.so.1 libarray_lib.so

dynamic_lib:
	gcc -fPIC -c array_lib.c -o array_lib.o
	gcc -Wl,-rpath=. -fPIC -shared -o libarray_lib.so array_lib.o
	
main_static:
	gcc -c main.c
	gcc main.o libarray_lib.a -o main

main_shared:
	gcc main.c -larray_lib -L ./ -o main

main_dynamic:
	gcc -L. -o main main.c -ldl -D DLL

start_static: 
	echo "TEST BIBLIOTEKI STATYCZNEJ" >> raport3a.txt
	echo "\n* TABLICA ALOKOWANA STATYCZNIE [1000]x[1001] *\n" >> raport3a.txt
	echo "Parametry dla fill delete fill_and_delete find => 1000 1000 1000 341 \n" >> raport3a.txt
	
	./main 1000 1000 static fill 1000 delete 1000 fill 1000 find 341 delete 1 fill_and_delete 1000
	
	echo "\n\n* TABLICA ALOKOWANA DYNAMICZNIE  [1000]x[1001]*\n" >> raport3a.txt
	echo "Parametry dla fill delete fill_and_delete find => 1000 999 999 341 \n" >> raport3a.txt
	./main 1000 1000 dynamic add 1000 delete 999 add 999 find 341 delete 1 fill_and_delete 1000

start_shared:
	echo "\n\n\nTEST BIBLIOTEKI WSPÓŁDZIELONEJ" >> raport3a.txt
	echo "\n* TABLICA ALOKOWANA STATYCZNIE [1000]x[1001] *\n" >> raport3a.txt
	echo "Parametry dla fill delete fill_and_delete find => 1000 1000 1000 341 \n" >> raport3a.txt
	
	export LD_LIBRARY_PATH="."; \
	./main 1000 1000 static fill 1000 delete 1000 fill 1000 find 341 delete 1 fill_and_delete 1000
	
	echo "\n\n* TABLICA ALOKOWANA DYNAMICZNIE  [1000]x[1001]*\n" >> raport3a.txt
	echo "Parametry dla fill delete fill_and_delete find => 1000 999 999 341 \n" >> raport3a.txt

	export LD_LIBRARY_PATH="."; \
	./main 1000 1000 dynamic add 1000 delete 999 add 999 find 341 delete 1 fill_and_delete 1000	

start_dynamic:
	echo "\n\n\nTEST BIBLIOTEKI DYNAMICZNEJ" >> raport3a.txt
	echo "\n* TABLICA ALOKOWANA STATYCZNIE [1000]x[1001] *\n" >> raport3a.txt
	echo "Parametry dla fill delete fill_and_delete find => 1000 1000 1000 341 \n" >> raport3a.txt
	
	./main 1000 1000 static fill 1000 delete 1000 fill 1000 find 341 delete 1 fill_and_delete 1000
	
	echo "\n\n* TABLICA ALOKOWANA DYNAMICZNIE  [1000]x[1001]*\n" >> raport3a.txt
	echo "Parametry dla fill delete fill_and_delete find => 1000 999 999 341 \n" >> raport3a.txt

	./main 1000 1000 dynamic add 1000 delete 999 add 999 find 341 delete 1 fill_and_delete 1000	

clean:
	rm *.o *.a main *.so


