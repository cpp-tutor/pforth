// These defines should not need changing
#define PF_NO_INIT          1
#define PF_NO_STDIO         1
#define PF_NO_CLIB          1
#define PF_NO_FILEIO        1

// Only used when PFORTH_USE_SDRAM is set in pforth_config.h
#if defined(ARDUINO_GIGA) || defined(ARDUINO_PORTENTA_H7_M7)
#define PF_USER_MALLOC      "pf_giga_r1.h"
#endif

// Make sure that there is enough free memory for these two
#define PF_EXTRA_HEADERS    0x10000
#define PF_EXTRA_CODE       0x20000

// Single precision floating-point enabled
#define PF_SUPPORT_FP       1

// CustomFunctionTable array and dummy CompileCustomFunctions
// function defined in sketch, need to recreate pfdicdat.h
// on 32-bit little-endian machine if changed
#define PF_USER_CUSTOM      1
