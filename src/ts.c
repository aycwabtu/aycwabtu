/* helper functions for
      read transport stream file,
      search for pusi packets within one key period,
      handle adaptation field,
      return the three data packets for brute force attack
      */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"


#define  PCKTSIZE             188

#define VERBOSELEVEL 0
#define msgDbg(level, ... ) if ((level)<=VERBOSELEVEL) {fprintf(stderr, __VA_ARGS__);}


/* read a new packet from TS input file and make basic plausibility checking */
unsigned char* ayc_read_packet(unsigned char* buf, int *pid, int *crypted, int *parity, int *pusi)
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

   if (buf[0] == 0x47)
   {
      *pid = 0x1FFF & (buf[1] << 8 | buf[2]);
      *pusi = buf[1] >> 6 & 0x01;
      *crypted = buf[3] >> 7;
      *parity = (buf[3] >> 6) & 1;

      msgDbg(2, "current packet: PID:0x%04x  PUSI: %d  crypted:%d parity:%d\n", *pid, *pusi, *crypted, *parity);

      switch (buf[3] >> 4 & 0x03)
      {
      case 1:
         // 01 – no adaptation field, payload only
         return &buf[4];
      case 2:
         // 10 – adaptation field only, no payload
         return NULL;
      case 3:
         // 11 – adaptation field followed by payload
         if (buf[4] < 188 - 4 - 1 - 16)
         {
            unsigned char* data;
            return &buf[4] + buf[4] + 1;   /* skipping adapt field. points now to 1st payload byte */
         }
        // data = &buf[4];
        // if (*data < 188 - 4 - 1 - 16)
        // {
        //    data += *data + 1;   /* skipping adapt field. points now to 1st payload byte */
        //    return 1;
        // }
         else
         {
            //printf("Adaptation Field is too long!.No space left for data\n");
            return NULL;
         }
      default:
         // 00 - RESERVED for future use
         return NULL;
      }
   }
   else
   {
      //printf("TS sync byte 0x47 not found at packet nr.: %lu (0x%08lx). TS corrupt?\r\n", gCurrentPacket + 1, gCurrentPacket*PCKTSIZE);
      printf("sync byte 0x47 not found. TS is corrupt!\n");
      exit(ERR_TS_CORRUPT);
   }
   return NULL;
}



/* read transport stream file and get three data blocks back to mount the brute forca attack on
   @tsfile     filename string
   @probedata  array [3][16]
   */
unsigned char ayc_read_ts(unsigned char *tsfile, unsigned char *probedata)
{
   unsigned long  len;
   FILE    *fptsfile;
   unsigned char     buf[PCKTSIZE];
   unsigned char     probefile[PCKTSIZE*4];
   unsigned char     *data, i;

   int pid, crypted, parity, pusi;
   signed int lockpid = 0, cryptedseen = 0, lockparity = 2, probecount = 0;


   printf("using TS file %s\n", tsfile);
   if (!tsfile || !(fptsfile = fopen(tsfile, "rb")))
   {
      printf("file open error\n");
      exit(ERR_TS_OPEN);
   }
   else
   {
      fseek(fptsfile, 0, SEEK_END);
      len = ftell(fptsfile);
      fseek(fptsfile, 0, SEEK_SET);
      if (len % PCKTSIZE)
      {
         printf("size of ts file is not multiple of 188. This is not a valid ts file.\n");
         exit(ERR_TS_CORRUPT);
      }
   }

   printf("searching for encrypted packets...\n", tsfile);
   while (fread(buf, sizeof(buf), 1, fptsfile) && probecount < 3)
   {
      data = ayc_read_packet(buf, &pid, &crypted, &parity, &pusi);
      if (data)
      {
         if (crypted)
         {
            cryptedseen = 1;
            if (pusi)
            {
               if (pid >= 0x0020 && pid <= 0x1FFE) // http://en.wikipedia.org/wiki/MPEG_transport_stream#Packet_Identifier_.28PID.29
               {
                  if ((lockpid != 0 && lockpid != pid)) 
                  {
                     msgDbg(2, "looking for pid %d but this packet has pid &d\n", lockpid, pid);
                     // keep on searching...
                     continue;   // waiting for our pid...
                  }
                  if ( (lockpid == 0) || (lockpid == pid) )
                  {
                     if (lockpid == 0)
                     {
                        lockpid = pid;
                        msgDbg(2, "locking to pid %d\n", pid);
                     }
                     if (lockparity !=2 && lockparity != parity)
                     {
                        printf("not enough encrypted packets with pid %d within key period with parity %d!\n", lockpid, lockparity);
                        // wait for another key period here?
                        exit(ERR_TS_UNUSABLE);
                     }
                     else
                     {
                        if (lockparity == 2)
                        {
                           lockparity = parity;
                           msgDbg(2, "locking to parity %d\n", parity);
                        }
                        memcpy(&probedata[probecount*16], data, 16);
                        memcpy(probefile + PCKTSIZE * probecount, buf, PCKTSIZE);
                        probecount++;
                     }
                  }
               } // valid pid
            } // pusi
         } // crypted
         else
         {
            /* this packet does not contain useful payload */
         }
      } // if (ayc_read_packet(buf, data, &pid, &crypted, &parity, &pusi))
   } // while (fread(buf, sizeof(buf), 1, fptsfile) && probecount < 3)
   fclose(fptsfile);

   if (!cryptedseen)
   {
      printf("TS file contains only unencrypted packets\n");
      exit(ERR_TS_UNENCRYPTED);
   }
   if (probecount < 3)
   {
      printf("TS file does not contain enough data packets for brute force\n");
      exit(ERR_TS_UNUSABLE);
   }

   printf("found three encrypted packets with pid %d (0x%X) cw parity %d:\n", pid, pid, parity);
   for (i = 0; i < 3 * 16; i++)
   {
      printf("%02X ", probedata[i]);
      if (!((i+1) % 8))  { printf(" "); }
      if (!((i+1) % 16)) { printf("\n"); }
   }

   if (
      (probedata[0 * 16 + 0] == 0x00 && probedata[0 * 16 + 1] == 0x00 && probedata[0 * 16 + 2] == 0x01) &&
      (probedata[1 * 16 + 0] == 0x00 && probedata[1 * 16 + 1] == 0x00 && probedata[1 * 16 + 2] == 0x01) &&
      (probedata[2 * 16 + 0] == 0x00 && probedata[2 * 16 + 1] == 0x00 && probedata[2 * 16 + 2] == 0x01)
      )
   {
      printf("This looks like decrypted content. No attack needed.\n");
      exit(ERR_TS_UNENCRYPTED);
   }

   if (len > 1 << 16)
   {
      unsigned char *append = "_ayc.ts";
      unsigned char probetsfilename[255] = "";
      /* write the important packets to a dummy ts file for sharing - unless the oringinal file is very small */
      if (!strstr(tsfile, append))
      {
         if (strlen(tsfile) < sizeof(probetsfilename) - strlen(append))
         {
            memcpy(probetsfilename, tsfile, strlen(tsfile));
            unsigned char *tmp = strrchr(probetsfilename, '.');
            if (tmp)
            {
               *tmp = 0;
               strcat(probetsfilename, append);
               tmp = "\x47\1\1\1\x0d\x0aThis file was generated by AYCWABTU\x0d\x0a";
               memset(probefile + PCKTSIZE * 3, 32, PCKTSIZE);
               memcpy(probefile + PCKTSIZE * 3, tmp, strlen(tmp));

               if (fptsfile = fopen(probetsfilename, "wb"))
               {
                  printf("writing dummy probe ts file %s\n", probetsfilename);
                  fwrite(probefile, sizeof(probefile), 1, fptsfile);
                  fclose(fptsfile);
               }
            }
         }
      }
   }
   return 1;
}