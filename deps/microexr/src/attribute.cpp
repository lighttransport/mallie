/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2012, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

/*
	class Attribute

	Single attribute block in an exr file
*/

#include "attribute.h"

int Attribute::WriteToFile(FILE *i_outputFile)const
{
	fputs(m_name, i_outputFile);
	fputc('\0', i_outputFile);
	fputs(m_type, i_outputFile);
	fputc('\0', i_outputFile);
	fwrite(&m_size, sizeof(m_size), 1, i_outputFile);
	fwrite(m_value, 1, m_size, i_outputFile);
	
	return IMF_ERROR_NOERROR;
}

size_t Attribute::Size() const
{
	return
		strlen(m_name) + 1 +
		strlen(m_type) + 1 +
		sizeof(m_size) +
		m_size;
}

