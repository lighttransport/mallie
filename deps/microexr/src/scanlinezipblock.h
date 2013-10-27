/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

/*
	class ScanLineZipBlock

	IMF_ZIP_COMPRESSION scan line block in exr file
*/

#ifndef __scanlinezipblock_h_
#define __scanlinezipblock_h_

#include "scanlineblock.h"

class ScanLineZipBlock : public ScanLineBlock
{
public:
	enum { kDefaultBlockSize = 16 };

	ScanLineZipBlock(
		FILE *i_file,
		size_t i_linesize,
		int i_firstline,
		int i_numlines )
	:
		ScanLineBlock(i_file, i_linesize, i_firstline, i_numlines)
	{
	}
	
	~ScanLineZipBlock()
	{
		if (m_currentLine%NumLinesInBlock() != 0)
		{
			WriteCurrentBlockToFile();
		}
	}
	
	/*
		StoreNextLine
		
		Copy data to ScanLineBlock's buffer
	*/
	int StoreNextLine(char* i_data, char* i_reserved=NULL);
	/*
		WriteToFile
		
		If buffer full, write stored data to exr file
	*/
	int WriteToFile()const;
	
private:
	/*
		WriteCurrentBlockToFile
		
		Write stored data to exr file
	*/
	int WriteCurrentBlockToFile()const;
};

#endif

