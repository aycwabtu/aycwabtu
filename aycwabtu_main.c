
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#  include <windows.h>
#else
#  include <time.h>
#endif

#include "aycwabtu_config.h"
#include "aycwabtu_bs_stream.h"
#include "aycwabtu_bs_block.h"
#include "aycwabtu_bs_block_ab.h"
#include "aycwabtu_bs_algo.h"
#include "aycwabtu_ts.h"

#include "aycwabtu_version.h"    // this file contains GITSHA1 define and is generated before build
#define VERSION   "V1.1"

/* bitslice test cases */
#include "aycwabtu_bs_testcases.h"

/* libdvbcsa non-bitsliced decrypt function for key candidate verification */
#include "dvbcsa.h"


/****************** definitions ***********************/
/* key bits calculated in inner bs loop - do not change */
#define INNERKEYBITS    16
/* number of keys calculated in inner bs loop - do not change */
#define KEYSPERINNERLOOP (1<<INNERKEYBITS)

#define RESUMEFILENAME  "resume"
#define FOUNDFILENAME   "keyfound"

/****************** globals ***********************/
uint32 currentkey32;
uint32 stopkey32;

void ayc_printhexbytes(unsigned char *c, uint8 len)
{
   int i;
   for (i = 0; i<len; i++)
   {
      if ( i && !(i%4) ) printf(" ");
      printf("%02X", c[i]);
   }
   printf("\n");
}


/* get system timer ticks in milli seconds */
long int aycw__getTicks_ms(void)
{
#ifdef WIN32
			return GetTickCount();
#else
   typedef struct {
   long    tv_sec;        /* seconds */
   long    tv_usec;    /* microseconds */
   } timeval;

	//get the current number of microseconds since january 1st 1970
	timeval ts;
	gettimeofday(&ts,0);
	return (long int)(ts.tv_sec * 1000 + (ts.tv_usec / 1000));
#endif
}

/* globals for performance measure */
unsigned long time_start, deltaticks, totalticks = 0;
int totalloops = 0;
int divider = 0;


void aycw_performance_start(void)
{
   if (!divider) time_start = aycw__getTicks_ms();
}

/* print performance measure to console */
void aycw_perf_show(void)
{
   const char prop[] = "|/-\\";

#ifdef _DEBUG
#define DIVIDER 1
#else
#define DIVIDER 16      // reduce update frequency for release
#endif

   divider++; 
   if (divider >= DIVIDER) divider = 0;
   if (!divider)
   {
      putc(prop[(totalloops & 3)],stdout);
      deltaticks = aycw__getTicks_ms() - time_start; /* quick'n dirty - no overflow checking... */
      totalticks += deltaticks; totalloops++;
      //printf(" time per %dk keys: %dms", KEYSPERINNERLOOP / 1000, deltaticks);
      if (deltaticks)
      {
         printf(" %.3f Mcw/s ", ((float)KEYSPERINNERLOOP*DIVIDER / deltaticks / 1000));
      }
      if (totalticks)
      {
         printf("avg: %.3f Mcw/s  ", ((float)KEYSPERINNERLOOP*DIVIDER / ((float)totalticks / totalloops)) / 1000);
      }
      printf("%02X %02X %02X [] %02X .. .. []\r",
         currentkey32 >> 24,
         currentkey32 >> 16 & 0xFF,
         currentkey32 >> 8 & 0xFF,
         currentkey32 & 0xFF);
   }
}

/* save to the current key to file to remember brute force progress */
void aycw_write_resumefile(void)
{
   static int divider = 10;    /* long live the ssd */
   FILE * filehdl;
   char string[64];

   divider++; divider &= 0x1ff;
   if (!divider)
   {
      if (filehdl = fopen(RESUMEFILENAME, "w"))
      {
         //printf("\nwriting resume file\n");
         sprintf(string, "%02X %02X %02X %02X %02X %02X %02X %02X\n",
            (uint8)(currentkey32 >> 24), (uint8)(currentkey32 >> 16), (uint8)(currentkey32 >> 8), 0, (uint8)currentkey32, 0, 0, 0);
         fwrite(string, 1, strlen(string), filehdl);
         fclose(filehdl);
      }
      else
      {
         printf("error writing resume file\n");
      }
   }
}

void aycw_read_resumefile(uint32 *key)
{
   FILE * filehdl;
   unsigned char buf[8 * 3 + 2 + 1];
   unsigned char tmp[8+3]; /* visual C writes 4 bytes to pointer though 'hh' specifier was given */

   if (filehdl = fopen(RESUMEFILENAME, "rb"))
   {
      fseek(filehdl, 0, SEEK_SET);
      fread(buf, sizeof(buf), 1, filehdl);
      fclose(filehdl);
      if (8 == sscanf(buf, "%02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX\n",
         &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5], &tmp[6], &tmp[7] ) )
      {
         *key = tmp[0] << 24 | tmp[1] << 16 | tmp[2] << 8 | tmp[4];
         printf("resuming at key %08X\n", *key);
      }
   }
   else
   {
      /* just ignore missing resume file */
   }
}

void aycw_write_keyfoundfile(unsigned char *cw)
{
   FILE * filehdl;
   int i;
   char string[8*3+2+1];

   printf("writing result to file \"%s\"\n", FOUNDFILENAME);
   if (filehdl = fopen(FOUNDFILENAME, "w"))
   {
      sprintf(string, "%02X %02X %02X %02X %02X %02X %02X %02X\n",
         cw[0], cw[1], cw[2], cw[3], cw[4], cw[5], cw[6], cw[7]);
      fwrite(string, 1, strlen(string), filehdl);
      fclose(filehdl);
   }
   else
   {
      printf("error opening file \"%s\" for writing\n", FOUNDFILENAME);
   }
}

aycw_tstRegister     stRegister;
   dvbcsa_bs_word_t	r[8 * (1 + 8 + 56)];        // working data block

   dvbcsa_bs_word_t     bs_128[8 * 16];
   dvbcsa_bs_word_t     bs_64_1[64];
   dvbcsa_bs_word_t     bs_64_2[64];
   dvbcsa_bs_word_t     bs_448[448];
void aycw_partsbench(void)
{
#ifdef USE_MEASURE
   int i;
   const long int max = 1 << 19;
   long int start, diff;

   printf("performance measurement of all algorithmic parts for %d loops\n", max);

   start = aycw__getTicks_ms();
   for (i = 0; i<max; i++) aycw_stream_decrypt(bs_64_2, 25, bs_64_1, bs_128);
   printf("aycw_stream_decrypt()             %.3fs\n", ((float)aycw__getTicks_ms() - start) / 1000);

   start = aycw__getTicks_ms();
   for (i = 0; i<max; i++) aycw__vInitShiftRegister(bs_64_1, &stRegister);
   printf("  aycw__vInitShiftRegister()      %.3fs\n", ((float)aycw__getTicks_ms() - start) / 1000);

   start = aycw__getTicks_ms();
#ifndef USEALLBITSLICE
   for (i = 0; i<max; i++) aycw_bit2byteslice(bs_448, 7);
#endif
   printf("aycw_bit2byteslice(7)             %.3fs\n", ((float)aycw__getTicks_ms() - start) / 1000);

   start = aycw__getTicks_ms();
   for (i = 0; i<max; i++) aycw_block_key_schedule(bs_64_1, bs_448);
   printf("aycw_block_key_schedule           %.3fs\n", ((float)aycw__getTicks_ms() - start) / 1000);

   start = aycw__getTicks_ms();
   for (i = 0; i<max; i++) aycw_block_decrypt(bs_448, r);
   printf("aycw_block_decrypt                %.3fs\n", ((float)aycw__getTicks_ms() - start) / 1000);

   start = aycw__getTicks_ms();
#ifdef USEALLBITSLICE
   for (i = 0; i<56 * max; i++) aycw_block_sbox(r, bs_448);
#else
   for (i = 0; i<56 * max; i++) aycw_block_sbox(r, bs_448);
#endif
   printf("  aycw_block_sbox  (56x)          %.3fs\n", ((float)aycw__getTicks_ms() - start) / 1000);

   start = aycw__getTicks_ms();
   for (i = 0; i<max; i++) aycw_checkPESheader(r, bs_64_1);
   printf("aycw_checkPESheader               %.3fs\n", ((float)aycw__getTicks_ms() - start) / 1000);

   //printf("%d %d %d\n", bs_64_1[0], r[0], bs_448[0]);
#endif
}

void aycw_welcome_banner(void)
{
   printf("AYCWABTU CSA brute forcer %s %s", VERSION, __DATE__);
#ifdef _DEBUG
   printf(" DEBUG");
#endif
   printf("\ngit version hash: %s\n", GITSHA1);
   printf("\nCPU only, single threaded version");
#ifdef USEALLBITSLICE
   printf(" - all bit slice (bool sbox)");
#else
   printf(" - table sbox");
#endif
   printf("\nparallel bitslice batch size is %d\n", BS_BATCH_SIZE);
   printf("----------------------------------------\n");
   setbuf(stdout, NULL);   // for gcc
}


int main(int argc, char *argv[])
{
   int      i, k;
   int      benchmark=0;
   
   /************** stream ***************/
   dvbcsa_bs_word_t     bs_data_sb0[8 * 16];    // constant scrambled data blocks SB0 + SB1, global init for stream, da_diett.pdf 5.1
   dvbcsa_bs_word_t     bs_data_ib0[8 * 16];    // IB0 is bit/byte sliced block init vector, ib1 is bit sliced stream output
   /************** block ***************/
   dvbcsa_bs_word_t     keys_bs[64];            // bit sliced keys for block
   dvbcsa_bs_word_t     keyskk[448];            // bit sliced scheduled keys (64 bit -> 448 bit)

#ifdef USEBLOCKVIRTUALSHIFT
   dvbcsa_bs_word_t	r[8 * (1 + 8 + 56)];        // working data block
#else                                           //
   dvbcsa_bs_word_t	r[8 * (1 + 8 + 0)];         //
#endif
   dvbcsa_bs_word_t  candidates;       /* 1 marks a key candidate in the batch */

   uint8 keylist[BS_BATCH_SIZE][8];     /* the list of keys for the batch run in non-bitsliced form */

   /************ dummy data for benchmark run *****************/
   /* first two 8 byte data blocks from three different encrypted ts packets for brute force attack.
      initialized with test data for benchmark run if no real data is available */
   unsigned char probedata[3][16] = {
      { 0xB2, 0x74, 0x85, 0x51, 0xF9, 0x3C, 0x9B, 0xD2,  0x30, 0x9E, 0x8E, 0x78, 0xFB, 0x16, 0x55, 0xA9},
      { 0x25, 0x2D, 0x3D, 0xAB, 0x5E, 0x3B, 0x31, 0x39,  0xFE, 0xDF, 0xCD, 0x84, 0x51, 0x5A, 0x86, 0x4A},
      { 0xD0, 0xE1, 0x78, 0x48, 0xB3, 0x41, 0x63, 0x22,  0x25, 0xA3, 0x63, 0x0A, 0x0E, 0xD3, 0x1C, 0x70} };
   currentkey32 = 0x00 << 24 | 0x11 << 16 | 0x15 << 8 | 0x00;
   /* key   00 11 22 33  44 00 00 44 decrypts to
               000001ff11111111aa11111111111155
               000001ff11111111aa11111111111156
               000001ff11111111aa11111111111157  */
   /* expected stream output in IB1 is 6F CA 96 27 30 91 03 71 */
   stopkey32 = -1;   /* search up to FFFFF... */

   /************ global initilaization *****************/
   aycw_welcome_banner();
   aycw_partsbench();

   if (argc == 4)
   {
      unsigned char tmp[8+3];

      /* usage:   aycwabtu.exe <tsfile> <start key 6 hex digits> <stop key 6 hex digits> */
      ayc_read_ts(argv[1], &probedata[0][0]);

      if (6 != sscanf(argv[2], "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX", &tmp[0], &tmp[1], &tmp[2], &tmp[4], &tmp[5], &tmp[6]))
      {
         printf("start key parameter format incorrect. 6 hex bytes expected.\n");
         exit(ERR_STARTKEY);
      }
      // For easier calculation. Endianess?? Take byte 4 as LSB, looks nice
      currentkey32 = tmp[0] << 24 | tmp[1] << 16 | tmp[2] << 8 | tmp[4];

      if (6 != sscanf(argv[3], "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX", &tmp[0], &tmp[1], &tmp[2], &tmp[4], &tmp[5], &tmp[6]))
      {
         printf("stop key parameter format incorrect. 6 hex bytes expected.\n");
         exit(ERR_STOPKEY);
      }
      // For easier calculation. Endianess?? Take byte 4 as LSB, looks nice
      stopkey32 = tmp[0] << 24 | tmp[1] << 16 | tmp[2] << 8 | tmp[4];
      
      aycw_read_resumefile(&currentkey32);
   }
   else if (argc == 1)
   {
      printf("no TS input file given and no resume file found.\nUsing test data for benchmark run\n");
      printf("(pssst, the correct key is 00 11 22 [33] 44 00 00 [44])\n");
      benchmark=1;
   }
   else
   {
      printf("usage:\n");
      printf("aycwabtu.exe ts-file startkey stopkey\n\n");
      printf("    ts-file        transport stream file to obtain three packets\n");
      printf("                   for brute force attack\n");
      printf("    startkey       cw to start the brute force attack with. Checksum\n");
      printf("                   bytes are omittted. Format:   112233556677\n");
      printf("    stopkey        when this key is reached, program terminates\n");
      printf("\n");
      printf("  for benchmark run aycwabtu.exe without parameters\n");
      exit(ERR_USAGE);
   }


   printf("start key is %02X %02X %02X [] %02X %02X %02X []\n",
      (uint8)(currentkey32 >> 24), (uint8)(currentkey32 >> 16), (uint8)(currentkey32 >> 8), (uint8)currentkey32,0,0);
   printf("stop key is  %02X %02X %02X [] %02X %02X %02X []\n",
      (uint8)(stopkey32 >> 24), (uint8)(stopkey32 >> 16), (uint8)(stopkey32 >> 8), (uint8)stopkey32, 0xFF, 0xFF);


#ifdef WIN32
   SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
#endif

   aycw_init_block();

   aycw_init_stream(&probedata[0], &bs_data_sb0);

   for (i = 0; i < 8 * 8; i++)
   {
      bs_data_ib0[i] = bs_data_sb0[i];
   }
#ifndef USEALLBITSLICE
   aycw_bit2byteslice(bs_data_ib0, 1);
#endif

   /************* outer loop ******************/
   // run over whole key search space
   // key bytes incremented: 0 + 1 + 2 + 4 
   while (currentkey32 <= stopkey32)
   {
      aycw_performance_start();

      /* bytes 5 + 6 belong to the inner loop
         aycw_bs_increment_keys_inner() increments every slice by one starting byte 5 LSB (bit 40) 
         from byte 6 MSB down the slices spread key ranges.
         example: BS_BATCH_SIZE=32  -> topmost 5 bits of byte 6 (2^5==32) contain different values for batches
         
         batch    byte 5   byte 6
         0        00       00
         1        00       08
         2        00       10
         3        00       18
         .....
         31       00       F8         */
#if BS_BATCH_SIZE>256
#error keylist calculation cannot yet handle BS_BATCH_SIZE>256
#endif
      for (i = 0; i < BS_BATCH_SIZE; i++)
      {
         keylist[i][0] = currentkey32 >> 24;
         keylist[i][1] = currentkey32 >> 16;
         keylist[i][2] = currentkey32 >> 8;
         keylist[i][3] = keylist[i][0] + keylist[i][1] + keylist[i][2];
         keylist[i][4] = currentkey32;
         keylist[i][5] = 0;
         keylist[i][6] = (0x0100 >> BS_BATCH_SHIFT)*i;
         keylist[i][7] = keylist[i][4] + keylist[i][5] + keylist[i][6];
      }
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
      aycw_key_transpose(&keylist[0][0], keys_bs);     // transpose BS_BATCH_SIZE keys into bitsliced form

      // check if all keys were transposed correctly
      aycw_assert_key_transpose(&keylist[0][0], keys_bs);

      // inner loop: process 2^16 keys - see aycw_bs_increment_keys_inner()
      for (k = 0; k < KEYSPERINNERLOOP / BS_BATCH_SIZE; k++)
      {

         /* check if initial (outer) key and subsequent (inner) key batches are correct */
         aycw_assertKeyBatch(keys_bs);

         /************** stream ***************/
         aycw_stream_decrypt(&bs_data_ib0[64], 25, keys_bs, bs_data_sb0);    // 3 bytes required for PES check, 25 bits for some reason

         aycw_assert_stream(&bs_data_ib0[64], 25, keys_bs, bs_data_sb0);     // check if first bytes of IB1 output are correct

#ifndef USEALLBITSLICE
         aycw_bit2byteslice(&bs_data_ib0[64], 1);
#endif

         /************** block ***************/
         for (i = 0; i < 8 * 8; i++)
         {
#ifdef USEBLOCKVIRTUALSHIFT
            r[8 * 56 + i] = bs_data_ib0[i];      // r is the input/output working data for block
#else                                            // restore after each block run
            r[i] = bs_data_ib0[i];               // 
#endif
         }

         /* block schedule key 64 bits -> 448 bits */  /* OPTIMIZEME: only the 16 inner bits in inner loop */
         aycw_block_key_schedule(keys_bs, keyskk);

         /* byte transpose */
#ifndef USEALLBITSLICE
         aycw_bit2byteslice(keyskk, 7);    // 448 scheduled key bits / 64 key bits
#endif

         aycw_block_decrypt(keyskk, r);   // r is the generated block output

         {
/*#ifdef USEALLBITSLICE
            uint8 dump[8];
            aycw_extractbsdata(r, 0, 64, dump);
            printf("%02x %02x %02x %02x  %02x %02x %02x %02x\n",dump[0],dump[1],dump[2],dump[3],dump[4],dump[5],dump[6],dump[7]);
#else
            printf("%02x %02x %02x %02x  %02x %02x %02x %02x\n",(uint8)BS_EXTLS32(r[8 * 0]),(uint8)BS_EXTLS32(r[8 * 1]),(uint8)BS_EXTLS32(r[8 * 2]),(uint8)BS_EXTLS32(r[8 * 3]),(uint8)BS_EXTLS32(r[8 * 4]),(uint8)BS_EXTLS32(r[8 * 5]),(uint8)BS_EXTLS32(r[8 * 6]),(uint8)BS_EXTLS32(r[8 * 7]));
#endif*/
         }

         /************** block xor stream ***************/
         aycw_bs_xor24(r, r, &bs_data_ib0[64]);

         //for (i = 32; i < 64; i++) r[i] = BS_VAL8(55);   // destroy decrypted bytes 4...7 of DB0 shouldnt matter

         aycw_assert_decrypt_result(probedata, keylist, r);

         i = aycw_checkPESheader(r, &candidates);  /* OPTIMIZEME: return value should be first possible slice number to let the loop below start right there */
         if (i)
         {
            // candidate keys marked with '1' for the last batch run
            //printf("\n %d key candidate(s) found\n", i);
            for (i = 0; i < BS_BATCH_SIZE; i++)
            {
               unsigned char cw[8];
               dvbcsa_key_t  key;
               unsigned char data[16];
               memset(cw, 255, 8);
               if (1 == BS_EXTLS32(BS_AND(BS_SHR(candidates, i), BS_VAL8(01))))
               {
                  // candidate bit set, now extract the key bits
                  aycw_extractbsdata(keys_bs, i, 64, cw);

                  dvbcsa_key_set(&cw, &key);

                  memcpy(&data, &probedata[0], 16);
                  dvbcsa_decrypt(&key, data, 16);
                  if (data[0] != 0x00 || data[1] != 0x00 || data[2] != 0x01)
                  {
                     /* bitslice and regular implementations calculated different results - should never happen */
                     printf("\nFatal error: candidate verification failed!\n");
                     printf("last key was: %02X %02X %02X [%02X]  %02X %02X %02X [%02X]\n",
                        cw[0], cw[1], cw[2], cw[3], cw[4], cw[5], cw[6], cw[7]);
                     exit(ERR_FATAL);
                  }

                  memcpy(&data, &probedata[1], 16);
                  dvbcsa_decrypt(&key, data, 16);
                  if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x01)
                  {
                     memcpy(&data, &probedata[2], 16);
                     dvbcsa_decrypt(&key, data, 16);
                     if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x01)
                     {
                        printf("\nkey candidate successfully decrypted three packets\n");
                        printf("KEY FOUND!!!    %02X %02X %02X [%02X]  %02X %02X %02X [%02X]\n",
                           cw[0], cw[1], cw[2], cw[3], cw[4], cw[5], cw[6], cw[7]);

                        if (!benchmark) aycw_write_keyfoundfile(cw);
                        exit(KEYFOUND);

                     }
                  }
               }
            }
         }

         // set up the next BS_BATCH_SIZE keys
         aycw_bs_increment_keys_inner(keys_bs);

      }  // inner loop

      /***********************************************************************************************************************/
      /***********************************************************************************************************************/
      /***********************************************************************************************************************/
      aycw_perf_show();

      if (!benchmark) aycw_write_resumefile();

      currentkey32++;   // prepare for next 2^16 keys

   };  // while (currentkey32 < stopkey32)

   printf("\nStop key reached. No key found\n");
   exit(WORKPACKAGEFINISHED);

} // main

