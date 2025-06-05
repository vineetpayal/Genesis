global print_int
global input_int
section .data
    digit_str db "0123456789", 0    ; For conversion reference (if needed)
    zero_msg  db "0", 0             ; Message for printing zero
    minus_msg db "-", 0            ; Minus sign for negative numbers
    newline db 10, 0


section .bss
    out_buffer resb 32              ; Buffer for number-to-string conversion
    in_buffer  resb 32              ; Buffer for reading input

section .text


; print_int: Print the integer in RDI
; Expected:
;   RDI - integer to print
; Uses:
;   RAX, RBX, RCX, RDX, RSI, RDI (clobbered), returns normally.
print_int:
    push rbp
    mov rbp, rsp
    sub rsp, 32               ; Reserve some stack space

    mov rbx, rdi              ; Store original value in RBX
    ; Check if number is zero
    cmp rbx, 0
    jne .convert_number

    ; If zero, write the "0" message.
    mov rax, 1                ; sys_write system call number
    mov rdi, 1                ; File descriptor: stdout
    lea rsi, [rel zero_msg]   ; Address of zero_msg
    mov rdx, 1                ; Write 1 byte ("0")
    syscall
    jmp .done

.convert_number:
    ; If negative, print '-' and negate number
    mov rax, rbx
    cmp rax, 0
    jge .convert_digits
    ; Print '-' sign
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel minus_msg]
    mov rdx, 1
    syscall
    neg rbx                 ; Convert to positive

.convert_digits:
    ; Initialize pointer to end of out_buffer.
    lea rdi, [rel out_buffer + 32]
    xor rcx, rcx            ; Digit count = 0

.convert_loop:
    ; Divide rbx by 10 repeatedly
    xor rdx, rdx            ; Clear RDX before division
    mov rax, rbx
    mov r8, 10
    div r8                  ; After division: RAX = quotient, RDX = remainder
    ; Convert remainder to ASCII character
    add rdx, '0'
    dec rdi                 ; Move pointer backwards
    mov [rdi], dl           ; Store the digit
    inc rcx                 ; Increase digit count
    mov rbx, rax            ; Update number (quotient)
    cmp rbx, 0
    jne .convert_loop

    ; --- After finishing conversion loop, RDI holds pointer to the start of our digit string ---
    mov rsi, rdi            ; Save the pointer to our string in RSI
    mov rax, 1              ; sys_write
    mov rdi, 1              ; STDOUT file descriptor (1)
    mov rdx, rcx            ; Number of bytes to write (number of digits)
    syscall

    ; Write the newline character
    mov rax, 1              ; sys_write
    mov rdi, 1              ; stdout file descriptor
    lea rsi, [rel newline]  ; Load the address of the newline character into RSI
    mov rdx, 1              ; Writing one byte
    syscall



.done:
    leave
    ret


; input_int: Reads an integer from STDIN
; Returns:
;   RAX contains the converted integer
input_int:
    push rbp
    mov rbp, rsp
    sub rsp, 32                ; Reserve some space

    ; Read from STDIN into in_buffer (max 32 bytes)
    mov rax, 0                 ; sys_read system call number
    mov rdi, 0                 ; File descriptor: STDIN
    lea rsi, [rel in_buffer]
    mov rdx, 32                ; Maximum bytes to read
    syscall

    ; At this point, RAX contains the number of bytes read (ignored here)
    ; Begin converting the input string in in_buffer to an integer.
    lea rsi, [rel in_buffer]   ; Pointer to the input buffer
    xor rax, rax               ; Clear accumulator (will hold the integer)
    mov bl, 1                  ; Assume positive integer (BL = sign multiplier)

    ; Check for optional sign at beginning
    mov cl, byte [rsi]
    cmp cl, '-'
    jne .check_plus
    mov bl, -1
    inc rsi
    jmp .parse_digits

.check_plus:
    cmp cl, '+'
    jne .parse_digits
    inc rsi

.parse_digits:
    mov cl, byte [rsi]      ; load the character into CL
    cmp cl, '0'
    jb .done_parse
    cmp cl, '9'
    ja .done_parse
    imul rax, rax, 10       ; multiply the accumulator by 10
    movzx rcx, cl           ; move the character to RCX, zero extended
    sub rcx, '0'            ; convert ASCII to numeric digit
    add rax, rcx            ; add the digit to the accumulator
    inc rsi
    jmp .parse_digits


.done_parse:
    ; Apply sign
    imul rax, rbx            ; Multiply by 1 or -1
    leave
    ret

