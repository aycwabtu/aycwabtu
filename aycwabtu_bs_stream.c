#include "aycwabtu_config.h"
#include "aycwabtu_bs_stream.h"


/* do the round mechanism and provide two stream bits per run */
void aycw__vRound(aycw_tstPQXYZ * stPQXYZ,
                   aycw_tstCDEF *stCDEF ,
                   aycw_tstRegister *stRegister,
                   dvbcsa_bs_word_t * BS_Streambit0,dvbcsa_bs_word_t * BS_Streambit1);

/* calculate init value of stream  */
void aycw__vInitRound(uint8 j, uint8 u8Byte, /*dvbcsa_bs_word_t iv, */
                       aycw_tstPQXYZ * stPQXYZ ,
                       aycw_tstCDEF *stCDEF, 
                       aycw_tstRegister *stRegister,
                       dvbcsa_bs_word_t *bs_data_sb0);

/* error C2719: '*fd': formal parameter with __declspec(align('16')) won't be aligned
Q: Can't align(16) structures be used as parameters?
A: Why would you want to pass a (large?) structure by value anyway?
Why not pass a pointer (or a reference) to the structure, then it will maintain its alignment since it's not actually copied anywhere.
*/
static AYCW_INLINE void aycw_bs_stream_sbox1(dvbcsa_bs_word_t *fa, dvbcsa_bs_word_t *fb,
               dvbcsa_bs_word_t *fc, 
               dvbcsa_bs_word_t *fd,
               dvbcsa_bs_word_t *fe,
               dvbcsa_bs_word_t *sa, 
               dvbcsa_bs_word_t *sb)
{
  dvbcsa_bs_word_t tmp0, tmp1, tmp2, tmp3;

  tmp0 = BS_XOR (*fa, BS_XOR (*fb, BS_NOT (BS_OR (BS_XOR (BS_OR (*fa, *fb), *fc), BS_XOR (*fc, *fd)))));
  tmp1 = BS_XOR (BS_OR (*fa, *fb), BS_NOT (BS_AND (*fc, BS_OR (*fa, BS_XOR (*fb, *fd)))));
  tmp2 = BS_XOR (*fa, BS_XOR (BS_AND (*fb, *fd), BS_OR (BS_AND (*fa, *fd), *fc)));
  tmp3 = BS_XOR (BS_AND (*fa, *fc), BS_XOR (*fa, BS_OR (BS_AND (*fa, *fb), *fd)));

  *sa = BS_XOR (tmp0, BS_AND (*fe, tmp1));
  *sb = BS_XOR (tmp2, BS_AND (*fe, tmp3));
}

static AYCW_INLINE void aycw_bs_stream_sbox2(dvbcsa_bs_word_t *fa, dvbcsa_bs_word_t *fb,
               dvbcsa_bs_word_t *fc, dvbcsa_bs_word_t *fd,
               dvbcsa_bs_word_t *fe,
               dvbcsa_bs_word_t *sa, dvbcsa_bs_word_t *sb)
{
  dvbcsa_bs_word_t tmp0, tmp1, tmp2, tmp3;

  tmp0 = BS_XOR (*fa, BS_XOR (BS_AND (*fb, BS_OR (*fc, *fd)), BS_XOR (*fc, BS_NOT (*fd))));
  tmp1 = BS_OR (BS_AND (*fa, BS_XOR (*fb, *fd)), BS_AND (BS_OR (*fa, *fb), *fc));
  tmp2 = BS_XOR (BS_AND (*fb, *fd), BS_OR (BS_AND (*fa, *fd), BS_XOR (*fb, BS_NOT (*fc))));
  tmp3 = BS_OR (BS_AND (*fa, *fd), BS_XOR (*fa, BS_XOR (*fb, BS_AND (*fc, *fd))));

  *sa = BS_XOR (tmp0, BS_AND (*fe, tmp1));
  *sb = BS_XOR (tmp2, BS_AND (*fe, tmp3));
}

static AYCW_INLINE void aycw_bs_stream_sbox3(dvbcsa_bs_word_t *fa, dvbcsa_bs_word_t *fb,
               dvbcsa_bs_word_t *fc, dvbcsa_bs_word_t *fd,
               dvbcsa_bs_word_t *fe,
               dvbcsa_bs_word_t *sa, dvbcsa_bs_word_t *sb)
{
  dvbcsa_bs_word_t tmp0, tmp1, tmp2;

  tmp0 = BS_XOR (*fa, BS_XOR (*fb, BS_XOR (BS_AND (*fc, BS_OR (*fa, *fd)), *fd)));
  tmp1 = BS_XOR (BS_AND (*fa, *fc), BS_OR (BS_XOR (*fa, *fd), BS_XOR (BS_OR (*fb, *fc), BS_NOT (*fd))));
  tmp2 = BS_XOR (*fa, BS_XOR (BS_AND (BS_XOR (*fb, *fc), *fd), *fc));

  *sa = BS_XOR (tmp0, BS_AND (BS_NOT (*fe), tmp1));
  *sb = BS_XOR (tmp2, *fe);
}

static AYCW_INLINE void aycw_bs_stream_sbox4(dvbcsa_bs_word_t *fa, dvbcsa_bs_word_t *fb,
               dvbcsa_bs_word_t *fc, dvbcsa_bs_word_t *fd,
               dvbcsa_bs_word_t *fe,
               dvbcsa_bs_word_t *sa, dvbcsa_bs_word_t *sb)
{
  dvbcsa_bs_word_t tmp0, tmp1, tmp2;

  tmp0 = BS_XOR (*fa, BS_OR (BS_AND (*fc, BS_XOR (*fa, *fd)), BS_XOR (*fb, BS_OR (*fc, BS_NOT (*fd)))));
  tmp1 = BS_XOR (BS_AND (*fa, *fb), BS_XOR (*fb, BS_XOR (BS_AND (BS_OR (*fa, *fc), *fd), *fc)));
  tmp2 = BS_XOR (*fa, BS_OR (BS_AND (*fb, *fc), BS_XOR (BS_OR (BS_AND (*fa, BS_XOR (*fb, *fd)), *fc), *fd)));

  *sa = BS_XOR (tmp0, BS_AND (*fe, BS_XOR (tmp1, tmp0)));
  *sb = BS_XOR (BS_XOR (*sa, tmp2), *fe);
}

static AYCW_INLINE void aycw_bs_stream_sbox5(dvbcsa_bs_word_t *fa, dvbcsa_bs_word_t *fb,
               dvbcsa_bs_word_t *fc, dvbcsa_bs_word_t *fd,
               dvbcsa_bs_word_t *fe,
               dvbcsa_bs_word_t *sa, dvbcsa_bs_word_t *sb)
{
  dvbcsa_bs_word_t tmp0, tmp1, tmp2, tmp3;

  tmp0 = BS_OR (BS_XOR (BS_AND (*fa, BS_OR (*fb, *fc)), *fb), BS_XOR (BS_OR (BS_XOR (*fa, *fc), *fd), BS_VAL8(FF)));
  tmp1 = BS_XOR (*fb, BS_AND (BS_XOR (*fc, *fd), BS_XOR (*fc, BS_OR (*fb, BS_XOR (*fa, *fd)))));
  tmp2 = BS_XOR (BS_AND (*fa, *fc), BS_XOR (*fb, BS_AND (BS_OR (*fb, BS_XOR (*fa, *fc)), *fd)));
  tmp3 = BS_OR (BS_AND (BS_XOR (*fa, *fb), BS_XOR (*fc, BS_VAL8(FF))), *fd);

  *sa = BS_XOR (tmp0, BS_AND (*fe, tmp1));
  *sb = BS_XOR (tmp2, BS_AND (*fe, tmp3));
}

static AYCW_INLINE void aycw_bs_stream_sbox6(dvbcsa_bs_word_t *fa, dvbcsa_bs_word_t *fb,
               dvbcsa_bs_word_t *fc, dvbcsa_bs_word_t *fd,
               dvbcsa_bs_word_t *fe,
               dvbcsa_bs_word_t *sa, dvbcsa_bs_word_t *sb)
{
  dvbcsa_bs_word_t tmp0, tmp1, tmp2, tmp3;

  tmp0 = BS_XOR (BS_AND (BS_AND (*fa, *fc), *fd), BS_XOR (BS_AND (*fb, BS_OR (*fa, *fd)), *fc));
  tmp1 = BS_NOT (BS_AND (BS_XOR (*fa, *fc), *fd));
  tmp2 = BS_XOR (BS_AND (*fa, BS_OR (*fb, *fc)), BS_XOR (*fb, BS_OR (BS_AND (*fb, *fc), *fd)));
  tmp3 = BS_AND (*fc, BS_XOR (BS_AND (*fa, BS_XOR (*fb, *fd)), BS_OR (*fb, *fd)));

  *sa = BS_XOR (tmp0, BS_AND (*fe, tmp1));
  *sb = BS_XOR (tmp2, BS_AND (*fe, tmp3));
}

static AYCW_INLINE void aycw_bs_stream_sbox7(dvbcsa_bs_word_t *fa, dvbcsa_bs_word_t *fb,
               dvbcsa_bs_word_t *fc, dvbcsa_bs_word_t *fd,
               dvbcsa_bs_word_t *fe,
               dvbcsa_bs_word_t *sa, dvbcsa_bs_word_t *sb)
{
  dvbcsa_bs_word_t tmp0, tmp1, tmp2, tmp3;

  tmp0 = BS_XOR (*fb, BS_OR (BS_AND (*fc, *fd), BS_XOR (*fa, BS_XOR (*fc, *fd))));
  tmp1 = BS_AND (BS_OR (*fb, *fd), BS_OR (BS_AND (*fa, *fc), BS_XOR (*fb, BS_XOR (*fc, *fd))));
  tmp2 = BS_XOR (BS_OR (*fa, *fb), BS_XOR (BS_AND (*fc, BS_OR (*fb, *fd)), *fd));
  tmp3 = BS_OR (*fd, BS_XOR (BS_AND (*fa, *fc), BS_VAL8(FF)));

  *sa = BS_XOR (tmp0, BS_AND (*fe, tmp1));
  *sb = BS_XOR (tmp2, BS_AND (*fe, tmp3));
}


AYCW_INLINE void aycw__vInitVariables(aycw_tstPQXYZ * stPQXYZ,
                          aycw_tstCDEF *stCDEF ,
                          dvbcsa_bs_word_t * BS_Streambit0,      
                          dvbcsa_bs_word_t * BS_Streambit1)
{
    uint8 i;
    /* init values */
    for (i = 0; i < 4; i++)
    {
        
        stPQXYZ->BS_X[i] = BS_VAL8(00); 
        stPQXYZ->BS_Y[i] = BS_VAL8(00); 
        stPQXYZ->BS_Z[i] = BS_VAL8(00); 
        stCDEF->BS_D[i]  = BS_VAL8(00); 
        stCDEF->BS_E[i]  = BS_VAL8(00);
        stCDEF->BS_F[i]  = BS_VAL8(00);
    }
    stPQXYZ->BS_P  = BS_VAL8(00); 
    stPQXYZ->BS_Q  = BS_VAL8(00);
    stCDEF->BS_C   = BS_VAL8(00);
    *BS_Streambit0 = BS_VAL8(00);
    *BS_Streambit1 = BS_VAL8(00);
}

static AYCW_INLINE void aycw__vInitShiftRegister(dvbcsa_bs_word_t * BS_key, aycw_tstRegister * stRegister)
{
   uint8 i;
   /******   A & B init ********************/
   /* set everything outside bit 32 to zero */


   /* load A and B into A_BS and B_BS */
   for (i = 0; i < 32; i+=8)
   {
      stRegister->A_BS[i + 0] = BS_key[i + 4];
      stRegister->A_BS[i + 1] = BS_key[i + 5];
      stRegister->A_BS[i + 2] = BS_key[i + 6];
      stRegister->A_BS[i + 3] = BS_key[i + 7];
      stRegister->A_BS[i + 4] = BS_key[i + 0];
      stRegister->A_BS[i + 5] = BS_key[i + 1];
      stRegister->A_BS[i + 6] = BS_key[i + 2];
      stRegister->A_BS[i + 7] = BS_key[i + 3];
      stRegister->B_BS[i + 0] = BS_key[i + 4 + 32];
      stRegister->B_BS[i + 1] = BS_key[i + 5 + 32];
      stRegister->B_BS[i + 2] = BS_key[i + 6 + 32];
      stRegister->B_BS[i + 3] = BS_key[i + 7 + 32];
      stRegister->B_BS[i + 4] = BS_key[i + 0 + 32];
      stRegister->B_BS[i + 5] = BS_key[i + 1 + 32];
      stRegister->B_BS[i + 6] = BS_key[i + 2 + 32];
      stRegister->B_BS[i + 7] = BS_key[i + 3 + 32];
   }

        for (/*i = 32*/; i < 40; i++)
   {
      stRegister->A_BS[i] =   BS_VAL8(00);
      stRegister->B_BS[i] =   BS_VAL8(00);
   }
}


AYCW_INLINE dvbcsa_bs_word_t aycw__BitExpandOfByteToBsWord(const uint8 *u8ByteToExpand, const uint8 u8Byte, const uint8 u8Bit)
{
   dvbcsa_bs_word_t ret;
   if ((0x1 << u8Bit) & u8ByteToExpand[u8Byte])
   {
      ret = BS_VAL8(FF);
   }
   else
   {
      ret = BS_VAL8(00);
   }
   return ret;
}


AYCW_INLINE void aycw__ShiftRegisterLeft(dvbcsa_bs_word_t * RegisterValue, uint8 u8ShiftValue, uint8 u8Arraysize)
{
    uint8 i;
    /* move first */
    for ( i = (u8Arraysize-1); i >= u8ShiftValue; i--)
    {
        RegisterValue[i] = RegisterValue[i-u8ShiftValue];
    }
    /* set rest to zero... */
    for ( i = 0; i < u8ShiftValue; i++)
    {
        RegisterValue[i] = BS_VAL8(00);
    }

}

void aycw__vCaculatePQXYZ(dvbcsa_bs_word_t * A_local, aycw_tstPQXYZ * stPQXYZ)
{   
    /*dvbcsa_bs_word_t s1a,s1b,s2a,s2b,s3a,s3b,s4a,s4b,s5a,s5b,s6a,s6b,s7a,s7b;*/

/*  
#define shift 4
    aycw_bs_stream_sbox1(A_local[2+shift],  A_local[21+shift], A_local[27+shift], A_local[32+shift], A_local[12+shift], &s1a, &s1b);
    aycw_bs_stream_sbox2(A_local[10+shift], A_local[23+shift], A_local[24+shift], A_local[33+shift], A_local[5+shift], &s2a, &s2b);
    aycw_bs_stream_sbox3(A_local[4+shift],  A_local[17+shift], A_local[19+shift], A_local[22+shift], A_local[3+shift], &s3a, &s3b);
    aycw_bs_stream_sbox4(A_local[1+shift],  A_local[7+shift],  A_local[14+shift], A_local[28+shift], A_local[11+shift], &s4a, &s4b);
    aycw_bs_stream_sbox5(A_local[15+shift], A_local[20+shift], A_local[29+shift], A_local[34+shift], A_local[18+shift], &s5a, &s5b);
    aycw_bs_stream_sbox6(A_local[13+shift], A_local[16+shift], A_local[26+shift], A_local[35+shift], A_local[9+shift], &s6a, &s6b);
    aycw_bs_stream_sbox7(A_local[8+shift],  A_local[25+shift], A_local[30+shift], A_local[31+shift], A_local[6+shift], &s7a, &s7b);
    stPQXYZ->BS_X[0] = s1a;
    stPQXYZ->BS_X[1] = s2a;
    stPQXYZ->BS_X[2] = s3b;
    stPQXYZ->BS_X[3] = s4b;
    stPQXYZ->BS_Y[0] = s3a;
    stPQXYZ->BS_Y[1] = s4a;
    stPQXYZ->BS_Y[2] = s5b;
    stPQXYZ->BS_Y[3] = s6b;
    stPQXYZ->BS_Z[0] = s5a;
    stPQXYZ->BS_Z[1] = s6a;
    stPQXYZ->BS_Z[2] = s1b;
    stPQXYZ->BS_Z[3] = s2b;
    stPQXYZ->BS_P = s7a;
    stPQXYZ->BS_Q = s7b;
    */
    aycw_bs_stream_sbox1(&A_local[6],  &A_local[25], &A_local[31], &A_local[36], &A_local[16], &stPQXYZ->BS_X[0], &stPQXYZ->BS_Z[2]);
    aycw_bs_stream_sbox2(&A_local[14], &A_local[27], &A_local[28], &A_local[37], &A_local[9],  &stPQXYZ->BS_X[1], &stPQXYZ->BS_Z[3]);
    aycw_bs_stream_sbox3(&A_local[8],  &A_local[21], &A_local[23], &A_local[26], &A_local[7],  &stPQXYZ->BS_Y[0], &stPQXYZ->BS_X[2]);
    aycw_bs_stream_sbox4(&A_local[5],  &A_local[11], &A_local[18], &A_local[32], &A_local[15], &stPQXYZ->BS_Y[1], &stPQXYZ->BS_X[3]);
    aycw_bs_stream_sbox5(&A_local[19], &A_local[24], &A_local[33], &A_local[38], &A_local[22], &stPQXYZ->BS_Z[0], &stPQXYZ->BS_Y[2]);
    aycw_bs_stream_sbox6(&A_local[17], &A_local[20], &A_local[30], &A_local[39], &A_local[13], &stPQXYZ->BS_Z[1], &stPQXYZ->BS_Y[3]);
    aycw_bs_stream_sbox7(&A_local[12], &A_local[29], &A_local[34], &A_local[35], &A_local[10], &stPQXYZ->BS_P ,   &stPQXYZ->BS_Q);




}
/****** execute init round **************/
AYCW_INLINE void aycw__vInitRound(uint8 j, uint8 u8Byte,
                                  aycw_tstPQXYZ * stPQXYZ ,
                                  aycw_tstCDEF *stCDEF, 
                                  aycw_tstRegister *stRegister,
                                  dvbcsa_bs_word_t *bs_data_sb0)
{

    uint8 k;
    dvbcsa_bs_word_t /*tmp,*/tmp0,tmp4,tmp1,tmp3;
    uint8 b;
    /*uint8 u32IV;*/
    dvbcsa_bs_word_t  BS_TMP_B[5];
    dvbcsa_bs_word_t  BS_TMP_B_Result[4];
    dvbcsa_bs_word_t  BS_Bout[4];
    dvbcsa_bs_word_t  BS_Enew[4];
/*
    if (j & 0x1) // all odd
    {
        u32IV = (iv >> 4 | (iv << 4));
    }
    else
    {
        u32IV = iv;
    }
*/
    aycw__ShiftRegisterLeft( stRegister->A_BS, 4, 44);
    for (k = 0; k < 4; k++)
    {
        stRegister->A_BS[k] = stRegister->A_BS[k+40]; 
        BS_XOREQ(stRegister->A_BS[k],stPQXYZ->BS_X[k]);
        BS_XOREQ(stRegister->A_BS[k],stCDEF->BS_D[k]);              /* data, byte, bit */
        BS_XOREQ(stRegister->A_BS[k],( (j & 0x1)?bs_data_sb0[k+u8Byte*8]:bs_data_sb0[k+4+u8Byte*8]));
    }

    for (k = 0; k < 4; k++)
    {

        BS_TMP_B[k] = BS_XOR(stRegister->B_BS[k+6*4], stRegister->B_BS[k+9*4]);
        BS_TMP_B[k] = BS_XOR(BS_TMP_B[k], stPQXYZ->BS_Y[k]);

        // OPTIMIZEME: optimizeable by using the upper A_tBITVALUE calculation
        /*tmp = (j & 0x1)?bs_data_sb0[k+4+u8Byte*8]:bs_data_sb0[k+u8Byte*8];
        tmp0 = aycw__BitExpandOfByteToBsWord(&u32IV, 0, k); 
        if (tmp0 != tmp)
            return 0;*/
        BS_TMP_B[k] = BS_XOR(BS_TMP_B[k], ( (j & 0x1)?bs_data_sb0[k+4+u8Byte*8]:bs_data_sb0[k+u8Byte*8]));

        BS_TMP_B_Result[k]  = BS_AND(BS_TMP_B[k], BS_NOT(stPQXYZ->BS_P)); //Die nicht zu rotierenden Daten zunächst zwischenspeichern.

    }
    /* yet rotate B */
    /* OPTIMIZEME: write into B directly?? */
    for (k = 4; k > 0; k--)
    {
        BS_TMP_B[k] = BS_TMP_B[k-1];
    }
    BS_TMP_B[0] = BS_TMP_B[4];

    /* B must not be moved by 4 before handling is finished */
    aycw__ShiftRegisterLeft(stRegister->B_BS, 4, 44);

    /* now write the result, both rotated and unrotated */
    for ( k = 0; k < 4; k++)
    {             // rotated         //not rotated
        stRegister->B_BS[k] = BS_OR(BS_AND(BS_TMP_B[k], stPQXYZ->BS_P), BS_TMP_B_Result[k]);
    }

    /********** Combiner calculation **********/
    /* calc bout
    Bout 3 := b2,0  b5,1  b6,2  b8,3
    Bout 2 := b5,0  b7,1  b2,3  b3,2
    Bout 1 := b4,3  b7,2  b3,0  b4,1
    Bout 0 := b8,2  b5,3  b2,1  b7,0
    */
    /*
#define shiftB 4
    BS_Bout[3] = stRegister->B_BS[2*4 + 0 + shiftB ] ^ stRegister->B_BS[5*4 + 1 + shiftB ]  ^ stRegister->B_BS[6*4 + 2 + shiftB ] ^ stRegister->B_BS[8*4 + 3  + shiftB ] ;
    BS_Bout[2] = stRegister->B_BS[5*4 + 0 + shiftB ] ^ stRegister->B_BS[7*4 + 1 + shiftB ]  ^ stRegister->B_BS[2*4 + 3 + shiftB ] ^ stRegister->B_BS[3*4 + 2  + shiftB ] ;
    BS_Bout[1] = stRegister->B_BS[4*4 + 3 + shiftB ] ^ stRegister->B_BS[7*4 + 2 + shiftB ]  ^ stRegister->B_BS[3*4 + 0 + shiftB ] ^ stRegister->B_BS[4*4 + 1  + shiftB ] ;
    BS_Bout[0] = stRegister->B_BS[8*4 + 2 + shiftB ] ^ stRegister->B_BS[5*4 + 3 + shiftB ]  ^ stRegister->B_BS[2*4 + 1 + shiftB ] ^ stRegister->B_BS[7*4 + 0  + shiftB ] ;

*/
    BS_Bout[3] = BS_XOR(BS_XOR(stRegister->B_BS[12], stRegister->B_BS[25]),  BS_XOR(stRegister->B_BS[30], stRegister->B_BS[39]));
    BS_Bout[2] = BS_XOR(BS_XOR(stRegister->B_BS[24], stRegister->B_BS[33]),  BS_XOR(stRegister->B_BS[15], stRegister->B_BS[18]));
    BS_Bout[1] = BS_XOR(BS_XOR(stRegister->B_BS[23], stRegister->B_BS[34]),  BS_XOR(stRegister->B_BS[16], stRegister->B_BS[21]));
    BS_Bout[0] = BS_XOR(BS_XOR(stRegister->B_BS[38], stRegister->B_BS[27]),  BS_XOR(stRegister->B_BS[13], stRegister->B_BS[32]));

    /* calc D */
    for (k = 0; k < 4; k++)
    {
        stCDEF->BS_D[k] = BS_XOR(BS_XOR(BS_Bout[k], stCDEF->BS_E[k]), stPQXYZ->BS_Z[k]);
    }


    for (b = 0; b < 4; b++)
        BS_Enew[b] = stCDEF->BS_F[b];

    tmp0 = BS_XOR (stPQXYZ->BS_Z[0], stCDEF->BS_E[0]);
    tmp1 = BS_AND (stPQXYZ->BS_Z[0], stCDEF->BS_E[0]);
    stCDEF->BS_F[0] = BS_XOR (stCDEF->BS_E[0], BS_AND (stPQXYZ->BS_Q, BS_XOR (stPQXYZ->BS_Z[0], stCDEF->BS_C)));
    tmp3 = BS_AND (tmp0, stCDEF->BS_C);
    tmp4 = BS_OR (tmp1, tmp3);

    tmp0 = BS_XOR (stPQXYZ->BS_Z[1], stCDEF->BS_E[1]);
    tmp1 = BS_AND (stPQXYZ->BS_Z[1], stCDEF->BS_E[1]);
    stCDEF->BS_F[1] = BS_XOR (stCDEF->BS_E[1], BS_AND (stPQXYZ->BS_Q, BS_XOR (stPQXYZ->BS_Z[1], tmp4)));
    tmp3 = BS_AND (tmp0, tmp4);
    tmp4 = BS_OR (tmp1, tmp3);

    tmp0 = BS_XOR (stPQXYZ->BS_Z[2], stCDEF->BS_E[2]);
    tmp1 = BS_AND (stPQXYZ->BS_Z[2], stCDEF->BS_E[2]);
    stCDEF->BS_F[2] = BS_XOR (stCDEF->BS_E[2], BS_AND (stPQXYZ->BS_Q, BS_XOR (stPQXYZ->BS_Z[2], tmp4)));
    tmp3 = BS_AND (tmp0, tmp4);
    tmp4 = BS_OR (tmp1, tmp3);

    tmp0 = BS_XOR (stPQXYZ->BS_Z[3], stCDEF->BS_E[3]);
    tmp1 = BS_AND (stPQXYZ->BS_Z[3], stCDEF->BS_E[3]);
    stCDEF->BS_F[3] = BS_XOR (stCDEF->BS_E[3], BS_AND (stPQXYZ->BS_Q, BS_XOR (stPQXYZ->BS_Z[3], tmp4)));
    tmp3 = BS_AND (tmp0, tmp4);
    stCDEF->BS_C = BS_XOR (stCDEF->BS_C, BS_AND (stPQXYZ->BS_Q, BS_XOR (BS_OR (tmp1, tmp3), stCDEF->BS_C)));    // ultimate carry

    for (b = 0; b < 4; b++)
        stCDEF->BS_E[b] = BS_Enew[b];

    aycw__vCaculatePQXYZ(stRegister->A_BS, stPQXYZ);

}


/****** execute round **************/
AYCW_INLINE void aycw__vRound(aycw_tstPQXYZ     *stPQXYZ,
                              aycw_tstCDEF      *stCDEF ,
                              aycw_tstRegister  *stRegister,
                              dvbcsa_bs_word_t  *BS_Streambit0,
                              dvbcsa_bs_word_t  *BS_Streambit1)
{

    uint8 k;
    dvbcsa_bs_word_t tmp0,tmp4,tmp1,tmp3;
    uint8 b;
    dvbcsa_bs_word_t  BS_TMP_B[5];
    dvbcsa_bs_word_t  BS_TMP_B_Result[4];
    dvbcsa_bs_word_t  BS_Bout[4];
    dvbcsa_bs_word_t  BS_Enew[4];


    aycw__ShiftRegisterLeft( stRegister->A_BS, 4, 80);
    for (k = 0; k < 4; k++)
    {
        stRegister->A_BS[k] = stRegister->A_BS[k+40]; 
        BS_XOREQ(stRegister->A_BS[k], stPQXYZ->BS_X[k]);

    }

    for (k = 0; k < 4; k++)
    {

        //A: 0x0000000bb6510468 
        //B: 0x00000006baf6a610
    //  BS_ConverToDoubleArray(&tmpDoubleArray,B_BS,40);
        BS_TMP_B[k] = BS_XOR(stRegister->B_BS[k+6*4], stRegister->B_BS[k+9*4]);
    //  BS_ConverToByteArray(&tmpByteArray,BS_TMP_B,4);
        BS_TMP_B[k] = BS_XOR(BS_TMP_B[k], stPQXYZ->BS_Y[k]);
    //  BS_ConverToIntArray(tmpIntArray,B_BS,40);
#if Init    
        BS_TMP_B[k] = BS_TMP_B[k] ^ A_tBITVALUE(&u32IV, 0, k);   
        BS_ConverToIntArray(tmpIntArray,B_BS,40);
#endif
        BS_TMP_B_Result[k]  = BS_AND(BS_TMP_B[k], BS_NOT(stPQXYZ->BS_P)); //Die nicht zu rotierenden Daten zunächst zwischenspeichern.

    //  BS_ConverToIntArray(tmpIntArray,B_BS,40);
        //B: 0x00000006baf6a617

    }
    for (k = 4; k > 0; k--)
    {
        BS_TMP_B[k] = BS_TMP_B[k-1];
    }
    BS_TMP_B[0] = BS_TMP_B[4];

    aycw__ShiftRegisterLeft( stRegister->B_BS, 4, 80);

    for ( k = 0; k < 4; k++)
    {
        stRegister->B_BS[k] = BS_OR(BS_AND(BS_TMP_B[k], stPQXYZ->BS_P), BS_TMP_B_Result[k]);
    }
//  BS_ConverToIntArray(tmpIntArray,B_BS,40);

//  BS_ConverToDoubleArray(&tmpDoubleArray,B_BS,40);

    /********** execute combiner calc **********/
    /* calc of bout*/
    /*
    Bout 3 := b2,0  b5,1  b6,2  b8,3
    Bout 2 := b5,0  b7,1  b2,3  b3,2
    Bout 1 := b4,3  b7,2  b3,0  b4,1
    Bout 0 := b8,2  b5,3  b2,1  b7,0
    */
    /*12 25 30 37*/
    /*    10000000 01000010000000000001000000000000 */
    /*15 18 24 33*/
    /*   00000010 00000001000001001000000000000000 */
    /*16 21 23 34*/
    /*  00000100 00000000101000010000000000000000 */
    /*13 27 32 38*/
    /* 01000001 00001000000000000010000000000000 */

    /*8  21  26  35*/
    /*extra_B[3] = BS_XOR (BS_XOR (BS_XOR (B[2][0], B[5][1]), B[6][2]), B[8][3]);
    extra_B[2] = BS_XOR (BS_XOR (BS_XOR (B[5][0], B[7][1]), B[2][3]), B[3][2]);
    extra_B[1] = BS_XOR (BS_XOR (BS_XOR (B[4][3], B[7][2]), B[3][0]), B[4][1]);
    extra_B[0] = BS_XOR (BS_XOR (BS_XOR (B[8][2], B[5][3]), B[2][1]), B[7][0]);*/
/*
#define shiftB 4
    BS_Bout[3] = stRegister->B_BS[2*4 + 0 + shiftB ] ^ stRegister->B_BS[5*4 + 1 + shiftB ]  ^ stRegister->B_BS[6*4 + 2 + shiftB ] ^ stRegister->B_BS[8*4 + 3  + shiftB ] ;
    BS_Bout[2] = stRegister->B_BS[5*4 + 0 + shiftB ] ^ stRegister->B_BS[7*4 + 1 + shiftB ]  ^ stRegister->B_BS[2*4 + 3 + shiftB ] ^ stRegister->B_BS[3*4 + 2  + shiftB ] ;
    BS_Bout[1] = stRegister->B_BS[4*4 + 3 + shiftB ] ^ stRegister->B_BS[7*4 + 2 + shiftB ]  ^ stRegister->B_BS[3*4 + 0 + shiftB ] ^ stRegister->B_BS[4*4 + 1  + shiftB ] ;
    BS_Bout[0] = stRegister->B_BS[8*4 + 2 + shiftB ] ^ stRegister->B_BS[5*4 + 3 + shiftB ]  ^ stRegister->B_BS[2*4 + 1 + shiftB ] ^ stRegister->B_BS[7*4 + 0  + shiftB ] ;
*/
    BS_Bout[3] = BS_XOR(BS_XOR(stRegister->B_BS[12], stRegister->B_BS[25]), BS_XOR(stRegister->B_BS[30], stRegister->B_BS[39]));
    BS_Bout[2] = BS_XOR(BS_XOR(stRegister->B_BS[24], stRegister->B_BS[33]), BS_XOR(stRegister->B_BS[15], stRegister->B_BS[18]));
    BS_Bout[1] = BS_XOR(BS_XOR(stRegister->B_BS[23], stRegister->B_BS[34]), BS_XOR(stRegister->B_BS[16], stRegister->B_BS[21]));
    BS_Bout[0] = BS_XOR(BS_XOR(stRegister->B_BS[38], stRegister->B_BS[27]), BS_XOR(stRegister->B_BS[13], stRegister->B_BS[32]));

    /*
    Bout[3] = B_BS[1*4 + 0 ] ^ B_BS[4*4 + 1 ]  ^ B_BS[5*4 + 2 ] ^ B_BS[7*4 + 3  ] ;
    Bout[2] = B_BS[4*4 + 0 ] ^ B_BS[6*4 + 1 ]  ^ B_BS[1*4 + 3 ] ^ B_BS[2*4 + 2  ] ;
    Bout[1] = B_BS[3*4 + 3 ] ^ B_BS[6*4 + 2 ]  ^ B_BS[2*4 + 0 ] ^ B_BS[3*4 + 1  ] ;
    Bout[0] = B_BS[7*4 + 2 ] ^ B_BS[4*4 + 3 ]  ^ B_BS[1*4 + 1 ] ^ B_BS[6*4 + 0  ] ;
    */


    /* calc of D */
    for (k = 0; k < 4; k++)
    {
        /*   use old E????*/
        stCDEF->BS_D[k] = BS_XOR(BS_XOR(BS_Bout[k], stCDEF->BS_E [k]), stPQXYZ->BS_Z[k]);
    //  BS_ConverToByteArray(&tmpByteArray,BS_E,4);
    //  BS_ConverToByteArray(&tmpByteArray,BS_Z,4);
    }
//  BS_ConverToByteArray(&tmpByteArray,BS_D,4);

    for (b = 0; b < 4; b++)
        BS_Enew[b] = stCDEF->BS_F[b];

    tmp0 = BS_XOR (stPQXYZ->BS_Z[0], stCDEF->BS_E[0]);
    tmp1 = BS_AND (stPQXYZ->BS_Z[0], stCDEF->BS_E[0]);
    stCDEF->BS_F[0] = BS_XOR (stCDEF->BS_E[0], BS_AND (stPQXYZ->BS_Q, BS_XOR (stPQXYZ->BS_Z[0], stCDEF->BS_C)));
    tmp3 = BS_AND (tmp0, stCDEF->BS_C);
    tmp4 = BS_OR (tmp1, tmp3);

    tmp0 = BS_XOR (stPQXYZ->BS_Z[1], stCDEF->BS_E[1]);
    tmp1 = BS_AND (stPQXYZ->BS_Z[1], stCDEF->BS_E[1]);
    stCDEF->BS_F[1] = BS_XOR (stCDEF->BS_E[1], BS_AND (stPQXYZ->BS_Q, BS_XOR (stPQXYZ->BS_Z[1], tmp4)));
    tmp3 = BS_AND (tmp0, tmp4);
    tmp4 = BS_OR (tmp1, tmp3);

    tmp0 = BS_XOR (stPQXYZ->BS_Z[2], stCDEF->BS_E[2]);
    tmp1 = BS_AND (stPQXYZ->BS_Z[2], stCDEF->BS_E[2]);
    stCDEF->BS_F[2] = BS_XOR (stCDEF->BS_E[2], BS_AND (stPQXYZ->BS_Q, BS_XOR (stPQXYZ->BS_Z[2], tmp4)));
    tmp3 = BS_AND (tmp0, tmp4);
    tmp4 = BS_OR (tmp1, tmp3);

    tmp0 = BS_XOR (stPQXYZ->BS_Z[3], stCDEF->BS_E[3]);
    tmp1 = BS_AND (stPQXYZ->BS_Z[3], stCDEF->BS_E[3]);
    stCDEF->BS_F[3] = BS_XOR (stCDEF->BS_E[3], BS_AND (stPQXYZ->BS_Q, BS_XOR (stPQXYZ->BS_Z[3], tmp4)));
    tmp3 = BS_AND (tmp0, tmp4);
    stCDEF->BS_C = BS_XOR (stCDEF->BS_C, BS_AND (stPQXYZ->BS_Q, BS_XOR (BS_OR (tmp1, tmp3), stCDEF->BS_C)));    // ultimate carry

    for (b = 0; b < 4; b++)
        stCDEF->BS_E[b] = BS_Enew[b];

    aycw__vCaculatePQXYZ(stRegister->A_BS, stPQXYZ);

    /* calc stream bit */
    *BS_Streambit0 = BS_XOR(stCDEF->BS_D[0],  stCDEF->BS_D[1]);
    *BS_Streambit1 = BS_XOR(stCDEF->BS_D[2],  stCDEF->BS_D[3]);

}

/**
set up data used for stream. Depends on scrambled data only, so can be global
@param   data_return[out]     bit sliced output data IB1
@param   outbits[in]          number of bits to calculate
@param   BS_key[in]           bit sliced key array 64 elements
@param   bs_data_sb0[in]      bit sliced 1st data block used for initialization
*/
void aycw_stream_decrypt(dvbcsa_bs_word_t * data_return, unsigned int outbits, dvbcsa_bs_word_t * BS_key, dvbcsa_bs_word_t *bs_data_sb0)
{
   unsigned int      i;
   dvbcsa_bs_word_t    BS_Streambit0, BS_Streambit1;
   aycw_tstRegister stRegister/*=
   {
   {0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,    0,0,0,0/*,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0* /},
   {0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,    0,0,0,0/*,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,0,0* /},
   }*/;
   aycw_tstPQXYZ stPQXYZ =
   {
      0,
      0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0
   };
   aycw_tstCDEF stCDEF =
   {
      0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0
   };
   // aycw__vInitVariables( &stPQXYZ, &stCDEF ,  & BS_Streambit0,   & BS_Streambit1);

   /*  aycw_vInitVariables();*/
   /*  BS_vTransformMultiKey((char *) cw);*/
   /* init A and B - bs */
   aycw__vInitShiftRegister(BS_key, &stRegister);
   /* init A and B - bs */
   for (i = 0; i < 8; i++)
   {
      aycw__vInitRound(0, i, &stPQXYZ, &stCDEF, &stRegister, bs_data_sb0);
      aycw__vInitRound(1, i, &stPQXYZ, &stCDEF, &stRegister, bs_data_sb0);
      aycw__vInitRound(2, i, &stPQXYZ, &stCDEF, &stRegister, bs_data_sb0);
      aycw__vInitRound(3, i, &stPQXYZ, &stCDEF, &stRegister, bs_data_sb0);
   }

   /* OPTIMIZEME: add blokc+stream xor + PES check, to stop bit generation immediately if foreseeable its no PES header */
   for (i = 0; i < outbits; i += 8)
   {
      aycw__vRound(&stPQXYZ, &stCDEF, &stRegister, &BS_Streambit0, &BS_Streambit1);
      data_return[i + 6] = BS_XOR(bs_data_sb0[i + 64 + 6], BS_Streambit0);
      data_return[i + 7] = BS_XOR(bs_data_sb0[i + 64 + 7], BS_Streambit1);
      aycw__vRound(&stPQXYZ, &stCDEF, &stRegister, &BS_Streambit0, &BS_Streambit1);
      data_return[i + 4] = BS_XOR(bs_data_sb0[i + 64 + 4], BS_Streambit0);
      data_return[i + 5] = BS_XOR(bs_data_sb0[i + 64 + 5], BS_Streambit1);
      aycw__vRound(  &stPQXYZ, &stCDEF, &stRegister, &BS_Streambit0, &BS_Streambit1);
      data_return[i + 2] = BS_XOR(bs_data_sb0[i + 64 + 2], BS_Streambit0);
      data_return[i + 3] = BS_XOR(bs_data_sb0[i + 64 + 3], BS_Streambit1);
      aycw__vRound(  &stPQXYZ, &stCDEF, &stRegister, &BS_Streambit0, &BS_Streambit1);
      data_return[i + 0] = BS_XOR(bs_data_sb0[i + 64 + 0], BS_Streambit0);
      data_return[i + 1] = BS_XOR(bs_data_sb0[i + 64 + 1], BS_Streambit1);
   }
}

/**
set up data used for stream. Depends on scrambled data only, so can be global
@param   iv[in]            data packets SB0 SB1
@param   bs_data_sb0[out]  data packets SB0 SB1 bit sliced
*/
void aycw_init_stream(const char *iv, dvbcsa_bs_word_t *bs_data_sb0)
{
    uint8 i,j;

    for (j = 0; j < 16; j++)
    {
        for (i = 0; i < 8; i++)
        {
            bs_data_sb0[i+j*8] = aycw__BitExpandOfByteToBsWord(iv  ,j   ,i);
        }
    }
}


