; This is for printing 32/64-bit ints
extern printf
global main

section .text
main:
    ; Create a stack-frame, re-aligning the stack to 16-byte alignment before calls
    push rbp

    mov	rdi, fmt
    mov	rsi, 2308
    mov	rax, 0

    ; Call printf
    call printf WRT ..plt
    
    pop	rbp		; Pop stack
;    mov	rax, 0  ; Exit code 0 (these two lines cause a segfault)
;    ret			; Return

    mov rax, 60
    mov rdi, 10
    syscall

section .data
    fmt: db "%d", 10, 0
    msg: db "Hello, World", 10, 0

;
; This is for floats but doesn't work
;
extern printf
global main

section .text
main:
    sub esp, 8  ;reserve stack for a double in stack
    mov rbx, result_num
    fld qword [ebx]   ;load float
    fstp qword [esp]  ;store double (8087 does the conversion internally)
    push fmt_flt
    call printf
    add esp, 12

    mov rax, 60
    mov rdi, 0
    syscall

section .data
    fmt_int: db "%d", 10, 0
    fmt_flt: db "%f", 10, 0

section .bss
    result_num: resb 8
