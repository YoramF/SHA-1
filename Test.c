/*
 * main.c
 *
 *  Created on: Aug 21, 2018
 *      Author: yoram
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>


#include "SHA_Functions.h"

int main (int argc, char *argv[])
{
	unsigned long DH[5];
	FILE *fp;
	int len;
	char str[1501];

	if (argc < 2)
	{
		printf("Usage: SHA-1 input_file");
		return 0;
	}

	if ((fp = fopen(argv[1], "r")) == NULL)
	{
		printf("failed to open file (%s), error: %s", argv[1], strerror(errno));
		return 0;
	}

	while (fgets(str, sizeof(str)-1, fp) != NULL)
	{
		len = strlen(str);

		// check if end of string include EOL, if it does take it out
		if (str[len-1] == '\n')
			len--;
		printf("\nInput message (len: %d)\n", len);
		printf("%s\n", str);

		// Call hash function
		if (calc_hash(str, len, DH))
		{
			printf("-----------------------------------------------------------------------\n");
			printf("Message Digest is   %08X %08X %08X %08X %08X\n", DH[0], DH[1], DH[2], DH[3], DH[4]);
		}
	}

	fclose(fp);
	return 1;
}
