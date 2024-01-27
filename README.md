# pForth for Arduino GIGA R1 WiFi and other larger-memory boards

## Description

This library and demonstration script are intended to show that a full-featured Forth language implementation can be made available on boards with 256K+ of usable RAM, with the possibility of creating custom Words and mapping them to Arduino functions.

The [pForth ("Portable Forth")](https://github.com/philburk/pforth) implementation was chosen because of its license and functionality, being written entirely in ANSI C with character I/O and memory usage being easily configurable. This library is a hard fork of version 2.0.1.

## Usage

Edit "pforth_config.h" depending on the features of your board. At present `PFORTH_USE_SDRAM` requires a Giga or Portenta H7 board (untested, feedback welcome), while `PFORTH_WEB_CONSOLE` requires the "WebSockets2_Generic" library (my [patched version](https://github.com/cpp-tutor/WebSockets2_Generic) 1.14+ for Giga). If using `PFORTH_WEB_CONSOLE` then edit file "arduino_secrets.h" to match your WiFi network.

Flashing to the board needs around 360-400K Flash Memory available. Runtime memory usage is at least the global variables (slightly above 64K) plus the two arrays in "src/pfdicdat.h", plus the values `PF_EXTRA_HEADERS` and `PF_EXTRA_CODE` in "src/pf_arduino.h". In case of failure to boot, reduce these last two and recompile the library and sketch.

Input and output are either through the serial console, or via a webpage (when `PF_WEB_CONSOLE` is set), the address of which is printed to the Serial Monitor on boot.

Pre-defined custom Words are: `MILLIS`, `MICROS`, `DELAY`, `DELAYMICROSECONDS`, `PINMODE`, `DIGITALWRITE`, `DIGITALREAD`, `ANALOGWRITERESOLUTION`, `ANALOGWRITE`, `ANALOGREADRESOLUTION`, `ANALOGREAD` and `ANALOGREFERENCE`. These have the same (integer) input and output parameter(s) as for the similarly named [Arduino functions](https://www.arduino.cc/reference/en/).

## Adding custom functions as Forth Words

It is anticipated that more functionality will be needed for advanced hardware-control projects. The steps involved in creating new built-in words are as follows:

1. Install pForth on a 32-bit machine (on Windows use Visual Studio in "Win32" configuration, on Linux use multiarch).

2. Edit the build flags to set `PF_SUPPORT_FP` and `PF_USER_CUSTOM`; the second of these disables the dummy file "pfcustom.c".

3. Add a new file "pf_arduino.c" to the project or Makefile (extending it as required):

```c
#include "pf_all.h"

/* Dummy table of null pointers - do not try to use words outside of Arduino! */
CFunc0 CustomFunctionTable[256] = { 0 };

Err CompileCustomFunctions(void)
{
    Err err;
    int i = 0;
    /* Compile Forth words that call your custom functions.
    ** Make sure order of functions matches that in sketch's CustomFunctionTable[].
    ** Parameters are: Name in UPPER CASE, Function Index, Mode, NumParams
    */
    err = CreateGlueToC("MILLIS", i++, C_RETURNS_VALUE, 0);
    if (err < 0) return err;
    err = CreateGlueToC("MICROS", i++, C_RETURNS_VALUE, 0);
    if (err < 0) return err;
    err = CreateGlueToC("DELAY", i++, C_RETURNS_VOID, 1);
    if (err < 0) return err;
    err = CreateGlueToC("DELAYMICROSECONDS", i++, C_RETURNS_VOID, 1);
    if (err < 0) return err;
    err = CreateGlueToC("PINMODE", i++, C_RETURNS_VOID, 2);
    if (err < 0) return err;
    err = CreateGlueToC("DIGITALWRITE", i++, C_RETURNS_VOID, 2);
    if (err < 0) return err;
    err = CreateGlueToC("DIGITALREAD", i++, C_RETURNS_VALUE, 1);
    if (err < 0) return err;
    err = CreateGlueToC("ANALOGWRITERESOLUTION", i++, C_RETURNS_VOID, 1);
    if (err < 0) return err;
    err = CreateGlueToC("ANALOGWRITE", i++, C_RETURNS_VOID, 2);
    if (err < 0) return err;
    err = CreateGlueToC("ANALOGREADRESOLUTION", i++, C_RETURNS_VOID, 1);
    if (err < 0) return err;
    err = CreateGlueToC("ANALOGREAD", i++, C_RETURNS_VALUE, 1);
    if (err < 0) return err;
    err = CreateGlueToC("ANALOGREFERENCE", i++, C_RETURNS_VOID, 1);
    if (err < 0) return err;

    return 0;
}
```

4. Compile "pforth.exe" (Windows) or "pforth" (Linux) and move this to directory "fth". Run (`./forth` for Linux:

```bash
pforth -i system.fth
pforth
INCLUDE savedicd.fth
SDAD
BYE
```

5. Locate the newly-created "pfdicdat.h" and move this into the "pForth/src/" directory (within your Arduino libraries directory).

6. Add your custom functions to the sketch in three ways: function declaration, function definition and entry in `CustomFunctionTable` (the order must match **exactly** that in "pf_arduino.c".

7. Compile the sketch and library.

## Future plans

* More custom words

* Explcit support for Nano33 boards

* Improve robustness of web terminal
