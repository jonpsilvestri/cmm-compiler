compile : parser.c driver.c ast.h ast-print.c codegen.c
	gcc -c parser.c -o parser.o
	gcc -c driver.c -o driver.o
	gcc -c ast-print.c -o ast-print.o
	gcc -c codegen.c -o codegen.o
	gcc -Wall -Werror driver.o parser.o ast-print.o codegen.o -o compile
clean : 
	rm -f compile
	rm -f parser.o
	rm -f driver.o
	rm -f codegen.o
	rm -f ast-print.o  