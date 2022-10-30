
#include <string.h>

#include "aycwabtu_config.h"

void AYCW_INLINE aycw_block_key_perm(dvbcsa_bs_word_t* in, dvbcsa_bs_word_t* out);
void aycw_block_key_schedule(const dvbcsa_bs_word_t* keys, dvbcsa_bs_word_t* kk);
void aycw_init_block(void);
void aycw_block_decrypt(const dvbcsa_bs_word_t* keys, dvbcsa_bs_word_t* r);
int aycw_checkPESheader(dvbcsa_bs_word_t *data, dvbcsa_bs_word_t *candidates);

