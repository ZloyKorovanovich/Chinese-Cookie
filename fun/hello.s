bits 64
default rel
global efi_main

%include "efi_tables.s"

section .data
; utf-16 Hello World!\r\n
hello_msg: dw 'H','e','l','l','o',' ','W','o','r','l','d','!',13,10,0

section .text

;params:
;rcx = ImageHandle
;rdx = SystemTable
efi_main:
    push rbp
    mov rbp, rsp
    sub rsp, 88

    ;save params on stack
    mov [rbp - 16], rcx
    mov [rbp - 24], rdx

    ; restore rdx
    mov  rdx, [rbp - 24]

    ; clear screen
    mov  r8 , [rdx + EFI_SYS_COUT_INTERFACE]
    mov  rax, [r8 + EFI_TEXT_OUT_CLEAR_SCREEN]
    mov  rcx, r8
    call rax

    test rax, rax
    jnz  .fail

    ; restore rdx
    mov  rdx, [rbp - 24]

    ; print hello
    mov  r8 , [rdx + EFI_SYS_COUT_INTERFACE]
    mov  rax, [r8 + EFI_TEXT_OUT_OUTPUT_STRING]
    mov  rcx, r8
    lea  rdx, [hello_msg]
    call rax

    ; restore rdx
    mov  rdx, [rbp - 24]

    ; get cpl (current privelege level)
    xor rax, rax
    mov ax , cs
    and ax , 0x3
    add ax , 48
    
    ; write str into memory
    mov word [rbp - 56], 'C'
    mov word [rbp - 54], 'P'
    mov word [rbp - 52], 'L'
    mov word [rbp - 50], ':'
    mov word [rbp - 48],  ax
    mov word [rbp - 46],  13
    mov word [rbp - 44],  10
    mov word [rbp - 42],  0

    ; print cpl
    mov  r8 , [rdx + EFI_SYS_COUT_INTERFACE]
    mov  rax, [r8 + EFI_TEXT_OUT_OUTPUT_STRING]
    mov  rcx, r8
    lea  rdx, [rbp - 56]
    call rax

    xor rax, rax

    mov rsp, rbp
    pop rbp
    ret

    .fail:
        mov rax, 0xffffffffffffffff
        mov rsp, rbp
        pop rbp
        ret
