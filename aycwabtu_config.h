/********************************************************************************
CSA brute force bitslice stream and block prototypes
********************************************************************************/
#ifndef aycw_H
#define aycw_H

#include <stdint.h>
typedef uint8_t      uint8;
typedef uint16_t     uint16;
typedef uint32_t     uint32;

#define PARALLEL_32_INT       1
#define PARALLEL_128_SSE2     2

#ifndef PARALLEL_MODE   // can be set outside
//#define PARALLEL_MODE      PARALLEL_32_INT
#define PARALLEL_MODE      PARALLEL_128_SSE2
#endif

//#define HAVE_X64_COMPILER

#if PARALLEL_MODE==PARALLEL_32_INT
#include "aycwabtu_bs_uint32.h"
#elif PARALLEL_MODE==PARALLEL_128_SSE2
#include "aycwabtu_bs_sse2.h"
#else
#error "unknown/undefined parallel mode"
#endif

#if !((1<<BS_BATCH_SHIFT) == BS_BATCH_SIZE) | (BS_BATCH_SIZE/8 != BS_BATCH_BYTES)
#error batch config wrong
#endif

/* enable cross checks of bitsliced and regular csa calculations.
Run test cases with self test enabled after changing calculation dependent parts */
#if _DEBUG
#define SELFTEST
#endif

#define AYCW_INLINE __inline


/* use virtual shifting for 56 block rounds? faster but needs more memory */
#define USEBLOCKVIRTUALSHIFT

/* use 16 bit block sbox lookup table. Faster but needs 64k RAM */
#define  USEWORDBLOCKSBOX

/* block key bits permutation uses direct assignments instead LUT*/
#define  USEFASTBLOCKKEYPERM

/* Use a generic (slow) version of function aycw_bit2byteslice() to check and debug 
   the faster batch sized implementation */
#define USE_SLOW_BIT2BYTESLICE 0

/****************************************** brute force helper prototypes ********************************/

AYCW_INLINE static uint32_t
acyw_load_le32(const uint8_t *p)
{
#if defined(__i386__) || defined(__x86_64__)
   /* target support non aligned le memory access */
   return *(uint32_t*)p;
#else
   return ((uint32_t)p[3] << 24) |
      ((uint32_t)p[2] << 16) |
      ((uint32_t)p[1] << 8) |
      (uint32_t)p[0];
#endif
}


/********************* console exit codes ********************************/
typedef enum {
   WORKPACKAGEFINISHED = 0,

   KEYFOUND = 1,

   ERR_FATAL = 16,
   ERR_USAGE,
   ERR_STARTKEY,
   ERR_STOPKEY,

   ERR_TS_OPEN = 32,
   ERR_TS_CORRUPT,
   ERR_TS_UNENCRYPTED,
   ERR_TS_UNUSABLE,
} exitcodes;

#endif      // #ifndef aycw_H
