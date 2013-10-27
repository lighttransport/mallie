/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

/*
	Reference:

	  [1] OpenEXR File Layout
	  http://www.openexr.com/openexrfilelayout.pdf
*/

#ifndef __exrfile_h_
#define __exrfile_h_

#include "ImfCFile.h"
#include "channellist.h"
#include "header.h"
#include "scanlineblock.h"

#include <stdint.h>

class EXRFile
{
public:
	EXRFile(Header* i_header):
		m_file(NULL),
		m_header(i_header),
		m_scanline_block(NULL),
		m_compression(0),
		m_channelList(NULL),
		
		m_fb_sampleCount(NULL),
		m_fb_base(NULL),
		m_fb_xStride(0),
		m_fb_yStride(0),
		m_fb_sampleStride(0),

		m_offset_table(NULL),
		m_blocks(0)
	{
	}
	
	/*
		OpenOutputFile

		Open file for writing. Save some attributes from header and write
		all data except scan line blocks
	*/
	int OpenOutputFile(const char* i_fileName);
	int CloseFile();

	/*
		SetFBData
		
		Save pixel data to temporary buffer
	*/
	int SetFBData(
		const char *i_base,
		size_t i_xStride,
		size_t i_yStride);

	/*
		SetFBDeepData

		Save deep pixel data to a temporary buffer
	*/
	int SetFBDeepData(
		const unsigned int *i_sampleCount,
		const char **i_base,
		size_t i_xStride,
		size_t i_yStride,
		size_t i_sampleStride);

	/*
		WriteOffsets
		
		Convert pixel data from temporary buffer to exr scanline blocks,
		compress and write it to exr file.
	*/
	int WriteFBPixels(int i_numScanLines);
	
private:

	/*
		WriteMagic
		
		Write magic number to file.
		The magic number, of type int, is always 20000630 (decimal).
	*/
	int WriteMagic()const;
	
	/*
		WriteVersion
		
		Write version block to file.
	*/
	int WriteVersion()const;
	
	/*
		WriteHeader
		
		Write header.
	*/
	int WriteHeader()const;

	/*
	   WriteZerroOffsets

	   Write zerros to the offset table
	*/
	int WriteZerroOffsets();

	/*
		WriteOffsets
		
		Write offset table to file
	*/
	int WriteOffsets();
	
	FILE *m_file;
	Header* m_header;
	ScanLineBlock* m_scanline_block;

	int m_dataType;
	
	int m_dataWindow[4];
	unsigned char m_compression;
	
	ChannelList* m_channelList;
	
	const unsigned int *m_fb_sampleCount;
	const char *m_fb_base;
	int m_fb_xStride;
	int m_fb_yStride;
	int m_fb_sampleStride;

	/* Position of offset table in the file */
	uint64_t m_offset_position;
	uint64_t *m_offset_table;

	unsigned int m_offset_table_counter;

	/* Number of blocks in the file */
	unsigned int m_blocks;
};

#endif

