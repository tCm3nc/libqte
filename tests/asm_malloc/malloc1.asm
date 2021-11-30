; malloc1.asm
; Assemble : nasm -f elf -l malloc1.lst malloc1.asm
; link : gcc -m32 -o malloc1 malloc1.o
; Run : malloc1
; Output : ??

; Declare some external functions
; 
    extern malloc, free
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

    ; free the malloced ptr
    push eax
    call free
    ; fix stack frame
    add esp, 1 * 4 ; 1 param to free

    ; teardown stackframe
    mov esp, ebp
    pop ebp
    ; return 0 as success
    mov eax, 0;
    ret
