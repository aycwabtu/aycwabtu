#ifndef aycw_bs_stream_h
#define aycw_bs_stream_h

#include "config.h"

/****************************************** stream prototypes ********************************/

struct aycw_stPQXYZ
{
   dvbcsa_bs_word_t BS_P;
   dvbcsa_bs_word_t BS_Q;
   dvbcsa_bs_word_t BS_X[4];
   dvbcsa_bs_word_t BS_Y[4];
   dvbcsa_bs_word_t BS_Z[4];
};
typedef struct aycw_stPQXYZ aycw_tstPQXYZ;

struct aycw_stCDEF
{
   dvbcsa_bs_word_t BS_C;
   dvbcsa_bs_word_t BS_D[4];
   dvbcsa_bs_word_t BS_E[4];
   dvbcsa_bs_word_t BS_F[4];
};
typedef struct aycw_stCDEF aycw_tstCDEF;

struct aycw_stRegister
{
   dvbcsa_bs_word_t A_BS[80];
   dvbcsa_bs_word_t B_BS[80];
};
typedef struct aycw_stRegister aycw_tstRegister;

void aycw_stream_key_schedule(const char*, char *);

/* transform regular key array key_in into bitsliced array key_bs_out */
void aycw_key_transpose(const uint8 *key_in, dvbcsa_bs_word_t *key_bs_out);

void aycw_vTransformKey(dvbcsa_bs_word_t * BS_key, char * cw, uint8 keyposition);

void aycw_init_stream(const char *iv, dvbcsa_bs_word_t *bs_data_sb0);

/**
set up data used for stream. Depends on scrambled data only, so can be global
@param   iv[in]            1st data packet SB0
@param   bs_data_sb0[out]  1st data packet SB0 bit sliced
*/
void aycw_stream_decrypt(dvbcsa_bs_word_t * data_return, unsigned int len, dvbcsa_bs_word_t * BS_key, dvbcsa_bs_word_t *bs_data_sb0);


static AYCW_INLINE void aycw__vInitShiftRegister(dvbcsa_bs_word_t * BS_key, aycw_tstRegister * stRegister);

#endif // #ifndef aycw_bs_stream_h
