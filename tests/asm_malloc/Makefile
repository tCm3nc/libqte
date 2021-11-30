all:
	nasm -f elf -l malloc1.lst malloc1.asm
	gcc -m32 -o malloc1 malloc1.o
clean:
	rm malloc1.o
	rm malloc1
