
#ifndef AYCW_UINT32_H_
#define AYCW_UINT32_H_

typedef uint32_t dvbcsa_bs_word_t;

#define BS_BATCH_SIZE 32
#define BS_BATCH_BYTES 4
#define BS_BATCH_SHIFT 5

#define BS_VAL_LSDW(n)	((dvbcsa_bs_word_t)(n))
#define BS_VAL(n)	((dvbcsa_bs_word_t)(n))
#define BS_VAL32(n)	BS_VAL(0x##n)
#define BS_VAL16(n)	BS_VAL32(n##n)
#define BS_VAL8(n)	BS_VAL16(n##n)

#define BS_AND(a, b)	((a) & (b))
#define BS_OR(a, b)	((a) | (b))
#define BS_XOR(a, b)	((a) ^ (b))
#define BS_XOREQ(a, b)	((a) ^= (b))
#define BS_NOT(a)	(~(a))

#define BS_SHL(a, n)	((a) << (n))
#define BS_SHR(a, n)	((a) >> (n))
#define BS_SHL8(a, n)	((a) << (8 * (n)))
#define BS_SHR8(a, n)	((a) >> (8 * (n)))
#define BS_EXTRACT8(a, n) ((a) >> (8 * (n)))
#define BS_EXTLS32(a)      (a)

#define BS_EMPTY()

#endif

