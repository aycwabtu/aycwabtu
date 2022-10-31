
#include "aycwabtu_config.h"

#if PARALLEL_MODE==PARALLEL_128_SSE2

#ifdef BS_SSE_X64
#error this file is implemented to use 32 integers to load mmx regs
#endif

/* input transpose key from normal to bitslice
uint8 key[BS_BATCH_SIZE][8]  -> dvbcsa_bs_word_t block row[64] */

void aycw_key_transpose(const uint8 key[BS_BATCH_SIZE][8], dvbcsa_bs_word_t *row)
{
   unsigned int i, j, k;

   // copy key 0 and key 1 to to row 0
   // copy key 2 and key 3 to to row 1
   // copy key 126 and key 127 to row 63
   for (k = 0; k < 64; k++)
   {
      row[k] = BS_VAL(
         acyw_load_le32(&key[(k + 64)][4]),
         acyw_load_le32(&key[(k + 64)][0]),
         acyw_load_le32(&key[(k + 0)][4]),
         acyw_load_le32(&key[(k + 0)][0]));
   }

   for (i = 0; i < 32; i++)
   {
      dvbcsa_bs_word_t t, b;

      t = row[i];
      b = row[32 + i];
#ifdef HAVE_X64_COMPILER
      row[i] =      BS_OR(BS_AND(t, BS_VAL64(00000000ffffffff)), BS_SHL8(BS_AND(b, BS_VAL64(00000000ffffffff)), 4));
      row[32 + i] = BS_OR(BS_AND(b, BS_VAL64(ffffffff00000000)), BS_SHR8(BS_AND(t, BS_VAL64(ffffffff00000000)), 4));
#else
      row[i] = BS_OR(BS_AND(t, BS_VAL(0x00000000, 0xffffffff, 0x00000000, 0xffffffff)), BS_SHL8(BS_AND(b, BS_VAL(0x00000000, 0xffffffff, 0x00000000, 0xffffffff)), 4));
      row[32 + i] = BS_OR(BS_AND(b, BS_VAL(0xffffffff, 0x00000000, 0xffffffff, 0x00000000)), BS_SHR8(BS_AND(t, BS_VAL(0xffffffff, 0x00000000, 0xffffffff, 0x00000000)), 4));
#endif
   }

   for (j = 0; j < 64; j += 32)
   {
      dvbcsa_bs_word_t t, b;

      for (i = 0; i < 16; i++)
      {
         t = row[j + i];
         b = row[j + 16 + i];
         row[j + i] = BS_OR(BS_AND(t, BS_VAL32(0000ffff)), BS_SHL8(BS_AND(b, BS_VAL32(0000ffff)), 2));
         row[j + 16 + i] = BS_OR(BS_AND(b, BS_VAL32(ffff0000)), BS_SHR8(BS_AND(t, BS_VAL32(ffff0000)), 2));
      }
   }

   for (j = 0; j < 64; j += 16)
   {
      dvbcsa_bs_word_t t, b;

      for (i = 0; i < 8; i++)
      {
         t = row[j + i];
         b = row[j + 8 + i];
         row[j + i] = BS_OR(BS_AND(t, BS_VAL16(00ff)), BS_SHL8(BS_AND(b, BS_VAL16(00ff)), 1));
         row[j + 8 + i] = BS_OR(BS_AND(b, BS_VAL16(ff00)), BS_SHR8(BS_AND(t, BS_VAL16(ff00)), 1));
      }
   }

   for (j = 0; j < 64; j += 8)
   {
      dvbcsa_bs_word_t t, b;

      for (i = 0; i < 4; i++)
      {
         t = row[j + i];
         b = row[j + 4 + i];
         row[j + i] = BS_OR(BS_AND(t, BS_VAL8(0f)), BS_SHL(BS_AND(b, BS_VAL8(0f)), 4));
         row[j + 4 + i] = BS_OR(BS_AND(b, BS_VAL8(f0)), BS_SHR(BS_AND(t, BS_VAL8(f0)), 4));
      }
   }

   for (j = 0; j < 64; j += 4)
   {
      dvbcsa_bs_word_t t, b;

      for (i = 0; i < 2; i++)
      {
         t = row[j + i];
         b = row[j + 2 + i];
         row[j + i] = BS_OR(BS_AND(t, BS_VAL8(33)), BS_SHL(BS_AND(b, BS_VAL8(33)), 2));
         row[j + 2 + i] = BS_OR(BS_AND(b, BS_VAL8(cc)), BS_SHR(BS_AND(t, BS_VAL8(cc)), 2));
      }
   }

   for (j = 0; j < 64; j += 2)
   {
      dvbcsa_bs_word_t t, b;

      t = row[j];
      b = row[j + 1];
      row[j] = BS_OR(BS_AND(t, BS_VAL8(55)), BS_SHL(BS_AND(b, BS_VAL8(55)), 1));
      row[j + 1] = BS_OR(BS_AND(b, BS_VAL8(aa)), BS_SHR(BS_AND(t, BS_VAL8(aa)), 1));
   }
}

#ifdef HAVE_X64_COMPILER
#error aycw_bit2byteslice() for 128 batch not available yet
#endif

#ifndef USE_SLOW_BIT2BYTESLICE
void aycw_bit2byteslice(dvbcsa_bs_word_t *data, int count)
{
   int i, j, k;
   dvbcsa_bs_word_t  *p = data;
   dvbcsa_bs_word_t  *row = data;

   for (k = 0; k < 8 * count; k++)
   {
      for (i = 0; i < 4; i++)
      {
         dvbcsa_bs_word_t t, b;

         t = p[i];
         b = p[4 + i];
         p[i + 0] = BS_OR(BS_AND(t, BS_VAL32(0000ffff)), BS_SHL8(BS_AND(b, BS_VAL32(0000ffff)), 2));
         p[i + 4] = BS_OR(BS_AND(b, BS_VAL32(ffff0000)), BS_SHR8(BS_AND(t, BS_VAL32(ffff0000)), 2));
      }

      for (j = 0; j < 8; j += 4)
      {
         dvbcsa_bs_word_t t, b;

         for (i = 0; i < 2; i++)
         {
            t = p[j + i];
            b = p[j + 2 + i];
            p[j + i + 0] = BS_OR(BS_AND(t, BS_VAL16(00ff)), BS_SHL8(BS_AND(b, BS_VAL16(00ff)), 1));
            p[j + i + 2] = BS_OR(BS_AND(b, BS_VAL16(ff00)), BS_SHR8(BS_AND(t, BS_VAL16(ff00)), 1));
         }
      }

      for (j = 0; j < 8; j += 2)
      {
         dvbcsa_bs_word_t t, b;

         t = p[j];
         b = p[j + 1];
         p[j + 0] = BS_OR(BS_AND(t, BS_VAL8(0f)), BS_SHL(BS_AND(b, BS_VAL8(0f)), 4)); //(t & 0x0f0f0f0f) | ((b & 0x0f0f0f0f) << 4);
         p[j + 1] = BS_OR(BS_AND(b, BS_VAL8(f0)), BS_SHR(BS_AND(t, BS_VAL8(f0)), 4));//((t & 0xf0f0f0f0) >> 4) | (b & 0xf0f0f0f0);
      }

      for (j = 0; j < 8; j++)
      {
         dvbcsa_bs_word_t t;

         t = p[j];

         t = BS_OR(
            BS_AND(t, BS_VAL32(cccc3333)),
            BS_OR(
            BS_SHR(BS_AND(t, BS_VAL32(33330000)), 14),
            BS_SHL(BS_AND(t, BS_VAL32(0000cccc)), 14)
            )
            );

         t = BS_OR(
            BS_AND(t, BS_VAL16(aa55)),
            BS_OR(
            BS_SHR(BS_AND(t, BS_VAL16(5500)), 7),
            BS_SHL(BS_AND(t, BS_VAL16(00aa)), 7)
            )
            );

         t = BS_OR(BS_AND(t, BS_VAL8(81)),

            BS_OR(BS_SHR(BS_AND(t, BS_VAL8(10)), 3),
            BS_OR(BS_SHR(BS_AND(t, BS_VAL8(20)), 2),
            BS_OR(BS_SHR(BS_AND(t, BS_VAL8(40)), 1),

            BS_OR(BS_SHL(BS_AND(t, BS_VAL8(02)), 1),
            BS_OR(BS_SHL(BS_AND(t, BS_VAL8(04)), 2),
            BS_SHL(BS_AND(t, BS_VAL8(08)), 3)))))));

         p[j] = t;
      }  /* for (j = 0; j < 8; j++) */

      /* OPTIMIZEME: following loops are only for re ordering bytes. Can be included in expressions above? */
      for (i = 0; i < 4; i++)
      {
         dvbcsa_bs_word_t t, b;

         t = p[i];
         b = p[4 + i];
         p[i + 0] = BS_OR(BS_AND(t, BS_VAL(0x00000000, 0x00000000, 0xffffffff, 0xffffffff)), BS_SHL8(BS_AND(b, BS_VAL(0x00000000, 0x00000000, 0xffffffff, 0xffffffff)), 8));
         p[i + 4] = BS_OR(BS_AND(b, BS_VAL(0xffffffff, 0xffffffff, 0x00000000, 0x00000000)), BS_SHR8(BS_AND(t, BS_VAL(0xffffffff, 0xffffffff, 0x00000000, 0x00000000)), 8));
      }
      for (j = 0; j < 8; j += 4)
      {
         for (i = 0; i < 2; i++)
         {
            dvbcsa_bs_word_t t, b;

            t = p[j + i];
            b = p[j + 2 + i];
            p[j + i + 0] = BS_OR(BS_AND(t, BS_VAL(0x00000000, 0x00000000, 0xffffffff, 0xffffffff)), BS_SHL8(BS_AND(b, BS_VAL(0x00000000, 0x00000000, 0xffffffff, 0xffffffff)), 8));
            p[j + i + 2] = BS_OR(BS_AND(b, BS_VAL(0xffffffff, 0xffffffff, 0x00000000, 0x00000000)), BS_SHR8(BS_AND(t, BS_VAL(0xffffffff, 0xffffffff, 0x00000000, 0x00000000)), 8));
            t = p[j + i];
            b = p[j + 2 + i];
            p[j + i + 0] = BS_OR(BS_AND(t, BS_VAL(0x00000000, 0xffffffff, 0x00000000, 0xffffffff)), BS_SHL8(BS_AND(b, BS_VAL(0x00000000, 0xffffffff, 0x00000000, 0xffffffff)), 4));
            p[j + i + 2] = BS_OR(BS_AND(b, BS_VAL(0xffffffff, 0x00000000, 0xffffffff, 0x00000000)), BS_SHR8(BS_AND(t, BS_VAL(0xffffffff, 0x00000000, 0xffffffff, 0x00000000)), 4));
         }
      }
      for (j = 0; j < 8; j += 2)
      {
         dvbcsa_bs_word_t t, b;

         t = p[j];
         b = p[j + 1];
         p[j + 0] = BS_OR(BS_AND(t, BS_VAL(0x00000000, 0xffffffff, 0x00000000, 0xffffffff)), BS_SHL8(BS_AND(b, BS_VAL(0x00000000, 0xffffffff, 0x00000000, 0xffffffff)), 4));
         p[j + 1] = BS_OR(BS_AND(b, BS_VAL(0xffffffff, 0x00000000, 0xffffffff, 0x00000000)), BS_SHR8(BS_AND(t, BS_VAL(0xffffffff, 0x00000000, 0xffffffff, 0x00000000)), 4));
      }

      p += 8;
	  row=p;
   } /* for (k=0; k<8 ; k++) */
}
#endif


#endif

