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

#include "scanlinedeepzipsblock.h"
#include "compressor.h"

#include <string.h>
#include <stdint.h>

ScanLineDeepZipSBlock::ScanLineDeepZipSBlock(
		FILE *i_file,
		size_t i_sampleOffsetsSize,
		size_t i_pixelSize,
		int i_firstline) :

	ScanLineDeepBlock(i_file, i_sampleOffsetsSize, i_pixelSize, i_firstline)
{
}

ScanLineDeepZipSBlock::~ScanLineDeepZipSBlock()
{
}

int ScanLineDeepZipSBlock::WriteToFile()const
{
	/* Each chunk of deep scan line data is a single scan line of data.
	 * The data in each chunk is laid out as follows:
	 *
	 * [part number] (if multipart bit is set) (currently is not supported))
	 * y coordinate
	 * packed size of pixel offset table
	 * packed size of sample data
	 * unpacked size of sample data
	 * compressed pixel offset table
	 * compressed sample data
	 */

	unsigned int line = m_currentLine + m_firstline - 1;

	/* Compress offsets table */
	char *offsets_compressed = (char*) malloc( m_linesize );
	uint64_t offsets_size =
		CompressData(m_data, m_linesize, offsets_compressed);

	/* Compress data */
	uint64_t data_size = m_sampleDataSize;
	char *data_compressed = (char*) malloc( data_size );
	uint64_t data_compressed_size =
		CompressData(m_sampleData, data_size, data_compressed);

	fwrite(&line, sizeof(line), 1, m_file);
	fwrite(&offsets_size, sizeof(offsets_size), 1, m_file);
	fwrite(&data_compressed_size, sizeof(data_compressed_size), 1, m_file);
	fwrite(&data_size, sizeof(data_size), 1, m_file);
	fwrite(offsets_compressed, 1, offsets_size, m_file);
	fwrite(data_compressed, 1, data_compressed_size, m_file);

	free(offsets_compressed);
	free(data_compressed);

	return IMF_ERROR_NOERROR;
}

