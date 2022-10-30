#include "aycwabtu_config.h"

extern unsigned char bs_tc_crypteddata[16];
extern const unsigned char bs_tc_keys[32*4][8];
extern const unsigned char bs_tc_expstream[32][16];
extern const unsigned char bs_tc_expblock[32][16];
extern const unsigned char bs_tc_expected[32*4][16];

int aycw_check_bs_testcases(dvbcsa_bs_word_t	*r);
