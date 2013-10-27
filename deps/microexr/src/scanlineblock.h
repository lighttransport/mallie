/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

/*
	class ScanLineBlock
	
	uncompressed scan line block in exr file
*/

#ifndef __scanlineblock_h_
#define __scanlineblock_h_

#include "ImfCFile.h"

#include <stdlib.h>
#include <stdio.h>

class ScanLineBlock
{
public:
	ScanLineBlock(
		FILE *i_file,
		size_t i_linesize,
		int i_firstline,
		int i_numlines = 1) :
		
		m_file(i_file),
	
		m_linesize(i_linesize),
		m_firstline(i_firstline),
		m_numlines(i_numlines),
		
		m_currentLine(0)
	{
		m_data = AllocateData();
	}
	
	virtual ~ScanLineBlock()
	{
		free(m_data);
	}
	
	/*
		NumLinesInBlock
		
		One or more scan lines are stored together as a scan-line block.
		The number of scan lines per block depends on how the pixel
		data are compressed.
	*/
	virtual int NumLinesInBlock()const
	{
		return m_numlines;
	}
	
	/*
		StoreNextLine
		
		Copy data to ScanLineBlock's buffer

		i_reserved is used for deep data in childs of this class
	*/
	virtual int StoreNextLine(char* i_data, char* i_reserved=NULL);
	/*
		WriteToFile
		
		Write stored data to exr file
	*/
	virtual int WriteToFile()const;
protected:
	/*
		AllocateData
		
		Used to allocate data for ScanLineBlock's buffer
	*/
	char* AllocateData()const;
	
	FILE *m_file;
	
	size_t m_linesize;
	int m_firstline;
	int m_numlines;
	
	int m_currentLine;
	
	char* m_data;
};
#endif

