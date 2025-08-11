/*-*****************************************************************************
MMBasic for STM32H743 [ZI2 and VIT6] (Armmite H7)

Version.h

Copyright 2011-2023 Geoff Graham and  Peter Mather.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
  be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham and Peter Mather.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/



//#define DEBUGMODE

#define VERSION     "5.08.00b0WithCANandUSBFIX"
#define MES_SIGNON  "ARMmiteH7 MMBasic Version " VERSION
#define YEAR		"2011-2025"			    // and the year
#define YEAR2       "2016-2025"
#define COPYRIGHT  "\r\nCopyright " YEAR " Geoff Graham\r\nCopyright " YEAR2 " Peter Mather \r\n"

/* Done to test
 *   Math Window and add to manual
 *
 *   b1 Removes automatic translation of PAGE to GUI PAGE
 *      fixes GUI PAGE error as well
 *      fixes cwd$ function.
 *
 *   b2 Fixed error if ADC trigger set on channel 3  (typo in code)
 *   b3 Added cmd_sync but not tested yet !!!!!
 */
/* TODO
 * BITBANG --> DEVICE as per Picomites
 * LCD --> BITBANG LCD
 * PAGE --> GUI PAGE
 * Restore ERASE command
 *
 * C style comments as per picomite
 *
 */
// These options are compiled conditionally
    // Uncomment the following line if you want the Command History included
    #define CMDHISTORY
