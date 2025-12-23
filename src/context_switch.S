.section .note.GNU-stack,"",@progbits
.intel_syntax noprefix

.text
.globl context_switch 
.type context_switch, @function
context_switch:
    # Note: return address is pushed onto stack first

    # save context on current stack
    push rbp
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    # save old stack pointer
    mov [rdi], rsp

    # switch to new stack pointer
    mov rsp, rsi

    # load context from new stack
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    pop rbp

    ret
    