; malloc1.asm
; Assemble : nasm -f elf -l malloc1.lst malloc1.asm
; link : gcc -m32 -o malloc1 malloc1.o
; Run : malloc1
; Output : ??

; Declare some external functions
; 
    extern malloc, free, memcpy
    SECTION .data ; Initialised variables
    ; foo

    SECTION .text ; Code section
    global main ; standard entry point for gcc. 
main:
    push ebp
    mov ebp, esp ; Setup the stack frame, save old and swap it to EBP
    mov eax, 0x1000; size
    push eax
    call malloc
    ; fix stack frame
    add esp, 1*4  ; 1 param only to malloc

    ; Use the malloc ptr to write some data
    mov dword[eax], 0xfeedface
    ; Allocate another round of memory.
    mov ebx, eax
    mov eax, 0x1000; size
    push eax
    call malloc
    ; fix stack frame
    add esp, 1*4 ; 1 param only to malloc
    ; Write to the LAST of the buffer
    mov dword[eax+0x1000-4], 0xdeadbeef

    ; memcpy from one to other
    mov ecx, 0x1000
    push ecx ; count
    push ebx ; src 
    push eax ; dest
    call memcpy
    ; fix stack frame
    add esp, 3*4; 3 params to memcpy

    ; free the malloced ptr
    push eax
    call free
    ; fix stack frame
    add esp, 1 * 4 ; 1 param to free

    ; free the first malloced ptr
    push ebx
    call free
    add esp, 1 * 4 ; 1 param to free

    ; teardown stackframe
    mov esp, ebp
    pop ebp
    ; return 0 as success
    mov eax, 0;
    ret
