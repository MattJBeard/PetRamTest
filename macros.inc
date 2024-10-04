; @File: macros.inc
; @Brief: Macro definitions, including hand-rolled stack in the screen memory
; @Author: Matt Beard
; @Date: 24/09/2024

; Rename bcs as bge for clarity
.macro bge label
	bcs label
.endmacro

; Rename bcc as blt for clarity
.macro blt label
	bcc label
.endmacro

; Push the return address onto the stack from the RetAddr location
.macro pushret
        txs                     ; Save X in the unused SP
        ldx StackPtr            ; Load the stack pointer into X
        lda RetAddr             ; Load the low byte of the return address
        sta Stack, X            ; Store the low byte of the return address on the stack
        inx                     ; Increment the stack pointer
        lda RetAddr + 1         ; Load the high byte of the return address
        sta Stack, X            ; Store the high byte of the return address on the stack
        inx                     ; Increment the stack pointer
        stx StackPtr            ; Store the new stack pointer
        tsx                     ; Restore X from the unused SP
.endmacro

; Pop the return address off the stack into RetAddr
.macro popret
        txs                     ; Save X in the unused SP
        ldx StackPtr            ; Load the stack pointer into X
        dex                     ; Decrement the stack pointer
        lda Stack, X            ; Load the high byte of the return address
        sta RetAddr + 1         ; Store the high byte of the return address
        dex                     ; Decrement the stack pointer
        lda Stack, X            ; Load the low byte of the return address
        sta RetAddr             ; Store the low byte of the return address
        stx StackPtr            ; Store the new stack pointer
        tsx                     ; Restore X from the unused SP
.endmacro

; Return from a subroutine by pulling the return address from the stack
.macro doreturn
        txs                     ; Save X in the unused SP
        ldx StackPtr            ; Load the stack pointer into X
        dex                     ; Decrement the stack pointer
        lda Stack, X            ; Load the high byte of the return address
        sta RetAddr + 1         ; Store the high byte of the return address
        dex                     ; Decrement the stack pointer
        lda Stack, X            ; Load the low byte of the return address
        sta RetAddr             ; Store the low byte of the return address
        stx StackPtr            ; Store the new stack pointer
        tsx                     ; Restore X from the unused SP
        jmp (RetAddr)           ; Jump to the return address
.endmacro

; Expensive way to push A on the stack
.macro dopushA
        txs                     ; Save X in the unused SP
        ldx StackPtr            ; Load the stack pointer into X
        sta Stack, X            ; Store the value on the stack
        inx                     ; Increment the stack pointer
        stx StackPtr            ; Store the new stack pointer
        tsx                     ; Restore X from the unused SP
.endmacro

; Expensive way to pop A from the stack
.macro dopopA
        txs                     ; Save X in the unused SP
        ldx StackPtr            ; Load the stack pointer into X
        dex                     ; Decrement the stack pointer
        lda Stack, X            ; Load the value from the stack
        stx StackPtr            ; Store the new stack pointer
        tsx                     ; Restore X from the unused SP
.endmacro

; Call a subroutine by storing the return address in the pre-stack area and jumping to the label
; Destroys A
.macro docall label
        .local @retlab          ; Local scope for the return label
        lda #<@retlab           ; Load the low byte of the return address
        sta RetAddr             ; Store the low byte of the return address
        lda #>@retlab           ; Load the high byte of the return address
        sta RetAddr + 1         ; Store the high byte of the return address
        jmp label               ; Jump to the subroutine
@retlab:                        ; Label for the return address, so we can push it on the stack
.endmacro

; Call a subroutine by storing the return address in the pre-stack area and jumping to the label
; Destroys X
.macro docallX label
        .local @retlab          ; Local scope for the return label
        ldx #<@retlab           ; Load the low byte of the return address
        stx RetAddr             ; Store the low byte of the return address
        ldx #>@retlab           ; Load the high byte of the return address
        stx RetAddr + 1         ; Store the high byte of the return address
        jmp label               ; Jump to the subroutine
@retlab:                        ; Label for the return address, so we can push it on the stack
.endmacro

; Print the character in A at the current cursor position and increment the cursor position, scrolling if needed
.macro PrintChar
        docallX PrintCharSub    ; Call the PrintCharSub subroutine destroying X, but preserving A
.endmacro

; Call a small subroutine in the screen memory that does lda (Addr16), Y without needing zero page!
; This allows the two bytes of Addr16 to be changed to read from different addresses
.macro ldaAddr16Y
        .local @retlab          ; Local scope for the return label
        lda #<@retlab           ; Load the low byte of the return address
        sta RetAddr             ; Store the low byte of the return address
        lda #>@retlab           ; Load the high byte of the return address
        sta RetAddr + 1         ; Store the high byte of the return address
        jmp ldaAddr16YSub       ; Jump to the subroutine
@retlab:                        ; Label for the return address, so we can push it on the stack
.endmacro

; Initialise the code to run ldaAddr16Y
.macro initldaAddr16Y
        lda #$b9                ; LDA (Addr16), Y opcode
        sta ldaAddr16YSub
        lda #$6c                ; JMP (RetAddr) opcode
        sta Addr16 + 2
        lda #<RetAddr           ; Low byte of the return address
        sta Addr16 + 3
        lda #>RetAddr           ; High byte of the return address
        sta Addr16 + 4
.endmacro

; Call a small subroutine in the screen memory that does sta (TestAddr),Y without needing zero page!
; This allows the two bytes of TestAddr to be changed to write to different addresses
.macro staTestAddrY
        .local @retlab          ; Local scope for the return label
        txs                     ; Save X in the unused SP (we can't use A to set return address as it has a value to store)
        ldx #<@retlab           ; Load the low byte of the return address
        stx RetAddr             ; Store the low byte of the return address
        ldx #>@retlab           ; Load the high byte of the return address
        stx RetAddr + 1         ; Store the high byte of the return address
        tsx                     ; Restore X from the unused SP
        jmp staTestAddrYSub     ; Jump to the subroutine
@retlab:                        ; Label for the return address, so we can push it on the stack
.endmacro

; Initialise the code to run staTestAddr
.macro initstaTestAddrY
        lda #$99                ; STA (CursorPos),Y opcode
        sta staTestAddrYSub
        lda #$6c                ; JMP (RetAddr) opcode
        sta TestAddr + 2
        lda #<RetAddr           ; Low byte of the return address
        sta TestAddr + 3
        lda #>RetAddr           ; High byte of the return address
        sta TestAddr + 4
.endmacro

; Call a small subroutine in the screen memory that does sta (CursorPos) without needing zero page!
; This allows the two bytes of CursorPos to be changed to write to different addresses
.macro staCursorPos
        .local @retlab          ; Local scope for the return label
        txs                     ; Save X in the unused SP (we can't use A to set return address as it has a value to store)
        ldx #<@retlab           ; Load the low byte of the return address
        stx RetAddr             ; Store the low byte of the return address
        ldx #>@retlab           ; Load the high byte of the return address
        stx RetAddr + 1         ; Store the high byte of the return address
        tsx                     ; Restore X from the unused SP
        jmp staCursorPosSub     ; Jump to the subroutine
@retlab:                        ; Label for the return address, so we can push it on the stack
.endmacro

; Initialise the code to run staCursorPos and home the cursor
.macro initstaCursorPos
        lda #$8d                ; STA (CursorPos) opcode
        sta staCursorPosSub
        lda #$00                ; Home the cursor
        sta CursorPos
        lda #$80
        sta CursorPos + 1
        lda #$6c                ; JMP (RetAddr) opcode
        sta CursorPos + 2
        lda #<RetAddr           ; Low byte of the return address
        sta CursorPos + 3
        lda #>RetAddr           ; High byte of the return address
        sta CursorPos + 4
.endmacro

; Print a message to the screen, but DON'T move the cursor to the next line
.macro printMessage message
        lda #<message
        sta Addr16
        lda #>message
        sta Addr16 + 1
        docall PrintMessageSub
.endmacro

; Print a message to the screen, and move the cursor to the next line
.macro printMessageNL message
        lda #<message
        sta Addr16
        lda #>message
        sta Addr16 + 1
        docall PrintMessageNLSub
.endmacro

