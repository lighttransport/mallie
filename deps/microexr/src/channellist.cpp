/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#include "channellist.h"

#include <string>
#include <algorithm>

int ChannelList::InsertChannel(
	const char *i_name,
	int i_type,
	unsigned char i_pLinear,
	int i_xSampling,
	int i_ySampling)
{
	Channel *channel =
		new Channel(
			i_name,
			i_type,
			i_pLinear,
			i_xSampling,
			i_ySampling,
			GetPixelSize() );
	
	m_channelList.push_back(channel);
	
	return IMF_ERROR_NOERROR;
}

/*
	ChannelList::WriteToFile
	
	Writes channel list block to i_outputFile according [1]:
	- size: int, channels data size
	- channels: in a row
	- zerro
*/
int ChannelList::WriteToFile(FILE *i_outputFile)const
{
	fputs("channels", i_outputFile);
	fputc('\0', i_outputFile);
	fputs("chlist", i_outputFile);
	fputc('\0', i_outputFile);
	
	int size = Size() - 16/* channels, chlist words */ - sizeof(size);
	fwrite(&size, sizeof(size), 1, i_outputFile);
	
	std::vector<Channel*>::const_iterator it;
	for ( it=m_channelList.begin(); it<m_channelList.end(); it++ )
	{
		(*it)->WriteToFile(i_outputFile);
	}
	
	fputc('\0', i_outputFile);
	
	return IMF_ERROR_NOERROR;
}

size_t ChannelList::Size()const
{
	int size = 
		sizeof(size) +
		16 + /* channels, chlist words */
		1; /* zerro */
	
	std::vector<Channel*>::const_iterator it;
	for ( it=m_channelList.begin(); it<m_channelList.end(); it++ )
	{
		size += (*it)->Size();
	}
	
	return size;
}

size_t ChannelList::GetPixelSize()const
{
	if (m_channelList.size() == 0)
	{
		return 0;
	}
	
	std::vector<Channel*>::const_iterator it;
	it = --m_channelList.end();
	
	/* offset plus pixel size of last item */
	return (*it)->GetOffset() + (*it)->GetPixelSize();
}

/*
	ChannelList::Sort
	
	Calculate offset for output data for all channels
*/
int ChannelList::Sort()
{
	/* name and position in current array */
	std::vector< std::pair<std::string, int> > stringarray;

	int i=0;
	std::vector<Channel*>::const_iterator ch_it;
	for (ch_it = m_channelList.begin(); ch_it<m_channelList.end(); ++ch_it)
	{
		stringarray.push_back(std::pair<std::string, int>(
			(*ch_it)->GetName(), i));
		i++;
	}

	std::sort(stringarray.begin(), stringarray.end());

	/* re-calculate offset with new order */
	size_t offset = 0;
	std::vector< std::pair<std::string, int> >::iterator it;
	for (it = stringarray.begin(); it<stringarray.end(); ++it)
	{
		Channel *channel = m_channelList[(*it).second];
		channel->SetAlphabetOffset( offset );
		
		offset += channel->GetPixelSize();
	}
	
	return IMF_ERROR_NOERROR;
}

