
void aycw_extractbsdata(dvbcsa_bs_word_t* bs_data, unsigned char slice, unsigned char len, uint8* extracteddata);

void aycw_assert_decrypt_result(unsigned char *probedata, uint8 *keylist, dvbcsa_bs_word_t *bys_data);

void aycw_assert_key_transpose(uint8 *keylist, dvbcsa_bs_word_t *bs_keys);

void aycw_assert_stream(dvbcsa_bs_word_t * data, unsigned int outbits, dvbcsa_bs_word_t * BS_key, dvbcsa_bs_word_t *bs_data_sb0);

int aycw_checkPESheader(dvbcsa_bs_word_t *data, dvbcsa_bs_word_t *candidates);

/* increment key bytes 5 and 6 and calculate checksum in byte 7 */
void aycw_bs_increment_keys_inner(dvbcsa_bs_word_t *keys_bs);

/* check for key candidates in bs data array */
int aycw_checkPESheader(dvbcsa_bs_word_t *, dvbcsa_bs_word_t *);

/* plausibility check if bs keys are correct */
void aycw_assertKeyBatch(dvbcsa_bs_word_t *keys_bs);

/**
transforms array of bit sliced elements into byte sliced array. stripped down version of dvbcsa_bs_stream_transpose_out
@param   data[in/out]      array of bit sliced elements in size: 64*count
@param   count[in]         number of 8 element (64 bit) blocks to rotate
independent from BATCH_SIZE, the first 8 elenemts contain byte0 of all the batches.
Every element contains BATCH_SIZE/8 elements
BATCH_SIZE     batches/element   dvbcsa_bs_word_t[0]           dvbcsa_bs_word_t[7]
------------------------------------------------------------------------------------------
32             4                 batch3/byte0...batch0/byte0   batch31/byte0...batch30/byte0
128            16                batch15/byte0...batch0/byte0  batch127/byte0...batch126/byte0
*/
void aycw_bit2byteslice(dvbcsa_bs_word_t *data, int count);
