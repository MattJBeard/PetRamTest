
; Test the memory from TestStartPage to TestEndPage
MainMemoryTest:
        pushret                 ; Store the return address

        printMessage TestStartMessage

        ldy TestStartPage
        docall PrintAddr00

        lda #'-'
        PrintChar

        ldy TestEndPage         ; High byte of the end address
        dey                     ; One less than the end address
        docall PrintAddrFF

        ; Zero phase error counters
        lda #$00
        ldy #PhaseErrorsBytes - 1
@loop:  sta BCDPhaseErrors, Y
        dey
        bpl @loop

        ; Zero the bit erros
        sta PhaseBitErrors

        docall printNL

MainMarchU:
        ; +(w0) Start by filling memory with the test value
        ldx #$00                ; Index of test value in TestTable   

NextTestValue:
        lda TestTable, X        ; Value to test
        bne @valid              ; Non-zero value is a valid test value
        jmp MemoryTestDone      ; If the value is zero then we are done and the test passed

@valid: inx                     ; Move to the next test value
        stx TestIndex           ; Store the index of the test value so we can re-use X below
        sta TestValue           ; Store a copy of the test value in X so A can be destroyed in tests
        ldy #$00                ; Index of byte being tested
        ldx TestStartPage       ; Start at the first page of memory to test
        stx TestAddr + 1        ; Store the test value in each page of the current test page
        ldx #0                  ; Start at the start of each page
        stx TestAddr

@loop:  staTestAddrY            ; Store in each page of the current test page
        iny                     ; Next byte
        bne @loop
        inc TestAddr + 1        ; Next page
        ldx TestEndPage         ; Have we reached the end of the test range?
        cpx TestAddr + 1
 
        bne @loop               ; If not, continue testing


        ; +(r0,w1,r1,w0) For each byte, check it was stored correctly, then store the inverse, check that, then restore the original value
Phase2: ldx TestStartPage       ; Start at the first page of memory to read
        stx Addr16 + 1          ; Reset the read address
        stx TestAddr + 1        ; Reset the write address
        ldx #0                  ; Start at the start of each page
        stx Addr16
        
@loop1: ldaAddr16Y              ; Test the value was stored correctly, and not damaged by other writes
        cmp TestValue           ; Check the value against the current TestValue
        beq @ok                 ; If it was then continue testing
        
        tax                     ; The value we read goes in X
        lda TestValue           ; Get the value we expected
        sta ByteStore           ; Store the expected value in ByteStore
        docall SetErrorFix      ; Record this error

@ok:    lda TestValue           ; Get the test value
        eor #$ff                ; Invert the test value
        sta ByteStore           ; Store the inverted value for testing
        staTestAddrY            ; Store the inverted value in the byte to test

        ldaAddr16Y              ; Re-read the written byte
        cmp ByteStore           ; Check it matches the inverted value we stored
        beq @ok2                ; If it does then continue testing

        tax                     ; The value we read goes in X
        sta $83bb
        docall SetErrorFix      ; Record this error (the expected value is already in ByteStore)

@ok2:   lda TestValue           ; Recover the test value
        staTestAddrY            ; Restore the original value to the test byte

        iny                     ; Next test location
        bne @loop1              ; Loop until this page is done

        inc TestAddr + 1        ; Next page (for writing)
        inc Addr16 + 1          ; Next page (for reading)
        ldx TestEndPage         ; Have we reached the end of the test range?
        cpx TestAddr + 1        ; Only compare writing page as we are reading the same page
beq @skip
jmp @loop1
        ;bne @loop1              ; If not, continue testing
@skip:

        ; Delay for about 100ms - bit-rot test
        docall Delay100ms

        ; +(r0, w1) For each byte, check it was stored correctly, then store the inverse
Phase3: ldx TestStartPage       ; Start at the first page of memory to read
        stx Addr16 + 1          ; Reset the read address
        stx TestAddr + 1        ; Reset the write address

        ldy #0                  ; Start at the start of each page (the delay code will have destroyed Y)
@loop1: ldaAddr16Y              ; Test the value was stored correctly, and not damaged by other writes or the delay
        cmp TestValue           ; Check the value against the current TestValue
        beq @ok3                ; If it was then continue testing
        
        tax                     ; The value we read goes in X
        lda TestValue           ; Get the value we expected
        sta ByteStore           ; Store the expected value in ByteStore
        sta $83bc
        docall SetErrorFix      ; Record this error

@ok3:   lda TestValue           ; Get the test value
        eor #$ff                ; Invert the test value
        sta ByteStore           ; Store the inverted value for testing
        staTestAddrY            ; Store the inverted value in the byte to test

        iny                     ; Next test location
        bne @loop1              ; Loop until this page is done

        inc TestAddr + 1        ; Next page (for writing)
        inc Addr16 + 1          ; Next page (for reading)
        ldx TestEndPage         ; Have we reached the end of the test range?
        cpx TestAddr + 1        ; Only compare writing page as we are reading the same page
        bne @loop1              ; If not, continue testing

        ; Delay for about 100ms - bit-rot test
        docall Delay100ms

        ; -(r1, w0, r0, w1) For each byte, check the inverted value was stored correctly, then restore the original value
        ;                   check that was stored correctly, then store the inverse
Phase4: ldx TestEndPage         ; Start at the last page of memory to read
        dex                     ; One page less as TestEndPage is the first byte after
        stx Addr16 + 1          ; Reset the read address
        stx TestAddr + 1        ; Reset the write address

        ldy #$ff                ; Start at the end of each page (the delay code will have destroyed Y)

@loop1: lda TestValue           ; Get the test value
        eor #$ff                ; Invert the test value
        sta ByteStore           ; Store the inverted value for testing
        ldaAddr16Y              ; Test the value was stored correctly in RAM, and not damaged by other writes or the delay
        cmp ByteStore           ; Check the value against the inverted value we stored
        beq @ok4                ; If it was then continue testing
        
        tax                     ; The value we read goes in X
        sta $83bd
        docall SetErrorFix      ; Record this error (the expected value is already in ByteStore)

@ok4:   lda TestValue           ; Get the test value
        staTestAddrY            ; Restore the original value to the test byte
        ldaAddr16Y              ; Re-read the written byte
        cmp TestValue           ; Check it matches the original value we stored
        beq @ok5                ; If it does then continue testing

        tax                     ; The value we read goes in X
        lda TestValue           ; Get the value we expected
        sta ByteStore           ; Store the expected value in ByteStore
        sta $83be
        docall SetErrorFix      ; Record this error

@ok5:   lda TestValue           ; Get the test value
        eor #$ff                ; Invert the test value
        staTestAddrY            ; Store the inverted value in the byte to test

        dey                     ; Next test location (backwards in this phase)
        cpy #$ff                ; Have we wrapped back to $ff?
        bne @loop1              ; If not, continue testing

        dec TestAddr + 1        ; Next page (for writing)
        dec Addr16 + 1          ; Next page (for reading)
        ldx TestStartPage       ; Have we reached the start of the test range?
        dex                     ; Actually test one less as we are looking for an underflow
        cpx TestAddr + 1        ; Only compare writing page as we are reading the same page
        beq @skip               ; If so, this phase is done
        jmp @loop1              ; If not, continue testing
@skip:

        ; -(r1, w0) For each byte, check the inverted value was stored correctly then restore the original value
Phase5: ldx TestEndPage         ; Start at the last page of memory to read
        dex                     ; One page less as TestEndPage is the first byte after
        stx Addr16 + 1          ; Reset the read address
        stx TestAddr + 1        ; Reset the write address

        ; Start at the end of each page - Y will already be $ff from above
@loop1: lda TestValue           ; Get the test value
        eor #$ff                ; Invert the test value
        sta ByteStore           ; Store the inverted value for testing
        ldaAddr16Y              ; Test the value was stored correctly in RAM, and not damaged by other writes
        cmp ByteStore           ; Check the value against the inverted value we stored
        beq @ok6                ; If it was then continue testing

        tax                     ; The value we read goes in X
        sta $83bf
        docall SetErrorFix      ; Record this error (the expected value is already in ByteStore)

@ok6:   lda TestValue           ; Get the test value
        staTestAddrY            ; Restore the original value to the test byte

        dey                     ; Next test location (backwards in this phase)
        cpy #$ff                ; Have we wrapped back to $ff?
        bne @loop1              ; If not, continue testing

        dec TestAddr + 1        ; Next page (for writing)
        dec Addr16 + 1          ; Next page (for reading)
        ldx TestStartPage       ; Have we reached the start of the test range?
        dex                     ; Actually test one less as we are looking for an underflow
        cpx TestAddr + 1        ; Only compare writing page as we are reading the same page
        bne @loop1              ; If not, continue testing

        ; All testing done for this test value - go back and get the next one

        ldx TestIndex           ; Get the index of the test value
        jmp NextTestValue       ; Go back and get the next test value

MemoryTestDone:
        lda PhaseBitErrors      ; Get the phase bit errors
        beq @done               ; If there are no errors then we are done

        docall printNL          ; There will be a bit-error display on the current line, so move down

        printMessage PhaseError ; Print the message for the errors found this phase

        lda #<BCDPhaseErrors    ; Print the phase errors BCD count
        sta Addr16
        lda #>BCDPhaseErrors
        sta Addr16 + 1
        ldy #PhaseErrorsBytes
        docall PrintBCDSub

        docall printNL          ; Plus a new line

@done:  doreturn

; Set the error bits and restore the test address in Addr16
SetErrorFix:
        pushret                 ; Store the return address
        docall SetError         ; Record the error
        lda TestAddr + 1        ; Get the page number
        sta Addr16 + 1          ; Store the page number
        lda TestAddr            ; Get the byte number
        sta Addr16              ; Store the byte number
        doreturn                ; Return to the caller


; Delay for about 100ms - bit-rot test (is this really needed for the screen RAM, which is static RAM?)
; This delay will destrop the contents of Y and X, but they can be restored afterwards as follows:
; Y will always be zero at his point after ending the above loop. X and A will contain the same value (the last test value)
; Inner-loop duration = 2+3 = 5 cycles, taken 256 times = 1280 cycles, less one for the final BNE = 1279 cycles
; 100ms = 100,000us = 100,000 cycles at 1MHz, so 100,000 / 1279 = 78.2 outer loops required, round up to 79 for safety
Delay100ms:
        ldx #79
@delay1:
        ldy #0                  ; 2-cycles
@delay2:
        dey                     ; 2-cycles
        bne @delay2             ; 3-cycles when taken, 2-cycles when not taken
        dex                     ; 2-cycles
        bne @delay1             ; 3-cycles when taken, 2-cycles when not taken

        ; Restore Y and X values
        ldy #$00                ; Restore Y to zero (although actually it should be this anyway)
        tax                     ; Restore X to the last test value

        jmp (RetAddr)           ; Return to the caller

; An error has occurred, the expected value will be in ByteStore, the actual value in X
; Destroys: A
SetError:
        txs                     ; Store X in the unused stack pointer

        ldx #PhaseErrorsBytes - 1
        sed                     ; Set decimal mode as the error counts are in BCD
@loop:  lda BCDPhaseErrors, X   ; Increment the phase error count
        clc
        adc #1
        sta BCDPhaseErrors, X
        bcc @skip               ; If it didn't wrap then we are done with incrementing the counter
        dex                     ; Move to the next byte
        bpl @loop               ; Loop until we have done all the bytes
@skip:  
        ldx #ErrorCountBytes - 1
@loop2: lda BCDErrorCount, X    ; Increment the full error count
        clc
        adc #1
        sta BCDErrorCount, X
        bcc @skip2              ; If it didn't wrap then we are done with incrementing the counter
        dex                     ; Move to the next byte
        bpl @loop2              ; Loop until we have done all the bytes
@skip2: cld                     ; Clear decimal mode
        
        tsx                     ; Restore the X from the stack pointer
        txa                     ; Get the actual value back
        eor ByteStore           ; Get the exact bits that are in error

        cmp PhaseBitErrors      ; Have the bits in error changed?
        beq @done               ; If they have not then we are done

        sta PhaseBitErrors      ; Store the new bits in error

        pushret                 ; Store the return address - late as we needed to use the value in A earlier!
        docall ShowPhaseErrors  ; Show the new bits in error

@done:  
        doreturn                ; Return to the caller


ShowPhaseErrors:
        pushret                 ; Store the return address

        docall CRSub            ; Start at the left of the screen, without a new line
        printMessage ThisPhaseMessage
        
        lda PhaseBitErrors      ; Get the phase bit errors
        ldy #7                  ; Current bit number (use 7 not 8 as it makes printing the bit number easier)
@NextBit:
        dopushA                 ; This needs to be stacked as we will be using A for the print
        lda #' '                ; Space before each bit number
        PrintChar               ; Print the bit number
        tya                     ; Get the current bit number
        ora #$30                ; Convert to ASCII
        PrintChar               ; Print the bit number
        lda #':'                ; Print a colon
        PrintChar               ; Print the bit number

        dopopA                  ; Get the phase bit errors back
        rol                     ; Roll this bit to the carry
        dopushA                 ; Stack the new value as the following prints will destroy it
        bcc @NotSet             ; If the carry is clear then the bit is not set
        
        lda #$98                ; Inverse uppercase 'x' to act as FAILED indicator
        jmp @BitDone

@NotSet:lda #$40                ; '-' act as PASSED indicator
@BitDone:
        PrintChar               ; Print the bit number
        dopopA                  ; Get the phase bit errors back
        dey                     ; Move to the next bit
        bmi @skip               ; If we have done all 8 bits then we are done
        jmp @NextBit            ; Otherwise, do the next bit
@skip:  doreturn                ; All done!

