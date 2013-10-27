/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

/*
	class Header
	
	header of exr file
*/

#ifndef __header_h_
#define __header_h_

#include "ImfCFile.h"
#include "channellist.h"
#include "attribute.h"

#include <vector>

class Header
{
public:
	~Header()
	{
		std::vector<Attribute*>::iterator it;
		for ( it=m_attributeList.begin(); it<m_attributeList.end(); it++ )
		{
			Attribute* attribute = *it;
			delete attribute;
		}
		m_attributeList.clear();
	}
	
	int InsertChannel(
		const char *i_name,
		int i_type,
		unsigned char i_pLinear = 0,
		int i_xSampling = 1,
		int i_ySampling = 1);
		
	int SetAttribute(
		const char* i_name,
		const char* i_type,
		int i_size,
		char* i_value);
	
	int GetAttribute(
		const char* i_name,
		const char* i_type,
		char* i_value)const;
		
	int RemoveAttribute(
		const char* i_name,
		const char* i_type);
		
	int WriteToFile(FILE *i_outputFile)const;

	/*
		Size
		
		Size of header in exr file
	*/
	size_t Size() const;
	
	ChannelList* GetChannelList()
	{
		return &m_channelList;
	}

	unsigned NumChannels()
	{
		return m_channelList.NumChannels();
	}

private:
	ChannelList m_channelList;
	std::vector<Attribute*> m_attributeList;
};

#endif

