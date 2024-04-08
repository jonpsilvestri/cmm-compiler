scanner : scanner.c scanner-driver.c
	gcc -c scanner-driver.c -o scanner-driver.o
	gcc -c scanner.c -o scanner.o
	gcc -Wall -Werror scanner-driver.o scanner.o -o scanner
clean : 
	rm -f scanner
	rm -f scanner-driver.o
	rm -f scanner.o