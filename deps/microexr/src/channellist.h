/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#ifndef __channellist_h_
#define __channellist_h_

#include "channel.h"

/*
	class ChannelList
	
	ChannelList block in exr file
*/
class ChannelList
{
public:
	ChannelList()
	{
	}

	/* Copy ChannelList */
	ChannelList( const ChannelList &i_channelList)
	{
		std::vector<Channel*>::const_iterator it;
		for(it = i_channelList.m_channelList.begin();
			it < i_channelList.m_channelList.end();
			it++ )
		{
			Channel* channel = new Channel(*it);
			m_channelList.push_back(channel);
		}
	}

	~ChannelList()
	{
		std::vector<Channel*>::iterator it;
		for ( it=m_channelList.begin(); it<m_channelList.end(); it++ )
		{
			Channel* channel = *it;
			delete channel;
		}
		m_channelList.clear();
	}

	int InsertChannel(
		const char *i_name,
		int i_type,
		unsigned char i_pLinear,
		int i_xSampling,
		int i_ySampling);
	
	/*
		WriteToFile
		
		Writes all channels to i_outputFile
	*/
	int WriteToFile(FILE *i_outputFile)const;
	
	/*
		Size
		
		Size of data block of ChannelList in exr file including all channels
	*/
	size_t Size() const;
	
	int NumChannels() const
	{
		return m_channelList.size();
	}
	
	size_t GetChannelOffset(int i)const
	{
		return m_channelList[i]->GetOffset();
	}
	
	size_t GetChannelAlphabetOffset(int i)const
	{
		return m_channelList[i]->GetAlphabetOffset();
	}
	
	size_t GetChannelPixelSize(int i)const
	{
		return m_channelList[i]->GetPixelSize();
	}
	
	/*
		GetPixelSize
		
		Sum of PixelSize of all channels
	*/
	size_t GetPixelSize()const;
	
	/*
		Sort
		
		Calculate m_alphabetOffset offset for output data for all channels
	*/
	int Sort();
	
private:
	std::vector<Channel*> m_channelList;
};

#endif

