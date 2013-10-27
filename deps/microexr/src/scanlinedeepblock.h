/******************************************************************************/
/*                                                                            */
/*    Copyright (c)The 3Delight Developers. 2013                              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************
 * = LIBRARY
 *     MicroEXR
 * = AUTHOR(S)
 *     Victor Yudin
 * = VERSION
 *     $Revision$
 * = DATE RELEASED
 *     $Date$
 * = RCSID
 *     $Id$
 ******************************************************************************/

#ifndef __scanlinedeepblock_h_
#define __scanlinedeepblock_h_

#include "scanlineblock.h"

/*
 * class ScanLineDeepBlock
 *
 * This class represents uncompressed scan line deep block in exr file
 */

class ScanLineDeepBlock : public ScanLineBlock
{
	public:
		ScanLineDeepBlock(
				FILE *i_file,
				size_t i_sampleOffsetsSize,
				size_t i_pixelSize,
				int i_firstline);

		~ScanLineDeepBlock();

		/*
		 * StoreNextLine
		 *
		 * Copy data to ScanLineBlock's buffer
		 */
		virtual int StoreNextLine( char* i_sampleOffsets, char* i_data );

		/*
		 * WriteToFile
		 *
		 * Write stored data to exr file
		 */
		virtual int WriteToFile()const;

	protected:

		/* Full size of a single pixel of all channels */
		size_t m_pixelSize;

		/* All samples that already is in correct order */
		char* m_sampleData;
		size_t m_sampleDataSize;
};

#endif

