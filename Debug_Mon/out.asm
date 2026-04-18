.686
.model flat, c
option casemap:none
public main
extern printf:PROC
extern scanf:PROC
extern sqrtf:PROC
extern putchar:PROC
extern getchar:PROC
extern pow:PROC
extern sinf:PROC
extern cosf:PROC
extern tanf:PROC
extern strlen:PROC
extern malloc:PROC
extern realloc:PROC
extern memcpy:PROC
extern free:PROC
extern fopen:PROC
extern fprintf:PROC
extern fclose:PROC


.data
stringLiteral_0 db 72,101,108,108,111,44,32,87,111,114,108,100,33,10,0



.code

main PROC
    push ebp
    mov ebp, esp
    sub esp, 16

    
    mov eax, offset printf
    mov dword ptr [ebp-4], eax
    
    
    mov eax, offset stringLiteral_0
    mov dword ptr [ebp-12], eax
    
    sub esp, 4
    mov eax, dword ptr [ebp-12]
    mov dword ptr [esp], eax
    
    mov eax, dword ptr [ebp-4]
    call eax
    add esp, 4
    
    mov dword ptr [ebp-8], eax
    
    mov eax, 0
    mov dword ptr [ebp-16], eax
    
    mov eax, dword ptr [ebp-16]
    jmp main_return

    main_return:
    mov esp, ebp
    pop ebp
    ret
main ENDP

end