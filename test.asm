extern printf
global main

section .text
main:
    xor rax, rax
    mov rax, 3
    push rax
    xor rax, rax
    mov rax, 15
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax

    xor rax, rax
    mov rax, 8
    push rax
    pop rbx
    pop rax
    sub rax, rbx
    push rax

    xor rax, rax
    mov rax, 0
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax


if1_0:
    xor rax, rax
    mov rax, 3
    push rax
    xor rax, rax
    mov rax, 1
    push rax
    pop rbx
    pop rax
    cmp rax, rbx
    jg comp_true0
comp_false0:
    mov rax, 0
    push rax
    jmp comp_end0
comp_true0:
    mov rax, 1
    push rax
comp_end0:

    pop rax
    test rax, rax
    jz if2_0

    push QWORD [rsp + 0]
    pop rax
    mov rbx, -1
    mul rbx
    push rax
    xor rax, rax
    mov rax, 3
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax

    pop rsi
    push rbp
    lea rdi, [rel fmt_int]
    mov	rax, 0
    call printf WRT ..plt
    pop	rbp
    xor rax, rax
    mov rax, 2
    push rax
    pop rsi
    push rbp
    lea rdi, [rel fmt_int]
    mov	rax, 0
    call printf WRT ..plt
    pop	rbp
    jmp if_end0

if2_0:
    push QWORD [rsp + 0]
    xor rax, rax
    mov rax, 2
    push rax
    pop rbx
    pop rax
    sub rax, rbx
    push rax

    xor rax, rax
    mov rax, 0
    push rax
    pop rbx
    pop rax
    cmp rax, rbx
    jg comp_true1
comp_false1:
    mov rax, 0
    push rax
    jmp comp_end1
comp_true1:
    mov rax, 1
    push rax
comp_end1:

    pop rax
    test rax, rax
    jz if3_0

    push QWORD [rsp + 0]
    pop rax
    mov rbx, -1
    mul rbx
    push rax
    pop rsi
    push rbp
    lea rdi, [rel fmt_int]
    mov	rax, 0
    call printf WRT ..plt
    pop	rbp
    xor rax, rax
    mov rax, 3
    push rax
    push QWORD [rsp + 8]
    pop rbx
    pop rax
    mul rbx
    push rax

    xor rax, rax
    mov rax, 3
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax

    pop rsi
    push rbp
    lea rdi, [rel fmt_int]
    mov	rax, 0
    call printf WRT ..plt
    pop	rbp
    jmp if_end0

if3_0:
    xor rax, rax
    mov rax, 5
    push rax
    pop rsi
    push rbp
    lea rdi, [rel fmt_int]
    mov	rax, 0
    call printf WRT ..plt
    pop	rbp
    jmp if_end0

if_end0:

    mov rax, 60
    mov rdi, 0
    syscall

section .data
    fmt_int: db "%d", 10, 0
    fmt_flt: db "%f", 10, 0
