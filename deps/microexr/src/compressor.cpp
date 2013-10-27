/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

/*
	compressor

	Performs zlib-style compression
*/
#include "compressor.h"

#include <stdlib.h>
#include <zlib.h>

/*
	CompressData

	This tries to re-order pixel data so that it compresses better.
	Unfortunately, this is a total failure for floating point data.
*/
int CompressData( char *inPtr, int inSize, char *outPtr)
{
	char *_tmpBuffer = (char*) malloc( inSize );

	char *t1 = _tmpBuffer;
	char *t2 = _tmpBuffer + (inSize + 1) / 2;
	const char *stop = inPtr + inSize;

	while (true)
	{
		if (inPtr < stop)
			*(t1++) = *(inPtr++);
		else
			break;

		if (inPtr < stop)
			*(t2++) = *(inPtr++);
		else
			break;
	}

	{
		unsigned char *t = (unsigned char *) _tmpBuffer + 1;
		unsigned char *stop = (unsigned char *) _tmpBuffer + inSize;
		int p = t[-1];

		while (t < stop)
		{
			int d = int (t[0]) - p + (128 + 256);
			p = t[0];
			t[0] = d;
			++t;
		}
	}

	uLongf outSize = inSize;
	int cstatus = ::compress(
			(Bytef *)outPtr, &outSize, (const Bytef *) _tmpBuffer, inSize);

	free( _tmpBuffer );

	return Z_OK == cstatus ? outSize : 0;
}
