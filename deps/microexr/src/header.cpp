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

#include "header.h"

int Header::InsertChannel(
	const char *i_name,
	int i_type,
	unsigned char i_pLinear,
	int i_xSampling,
	int i_ySampling)
{
	m_channelList.InsertChannel(i_name, i_type, 0, 1, 1);
	
	return IMF_ERROR_NOERROR;
}

int Header::SetAttribute(
	const char* i_name,
	const char* i_type,
	int i_size,
	char* i_value)
{
	/* Find and remove attribute with current name and current type */
	RemoveAttribute(i_name, i_type);

	Attribute *attribute = new Attribute(i_name, i_type, i_size, i_value);
	
	m_attributeList.push_back(attribute);
	
	return IMF_ERROR_NOERROR;
}

int Header::GetAttribute(
	const char* i_name,
	const char* i_type,
	char* i_value)const
{
	std::vector<Attribute*>::const_iterator it;
	for ( it=m_attributeList.begin(); it<m_attributeList.end(); it++ )
	{
		Attribute* attribute = *it;
		if (strcmp(attribute->GetName(), i_name) == 0)
		{
			if (strcmp(attribute->GetType(), i_type) == 0)
			{
				memmove(
					i_value,
					attribute->GetData(),
					attribute->GetDataSize());
				return IMF_ERROR_NOERROR;
			}
		}
	}
	
	return IMF_ERROR_ERROR;
}

int Header::RemoveAttribute(
	const char* i_name,
	const char* i_type)
{
	std::vector<Attribute*>::iterator it;
	for ( it=m_attributeList.begin(); it<m_attributeList.end(); it++ )
	{
		Attribute* attribute = *it;
		if (strcmp(attribute->GetName(), i_name) == 0)
		{
			if (strcmp(attribute->GetType(), i_type) == 0)
			{
				delete attribute;
				m_attributeList.erase( it );
				return IMF_ERROR_NOERROR;
			}
		}
	}
	
	return IMF_ERROR_ERROR;
}

int Header::WriteToFile(FILE *i_outputFile)const
{
	m_channelList.WriteToFile(i_outputFile);
	
	std::vector<Attribute*>::const_iterator it;
	for ( it=m_attributeList.begin(); it<m_attributeList.end(); it++ )
	{
		(*it)->WriteToFile(i_outputFile);
	}
	
	fputc('\0', i_outputFile);
	
	return IMF_ERROR_NOERROR;
}

size_t Header::Size()const
{
	size_t size =
		m_channelList.Size();
	
	std::vector<Attribute*>::const_iterator it;
	for ( it=m_attributeList.begin(); it<m_attributeList.end(); it++ )
	{
		size += (*it)->Size();
	}

	size += 1;
	
	return size;
}
