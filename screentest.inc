; @file screentest.inc
; @brief Screen memory test for the PET Memory Test program
; @author Matt Beard
; @date 24/09/2024

DoDelayForScreen = 0            ; Set to 1 to delay for about 100ms between tests to test for bit-rot (not needed for static RAM?)
UseShortScreenTest = 1          ; Set to 1 to use a short test to save memory

ScreenTest:
.if UseShortScreenTest

ScreenMATS:
        ; +(w0) Start by filling screen memory with the test value
        ldx #$00                ; Index of test value in TestTable

@nextvalue:
        lda TestTable, X        ; Value to test
        bne @valid              ; Non-zero value is a valid test value
        jmp ScreenMemoryOK      ; If the value is zero then we are done and the test passed

@valid: inx                     ; Move to the next test value
        txs                     ; Store the index of the test value in the SP so we can re-use X below
        tax                     ; Store a copy of the test value in X so A can be destroyed in tests
        ldy #$00                ; Index of byte being tested

@loop:  sta ScreenMem, Y        ; Store in each of the four 256 byte pages of the screen memory
        sta ScreenMem + 256, Y
        sta ScreenMem + 512, Y
        sta ScreenMem + 768, Y
        iny
        bne @loop

        ; +(r0,w1) For each byte, check it was stored correctly, then store the inverse
        ; Page 1 of the Screen Memory (then must be tested one after the other to detect page errors in the RAM chips)
@loop1: txa                     ; Recover the test value
        cmp ScreenMem, Y        ; Read the current test location and see if it still contains the test value
        beq @ok                 ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok:    eor #$ff                ; Invert the test value
        sta ScreenMem, Y        ; Store the inverted value
 
        iny                     ; Next test location
        bne @loop1              ; Loop until this page is done

        ; Page 2 of the Screen Memory (then must be tested one after the other to detect page errors in the RAM chips)
@loop2: txa                     ; Recover the test value
        cmp ScreenMem + 256, Y  ; Read the current test location and see if it still contains the test value
        beq @ok3                ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok3:   eor #$ff                ; Invert the test value
        sta ScreenMem + 256, Y  ; Store the inverted value

        iny                     ; Next test location
        bne @loop2              ; Loop until this page is done

        ; Page 3 of the Screen Memory (then must be tested one after the other to detect page errors in the RAM chips)
@loop3: txa                     ; Recover the test value
        cmp ScreenMem + 512, Y  ; Read the current test location and see if it still contains the test value
        beq @ok5                ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok5:   eor #$ff                ; Invert the test value
        sta ScreenMem + 512, Y  ; Store the inverted value

        iny                     ; Next test location
        bne @loop3              ; Loop until this page is done

        ; Page 4 of the Screen Memory (then must be tested one after the other to detect page errors in the RAM chips)
@loop4: txa                     ; Recover the test value
        cmp ScreenMem + 768, Y  ; Read the current test location and see if it still contains the test value
        beq @ok7                ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok7:   eor #$ff                ; Invert the test value
        sta ScreenMem + 768, Y  ; Store the inverted value

        iny                     ; Next test location
        bne @loop4              ; Loop until this page is done

        ; +(r1) For each byte, check the inverse was stored correctly
        ; Page 1 of the Screen Memory
        txa                     ; Recover the test value
        eor #$ff                ; Invert the test value
@loop5: cmp ScreenMem, Y        ; Read the current test location and see if it still contains the inverted test value
        beq @ok9                ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

 @ok9:  iny                     ; Next test location
        bne @loop5              ; Loop until this page is done
        
        ; Page 2 of the Screen Memory
@loop6: cmp ScreenMem + 256, Y  ; Read the current test location and see if it still contains the inverted test value
        beq @ok10               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok10:  iny                     ; Next test location
        bne @loop6              ; Loop until this page is done
        
        ; Page 3 of the Screen Memory
@loop7: cmp ScreenMem + 512, Y  ; Read the current test location and see if it still contains the inverted test value
        beq @ok11               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok11:  iny                     ; Next test location
        bne @loop7              ; Loop until this page is done
        
        ; Page 4 of the Screen Memory
@loop8: cmp ScreenMem + 768, Y  ; Read the current test location and see if it still contains the inverted test value
        beq @ok12               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok12:  iny                     ; Next test location
        bne @loop8              ; Loop until this page is done

        ; All tests passed for this test value, move to the next test value
        tsx                     ; Recover the index of the next test value from the SP
        jmp @nextvalue          ; Use this next test value

.else ; UseShortScreenTest
       
ScreenMarchU:
        ; +(w0) Start by filling screen memory with the test value
        ldx #$00                ; Index of test value in TestTable   

@nextvalue:
        lda TestTable, X        ; Value to test
        bne @valid              ; Non-zero value is a valid test value
        jmp ScreenMemoryOK      ; If the value is zero then we are done and the test passed

@valid: inx                     ; Move to the next test value
        txs                     ; Store the index of the test value in the SP so we can re-use X below
        tax                     ; Store a copy of the test value in X so A can be destroyed in tests
        ldy #$00                ; Index of byte being tested

@loop:  sta ScreenMem, Y        ; Store in each of the four 256 byte pages of the screen memory
        sta ScreenMem + 256, Y
        sta ScreenMem + 512, Y
        sta ScreenMem + 768, Y
        iny
        bne @loop


        ; +(r0,w1,r1,w0) For each byte, check it was stored correctly, then store the inverse, check that, then restore the original value
        ; Page 1 of the Screen Memory (then must be tested one after the other to detect page errors in the RAM chips)
@loop1: txa                     ; Recover the test value
        cmp ScreenMem, Y        ; Read the current test location and see if it still contains the test value
        beq @ok                 ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok:    eor #$ff                ; Invert the test value
        sta ScreenMem, Y        ; Store the inverted value
        eor ScreenMem, Y        ; Check the inverted value was stored correctly
        beq @ok2                ; If it was then continue testing
        jmp ScreenError         ; If it was not then jump to the screen memory error routine

@ok2:   txa                     ; Recover the test value
        sta ScreenMem, Y        ; Restore the original value

        iny                     ; Next test location
        bne @loop1              ; Loop until this page is done

        ; Page 2 of the Screen Memory (then must be tested one after the other to detect page errors in the RAM chips)
@loop2: txa                     ; Recover the test value
        cmp ScreenMem + 256, Y  ; Read the current test location and see if it still contains the test value
        beq @ok3                ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok3:   eor #$ff                ; Invert the test value
        sta ScreenMem + 256, Y  ; Store the inverted value
        eor ScreenMem + 256, Y  ; Check the inverted value was stored correctly
        beq @ok4                ; If it was then continue testing
        jmp ScreenError         ; If it was not then jump to the screen memory error routine

@ok4:   txa                     ; Recover the test value
        sta ScreenMem + 256, Y  ; Restore the original value

        iny                     ; Next test location
        bne @loop2              ; Loop until this page is done


        ; Page 3 of the Screen Memory (then must be tested one after the other to detect page errors in the RAM chips)
@loop3: txa                     ; Recover the test value
        cmp ScreenMem + 512, Y  ; Read the current test location and see if it still contains the test value
        beq @ok5                ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok5:   eor #$ff                ; Invert the test value
        sta ScreenMem + 512, Y  ; Store the inverted value
        eor ScreenMem + 512, Y  ; Check the inverted value was stored correctly
        beq @ok6                ; If it was then continue testing
        jmp ScreenError         ; If it was not then jump to the screen memory error routine

@ok6:   txa                     ; Recover the test value
        sta ScreenMem + 512, Y  ; Restore the original value

        iny                     ; Next test location
        bne @loop3              ; Loop until this page is done


        ; Page 4 of the Screen Memory (then must be tested one after the other to detect page errors in the RAM chips)
@loop4: txa                     ; Recover the test value
        cmp ScreenMem + 768, Y  ; Read the current test location and see if it still contains the test value
        beq @ok7                ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok7:   eor #$ff                ; Invert the test value
        sta ScreenMem + 768, Y  ; Store the inverted value
        eor ScreenMem + 768, Y  ; Check the inverted value was stored correctly
        beq @ok8                ; If it was then continue testing
        jmp ScreenError         ; If it was not then jump to the screen memory error routine

@ok8:   txa                     ; Recover the test value
        sta ScreenMem + 768, Y  ; Restore the original value

        iny                     ; Next test location
        bne @loop4              ; Loop until this page is done


.if DoDelayForScreen
        ; Delay for about 100ms - bit-rot test (is this really needed for the screen RAM, which is static RAM?)
        ; This delay will destrop the contents of Y and X, but they can be restored afterwards as follows:
        ; Y will always be zero at his point after ending the above loop. X and A will contain the same value (the last test value)
        ; Inner-loop duration = 2+3 = 5 cycles, taken 256 times = 1280 cycles, less one for the final BNE = 1279 cycles
        ; 100ms = 100,000us = 100,000 cycles at 1MHz, so 100,000 / 1279 = 78.2 outer loops required, round up to 79 for safety

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
.endif
        
        ; +(r0, w1) For each byte, check it was stored correctly, then store the inverse
        ; Page 1 of the Screen Memory
@loop5: txa                     ; Recover the test value
        cmp ScreenMem, Y        ; Read the current test location and see if it still contains the test value
        beq @ok9                ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok9:   eor #$ff                ; Invert the test value
        sta ScreenMem, Y        ; Store the inverted value

        iny                     ; Next test location
        bne @loop5              ; Loop until this page is done

        ; Page 2 of the Screen Memory
@loop6: txa                     ; Recover the test value
        cmp ScreenMem + 256, Y  ; Read the current test location and see if it still contains the test value
        beq @ok10               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok10:  eor #$ff                ; Invert the test value
        sta ScreenMem + 256, Y  ; Store the inverted value

        iny                     ; Next test location
        bne @loop6              ; Loop until this page is done

        ; Page 3 of the Screen Memory
@loop7: txa                     ; Recover the test value
        cmp ScreenMem + 512, Y  ; Read the current test location and see if it still contains the test value
        beq @ok11               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok11:  eor #$ff                ; Invert the test value
        sta ScreenMem + 512, Y  ; Store the inverted value

        iny                     ; Next test location
        bne @loop7              ; Loop until this page is done

        ; Page 4 of the Screen Memory
@loop8: txa                     ; Recover the test value
        cmp ScreenMem + 768, Y  ; Read the current test location and see if it still contains the test value
        beq @ok12               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok12:  eor #$ff                ; Invert the test value
        sta ScreenMem + 768, Y  ; Store the inverted value

        iny                     ; Next test location
        bne @loop8              ; Loop until this page is done

.if DoDelayForScreen
        ; Delay for about 100ms - bit-rot test (is this really needed for the screen RAM, which is static RAM?)
        ; This delay will destrop the contents of Y and X, but they can be restored afterwards as follows:
        ; Y is about to be changed below for the direction change, so no need to restore. X will be stored in A during the delay
        ; Inner-loop duration = 2+3 = 5 cycles, taken 256 times = 1280 cycles, less one for the final BNE = 1279 cycles
        ; 100ms = 100,000us = 100,000 cycles at 1MHz, so 100,000 / 1279 = 78.2 outer loops required, round up to 79 for safety

        txa                     ; Recover the test value (so we can put it back in X later)
        ldx #79
@delay3:
        ldy #0                  ; 2-cycles
@delay4:
        dey                     ; 2-cycles
        bne @delay4             ; 3-cycles when taken, 2-cycles when not taken
        dex                     ; 2-cycles
        bne @delay3             ; 3-cycles when taken, 2-cycles when not taken

        ; Restore X value
        tax                     ; Restore X to the last test value
.endif
        
        ; -(r1, w0, r0, w1) For each byte, check the inverted value was stored correctly, then restore the original value
        ;                   check that was stored correctly, then store the inverse
        ; Page 1 of the Screen Memory

        ldy #$ff                ; Start at the end of the page
@loop9: txa                     ; Recover the test value
        eor #$ff                ; Invert the test value
        cmp ScreenMem, Y        ; Read the current test location and see if it still contains the inverted test value
        beq @ok13               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok13:  txa                     ; Recover the test value
        sta ScreenMem, Y        ; Restore the original value
        cmp ScreenMem, Y        ; Check the original value was stored correctly
        beq @ok14               ; If it was then continue testing
        jmp ScreenError         ; If it was not then jump to the screen memory error routine

@ok14:  eor #$ff                ; Invert the test value
        sta ScreenMem, Y        ; Store the inverted value

        dey                     ; Next test location
        cpy #$ff                ; Loop until this page is done
        bne @loop9

        ; Page 2 of the Screen Memory
@loop10:txa                     ; Recover the test value
        eor #$ff                ; Invert the test value
        cmp ScreenMem + 256, Y  ; Read the current test location and see if it still contains the inverted test value
        beq @ok15               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok15:  txa                     ; Recover the test value
        sta ScreenMem + 256, Y  ; Restore the original value
        cmp ScreenMem + 256, Y  ; Check the original value was stored correctly
        beq @ok16               ; If it was then continue testing
        jmp ScreenError         ; If it was not then jump to the screen memory error routine

@ok16:  eor #$ff                ; Invert the test value
        sta ScreenMem + 256, Y  ; Store the inverted value

        dey                     ; Next test location
        cpy #$ff                ; Loop until this page is done
        bne @loop10

        ; Page 3 of the Screen Memory
@loop11:txa                     ; Recover the test value
        eor #$ff                ; Invert the test value
        cmp ScreenMem + 512, Y  ; Read the current test location and see if it still contains the inverted test value
        beq @ok17               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok17:  txa                     ; Recover the test value
        sta ScreenMem + 512, Y  ; Restore the original value
        cmp ScreenMem + 512, Y  ; Check the original value was stored correctly
        beq @ok18               ; If it was then continue testing
        jmp ScreenError         ; If it was not then jump to the screen memory error routine

@ok18:  eor #$ff                ; Invert the test value
        sta ScreenMem + 512, Y  ; Store the inverted value

        dey                     ; Next test location
        cpy #$ff                ; Loop until this page is done
        bne @loop11

        ; Page 4 of the Screen Memory
@loop12:txa                     ; Recover the test value
        eor #$ff                ; Invert the test value
        cmp ScreenMem + 768, Y  ; Read the current test location and see if it still contains the inverted test value
        beq @ok19               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok19:  txa                     ; Recover the test value
        sta ScreenMem + 768, Y  ; Restore the original value
        cmp ScreenMem + 768, Y  ; Check the original value was stored correctly
        beq @ok20               ; If it was then continue testing
        jmp ScreenError         ; If it was not then jump to the screen memory error routine

@ok20:  eor #$ff                ; Invert the test value
        sta ScreenMem + 768, Y  ; Store the inverted value

        dey                     ; Next test location
        cpy #$ff                ; Loop until this page is done
        bne @loop12


        ; -(r1, w0) For each byte, check the inverted value was stored correctly then restore the original value
        ; Page 1 of the Screen Memory
@loop13:txa                     ; Recover the test value
        eor #$ff                ; Invert the test value
        cmp ScreenMem, Y        ; Read the current test location and see if it still contains the inverted test value
        beq @ok21               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok21:  txa                     ; Recover the test value
        sta ScreenMem, Y        ; Restore the original value

        dey                     ; Next test location
        cpy #$ff                ; Loop until this page is done
        bne @loop13

        ; Page 2 of the Screen Memory
@loop14:txa                     ; Recover the test value
        eor #$ff                ; Invert the test value
        cmp ScreenMem + 256, Y  ; Read the current test location and see if it still contains the inverted test value
        beq @ok22               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok22:  txa                     ; Recover the test value
        sta ScreenMem + 256, Y  ; Restore the original value

        dey                     ; Next test location
        cpy #$ff                ; Loop until this page is done
        bne @loop14

        ; Page 3 of the Screen Memory
@loop15:txa                     ; Recover the test value
        eor #$ff                ; Invert the test value
        cmp ScreenMem + 512, Y  ; Read the current test location and see if it still contains the inverted test value
        beq @ok23               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok23:  txa                     ; Recover the test value
        sta ScreenMem + 512, Y  ; Restore the original value

        dey                     ; Next test location
        cpy #$ff                ; Loop until this page is done
        bne @loop15

        ; Page 4 of the Screen Memory
@loop16:txa                     ; Recover the test value
        eor #$ff                ; Invert the test value
        cmp ScreenMem + 768, Y  ; Read the current test location and see if it still contains the inverted test value
        beq @ok24               ; If it does then continue testing
        jmp ScreenError         ; If it does not then jump to the screen memory error routine

@ok24:  txa                     ; Recover the test value
        sta ScreenMem + 768, Y  ; Restore the original value

        dey                     ; Next test location
        cpy #$ff                ; Loop until this page is done
        bne @loop16

        ; All tests passed for this test value, move to the next test value
        tsx                     ; Recover the index of the next test value from the SP
        jmp @nextvalue          ; Use this next test value

.endif ; UseShortScreenTest


ScreenError:
        ; Clear screen
        ldx #00                 ; Working Index
        lda #32                 ; Space character to write
@loop:  sta ScreenMem, X        ; Clear page 1
        sta ScreenMem + 256, X  ; Clear page 2
        sta ScreenMem + 512, X  ; Clear page 3
        sta ScreenMem + 768, X  ; Clear page 4
        inx                     ; Next character
        bne @loop               ; Loop until pages are done

        ; Show the failed message (which may or may not be vissible)
        ldx #00                 ; Working Index
@loop2: lda ScreenRAMError, x   ; Get the character to write
        beq @Done               ; If the character is zero then we are done
        sta ScreenMem, X        ; Write the character to the screen, starting at the top left
        inx                     ; Next character
        bne @loop2              ; Loop until the message is done

        ; Try and display 3 sets of 256 characters on the screen
@Done:  ldx #00
@loop3: txa
        sta ScreenMem + 80, X
        sta ScreenMem + 400, X
        sta ScreenMem + 720, X
        inx
        bne @loop3

Halt2:  lda #$fe                ; Speaker off code
@loop0: ldy #0                  ; Outer loop counter
@loop:  ldx #0                  ; Inner loop counter
@loop2: nop                     ; 2-cycles
        nop                     ; 2-cycles
        nop                     ; 2-cycles
        inx                     ; Continue the inner loop
        bne @loop2              ; ... until the inner loop counter wraps

        tax                     ; stash the current speaker code in X
        tya                     ; Get the outer loop counter
        cmp #$d8                ; Are we nearly finished with the outer loop?
        blt @skip               ; If so, we don't sound the speaker (give a short pause each second or so)
        and #$02                ; We are sounding... but only when the outer loop count bit 1 is set (better tone)
        bne @skip               ; If not, we don't change the speaker
        txa                     ; Get the speaker code back
        eor #$20                ; Invert the speaker bit
        sta VIAPCR              ; Send to the VIA to sound the speaker
        jmp @skip2              ; Skip the txa instruction as we now have the new speaker code in A

@skip:  txa                     ; Not sounding this time - get the old speaker code back
@skip2: iny                     ; Increment the outer loop counter
        bne @loop               ; Loop until the outer loop counter wraps

        jmp @loop0              ; Loop forever - RAM is bad so we just keep sounding the speaker until we are turned off!


