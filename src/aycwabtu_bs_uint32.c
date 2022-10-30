
#include "aycwabtu_config.h"

#if PARALLEL_MODE==PARALLEL_32_INT

/* input transpose key from normal to bitslice
uint8 key[BS_BATCH_SIZE][8]  -> dvbcsa_bs_word_t block row[64] */

void aycw_key_transpose(const uint8 *key, dvbcsa_bs_word_t *row)
{
   int i, j, k;

   // copy byte 0..3 of key 0 to row 0
   // copy byte 0..3 of key 1 to row 1
   // copy byte 4..7 of key 0 to row 32
   for (k = 0; k<32; k++)
   {
      row[k     ] = BS_VAL(acyw_load_le32(&key[k*8]    ));
      row[k + 32] = BS_VAL(acyw_load_le32(&key[k*8] + 4));
   }

   for (j = 0; j < 64; j += 32)
   {
      dvbcsa_bs_word_t t, b;

      for (i = 0; i < 16; i++)
      {
         t = row[j + i];
         b = row[j + 16 + i];
         row[j + i     ] = BS_OR(BS_AND(t, BS_VAL32(0000ffff)), BS_SHL8(BS_AND(b, BS_VAL32(0000ffff)), 2));
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
         row[j + i    ] = BS_OR(BS_AND(t, BS_VAL16(00ff)), BS_SHL8(BS_AND(b, BS_VAL16(00ff)), 1));
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
         row[j + i    ] = BS_OR(BS_AND(t, BS_VAL8(0f)), BS_SHL(BS_AND(b, BS_VAL8(0f)), 4));
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
         row[j + i    ] = BS_OR(BS_AND(t, BS_VAL8(33)), BS_SHL(BS_AND(b, BS_VAL8(33)), 2));
         row[j + 2 + i] = BS_OR(BS_AND(b, BS_VAL8(cc)), BS_SHR(BS_AND(t, BS_VAL8(cc)), 2));
      }
   }

   for (j = 0; j < 64; j += 2)
   {
      dvbcsa_bs_word_t t, b;

      t = row[j];
      b = row[j + 1];
      row[j    ] = BS_OR(BS_AND(t, BS_VAL8(55)), BS_SHL(BS_AND(b, BS_VAL8(55)), 1));
      row[j + 1] = BS_OR(BS_AND(b, BS_VAL8(aa)), BS_SHR(BS_AND(t, BS_VAL8(aa)), 1));
   }
}

#ifndef USE_SLOW_BIT2BYTESLICE
void aycw_bit2byteslice(dvbcsa_bs_word_t *data, int count)
{
   int i, j, k;
   dvbcsa_bs_word_t  *p = data;

   for (k = 0; k < 8 * count; k++)
   {
      for (i = 0; i < 4; i++)
      {
         dvbcsa_bs_word_t t, b;

         t = p[i];
         b = p[4 + i];
         p[i    ] = BS_OR(BS_AND(t, BS_VAL32(0000ffff)), BS_SHL8(BS_AND(b, BS_VAL32(0000ffff)), 2));
         p[4 + i] = BS_OR(BS_AND(b, BS_VAL32(ffff0000)), BS_SHR8(BS_AND(t, BS_VAL32(ffff0000)), 2));
      }

      for (j = 0; j < 8; j += 4)
      {
         dvbcsa_bs_word_t t, b;

         for (i = 0; i < 2; i++)
         {
            t = p[j + i];
            b = p[j + 2 + i];
            p[j + i    ] = BS_OR(BS_AND(t, BS_VAL16(00ff)), BS_SHL8(BS_AND(b, BS_VAL16(00ff)), 1));
            p[j + i + 2] = BS_OR(BS_AND(b, BS_VAL16(ff00)), BS_SHR8(BS_AND(t, BS_VAL16(ff00)), 1));
         }
      }

      for (j = 0; j < 8; j += 2)
      {
         dvbcsa_bs_word_t t, b;

         t = p[j];
         b = p[j + 1];
         p[j    ] = BS_OR(BS_AND(t, BS_VAL8(0f)), BS_SHL(BS_AND(b, BS_VAL8(0f)), 4)); //(t & 0x0f0f0f0f) | ((b & 0x0f0f0f0f) << 4);
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
      p += 8;
   } /* for (k=0; k<8 ; k++) */
}
#endif

#endif   //#if PARALLEL_MODE==PARALLEL_32_INT

