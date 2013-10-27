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

#include "scanlineblock.h"

#include <string.h>

char* ScanLineBlock::AllocateData()const
{
	return (char*)malloc( m_linesize * m_numlines );
}

int ScanLineBlock::StoreNextLine(char* i_data, char* i_reserved)
{
	memmove(m_data, i_data, m_linesize);
	++m_currentLine;
	return IMF_ERROR_NOERROR;
}

int ScanLineBlock::WriteToFile()const
{	
	int datasize = m_linesize;
	int line = m_currentLine + m_firstline - 1;
	
	fwrite(&line, sizeof(line), 1, m_file);
	fwrite(&datasize, sizeof(datasize), 1, m_file);
	fwrite(m_data, 1, m_linesize, m_file);
	
	return IMF_ERROR_NOERROR;
}

