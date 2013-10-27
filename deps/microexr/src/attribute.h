/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#ifndef __attribute_h_
#define __attribute_h_

#include "ImfCFile.h"

#include <string.h>
#include <stdlib.h>

/*
	class Attribute
	
	Single attribute block in exr file
*/
class Attribute
{
public:
	Attribute(
		const char* i_name,
		const char* i_type,
		int i_size,
		char* i_value):
		
		m_name( strdup(i_name) ),
		m_type( strdup(i_type) ),
		m_size( i_size )
	{
		m_value = (char*) malloc(m_size);
		memcpy(m_value, i_value, m_size);
	}
	
	~Attribute()
	{
		free(m_name);
		free(m_type);
		free(m_value);
	}
	
	const char* GetName() const
	{
		return m_name;
	}
	
	const char* GetType()const
	{
		return m_type;
	}
	
	int GetDataSize()const
	{
		return m_size;
	}
	
	const char* GetData()const
	{
		return m_value;
	}
	
	/*
		WriteToFile
		
		Writes attribute block to i_outputFile
	*/
	int WriteToFile(FILE *i_outputFile)const;
	
	/*
		Size
		
		Size of data block of attribute in exr file
	*/
	size_t Size() const;
	
private:
	char* m_name;
	char* m_type;
	int m_size;
	char* m_value;
};


#endif

