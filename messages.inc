; @file messages.inc
; @brief Messages for the PET Memory Test program
; @author Matt Beard
; @date 24/09/2024

.RODATA

WelcomeMessage:
	.byt "PET RAM TEST BY MATT BEARD 0.1 4/10/2024", 0

ScreenRAMError:
        .byt "SCREEN RAM ERROR", 0

ScreenNoError:
        .byt "SCREEN RAM OK", 0

FoundMem:
        .byt "FOUND ", 0

FailedAt:
        .byt "FAILED @ ", 0

EchosZero:
        .byt " ECHOS $0000", 0

CommaMessage:
        .byt ", ", 0

MemorySizeMessage:
        .byt "DETECTED: ", 0

KMessage:
        .byt "K", 0

LoopCountMessage:
        .byt "PASS: ", 0

ErrorCountMessage:
        .byt ", TOTAL ERRORS: ", 0

TestStartMessage:
        .byt "TEST ", 0

ThisPhaseMessage:
        .byt "BITS:", 0
        
PhaseError:
        .byt "ERRORS: ", 0