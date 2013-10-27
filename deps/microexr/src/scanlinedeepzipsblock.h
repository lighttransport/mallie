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

#ifndef __scanlinedeepzipsblock_h_
#define __scanlinedeepzipsblock_h_

#include "scanlinedeepblock.h"

/*
 * class ScanLineDeepBlock
 *
 * This class represents scan line deep block with IMF_ZIPS_COMPRESSION
 * compression in exr file
 */

class ScanLineDeepZipSBlock : public ScanLineDeepBlock
{
	public:
		ScanLineDeepZipSBlock(
				FILE *i_file,
				size_t i_sampleOffsetsSize,
				size_t i_pixelSize,
				int i_firstline);

		~ScanLineDeepZipSBlock();

		/*
		 * WriteToFile
		 *
		 * Write stored data to exr file
		 */
		virtual int WriteToFile()const;
};

#endif

