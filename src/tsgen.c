#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dvbcsa.h"

int main(int argc, char *argv[])
{
   int            i;
   FILE           *filehdl;
   unsigned char  cw[8 + 3];
   unsigned char  buf[188];
   dvbcsa_key_t   key;

   if (argc != 3)
   {
      printf("usage:\n");
      printf("tsgen.exe tsfile key\n\n");
      printf("    tsgen will generate a transport stream (ts) file containing three packets encrypted with the same key\n");
      printf("    These packets decrypt to packets starting with MPEG PES header (0x00 0x00 0x01...).\n");
      printf("    can be used for testing key finders like aycwabtu\n\n");
      printf("    ts-file        output ts file name\n");
      printf("    cw             control word to encrypt the packets with. Format:   112233556677\n");
      printf("\n");
      exit(1);
   }

   if (6 != sscanf(argv[2], "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX", &cw[0], &cw[1], &cw[2], &cw[4], &cw[5], &cw[6]))
   {
      printf("cw format incorrect. 6 hex bytes expected.\n");
      exit(4);
   }
   cw[3] = cw[0] + cw[1] + cw[2];
   cw[7] = cw[4] + cw[5] + cw[6];

   if (filehdl = fopen(argv[1], "wb"))
   {
      for (i = 0; i < 3; i++)
      {
         /* The first 32 bits of one TS packet:
         ####### byte 0
         8  Sync  47H/01000111

         ####### byte 1+2
         1  TEI   Transport Error Indicator
         1  PUSI  Payload Unit Start Indicator
         1  TP    Transport Priority
         13 PID   packet ID (which prgramm in stream) 17 have special meaning, therefore 8175 left

         ####### byte 3
         2  TSC   Transport Scramble Control
            2-bit TSC Transport Scramble control;
            00 unencrypted packet;
            10 encrypted even
            11 encrypted odd
         2  AFC   Adaption Field Control
            1. 01 – no adaptation field, payload only
            2. 10 – adaptation field only, no payload
            3. 11 – adaptation field followed by payload
            4. 00 - RESERVED for future use
         4  CC    Continuity Counter; counts 0 to 15 sequentially for packets of same PID value */

         memset(buf, 0, 188);
         buf[0] = 0x47; 
         buf[1] = 0x40; 
         buf[2] = 0x7B; 
         buf[3] = 0x93;
         buf[6] = 1;
         buf[8] = i;    // generate three different packets
         dvbcsa_key_set(&cw, &key);
         dvbcsa_encrypt(&key, &buf[4], 184);
         fwrite(buf, 1, 188, filehdl);
      }
      fclose(filehdl);
   }
   else
   {
      printf("error opening file %s for writing\n", argv[1]);
      exit(2);
   }
   exit(0);
}