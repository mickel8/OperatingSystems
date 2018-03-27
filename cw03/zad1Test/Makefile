RECORDS_V1=1500
RECORDS_V2=3000
CHARS_V4=4
CHARS_V512=512
CHARS_V4096=4096
CHARS_V8192=8192

all: compile v1_4 v2_4 v1_512 v2_512 v1_4096 v2_4096 v1_8192 v2_8192

compile:
	gcc -Wall main.c -o main
v1_4:
	./main generate dane.txt $(RECORDS_V1) $(CHARS_V4)
	cp dane.txt dane2.txt
	echo "\n* Test sort_sys dla $(RECORDS_V1) x $(CHARS_V4) *" >> wyniki.txt
	./main sort dane.txt $(RECORDS_V1) $(CHARS_V4) sys 
	echo "\n* Test sort_lib dla $(RECORDS_V1) x $(CHARS_V4) *" >> wyniki.txt
	./main sort dane2.txt $(RECORDS_V1) $(CHARS_V4) lib
	echo "\n* Test copy_sys dla $(RECORDS_V1) x $(CHARS_V4) *" >> wyniki.txt
	./main copy dane.txt backup.txt $(RECORDS_V1) $(CHARS_V4) sys
	echo "\n* Test copy_lib dla $(RECORDS_V1) x $(CHARS_V4) *" >> wyniki.txt
	./main copy dane.txt backup2.txt $(RECORDS_V1) $(CHARS_V4) lib  
	make clean
v2_4:
	./main generate dane.txt $(RECORDS_V2) $(CHARS_V4)
	cp dane.txt dane2.txt
	echo "\n* Test sort_sys dla $(RECORDS_V2) x $(CHARS_V4) *" >> wyniki.txt
	./main sort dane.txt $(RECORDS_V2) $(CHARS_V4) sys 
	echo "\n* Test sort_lib dla $(RECORDS_V2) x $(CHARS_V4) *" >> wyniki.txt
	./main sort dane2.txt $(RECORDS_V2) $(CHARS_V4) lib
	echo "\n* Test copy_sys dla $(RECORDS_V2) x $(CHARS_V4) *" >> wyniki.txt
	./main copy dane.txt backup.txt $(RECORDS_V2) $(CHARS_V4) sys
	echo "\n* Test copy_lib dla $(RECORDS_V2) x $(CHARS_V4) *" >> wyniki.txt
	./main copy dane.txt backup2.txt $(RECORDS_V2) $(CHARS_V4) lib  
	make clean
v1_512:
	./main generate dane.txt $(RECORDS_V1) $(CHARS_V512)
	cp dane.txt dane2.txt
	echo "\n* Test sort_sys dla $(RECORDS_V1) x $(CHARS_V512) *" >> wyniki.txt
	./main sort dane.txt $(RECORDS_V1) $(CHARS_V512) sys 
	echo "\n* Test sort_lib dla $(RECORDS_V1) x $(CHARS_V512) *" >> wyniki.txt
	./main sort dane2.txt $(RECORDS_V1) $(CHARS_V512) lib
	echo "\n* Test copy_sys dla $(RECORDS_V1) x $(CHARS_V512) *" >> wyniki.txt
	./main copy dane.txt backup.txt $(RECORDS_V1) $(CHARS_V512) sys
	echo "\n* Test copy_lib dla $(RECORDS_V1) x $(CHARS_V512) *" >> wyniki.txt
	./main copy dane.txt backup2.txt $(RECORDS_V1) $(CHARS_V512) lib  
	make clean
v2_512:
	./main generate dane.txt $(RECORDS_V2) $(CHARS_V512)
	cp dane.txt dane2.txt
	echo "\n* Test sort_sys dla $(RECORDS_V2) x $(CHARS_V512) *" >> wyniki.txt
	./main sort dane.txt $(RECORDS_V2) $(CHARS_V512) sys 
	echo "\n* Test sort_lib dla $(RECORDS_V2) x $(CHARS_V512) *" >> wyniki.txt
	./main sort dane2.txt $(RECORDS_V2) $(CHARS_V512) lib
	echo "\n* Test copy_sys dla $(RECORDS_V2) x $(CHARS_V512) *" >> wyniki.txt
	./main copy dane.txt backup.txt $(RECORDS_V2) $(CHARS_V512) sys
	echo "\n* Test copy_lib dla $(RECORDS_V2) x $(CHARS_V512) *" >> wyniki.txt
	./main copy dane.txt backup2.txt $(RECORDS_V2) $(CHARS_V512) lib  
	make clean
v1_4096:
	./main generate dane.txt $(RECORDS_V1) $(CHARS_V4096)
	cp dane.txt dane2.txt
	echo "\n* Test sort_sys dla $(RECORDS_V1) x $(CHARS_V4096) *" >> wyniki.txt
	./main sort dane.txt $(RECORDS_V1) $(CHARS_V4096) sys 
	echo "\n* Test sort_lib dla $(RECORDS_V1) x $(CHARS_V4096) *" >> wyniki.txt
	./main sort dane2.txt $(RECORDS_V1) $(CHARS_V4096) lib
	echo "\n* Test copy_sys dla $(RECORDS_V1) x $(CHARS_V4096) *" >> wyniki.txt
	./main copy dane.txt backup.txt $(RECORDS_V1) $(CHARS_V4096) sys
	echo "\n* Test copy_lib dla $(RECORDS_V1) x $(CHARS_V4096) *" >> wyniki.txt
	./main copy dane.txt backup2.txt $(RECORDS_V1) $(CHARS_V4096) lib  
	make clean
v2_4096:
	./main generate dane.txt $(RECORDS_V2) $(CHARS_V4096)
	cp dane.txt dane2.txt
	echo "\n* Test sort_sys dla $(RECORDS_V2) x $(CHARS_V4096) *" >> wyniki.txt
	./main sort dane.txt $(RECORDS_V2) $(CHARS_V4096) sys 
	echo "\n* Test sort_lib dla $(RECORDS_V2) x $(CHARS_V4096) *" >> wyniki.txt
	./main sort dane2.txt $(RECORDS_V2) $(CHARS_V4096) lib
	echo "\n* Test copy_sys dla $(RECORDS_V2) x $(CHARS_V4096) *" >> wyniki.txt
	./main copy dane.txt backup.txt $(RECORDS_V2) $(CHARS_V4096) sys
	echo "\n* Test copy_lib dla $(RECORDS_V2) x $(CHARS_V4096) *" >> wyniki.txt
	./main copy dane.txt backup2.txt $(RECORDS_V2) $(CHARS_V4096) lib  
	make clean
v1_8192:
	./main generate dane.txt $(RECORDS_V1) $(CHARS_V8192)
	cp dane.txt dane2.txt
	echo "\n* Test sort_sys dla $(RECORDS_V1) x $(CHARS_V8192) *" >> wyniki.txt
	./main sort dane.txt $(RECORDS_V1) $(CHARS_V8192) sys 
	echo "\n* Test sort_lib dla $(RECORDS_V1) x $(CHARS_V8192) *" >> wyniki.txt
	./main sort dane2.txt $(RECORDS_V1) $(CHARS_V8192) lib
	echo "\n* Test copy_sys dla $(RECORDS_V1) x $(CHARS_V8192) *" >> wyniki.txt
	./main copy dane.txt backup.txt $(RECORDS_V1) $(CHARS_V8192) sys
	echo "\n* Test copy_lib dla $(RECORDS_V1) x $(CHARS_V8192) *" >> wyniki.txt
	./main copy dane.txt backup2.txt $(RECORDS_V1) $(CHARS_V8192) lib  
	make clean
v2_8192:
	./main generate dane.txt $(RECORDS_V2) $(CHARS_V8192)
	cp dane.txt dane2.txt
	echo "\n* Test sort_sys dla $(RECORDS_V2) x $(CHARS_V8192) *" >> wyniki.txt
	./main sort dane.txt $(RECORDS_V2) $(CHARS_V8192) sys 
	echo "\n* Test sort_lib dla $(RECORDS_V2) x $(CHARS_V8192) *" >> wyniki.txt
	./main sort dane2.txt $(RECORDS_V2) $(CHARS_V8192) lib
	echo "\n* Test copy_sys dla $(RECORDS_V2) x $(CHARS_V8192) *" >> wyniki.txt
	./main copy dane.txt backup.txt $(RECORDS_V2) $(CHARS_V8192) sys
	echo "\n* Test copy_lib dla $(RECORDS_V2) x $(CHARS_V8192) *" >> wyniki.txt
	./main copy dane.txt backup2.txt $(RECORDS_V2) $(CHARS_V8192) lib  
	make clean			

clean:
	rm dane.txt dane2.txt backup.txt backup2.txt
