compile : parser.c driver.c
	gcc -c parser.c -o parser.o
	gcc -c driver.c -o driver.o
	gcc -Wall -Werror driver.o parser.o -o compile
scanner : scanner.c scanner-driver.c
	gcc -c scanner-driver.c -o scanner-driver.o
	gcc -c scanner.c -o scanner.o
	gcc -Wall -Werror scanner-driver.o scanner.o -o scanner
clean : 
	rm -f scanner
	rm -f scanner.o
	rm -f compile
	rm -f parser.o
	rm -f driver.o
	rm -f scanner-driver.o