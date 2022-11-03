%define SYS_EXT 60
%define SYS_WRITE 1
%define ASCII_SHIFT 48
%define HEX_32 0x20
%define HEX_9 0x9
%define HEX_10 0xA

section .text
 
; Принимает код возврата и завершает текущий процесс
exit: 
    mov rax, SYS_EXT
    syscall

; Принимает указатель на нуль-терминированную строку, возвращает её длину
string_length:
    xor rax, rax
    .loop:
        cmp byte [rdi+rax],0
        je .ext
        inc rax
        jmp .loop
    .ext:
        ret

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string:
    mov rsi, rdi
    call string_length
    mov rdx, rax
    mov rdi, 1
    mov rax, 1
    syscall
    ret

; Принимает код символа и выводит его в stdout
print_char:
    push rdi
    mov rdi, rsp
    call print_string
    pop rdi
    ret

; Переводит строку (выводит символ с кодом 0xA)
print_newline:
    mov rdi, 10
    call print_char

; Выводит беззнаковое 8-байтовое число в десятичном формате 
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
print_uint:
    mov r9, rsp
    dec rsp
    mov byte [rsp], 0x0
    xor r8, r8
    xor rax, rax
    mov rax, rdi
    mov r8,10
    .loop:
        xor rdx, rdx
        div r8
        add rdx, ASCII_SHIFT
        dec rsp
        mov [rsp], dl
        test rax, rax
        jz .ext
        jmp .loop
    .ext:
        mov rdi, rsp
        call print_string
        mov rsp, r9
        ret

; Выводит знаковое 8-байтовое число в десятичном формате 
print_int:
    test rdi, rdi
    jns print_uint
    push rdi
    mov rdi, '-'
    call print_char
    pop rdi
    neg rdi
    call print_uint

; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
    xor rax, rax
    xor rdx, rdx
    xor r8,r8
    xor r9,r9
    .loop:
        mov r8b, byte[rdi + rdx]
        mov r9b, byte[rsi + rdx]
        cmp r8b, r9b
        jne .end
        add r8b, r9b
        cmp r8b, 0
        jz .true
        inc rdx
        jmp .loop
    .true:
        inc rax
    .end:
        ret

; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:    
    xor rax, rax 
    xor rdi, rdi
    push 0 
    mov rsi, rsp 
    mov rdx, 1
    syscall
    pop rax
    ret

; Принимает: адрес начала буфера, размер буфера
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор

read_word:
    xor rcx, rcx 
    mov r8, rdi
    mov r9, rsi
    .loop:
        push rcx
        call read_char
        pop rcx
        cmp rax, HEX_32
        je .b
        cmp rax, HEX_9
        je .b
        cmp rax, HEX_10
        je .b
        cmp rax, 0
        jz .ext
        cmp rcx, r9
        jnl .E
        mov byte[r8+rcx], al
        inc rcx
        jmp .loop
    .b:
        cmp rcx, 0
        je .loop
    .ext:   
        mov byte[r8+rcx], 0
        mov rax, r8
        mov rdx, rcx
        ret 
    .E:
        xor rax, rax
        ret

; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
    mov r8, 10
    xor rax, rax
    xor rcx, rcx
    .loop:
        movzx rsi, byte [rdi + rcx] 
        cmp sil, '0'
        jb .ext
        cmp sil, '9'
        ja .ext
        xor rdx, rdx 
        mul r8
        and sil, 0x0f
        add rax, rsi
        inc rcx 
        jmp .loop
    .ext:
        mov rdx, rcx
        ret

; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был) 
; rdx = 0 если число прочитать не удалось
parse_int:
    mov al, byte [rdi]
    cmp al, '-'
    je .f
    call parse_uint
    ret
.f:
    inc rdi
    call parse_uint
    neg rax
    test rdx, rdx
    jz .b
    inc rdx
    ret
.b:
    xor rax, rax
    ret 


; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
    push rdi
    push rsi
    push rdx
    call string_length
    pop rdx
    pop rsi
    pop rdi
    cmp rdx, rax
    jbe .ext
    push rax
    .loop: 
        mov dl, byte[rdi]
        mov byte[rsi], dl
        inc rdi
        inc rsi
        cmp dl, 0
        jnz .loop 
        pop rax 
        ret
    .ext:
        xor rax, rax
        ret