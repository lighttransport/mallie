/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#include "scanlinezipblock.h"
#include "compressor.h"

#include <string.h>

int ScanLineZipBlock::StoreNextLine(char* i_data, char* i_reserved)
{
	/* number of line in block */
	int line = m_currentLine % NumLinesInBlock();
	char* data_dst = m_data + m_linesize*line;
	
	memmove(data_dst, i_data, m_linesize);
	
	++m_currentLine;
	return IMF_ERROR_NOERROR;
}

int ScanLineZipBlock::WriteToFile() const
{
	if (m_currentLine%NumLinesInBlock() == 0)
	{
		return WriteCurrentBlockToFile();
	}
	
	return IMF_ERROR_NOERROR;
}

int ScanLineZipBlock::WriteCurrentBlockToFile()const
{
	/* first line of block */
	int line =
		( (m_currentLine-1)/NumLinesInBlock() ) * NumLinesInBlock();
	
	/* number of lines in current block to write */
	int numlines = m_currentLine - line;
	int datasize = m_linesize * numlines;
	
	/* first line of block in file */
	line += m_firstline;
	
	char *data_compressed = (char*) malloc( datasize );
	int datasize_compressed =
		CompressData(m_data, datasize, data_compressed);

	fwrite(&line, sizeof(line), 1, m_file);
	fwrite(&datasize_compressed, sizeof(datasize_compressed), 1, m_file);
	fwrite(data_compressed, 1, datasize_compressed, m_file);
	
	free(data_compressed);

	return IMF_ERROR_NOERROR;
}

