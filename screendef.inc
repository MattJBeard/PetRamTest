; @file screendef.inc
; @brief Screen memory definitions, including using the bottom two lines of the screen for working memory
; @author Matt Beard
; @date 24/09/2024

; Screen memory definitions

; Reserve the bottom two lines of the screen, plus the 24 Hidden bytes for working memory 
ReserveLines = 2                ; Number of lines to reserve at the bottom of the screen
ScreenLines = 25 - 2            ; Number of lines on the usable screen

; The actual working space is 40 bytes for each reserved line, plus the 24 bytes beyond the screen
WorkingSpaceSize = (40 * ReserveLines) + 24

LoopCountBytes = 2              ; Size of loop count, 4 bcd digits
ErrorCountBytes = 5             ; Size of total error count, 10 bcd digits
PhaseErrorsBytes = 3            ; Size of phase error count, 6 bcd digits

.segment "SCREEN"
ScreenMem:
            ; Reserve 1024 bytes of screen memory, but leave reserved space bytes for working memory
            .res 1024 - WorkingSpaceSize

WorkingSpace:                   ; Working space starts here
Stack:      .res 40             ; Reserve 40 bytes for Stack
StackPtr:   .res 1              ; Reserve 1 byte for the stack pointer
NLFlag:     .res 1              ; Reserve 1 byte for the Newline flag
ByteStore:  .res 1              ; A short-term storage location for a byte value
RetAddr:    .res 2              ; Reserve 2 bytes for the return address
MemSize:    .res 1              ; Reserve 1 byte for the memory size
HexValue:   .res 1              ; Reserve 1 byte for the value to print in HexPrintSub

ldaAddr16YSub: 
            .res 1              ; Reserve 1 byte for the lda (Addr16), Y opcode
Addr16:     .res 2              ; Reserve 2 bytes for the address to read from
            .res 3              ; Reserve 3 bytes for the JMP (RetAddr) opcode

staTestAddrYSub:                ; Subroutine in screen memory to store absolute indexed by Y
            .res 1              ; Reserve 1 byte for the sta (TestAddr), Y opcode
TestAddr:   .res 2              ; Reserve 2 bytes for the address to read from
            .res 3              ; Reserve 3 bytes for the JMP (RetAddr) opcode

staCursorPosSub:
            .res 1              ; Reserve 1 byte for the sta (CursorPos) opcode
CursorPos:
            .res 2              ; Reserve 2 bytes for the cursor address
            .res 3              ; Reserve 3 bytes for the JMP (RetAddr) opcode
BCDLoopCount:                   ; Main test loop counter in bcd
            .res LoopCountBytes 
BCDErrorCount:                  ; Error count in bcd
            .res ErrorCountBytes
BCDPhaseErrors:                 ; Errors this phase in bcd
            .res PhaseErrorsBytes
PhaseBitErrors:
            .res 1              ; Bit errors this phase, each high bit is a bit in error
TestStartPage:
            .res 1              ; High byte of start address for main test
TestEndPage:
            .res 1              ; High byte of end address for main test
BCDLen:     .res 1              ; Length of the BCD value to print
TestIndex:  .res 1              ; Index of the test value in TestTable
TestValue:  .res 1              ; Value currently being tested
