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

#include "scanlinedeepblock.h"

#include <assert.h>
#include <string.h>
#include <stdint.h>

ScanLineDeepBlock::ScanLineDeepBlock(
		FILE *i_file,
		size_t i_sampleOffsetsSize,
		size_t i_pixelSize,
		int i_firstline) :

	m_pixelSize(i_pixelSize),
	m_sampleData(NULL),
	m_sampleDataSize(0),
	/* m_data of parent object is used to store sample offset table.
	 * m_linesize is used to store size of sample offset table. */
	ScanLineBlock(i_file, i_sampleOffsetsSize, i_firstline)
{
}

ScanLineDeepBlock::~ScanLineDeepBlock()
{
	if( m_sampleData )
	{
		free( m_sampleData );
	}
}

int ScanLineDeepBlock::StoreNextLine( char* i_sampleOffsets, char* i_data )
{
	assert( i_data );

	memmove(m_data, i_sampleOffsets, m_linesize);

	if( m_sampleData )
	{
		free( m_sampleData );
	}

	/* Get size of current line. Size of current line is the size of one
	 * pixel multiplied by last offset from the sample offset table */
	m_sampleDataSize =
		m_pixelSize *
		((unsigned int*)(m_data))[m_linesize/sizeof(unsigned int)-1];

	m_sampleData = (char*)malloc(m_sampleDataSize);
	memmove(m_sampleData, i_data, m_sampleDataSize);

	++m_currentLine;
	return IMF_ERROR_NOERROR;
}

int ScanLineDeepBlock::WriteToFile()const
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

	uint64_t offsets_size = m_linesize;
	uint64_t data_size = m_sampleDataSize;

	unsigned int line = m_currentLine + m_firstline - 1;

	fwrite(&line, sizeof(line), 1, m_file);
	fwrite(&offsets_size, sizeof(offsets_size), 1, m_file);
	fwrite(&data_size, sizeof(data_size), 1, m_file);
	fwrite(&data_size, sizeof(data_size), 1, m_file);
	fwrite(m_data, 1, m_linesize, m_file);
	fwrite(m_sampleData, 1, m_sampleDataSize, m_file);

	return IMF_ERROR_NOERROR;
}

