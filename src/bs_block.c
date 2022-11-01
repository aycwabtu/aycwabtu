/*
   aycwabtu_bs_block.c

   this file contains the block implementation in byte sliced form where
   the sbox is a look up table. See #define  USEALLBITSLICE

*/


#include <string.h>

#include "config.h"
#include "bs_block.h"

#ifndef USEALLBITSLICE

void AYCW_INLINE aycw_block_sbox(dvbcsa_bs_word_t *);

#ifdef USEWORDBLOCKSBOX
static uint16   aycw_block_sbox16[0x10000];
#endif


const uint8		cu8_aycw_block_sbox[256] =
{
   0x3a, 0xea, 0x68, 0xfe, 0x33, 0xe9, 0x88, 0x1a, 0x83, 0xcf, 0xe1, 0x7f, 0xba, 0xe2, 0x38, 0x12,
   0xe8, 0x27, 0x61, 0x95, 0x0c, 0x36, 0xe5, 0x70, 0xa2, 0x06, 0x82, 0x7c, 0x17, 0xa3, 0x26, 0x49,
   0xbe, 0x7a, 0x6d, 0x47, 0xc1, 0x51, 0x8f, 0xf3, 0xcc, 0x5b, 0x67, 0xbd, 0xcd, 0x18, 0x08, 0xc9,
   0xff, 0x69, 0xef, 0x03, 0x4e, 0x48, 0x4a, 0x84, 0x3f, 0xb4, 0x10, 0x04, 0xdc, 0xf5, 0x5c, 0xc6,
   0x16, 0xab, 0xac, 0x4c, 0xf1, 0x6a, 0x2f, 0x3c, 0x3b, 0xd4, 0xd5, 0x94, 0xd0, 0xc4, 0x63, 0x62,
   0x71, 0xa1, 0xf9, 0x4f, 0x2e, 0xaa, 0xc5, 0x56, 0xe3, 0x39, 0x93, 0xce, 0x65, 0x64, 0xe4, 0x58,
   0x6c, 0x19, 0x42, 0x79, 0xdd, 0xee, 0x96, 0xf6, 0x8a, 0xec, 0x1e, 0x85, 0x53, 0x45, 0xde, 0xbb,
   0x7e, 0x0a, 0x9a, 0x13, 0x2a, 0x9d, 0xc2, 0x5e, 0x5a, 0x1f, 0x32, 0x35, 0x9c, 0xa8, 0x73, 0x30,
   0x29, 0x3d, 0xe7, 0x92, 0x87, 0x1b, 0x2b, 0x4b, 0xa5, 0x57, 0x97, 0x40, 0x15, 0xe6, 0xbc, 0x0e,
   0xeb, 0xc3, 0x34, 0x2d, 0xb8, 0x44, 0x25, 0xa4, 0x1c, 0xc7, 0x23, 0xed, 0x90, 0x6e, 0x50, 0x00,
   0x99, 0x9e, 0x4d, 0xd9, 0xda, 0x8d, 0x6f, 0x5f, 0x3e, 0xd7, 0x21, 0x74, 0x86, 0xdf, 0x6b, 0x05,
   0x8e, 0x5d, 0x37, 0x11, 0xd2, 0x28, 0x75, 0xd6, 0xa7, 0x77, 0x24, 0xbf, 0xf0, 0xb0, 0x02, 0xb7,
   0xf8, 0xfc, 0x81, 0x09, 0xb1, 0x01, 0x76, 0x91, 0x7d, 0x0f, 0xc8, 0xa0, 0xf2, 0xcb, 0x78, 0x60,
   0xd1, 0xf7, 0xe0, 0xb5, 0x98, 0x22, 0xb3, 0x20, 0x1d, 0xa6, 0xdb, 0x7b, 0x59, 0x9f, 0xae, 0x31,
   0xfb, 0xd3, 0xb6, 0xca, 0x43, 0x72, 0x07, 0xf4, 0xd8, 0x41, 0x14, 0x55, 0x0d, 0x54, 0x8b, 0xb9,
   0xad, 0x46, 0x0b, 0xaf, 0x80, 0x52, 0x2c, 0xfa, 0x8c, 0x89, 0x66, 0xfd, 0xb2, 0xa9, 0x9b, 0xc0,
};

/* table 19,27,55 is achieved from table 17, 35, 8,... by applying 7-x to lower 3 bits */
static const uint8 bf_key_perm[64] = {19, 27, 55, 46,  1, 15, 36, 22, 56, 61, 39, 21, 54, 58, 50, 28, 7, 29, 51,  6, 33, 35, 20, 16, 47, 30, 32, 63, 10, 11,  4, 38, 62, 26, 40, 18, 12, 52, 37, 53, 23, 59, 41, 17, 31,  0, 25, 43, 44, 14,  2, 13, 45, 48,  3, 60, 49,  8, 34,  5,  9, 42, 57, 24,};

/*static const uint8 bf_key_perm[64] = {17, 35, 8, 6, 41, 48, 28, 20, 27, 53, 61, 49, 18, 32, 58, 63, 23, 19, 36, 38, 1, 52, 26, 0, 33, 3, 12, 13, 56, 39, 25, 40, 50, 34, 51, 11, 21, 47, 29, 57, 44, 30, 7, 24, 22, 46, 60, 16, 59, 4, 55, 42, 10, 5, 9, 43, 31, 62, 45, 14, 2, 37, 15, 54}; */

void AYCW_INLINE aycw_block_key_perm(dvbcsa_bs_word_t* in, dvbcsa_bs_word_t* out)
{
#ifndef USEFASTBLOCKKEYPERM
   int i;

   for(i=0; i<64; i++)
      out[bf_key_perm[i]] = in[i];

#else
/* csa block key schedule bit permutation */
#define CPY(a,b) (out)[(b)] = (in)[(a)]
CPY(0	,19 );
CPY(1	,27 );
CPY(2	,55 );
CPY(3	,46 );
CPY(4	,1  );
CPY(5	,15 );
CPY(6	,36 );
CPY(7	,22 );
CPY(8	,56 );
CPY(9	,61 );
CPY(10,39 );
CPY(11,21 );
CPY(12,54 );
CPY(13,58 );
CPY(14,50 );
CPY(15,28 );
CPY(16,7  );
CPY(17,29 );
CPY(18,51 );
CPY(19,6  );
CPY(20,33 );
CPY(21,35 );
CPY(22,20 );
CPY(23,16 );
CPY(24,47 );
CPY(25,30 );
CPY(26,32 );
CPY(27,63 );
CPY(28,10 );
CPY(29,11 );
CPY(30,4  );
CPY(31,38 );
CPY(32,62 );
CPY(33,26 );
CPY(34,40 );
CPY(35,18 );
CPY(36,12 );
CPY(37,52 );
CPY(38,37 );
CPY(39,53 );
CPY(40,23 );
CPY(41,59 );
CPY(42,41 );
CPY(43,17 );
CPY(44,31 );
CPY(45,0  );
CPY(46,25 );
CPY(47,43 );
CPY(48,44 );
CPY(49,14 );
CPY(50,2  );
CPY(51,13 );
CPY(52,45 );
CPY(53,48 );
CPY(54,3  );
CPY(55,60 );
CPY(56,49 );
CPY(57,8  );
CPY(58,34 );
CPY(59,5  );
CPY(60,9  );
CPY(61,42 );
CPY(62,57 );
CPY(63,24 );

#endif
}

/**
 Calculates expanded key 
 @parameter keys[in]    input bitsliced key array BS_BATCH_SIZE x 64
            kk[out]     output bytesliced expanded key array BS_BATCH_SIZE x 448
*/
void aycw_block_key_schedule(const dvbcsa_bs_word_t* keys, dvbcsa_bs_word_t* kk)
{
   int i,j;

   for (i = 0; i < 64; i++)  
      kk[6 * 64 + i] = keys[i];
   aycw_block_key_perm(&kk[6*64], &kk[5*64]);
   aycw_block_key_perm(&kk[5*64], &kk[4*64]);
   aycw_block_key_perm(&kk[4*64], &kk[3*64]);
   aycw_block_key_perm(&kk[3*64], &kk[2*64]);
   aycw_block_key_perm(&kk[2*64], &kk[1*64]);
   aycw_block_key_perm(&kk[1*64], &kk[0*64]);

   for (i = 6; i>0; i--)      /* i = 6...1  xor 0 skipped */
   {
      for (j = 7; j>=0; j--)
      {
         switch(i)
         {
         case 1:
            kk[1*64 + j*8 + 0] 
				= 
				BS_NOT(
				kk[1*64 + j*8 + 0]
				);
            break;
         case 2:
            kk[2*64 + j*8 + 1] = BS_NOT(kk[2*64 + j*8 + 1]);
            break;
         case 3:
            kk[3*64 + j*8 + 0] = BS_NOT(kk[3*64 + j*8 + 0]);
            kk[3*64 + j*8 + 1] = BS_NOT(kk[3*64 + j*8 + 1]);
            break;
         case 4:
            kk[4*64 + j*8 + 2] = BS_NOT(kk[4*64 + j*8 + 2]);
            break;
         case 5:
            kk[5*64 + j*8 + 0] = BS_NOT(kk[5*64 + j*8 + 0]);
            kk[5*64 + j*8 + 2] = BS_NOT(kk[5*64 + j*8 + 2]);
            break;
         case 6:
            kk[6*64 + j*8 + 1] = BS_NOT(kk[6*64 + j*8 + 1]);
            kk[6*64 + j*8 + 2] = BS_NOT(kk[6*64 + j*8 + 2]);
            break;
         }
      }
   }
}

AYCW_INLINE void aycw_block_sbox(dvbcsa_bs_word_t *w)
{
#ifndef USEWORDBLOCKSBOX
   // table lookup, works one byte at a time
   uint8 *si = (uint8 *)w;
   int i;

   for (i = 0; i < BS_BATCH_BYTES; i++)
      si[i] = cu8_aycw_block_sbox[si[i]];
#else
#  if BS_BATCH_BYTES < 2
#  error BS_BATCH_BYTES < 2
#  endif
   uint16 *si = (uint16 *)w;
   int i;

   for (i = 0; i < BS_BATCH_BYTES/2; i++)
      si[i] = aycw_block_sbox16[si[i]];
#endif
}

#define SBOXSSE2(in, ret, shift) {                          \
   uint16      u16tmp;                                      \
   u16tmp = (uint16)BS_EXTLS32(BS_SHR8(in, shift));         \
   u16tmp = aycw_block_sbox16[u16tmp];                      \
   ret = BS_OR(ret, BS_SHL8(BS_VAL_LSDW(u16tmp), shift));   }


AYCW_INLINE dvbcsa_bs_word_t aycw_block_sbox_by_value(dvbcsa_bs_word_t in)
{
/* the 8 bit lookup part (!USEWORDBLOCKSBOX) could be implemented here too... */
   dvbcsa_bs_word_t  ret = BS_VAL8(00);

#if PARALLEL_MODE==PARALLEL_128_SSE2
   /* BS_SHR8 does not take an immediate shift count in sse2 */
   SBOXSSE2(in, ret, 0);
   SBOXSSE2(in, ret, 2);
   SBOXSSE2(in, ret, 4);
   SBOXSSE2(in, ret, 6);
   SBOXSSE2(in, ret, 8);
   SBOXSSE2(in, ret, 10);
   SBOXSSE2(in, ret, 12);
   SBOXSSE2(in, ret, 14);
   SBOXSSE2(in, ret, 16);
#else
   int i;

for (i = 0; i < BS_BATCH_BYTES / 2; i++)
   {
      uint16      u16tmp;

      u16tmp = (uint16)BS_SHR8(in, 2 * i);;
      u16tmp = aycw_block_sbox16[u16tmp];
      ret = BS_OR(ret, BS_SHL8((dvbcsa_bs_word_t)u16tmp, 2 * i));
   }
#endif
return ret;
}


/**
   initialize block sbox table in RAM
*/
void aycw_init_block(void)
{
#ifdef USEWORDBLOCKSBOX
   int i;
   for (i=0; i<0x10000; i++)
      aycw_block_sbox16[i] = (uint16)cu8_aycw_block_sbox[i/256]<<8 | cu8_aycw_block_sbox[i%256];
#endif
}


/**
   Nearly the same as dvbcsa_bs_block_decrypt_register. kkmulti is now 8 times in size.
   @parameter keys[in]  the keys array contains the bytesliced array [56][8][BS_BATCH_BYTES] bytes but is 
                        now treated as 56 x 8 x dvbcsa_bs_word_t.
   @parameter r         ts input data ib0 + some free space left in front for virtual shift
*/
void aycw_block_decrypt(const dvbcsa_bs_word_t* keys, dvbcsa_bs_word_t* r)
{
   int  i;                      // 56 rounds
   int  j;                      // 8 batches

#ifdef USEBLOCKVIRTUALSHIFT
   r += 8 * 56;
#endif

   // loop over kk[55]..kk[0]
   for (i = 55; i >= 0; i--)
   {
      dvbcsa_bs_word_t *r6xK;         // help pointer: r6 (before shift) after xoring with keys
      dvbcsa_bs_word_t r7xS;          // sbox output xor r7 (before shift), aka 'L'
      dvbcsa_bs_word_t perm;          // sbox out + permutation

      r6xK = &r[8 * 6];

#ifdef USEBLOCKVIRTUALSHIFT
      r -= 8;	/* virtual shift of registers */
#endif

      for (j = 0; j < 8; j++)
      {

#if 0
         /* calling per reference is 19% faster with MS CC but gcc decided 
            to optimize the whole funtion call away for some reason ?!? :-( */
         dvbcsa_bs_word_t sbox_out = BS_XOR(keys[i * 8 + j], r6xK[j]);
         aycw_block_sbox(&sbox_out);
#else
         dvbcsa_bs_word_t sbox_out = aycw_block_sbox_by_value(BS_XOR(keys[i * 8 + j], r6xK[j]));
#endif
#ifdef BLOCKDEBUG
         if (j==0) {
            // debug dump slice 0 regs 'before' 1st shift
            uint8 sn, wn;
            dvbcsa_bs_word_t tmp1 = BS_XOR(keys[i * 8 + j], r6xK[j]);
            printf("%d: %02x %02x %02x %02x  %02x %02x %02x %02x  r6xK %02x sout %02x\n", i,
               (uint8)BS_EXTLS32(r[8 * 1]),
               (uint8)BS_EXTLS32(r[8 * 2]),
               (uint8)BS_EXTLS32(r[8 * 3]),
               (uint8)BS_EXTLS32(r[8 * 4]),
               (uint8)BS_EXTLS32(r[8 * 5]),
               (uint8)BS_EXTLS32(r[8 * 6]),
               (uint8)BS_EXTLS32(r[8 * 7]),
               (uint8)BS_EXTLS32(r[8 * 8]),
               (uint8)BS_EXTLS32(tmp1),
               (uint8)BS_EXTLS32(sbox_out)
            );
         }
#endif

         // bit permutation

         perm = 
            BS_OR(
               BS_OR(
                  BS_OR (
                     BS_SHL (BS_AND (sbox_out, BS_VAL8(29)), 1),
                     BS_SHL (BS_AND (sbox_out, BS_VAL8(02)), 6)
                  ),
                  BS_OR (
                     BS_SHL (BS_AND (sbox_out, BS_VAL8(04)), 3),
                     BS_SHR (BS_AND (sbox_out, BS_VAL8(10)), 2)
                  )
               ),
               BS_OR(       
                  BS_SHR (BS_AND (sbox_out, BS_VAL8(40)), 6),
				      BS_SHR(BS_AND(sbox_out, BS_VAL8(80)), 4)
				  )
            );

#ifdef USEBLOCKVIRTUALSHIFT
         r7xS = BS_XOR(r[8 * 8 + j], sbox_out);

         r[8 * 0 + j] = r7xS;
         BS_XOREQ(r[8 * 2 + j], r7xS);
         BS_XOREQ(r[8 * 3 + j], r7xS);
         BS_XOREQ(r[8 * 4 + j], r7xS);
         BS_XOREQ(r[8 * 6 + j], perm);
#else
         r7xS = BS_XOR(r[8 * 7 + j], sbox_out);

         r[8 * 7 + j] = r[8 * 6 + j];
         r[8 * 6 + j] = BS_XOR(r[8 * 5 + j], perm);
         r[8 * 5 + j] = r[8 * 4 + j];
         r[8 * 4 + j] = BS_XOR(r[8 * 3 + j], r7xS);
         r[8 * 3 + j] = BS_XOR(r[8 * 2 + j], r7xS);
         r[8 * 2 + j] = BS_XOR(r[8 * 1 + j], r7xS);
         r[8 * 1 + j] = r[8 * 0 + j];
         r[8 * 0 + j] = r7xS;
#endif
      }
   }
}

int aycw_checkPESheader(dvbcsa_bs_word_t *data, dvbcsa_bs_word_t *candidates)
{
   dvbcsa_bs_word_t tmp, c;
   int i, j, ret;
   uint8 a;

   c = BS_VAL8(00);
   ret = 0;

   // every 8 elements in bytesliced data array are belong to the same data block
   for (i = 0; i < 8; i++)
   {
      // check also for 4th byte Audio streams (0xC0-0xDF), Video streams (0xE0-0xEF) ?
      tmp = BS_OR(BS_OR(data[i], data[i + 8]), BS_XOR(data[i + 16], BS_VAL8(01)));    // 0x00 | 0x00 | 0x01^0x01 == 0x00
      // OPTIMIZEME: check whole batch for zero bits at once?
      for (j = 0; j < BS_BATCH_BYTES; j++)
      {
         a = BS_EXTRACT8(tmp, j);
         if (a == 0)
         {
            // key candidate found in bytesliced data array at i, j
            c = BS_OR(c, BS_SHL(BS_VAL_LSDW(1), i*BS_BATCH_BYTES + j));
            ret++;
         }
      }
   }
   *candidates = c;
   return ret;
}

#endif //#ifndef USEALLBITSLICE
