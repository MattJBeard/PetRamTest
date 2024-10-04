; @file screenio.inc
; @brief Screen I/O routines for the PET Memory Test program
; @author Matt Beard
; @date 24/09/2024

; Clear the screen - but NOT the reserved working memory area
; Destroys A, SP
ClearScreenSub:
        txs                         ; Stash X in the unused SP

        ldx #00                     ; Working Index
        lda #32                     ; Space character to write
@loop:  sta ScreenMem, X            ; Clear page 1
        sta ScreenMem + 256, X      ; Clear page 2
        sta ScreenMem + 512, X      ; Clear page 3
        inx                         ; Next character
        bne @loop                   ; Loop until the first three pages are done

        ; Clear the last page, excluding the working memory area
@cls1:  sta ScreenMem + 768, X      ; Clear page 4
        inx                         ; Next character

        ; Have we reached the end of the usable screen memory?
        cpx #(256 - WorkingSpaceSize) 
        bne @cls1                   ; If not then clear the next character

        ; Home the cursor
        lda #$00                    ; LSB of the cursor position
        sta CursorPos               ; Store the cursor position LSByte
        lda #$80                    ; MSByte of the cursor position
        sta CursorPos + 1           ; Store the cursor position MSByte

        tsx                         ; Restore X from the unused SP
        jmp (RetAddr)               ; Return to the calling routine (stackless)


; Print a message to the screen from Addr16, and move the cursor to the next line
; Destroys A, X, Y
PrintMessageNLSub:
        lda #01                     ; Set the Newline flag
        sta NLFlag
        jmp PrintMessageCommon      ; Jump to the common print message code

; Print a message to the screen from Addr16, without moving the cursor to the next line
; Destroys A, X, Y
PrintMessageSub:
        lda #00                     ; Clear the Newline flag
        sta NLFlag

; Print a message to the screen, and move the cursor to the next line if needed
PrintMessageCommon:
        pushret                     ; Push the return address onto the stack from RetAddr

        ldy #00                     ; Message index
PMCloop:
        ldaAddr16Y                  ; Get the message byte via the on-screen lda (Addr16), Y code
        beq PMCDone                 ; If it is zero then we are done 
        cmp #64                     ; PETSCII to Screen Code conversion
        blt @store                  ; If it is less than 64 then it is already a screen code
        cmp #96                     ; Is it less than 96?
        blt @sub64                  ; If it is then we must remove 64 to convert to a screen code
        eor #32                     ; If it is 96 or more then we must remove 32 to convert to a screen code (eor will do this)
        bne @store                  ; This won't be zero, so this just jumps to the store code
@sub64: eor #64                     ; Remove 64 to convert to a screen code (eor will do this)

@store: staCursorPos                ; Store the message byte on the screen via the on-screen staCursorPos code
        docall IncCursorPosSub

        iny
        bne PMCloop                 ; Loop until the message is done

PMCDone:
        lda NLFlag
        beq PMCDone2                ; If we don't need a newline then we are done

        docall NewLineSub           ; Move the cursor to the next line

PMCDone2:
        doreturn                    ; Return to the calling routine


; Print a newline to the screen - even if we are currently at the start of a line
printNL:
        pushret                     ; Push the return address onto the stack from RetAddr
        lda CursorPos               ; Get the cursor position LSByte
        and #$07                    ; Mask out only the lower 3 bits
        bne @notNL                  ; If these bits are zero the cursor could be at the start of a line
        docall IncCursorPosSub      ; We could be at the start of a line, so move forwards one character to ensure we do a new line next
        
 @notNL:
        jmp nlCommon                ; Jump to the common new line code


; Move the cursor to the start of the next line without moving to the next line
; Destroys A, X, ByteStore
NewLineSub:
        pushret                     ; Push the return address onto the stack from RetAddr

nlCommon:
        docall IsCursorLeft         ; Is the cursor at the left edge of the screen?
        beq NLDone                  ; If it is then we are already at the start of a new line

        docall IncCursorPosSub      ; We are not at the start of a new line, so just increment the cursor position and scroll if needed
        jmp nlCommon                ; And loop back to check if we are at the start of a new line

NLDone: doreturn                    ; Return to the calling routine

CRSub:
        pushret                     ; Push the return address onto the stack from RetAddr

@CRLoop:
        docall IsCursorLeft         ; Is the cursor at the left edge of the screen?
        beq CRDone                  ; If it is then we are already at the start of a new line

        dec CursorPos               ; Move the cursor left one character
        lda #$ff
        cmp CursorPos               ; Did the decrement roll over?
        bne @CRLoop                 ; If it did not then we don't need to decrement the MSByte of the cursor position
        dec CursorPos + 1           ; If it did then decrement the MSByte of the cursor position
        jmp @CRLoop

CRDone: doreturn                    ; Return to the calling routine



; Is the cursor position at the left edge of the screen - i.e. a multiple of 40?
; On return Z-flag will be set if at the left edge, otherwise it will be clear
; Destroys A, ByteStore
IsCursorLeft:
        lda CursorPos               ; Get the cursor position LSByte
        and #$07                    ; Mask out only the lower 3 bits
        bne @NotLeft                ; If these bits are not zero the cursor is not at the start of a line
        lda CursorPos               ; Get the lower byte of the cursor position
        lsr                         ; Divide by 2
        lsr                         ; Divide by 2
        lsr                         ; Divide by 2 again, now it is CursorPos/8
        sta ByteStore               ; Store the result in ByteStore so we can use again later
        lda CursorPos + 1           ; Get the MSByte of the cursor position
        asl                         ; Multiply by 2
        asl                         ; Multiply by 2 again
        asl                         ; Multiply by 2 again
        asl                         ; Multiply by 2 again
        asl                         ; Multiply by 2 again - now the top 3 bits of CursorPos/8 are in bits 5, 6 and 7
        clc                         ; Clear the carry flag
        adc ByteStore               ; Add the lower 5 bits of the cursor position to the top 3 bits so we have the complete (CursorPos / 8)

        ; Now we need to determine if the value in A is a multiple of 5
@loop:  cmp #5                      ; Is it less than 5?
        blt @NotLeft                ; If so not a multiple of 5
        sbc #5                      ; Subtract 5 (the carry will already be set as blt is bcc)
        bne @loop                   ; If it is not zero then loop again

        lda #0                      ; Must be a multiple of 40, so set the Z flag
        jmp (RetAddr)               ; And return

@NotLeft:                           ; We are not at the start a line
        lda #1                      ; So clear the Z flag
        jmp (RetAddr)               ; And return


; Move the cursor to the next screen location, and scroll if needed
; Destroys A, X
IncCursorPosSub:
        pushret                     ; Push the return address onto the stack from RetAddr

        inc CursorPos               ; Move to the next screen location
        bne @skipInc                ; If not then skip the next section
        inc CursorPos + 1           ; If it did then increment the MSByte of the cursor position

@skipInc:
        lda #<WorkingSpace          ; The LSB of the start of the screen memory we are using for working memory
        cmp CursorPos               ; If this matches, we may need to scroll
        bne skipScroll              ; If it does not match then we don't need to scroll
        lda #>WorkingSpace          ; The MSB of the start of the screen memory we are using for working memory
        cmp CursorPos + 1           ; If this matches, we do need to scroll
        bne skipScroll              ; If it does not match then we don't need to scroll

        ; Scroll the screen
        docall ScrollScreenSub
        
skipScroll:
        doreturn                    ; Return to the calling routine


; Scroll the screen up by one line, excluding the working memory area
; Destroys A, X
ScrollScreenSub:
        ldx #0                      ; Working Index
@loop:  lda ScreenMem + 40, X       ; Load the character from the next line
        sta ScreenMem, X            ; Store it on the current line
        inx                         ; Next character
        bne @loop                   ; Loop until the first page is done
        
        ; Page 2
@loop2: lda ScreenMem + 256 + 40, X ; Load the character from the next line
        sta ScreenMem + 256, X      ; Store it on the current line
        inx                         ; Next character
        bne @loop2                  ; Loop until the second page is done

        ; Page 3
@loop3: lda ScreenMem + 512 + 40, X ; Load the character from the next line
        sta ScreenMem + 512, X      ; Store it on the current line
        inx                         ; Next character
        bne @loop3                  ; Loop until the third page is done

        ; Page 4
@loop4: lda ScreenMem + 768 + 40, X ; Load the character from the next line
        sta ScreenMem + 768, X      ; Store it on the current line
        inx                         ; Next character
        cpx #<(256 - WorkingSpaceSize); Have we reached the end of the usable screen memory?
        bne @loop4                  ; If not then loop back to copy the next character

        ; Clear the last line
        ldx #0                      ; Working Index
        lda #32
@clear: sta WorkingSpace - 40, X
        inx
        cpx #40
        bne @clear

        lda #<WorkingSpace - 40     ; Move the cursor to the start of the working memory area
        sta CursorPos

        jmp (RetAddr)               ; Return to the calling routine (stackless)


; Print the character in A to the screen, then increment the cursor position and scroll if needed
; Destroys A, X, ByteStore
PrintCharSub:
        sta ByteStore           ; Save the character to print as pushret destroys it
        pushret                 ; Push the return address onto the stack from RetAddr
        lda ByteStore           ; Get the character to print back from the store
        staCursorPos            ; Store the character on the screen
        popret                  ; Pop the return address from the stack into RetAddr
        jmp IncCursorPosSub     ; Increment the cursor position and scroll if needed (returning from there directly)


; Print the byte in Y to the screen in hex
; Destroys A, X
HexPrintSub:
        pushret                 ; Push the return address onto the stack from RetAddr

        tya                     ; Get the value to print
        lsr                     ; Shift right to get the high nibble
        lsr
        lsr
        lsr
        and #$0f                ; Mask out the high nibble
        cmp #$0a                ; Is it less than 10?
        bcc @skip1              ; If it is then skip to print as a decimal digit
        sbc #$09                ; If it is 10 or more then subtract 10 (remember the carry is set!), making screen codes A-F
        bne @store1             ; Then display this value (it won't be zero so the branch is unconditional)
@skip1: adc #$30                ; Add $30 to get the ASCII value for 0-9 or A-F
@store1:PrintChar               ; Print the high nibble

        tya                     ; Get the value to print
        and #$0f                ; Mask out the low nibble
        cmp #$0a                ; Is it less than 10?
        bcc @skip2              ; If it is then skip to print as a decimal digit
        sbc #$09                ; If it is 10 or more then subtract 10 (remember the carry is set!), making screen codes A-F
        bne @store2             ; Then display this value (it won't be zero so the branch is unconditional)
@skip2: adc #$30                ; Add $30 to get the ASCII value for 0-9 or A-F
@store2:PrintChar               ; Print the low nibble

        doreturn                ; Return to the calling routine

; Print a BCD value, value starts at Addr16 and consists of Y bytes
; Destroys A, X, Y
PrintBCDSub:
        pushret                 ; Push the return address onto the stack from RetAddr

        ; Scan for the first non-zero byte
        sty BCDLen              ; Store the max bytes NLFlag
        ldy #0

@loop:  cpy BCDLen              ; Loop until we have scanned all the bytes
        bne @nonZero            ; If we have not reached the last byte then check the value
        jmp @zeroValue          ; We reached the last byte without finding a non-zero-byte total value is zero

@nonZero:
        ldaAddr16Y              ; Get the next byte of the value
        bne @firstByte          ; Did we find a non-zero byte?
        iny                     ; If not then move to the next byte
        bne @loop               ; Loop until we have scanned all the bytes - this is effectivly bra as iny won't wrap!

@firstByte:
        ; Print the first byte - which may have a leading zero that needs skipping
        dopushA                 ; Store the byte value on the stack as we are about to destroy it
        cmp #$10                ; Is it less than 10?
        blt @printLSD           ; If it is then skip the first digit
        
        ; Print the first digit
@printMSD:        
        lsr
        lsr
        lsr
        lsr
        clc
        adc #$30                ; Add $30 to get the ASCII value for 0-9
        PrintChar               ; Print the digit
        
@printLSD:
        dopopA                  ; Get the byte value back
        and #$0f                ; Get only the low nibble
        clc
        adc #$30                ; Add $30 to get the ASCII value for 0-9
        PrintChar               ; Print the digit

        iny                     ; Move to the next byte
        cpy BCDLen              ; Have we reached the last byte?
        beq @done               ; If we have then we are done
        
        ldaAddr16Y              ; Get the next byte of the value 
        dopushA                 ; Store the byte value on the stack as we are about to destroy it
        jmp @printMSD           ; Go print the byte

        ; The value is zero, so print a single zero character
@zeroValue:
        lda #$30                ; Print a zero
        PrintChar

@done:
        doreturn                ; Return to the calling routine


; Print an address as $yy00 (yy is the contents of Y)
PrintAddr00:
        pushret                 ; Store the return address
        lda #'$'                ; Print $ before the address
        PrintChar
        docall HexPrintSub      ; Print the high byte of the address
        lda #'0'                ; Low byte will be 00
        PrintChar
        lda #'0'                ; Low byte will be 00
        PrintChar
        doreturn

PrintAddr00NL:
        pushret
        docall PrintAddr00
        docall printNL
        doreturn

; Print an address as $yyFF (yy is the contents of Y)
PrintAddrFF:
        pushret                 ; Store the return address
        lda #'$'                ; Print $ before the address
        PrintChar
        docall HexPrintSub      ; Print the high byte of the address
        lda #'F' - 64           ; Low byte will be FF
        PrintChar
        lda #'F' - 64           ; Low byte will be FF
        PrintChar
        doreturn
