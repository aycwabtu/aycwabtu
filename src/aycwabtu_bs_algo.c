/* general functions for brute force attack */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aycwabtu_config.h"
#include "aycwabtu_bs_algo.h"

#ifdef SELFTEST
#include "dvbcsa.h"
#endif

void aycw_fatal_error(unsigned char* message)
{
   printf("%s", message);
   exit(ERR_FATAL);      /* breakpoint here */
}

/* add value and carry(in), return sum in val and carry(out) */
AYCW_INLINE void aycw_bs_halfadd(dvbcsa_bs_word_t *val, dvbcsa_bs_word_t *carry)
{
   /* a  = BS_VAL8(aa);    // 10101010
      b  = BS_VAL8(cc);    // 11001100
      ci = BS_VAL8(f0);    // 11110000

                           // 32212110
      s  = BS_VAL8(96);    // 10010110
      co = BS_VAL8(e8);    // 11101000 */

   dvbcsa_bs_word_t sum;
   
   sum    = BS_XOR(*val, *carry);
   *carry = BS_AND(*val, *carry);
   *val = sum;
}

AYCW_INLINE void aycw_bs_fulladd(dvbcsa_bs_word_t *a, dvbcsa_bs_word_t *b, dvbcsa_bs_word_t *c, dvbcsa_bs_word_t *sum, dvbcsa_bs_word_t *co)
{
   dvbcsa_bs_word_t tmp;

   tmp = BS_XOR(*a, *b);
   *sum = BS_XOR(tmp, *c);
   *co = BS_XOR(
      BS_AND(*a, *b),
      BS_AND(tmp, *c)
      );
}


void aycw_bs_increment_keys_inner(dvbcsa_bs_word_t *keys_bs)
{
   /*                   before         after
      keys_bs[40]      10101010  aa   01010101 55
      keys_bs[41]      11001100  cc   01100110 66
      keys_bs[42]      00000000  00   10001000 88
      keys_bs[43]      00000000  00   00000000 00 */

   dvbcsa_bs_word_t carry, carry1, sum456, co456;

   int i;

   // increment byte 5 + 6 (except upper bits)
   carry = BS_VAL16(ffff);
   for (i = 40; i < 56 - BS_BATCH_SHIFT; i++)  
   {
      aycw_bs_halfadd(&keys_bs[i], &carry);
   }


   // re calc byte 7 checksum
   carry = carry1 = BS_VAL16(0000);
   /* expected:
      keys_bs[32] = BS_VAL32(aaaaaaaa);  //  10101010 10101010 10101010 10101010 
      keys_bs[40] = BS_VAL32(cccccccc);  //  11001100 11001100 11001100 11001100 
      keys_bs[48] = BS_VAL32(f0f0f0f0);  //  11110000 11110000 11110000 11110000 
      carry       = BS_VAL32(ff00ff00);  //  11111111 00000000 11111111 00000000
      carry1      = BS_VAL32(ffff0000);  //  11111111 11111111 00000000 00000000
                                                                        
                                         //  32212110 32212110 32212110 32212110  dec 4+5+6
      //sum456      == BS_VAL32(96969696)    10010110 10010110 10010110 10010110 
      //co456       == BS_VAL32(e8e8e8e8)    11101000 11101000 11101000 11101000
      //cohalf      == BS_VAL32(96009600)    10010110 00000000 10010110 00000000 sum456+carry

      //                                     43323221 43323221 32212110 32212110  dec 4+5+6+carry
      //keys_bs[56] == BS_VAL32(69966996);   01101001 10010110 01101001 10010110  sum out halfadd

      //cohalf      == BS_VAL32(96009600)    10010110 00000000 10010110 00000000 sum456+carry
      //co456       == BS_VAL32(e8e8e8e8)    11101000 11101000 11101000 11101000
      //carry1       = BS_VAL32(ffff0000);   11111111 11111111 00000000 00000000
      //                                     32222221 22212111 21111110 11101000  dec cohalf+co456+carry1

      //carry       == BS_VAL32(81177ee8);   10000001 00010111 01111110 11101000
      //carry1      == BS_VAL32(fee88000);   11111110 11101000 10000000 00000000
      */

   for (i = 32; i < 40; i++)
   {
      dvbcsa_bs_word_t tmp1, tmp2;
      aycw_bs_fulladd(&keys_bs[i], &keys_bs[i + 8], &keys_bs[i + 16], &sum456, &co456);
      aycw_bs_halfadd(&sum456, &carry); keys_bs[i + 24] = sum456;
      aycw_bs_fulladd(&carry, &co456, &carry1, &tmp1, &tmp2);
      carry = tmp1;
      carry1= tmp2;
   }
}




void aycw_assert_key_transpose(uint8 *keylist, dvbcsa_bs_word_t *bs_keys)
{
#ifdef SELFTEST
   uint8 i, k;
   uint8 mykeylist[BS_BATCH_SIZE][8];
   uint8 keylist2[BS_BATCH_SIZE][8];

   uint8 *a, *b, *c;

   memset(mykeylist, 0, sizeof(mykeylist));
   memcpy(keylist2, keylist, BS_BATCH_SIZE * 8);

   for (i = 0; i < BS_BATCH_SIZE; i++)
   {

      aycw_extractbsdata(bs_keys, i, 64, &mykeylist[i][0]);

      a = &keylist[i * 8];
      c = &keylist2[i][0];
      b = &mykeylist[i][0];
      if (k = memcmp(a, b, 8))
      {
         aycw_fatal_error("fail!\n");
      }
   }
#endif
}


/* plausibility check if bs keys are correct */
void aycw_assertKeyBatch(dvbcsa_bs_word_t *keys_bs)
{
#ifdef SELFTEST
   uint8 keys[BS_BATCH_SIZE][8];
   uint8 i,j;

   /* calc keys in normal form */
   memset(keys, 0, BS_BATCH_SIZE * 8);
   for (i = 0; i<BS_BATCH_SIZE; i++)
   {
      aycw_extractbsdata(keys_bs, i, 64, (uint8*)&keys[i]);
   }

   /* checksum correct? */
   for (i = 0; i < BS_BATCH_SIZE; i++)
   {
      if ((uint8)(keys[i][0] + keys[i][1] + keys[i][2]) != keys[i][3])
         aycw_fatal_error("aycw_assertKeyBatch() checksum wrong\n");
      if ((uint8)(keys[i][4] + keys[i][5] + keys[i][6]) != keys[i][7])
         aycw_fatal_error("aycw_assertKeyBatch() checksum wrong\n");
   }

   /* check key bytes of batches against each other */
   for (i = 0; i < BS_BATCH_SIZE; i++)
   {
      for (j = 0; j < i; j++)
      {
         /* byte 0...4 + byte 5 and 6 (partly) of all batches need to be identical */
         if (keys[i][0] != keys[j][0] || // key
             keys[i][1] != keys[j][1] || // key
             keys[i][2] != keys[j][2] || // key
             keys[i][3] != keys[j][3] || // crc
             keys[i][4] != keys[j][4] || // key
             keys[i][5] != keys[j][5] || // zero
             (keys[i][6] & (0xFF >> BS_BATCH_SHIFT)) != (keys[j][6] & (0xFF >> BS_BATCH_SHIFT)))  // all except upper bits are equal

             aycw_fatal_error("key bytes in batch error 1\n");
            /* byte 5...7 of all batches need to be different */
         if (keys[i][6] == keys[j][6] || // zero / batch divers
             keys[i][7] == keys[j][7])   // crc
             aycw_fatal_error("key bytes in batch error 2\n");
      }
   }
#endif
}

/**
set up data used for stream. Depends on scrambled data only, so can be global
@param   data_return[in]      result from aycw_stream_decrypt()
@param   outbits[in]          number of bits to calculate
@param   BS_key[in]           bit sliced key array 64 elements
@param   bs_data_sb0[in]      bit sliced 1st data block used for initialization
*/
void aycw_assert_stream(dvbcsa_bs_word_t * data, unsigned int outbits, dvbcsa_bs_word_t * BS_key, dvbcsa_bs_word_t *bs_data_sb0)
{
#ifdef SELFTEST
   int   i;

   for (i = 0; i < 1; i++)
   {
      dvbcsa_key_t   key;
      unsigned char  cw[8];
      unsigned char  iv[16];

      unsigned char ayc_stream[8];

      aycw_extractbsdata(BS_key, i, 64, cw);

      dvbcsa_key_set(cw, &key);

      aycw_extractbsdata(bs_data_sb0, i, 128, iv);

      dvbcsa_stream_xor(key.cws, iv, iv+8, 8);

      aycw_extractbsdata(data, i, 64, ayc_stream);

      if (memcmp(ayc_stream, iv+8, outbits/8))
      {
         aycw_fatal_error("aycw_assert_stream() failed\n");
      }
   }
#endif
}



/**
extract the data of one slice from a bitsliced array
@param   bs_data[in]     pointer to input bs array, length len
@param   slice[in]       number of slice to extract 0...BS_BATCH_SIZE
@param   len[in]         number of bits to extract (e.g. 64 for a key bs array)
@param   data[out]       pointer to regular data. this array must be at least len/8 bytes large!
*/
void aycw_extractbsdata(dvbcsa_bs_word_t* bs_data, unsigned char slice, unsigned char len, uint8* extracteddata)
{
   unsigned char     i, j, tmp;

   for (i = 0; i < (len / 8); i++)
   {
      tmp = 0;
      for (j = 0; j < 8; j++)
      {
         dvbcsa_bs_word_t  bs_tmp;
         tmp = tmp >> 1;
         bs_tmp = BS_AND(BS_SHR(bs_data[i * 8 + j], slice), BS_VAL_LSDW(1));
         tmp |= (uint8)BS_EXTLS32(BS_SHL(bs_tmp, 7));
      }
      extracteddata[i] = tmp;
   }
}

/**
   cross check the whole bitsliced decryption against the regular decryption
   Enable this after serious changes of the algorithm
   @param   probedata[in]     encrypted packet data (regular) 
   @param   keylist[in]       BS_BATCH_SIZE keys in regular form
   @param   data[in]          decryption result in bit/byte-sliced form
   @return  1 if successful
*/
void aycw_assert_decrypt_result(unsigned char *probedata, uint8 *keylist, dvbcsa_bs_word_t *data)
{
#ifdef SELFTEST
   int            i,j;

   for (i = 0; i < BS_BATCH_SIZE; i++)
   {
      dvbcsa_key_t   key;

      unsigned char lib_data[16];
      unsigned char ayc_data[8];

      dvbcsa_key_set(&keylist[i*8], &key);

      memcpy(lib_data, probedata, 16);    // 16 data bytes needed as input for algo, though only 8 are produced
      dvbcsa_decrypt(&key, lib_data, 16);

      // loop over DB0
      // aycw_extract_bytesliced_data(...);

#ifdef USEALLBITSLICE
      aycw_extractbsdata(data, i, 4*8, ayc_data);
#else
      for (j = 0; j < 8; j++)
      {
         int elem = i / BS_BATCH_BYTES + j * 8;    // number of bytesliced element
         int shift = i % BS_BATCH_BYTES * 8 ;      // byte shift inside element
         ayc_data[j] = (uint8)BS_EXTLS32(BS_SHR(data[elem], shift));
      }
#endif

      if (memcmp(lib_data, ayc_data, 3))
      {
         aycw_fatal_error("aycw_assert_decrypt_result() failed\n");
      }
   }
   /* keylist is usually not incremented during inner loop - it's not needed.
      For the cross check we need to make sure the keylist matches the keys incremented inner.
      So we apply the same algo to the keylist. A bit dirty, but does not affect the main loop */
   for (i = 0; i < BS_BATCH_SIZE; i++)
   {
      // 0...4 not changed in inner loop
      if (! (++keylist[i * 8 + 5])) keylist[i * 8 + 6]++;
      keylist[i * 8 + 7] = keylist[i * 8 + 4] + keylist[i * 8 + 5] + keylist[i * 8 + 6];
   }
#endif
}

/**
out[i] = in1[i] ^ in2[i]  for i 0...23
*/
void aycw_bs_xor24(dvbcsa_bs_word_t *out, dvbcsa_bs_word_t *in1, dvbcsa_bs_word_t *in2)
{
   int i;
   for (i = 0; i < 24; i++)
      out[i] = BS_XOR(in1[i], in2[i]);
}

#if 0
void aycw_selftest(void)
{
   dvbcsa_bs_word_t     tmp[64];
   int   i,row,col;

   for (row = 0; row < 64; row++)
   {
      for (col = 0; col < BS_BATCH_SIZE; col++)
      {
         for (i = 0; i < 64; i++) tmp[i] = 0;

         tmp[row] = BS_SHL(BS_VAL_LSDW(1)), col);

         aycw_bit2byteslice(tmp, 1);

         int byte = row / 8;
         int bit = row % 8;
         int byteoffset_col = col % BS_BATCH_BYTES;
         int newrow = 
         
         BS_SHR(tmp[newrow], + row%BS_BATCH_BYTES + col


   aycw_fatal_error("self test failed!\n");
}
#endif

#if 0
void aycw_testpattern(dvbcsa_bs_word_t *data, int count)
{
   int i, j, k;
   dvbcsa_bs_word_t  *p = data;
   dvbcsa_bs_word_t  tmp;

   for (k = 0; k < 8 * count; k++)
   {
      tmp = BS_VAL8(00);
      for (i = 0; i < BS_BATCH_SIZE; i++)
      {
         if (i & (1 << k))
         {
            tmp = BS_OR(tmp, BS_SHL(BS_VAL_LSDW(1), i));
         }
         else
         {
            tmp = BS_OR(tmp, BS_SHL(BS_VAL_LSDW(0), i));
         }
      }
      p[k] = tmp;
   }
}
#endif

#ifdef USE_SLOW_BIT2BYTESLICE
void aycw_bit2byteslice(dvbcsa_bs_word_t *data, int count)
{
   int i, j, k;
   dvbcsa_bs_word_t  *p = data;
   dvbcsa_bs_word_t  tmp;
   ;

   for (k = 0; k < 8 * count; k++)
   {
      dvbcsa_bs_word_t  tmp[8];
      for (i = 0; i < BS_BATCH_SIZE; i++)
      {
         dvbcsa_bs_word_t  tmp2;
         int               byte_offfset, word_offfset;

         byte_offfset = i % (BS_BATCH_SIZE / 8);   // to access bytes in dvbcsa_bs_word_t - 32bit: 0...3  128bit: 0...15
         word_offfset = i / (BS_BATCH_SIZE / 8);   // to access element of dvbcsa_bs_word_t array - always 0...8
         if (byte_offfset == 0)
         {
            tmp[word_offfset] = BS_VAL8(0);
         }

         tmp2 = BS_VAL8(0);
         aycw_extractbsdata(p, i, 8, &tmp2);
         tmp[word_offfset] = BS_OR(tmp[word_offfset], BS_SHL(tmp2, byte_offfset*8));
      }
      for (i = 0; i < 8; i++) p[i] = tmp[i];
      p += 8;
   }
}
#endif