# PetRamTest

 (c) Matt Beard <matt@beard.tv> 2024 

> [!IMPORTANT]
> This is a pre-release version of the software, and should be treated as Alpha - please read the Restrictions section!

## ABOUT
This program is designed to test the memory of a 40-column Commodore PET.
The algorithm used is March-UD, and the implimentaion is partly inspired by the Apple II Dead Test,
written by David Giller, which can be found at https://github.com/misterblack1/appleII_deadtest

## Outline of the Code:
* Step one - Play a boot chime to show that the CPU and VIA are working
* Step two - Test the screen memory. This is a challenge as we cannot use zero page or the stack at this point.
* Step three - Test the memory at $0000, $2000 and $4000 to determine how much memory we have. 8k, 16k and 32k are valid values.
* Step four - Test 4k, 8k or 16k sections (depending on the memory size detected in step two).
* Repeat step four ad infinitum.

## Special Implementation Notes:
* The screen memory test is done without using the stack or zero page, so is quite long-winded.
* If the screen memory test fails, a fail message will be displayed, along with three full character sets.
  This is intended to help diagnose the fault as well as flag the issue. Also, the CB2 speaker will beep if there is one.
* If the screen memory test passes, the screen will be cleared and a welcome message displayed, after which we set up
  a small stack area and some working memory in the bottom two lines of the screen memory.
* From this point onwards we can call subroutines, using the working space in the screen memory. There are two different
  methods for subrouitine calls, both of which store a return address in byte pair RetAddr in the screen memory, then jump
  to the subroutine. The simplest case is that this subroutine does something, then ends with jmp (RetAddr) to return.
  This cannot allow any further calls inside the subroutine, as this would destroy the return address. The other method is
  to make the call in the same way, but at the start of the subroutine the macro pushret is used to transfer the return
  address to a stack area in the screen memory, then the subroutine can call other subroutines, which can in turn call
  other subroutines, etc. The return from such a subroutine is done with the doreturn macro, which pops the return address 
  from the stack and jumps to it using jmp (RetAddr). Complex subroutines can call other comlex or simple subroutines, 
  but the simple subroutines cannot call other subroutines of any kind.
* The memory size detection is done by writing to $0000, $2000 and $4000, then reading them back. If the values are correct
  then we assume that we have 8k, 16k or 32k of memory. If the first test fails then we display a message and halt.

 # RESTRICTIONS
 Please feel free to download and try out this code, however:

 1. It is expected at this stage that anyone trying out the code report their experience using it
 1. Feel free to download the source code, and/or any binary files, and to modify, build and use this code
 1. DO NOT: Destribute the pre-release code in any form, source or binary, modified or unmodified
 1. DO: Destribute links to this GitHub project so that anyone interested gets updated versions

> [!NOTE]
>  The project will be released with a full Open Source license once stable
