/*
 * SHA_Functions.c
 *
 *  Created on: Aug 20, 2018
 *      Author: yoram
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

// Key values are already set for little endian - no need to convert!
unsigned long key [80] = {
		0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999,
		0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999,
		0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1,
		0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1,
		0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC,
		0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC,
		0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6,
		0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6
	};

// H values are already set for little endian - no need to convert!
unsigned long H[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};

typedef union _Block
{
	unsigned long W[16];		// 16 words of 32 bits each
	unsigned char bytes[64]; 	// 512 bits
} Block;

// function called for iterations 1-20
unsigned long f0_19 (unsigned long B, unsigned long C, unsigned long D)
{
	return ((B & C) | ((~B) & D));
}

// function called for iterations 21-40
unsigned long f20_39 (unsigned long B, unsigned long C, unsigned long D)
{
	return B ^ C ^ D ;
}

// function called for iterations 41-60
unsigned long f40_59 (unsigned long B, unsigned long C, unsigned long D)
{
	return (B & C) | (B & D) | (C & D);
}

// function called for iterations 61-80
unsigned long f60_79 (unsigned long B, unsigned long C, unsigned long D)
{
	return B ^ C ^ D;
}

// select the right iteration_function based on iteration
unsigned long f (int t, unsigned long B, unsigned long C, unsigned long D)
{
	if (t >= 0 && t < 20)
		return f0_19(B, C, D);
	else if (t >= 20 && t < 40)
		return f20_39(B, C, D);
	else if (t >= 40 && t < 60)
		return f40_59(B, C, D);
	else
		return f60_79(B, C, D);
}

// convert 8 bytes unsigned integer from little Endian to Big Endian and vise versa
unsigned long long endianCqw (unsigned long long ull)
{
	union _ull2c
	{
		unsigned long long ull;
		unsigned char c[8];
	} ull2c1, ull2c2;

	int i;

	ull2c1.ull = ull;
	for(i=0; i<8; i++)
		ull2c2.c[i] = ull2c1.c[7-i];

	return ull2c2.ull;
}

// convert 4 bytes unsigned integer from Big Endian to Little Endian and vice versa
unsigned long endianCdw (unsigned long ul)
{
	union _ul2c
	{
		unsigned long ul;
		unsigned char c[4];
	} ul2c1, ul2c2;

	int i;

	ul2c1.ul = ul;
	for (i=0; i<4; i++)
		ul2c2.c[i] = ul2c1.c[3-i];

	return ul2c2.ul;
}

// Left rotate 4 bytes word
unsigned long rotateLdw(unsigned long num, int bits)
{
  return ((num << bits) | (num >> (32 -bits)));
}

// Right rotate 4 bytes word
unsigned long rotateRdw(unsigned long num, int bits)
{
  return ((num >> bits) | (num << (32 -bits)));
}

// Generate the 512bit blocks for processing
// Input parameters:
//	String message and its length
// Output parameter:
//	Pointer to first block
// Return valus: number of blocks
int prepare_blocks (char *message, int m_len, Block **bptr)
{
	int bc = m_len+9;	// we need to allocate one byte for end of message + 8 bytes for original message length
	int r;
	Block *bp;
	unsigned char *sp;
	int mi, tb;

	r = bc % 64;			// check if there is a reminder
	bc >>= 6;				// devide by 64 bytes (512 bits)
	bc += (r > 0 ? 1 : 0);	// if there is a reminder than add one more block which will be padded with Zeros

	tb = bc * sizeof(Block);

	// allocate blocks
	if (((*bptr) = malloc(tb)) == NULL)
	{
		printf("prepare_blocks: failed to allocate memory, %s\n", strerror(errno));
		return 0;
	}
	else
	{
		bp = *bptr;
		sp = (unsigned char *)bp;

		// copy message into blocks
		// to make things simple we treat all blocks as one long array of chars
		for (mi = 0; mi<m_len; mi++)
			sp[mi] = message[mi];

		// add 0x80 byte at the end of the message
		sp[mi++] = 0x80;

		// pad the rest of the block with Zeros up to last 8 bytes - leave last 8 bytes for origin message length in quadword
		tb -= 8;
		while (mi < tb)
			sp[mi++] = 0x0;

		// insert last quadword with origin message length in bits to last block
		// all words and qwords in input blocks are assumed to use BIG Endian format.
		// therefore, we must convert message length to Big Endian
 		*(unsigned long long *)&sp[mi] = endianCqw((unsigned long long) m_len*8);

		return bc;
	}
}

// main SAH-1 function.
// input parameters:
//		Messge - string
//		Message length - int
// output parameters:
//		DH - Digested Message (5 WORDs)
// Return value - True/False
int calc_hash (char *msg, int m_len, unsigned long DH[])
{
	int i, j, t;
	int bc;
	Block *bptr;
	unsigned long A, B, C, D, E, TEMP;
	unsigned long W[80];

	// Generate message blocks out of input messgae
	if ((bc = prepare_blocks(msg, m_len, &bptr)) == 0)
		return 0; //  we had memory allocate error - return FALSE
	else
	{
		// Convert the input blocks from BIG Endian to Little Endian to match X86 architecture
		printf("Input message processed to blocks:\n");
		printf("----------------------------------\n");
		for (i=0; i<bc; i++)
			for (j=0; j<16; j++)
			{
				bptr[i].W[j] = endianCdw(bptr[i].W[j]);
				printf("B[%2d] W[%2d] %08X\n",i, j, bptr[i].W[j]);
			}

		// inint DH with initial values
		for (i=0; i<5; i++)
				DH[i] = H[i];


		// Start the iterations on all blocks and calculate the HAS
		printf("\nCalculating Hash ========\n");
		printf("                  A        B        C        D        E\n");
		for (i=0; i<bc; i++)
		{

			// initialze the W array with the first 16 words from current block
			for (j=0; j<16; j++)
				W[j] = bptr[i].W[j];

			// fill the rest of the 80 words buffer
			// the algorithm uses Bit ROTATE operation
			for (t=16; t<80; t++)
				W[t] = rotateLdw((W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]),1);


			A = DH[0];
			B = DH[1];
			C = DH[2];
			D = DH[3];
			E = DH[4];

			for (t=0; t<80; t++)
			{
				TEMP = f(t, B, C, D);
				TEMP = rotateLdw(A,5) + f(t, B, C, D) + E + W[t] + key[t];
				E = D;
				D = C;
				C = rotateLdw(B,30);
				B = A;
				A = TEMP;

				// printout current iteration state
				printf("B[%2d] t[%2d]    %08X %08X %08X %08X %08X\n", i, t, A, B, C, D, E);
			}

			DH[0] += A;
			DH[1] += B;
			DH[2] += C;
			DH[3] += D;
			DH[4] += E;

		}

		free(bptr);	// relese the allocated RAM
		return 1;	// the Digested 5 WORDs are DH[0-4], return TRUE
	}
}

