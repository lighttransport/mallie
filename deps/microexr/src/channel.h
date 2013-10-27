/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#ifndef __channel_h_
#define __channel_h_

/*
	class Channel
	
	Single channel block in exr file
*/

#include "ImfCFile.h"

#include <stdlib.h>
#include <string.h>
#include <vector>

class Channel
{
public:
	Channel(
		const char *i_name,
		int i_type,
		unsigned char i_pLinear,
		int i_xSampling,
		int i_ySampling,
		size_t i_offset = 0):
		
		m_name( strdup(i_name) ),
		m_type( i_type ),
		m_pLinear( i_pLinear ),
		m_xSampling( i_xSampling ),
		m_ySampling( i_ySampling ),
		m_offset( i_offset )
	{
	}
	
	/* Copy Channel */
	Channel(
		const Channel *i_channel):
		
		m_name( strdup(i_channel->m_name) ),
		m_type( i_channel->m_type ),
		m_pLinear( i_channel->m_pLinear ),
		m_xSampling( i_channel->m_xSampling ),
		m_ySampling( i_channel->m_ySampling ),
		m_offset( i_channel->m_offset )
	{
	}
	
	~Channel()
	{
		free(m_name);
	}
	
	/*
		WriteToFile
		
		Writes channel block to i_outputFile
	*/
	int WriteToFile(FILE *i_outputFile)const;
	
	/*
		Size
		
		Size of a channel in exr file
	*/
	size_t Size() const;
	
	/*
		GetPixelSize
		
		Size of Pixel of current channel
	*/
	size_t GetPixelSize()const
	{
		if (m_type == IMF_PIXEL_UINT || m_type == IMF_PIXEL_FLOAT)
		{
			return sizeof(float);
		}

		if (m_type == IMF_PIXEL_HALF)
		{
			return 2;
		}

		return 0;
	}
	
	/*
		GetOffset
		
		Offset of current channel in input data
	*/
	size_t GetOffset()const
	{
		return m_offset;
	}
	
	char* GetName()
	{
		return m_name;
	}
	
	/*
		SetAlphabetOffset
		
		Set offset of current channel in output data
	*/
	int SetAlphabetOffset(size_t i_offset)
	{
		m_alphabetOffset = i_offset;
		return IMF_ERROR_NOERROR;
	}
	
	/*
		GetAlphabetOffset
		
		Offset of current channel in output data
	*/
	size_t GetAlphabetOffset()
	{
		return m_alphabetOffset;
	}
	
private:
	char *m_name;
	int m_type;
	unsigned char m_pLinear;

	int m_xSampling;
	int m_ySampling;

	size_t m_offset;
	size_t m_alphabetOffset;
};

#endif

