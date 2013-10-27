/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

/*
	class Channel
	
	Single channel block in exr file
*/

#include "channel.h"

/*
	Channel::WriteToFile
	
	Writes channel block to i_outputFile according [1]:
	
	Channel layout:
	- name: zero-terminated string, from 1 to 31 bytes long
	- pixel type: int, possible values are UINT = 0, HALF = 1, FLOAT = 2
	- pLinear: unsigned char, possible values are 0 and 1
	- reserved: three chars, should be zero
	- xSampling: int
	- ySampling: int
*/
int Channel::WriteToFile(FILE *i_outputFile)const
{
	fputs(m_name, i_outputFile);
	fputc('\0', i_outputFile);
	fwrite(&m_type, sizeof(m_type), 1, i_outputFile);
	fwrite(&m_pLinear, sizeof(m_pLinear), 1, i_outputFile);
	fputc('\0', i_outputFile);
	fputc('\0', i_outputFile);
	fputc('\0', i_outputFile);
	fwrite(&m_xSampling, sizeof(m_xSampling), 1, i_outputFile);
	fwrite(&m_ySampling, sizeof(m_ySampling), 1, i_outputFile);
	
	return IMF_ERROR_NOERROR;
}

/*
	Channel::Size
	
	Returns size of channel's data in bytes
*/
size_t Channel::Size()const
{
	int size = 
		strlen(m_name) + 1 +
		sizeof(m_type) +
		sizeof(m_pLinear) +
		3 +
		sizeof(m_xSampling) +
		sizeof(m_ySampling);
		
	return size;
}

