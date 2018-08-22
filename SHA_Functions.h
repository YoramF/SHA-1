/*
 * SHA_Functions.h
 *
 *  Created on: Aug 21, 2018
 *      Author: yoram
 */

#ifndef SHA_FUNCTIONS_H_
#define SHA_FUNCTIONS_H_

typedef union _Block
{
	unsigned long W[16];		// 16 words of 32 bits each
	unsigned char bytes[64]; 	// 512 bits
} Block;


// convert 4 bytes unsigned integer from Big Endian to Little Endian and vice versa
unsigned long endianCdw (unsigned long ul);

int calc_hash (char *msg, int m_len, unsigned long DH[]);

#endif /* SHA_FUNCTIONS_H_ */
