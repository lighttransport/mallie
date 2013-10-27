/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#include <assert.h>

#include "exrfile.h"
#include "scanlinezipblock.h"
#include "scanlinedeepblock.h"
#include "scanlinedeepzipsblock.h"


int EXRFile::OpenOutputFile(const char* i_fileName)
{
	m_file = fopen(i_fileName, "wb");
	
	if (!m_file)
	{
		return IMF_ERROR_ERROR;
	}
	
	m_channelList = new ChannelList(*m_header->GetChannelList());
	m_channelList->Sort();
	
	m_header->GetAttribute("dataWindow", "box2i", (char*)&m_dataWindow[0]);
	m_header->GetAttribute("compression", "compression", (char*)&m_compression);

	/* TODO: Get and use length of the attribute */
	char type[256];
	const char* deepscanline = "deepscanline";
	m_header->GetAttribute("type", "string", type);
	if (strncmp(type, deepscanline, strlen(deepscanline)) == 0)
	{
		m_dataType = IMF_DEEPSCANLINE;
	}
	else
	{
		m_dataType = IMF_SCANLINEIMAGE;
	}

	int width = m_dataWindow[2] - m_dataWindow[0] + 1;
	size_t datasize = m_channelList->GetPixelSize() * width;

	if( m_dataType == IMF_DEEPSCANLINE )
	{
		if (m_compression == IMF_ZIPS_COMPRESSION)
		{
			m_scanline_block =
				new ScanLineDeepZipSBlock(
						m_file,
						width * sizeof(m_fb_sampleCount[0]),
						m_channelList->GetPixelSize(),
						m_dataWindow[1]);
		}
		else
		{
			m_scanline_block =
				new ScanLineDeepBlock(
						m_file,
						width * sizeof(m_fb_sampleCount[0]),
						m_channelList->GetPixelSize(),
						m_dataWindow[1]);
		}
	}
	else
	switch( m_compression )
	{
		case IMF_ZIP_COMPRESSION:
			m_scanline_block =
				new ScanLineZipBlock(
					m_file,
					datasize,
					m_dataWindow[1],
					ScanLineZipBlock::kDefaultBlockSize);
			break;
		case IMF_ZIPS_COMPRESSION:
			m_scanline_block =
				new ScanLineZipBlock(
					m_file,
					datasize,
					m_dataWindow[1],
					1);
			break;
		default:
			m_scanline_block =
				new ScanLineBlock(
					m_file,
					datasize,
					m_dataWindow[1]);
	}

	int height = m_dataWindow[3] - m_dataWindow[1] + 1;

	m_blocks =  height / m_scanline_block->NumLinesInBlock();
	m_blocks += height % m_scanline_block->NumLinesInBlock() > 0;

	m_offset_table = new uint64_t [m_blocks];
	m_offset_table_counter = 0;
	
	WriteMagic();
	WriteVersion();
	WriteHeader();
	WriteZerroOffsets();

	return IMF_ERROR_NOERROR;
}

int EXRFile::CloseFile()
{
	delete m_channelList;
	m_channelList = 0x0;

	delete m_scanline_block;
	m_scanline_block = 0x0;

	WriteOffsets();

	delete [] m_offset_table;
	m_offset_table = 0x0;

	fclose(m_file);
	m_file = 0x0;
	
	return IMF_ERROR_NOERROR;
}

int EXRFile::WriteMagic()const
{
	/*
		According to [1]:
		The magic number, of type int, is always 20000630 (decimal).
		It allows file readers to distinguish OpenEXR files from other files,
		since the first four bytes of an OpenEXR file are always
		0x76, 0x2f, 0x31 and 0x01.
	*/
	char magic[] = {0x76, 0x2f, 0x31, 0x01};
	
	fwrite(magic, 1, 4, m_file);
	
	return IMF_ERROR_NOERROR;
}

int EXRFile::WriteVersion()const
{
	/*
		According [1]:
		The version field, of type int, is treated as two separate bit fields.
		The 8 least significant bits (bits 0 through 7) contain the file format
		version number. The 24 most significant bits (8 through 31) are treated
		as a set of boolean flags.
	*/
	char version[] = {0x02, 0x00, 0x00, 0x00};
	
	if( m_dataType == IMF_DEEPSCANLINE )
	{
		/* Set a bit that indicates whether the file contains
		 * any “non-image parts” (deep data).*/
		version[1] |= 0x08;
	}

	fwrite(version, 1, 4, m_file);
	
	return IMF_ERROR_NOERROR;
}

int EXRFile::WriteHeader()const
{
	m_header->WriteToFile(m_file);

	return IMF_ERROR_NOERROR;
}

int EXRFile::WriteZerroOffsets()
{
	/*
		According to [1]:
		The line offset table allows random access to scan line blocks.
		The table is a sequence of scan line offsets, with one offset per
		scan line block. A scan line offset, of type unsigned long, indicates
		the distance, in bytes, between the start of the file and the start
		of the scan line block.

		So start by saving the current position and store some 0s as a temporary
		place holder: we will be writing the good offsets at file close time.
	*/

	m_offset_position = ftell(m_file);
	fwrite(m_offset_table, sizeof(m_offset_table[0]), m_blocks, m_file);

	return IMF_ERROR_NOERROR;
}


/*
	see comments in WriteZerroOffsets.
*/
int EXRFile::WriteOffsets()
{
	/*
		Move current position to start of offset table.
		NOTE: casting to 'long' here makes sure that we are respecting the
		fseek signature. On windows this means that we won't support files
		that are larger than 4gigs because a long on windows is 32 bits.
	 */
	long position = m_offset_position;
	int ret = fseek( m_file, position, SEEK_SET );

	if( ret == -1 )
	{
		return IMF_ERROR_ERROR;
	}

	/* Write the entire offset table at once. */
	fwrite(
		m_offset_table, sizeof(m_offset_table[0]), m_blocks, m_file );

	return IMF_ERROR_NOERROR;
}

int EXRFile::SetFBData(
	const char *i_base,
	size_t i_xStride,
	size_t i_yStride)
{
	assert( m_dataType != IMF_DEEPSCANLINE );

	m_fb_sampleCount = NULL;
	m_fb_base = i_base;
	m_fb_xStride = i_xStride;
	m_fb_yStride = i_yStride;
	m_fb_sampleStride = 0;

	return IMF_ERROR_NOERROR;
}

int EXRFile::SetFBDeepData(
	const unsigned int *i_sampleCount,
	const char **i_base,
	size_t i_xStride,
	size_t i_yStride,
	size_t i_sampleStride)
{
	assert( m_dataType == IMF_DEEPSCANLINE );

	m_fb_sampleCount = i_sampleCount;
	m_fb_base = (const char *)i_base;
	m_fb_xStride = i_xStride;
	m_fb_yStride = i_yStride;
	m_fb_sampleStride = i_sampleStride;

	return IMF_ERROR_NOERROR;
}

int EXRFile::WriteFBPixels(int i_numScanLines)
{
	if( m_dataType == IMF_DEEPSCANLINE )
	{
		assert( m_fb_sampleCount );
		assert( m_fb_base );

		/*
		   Sort data in right order and write to file

		   input data:
			table with number of samples for each pixel is in m_fb_sampleCount
			pixel data source is m_fb_base,
			Pointer to samples of n-th channel of pixel (x, y) is at address
			data + x * xStride + y * yStride + n*sampleStride

			in following order
			r1=>(s1,s2,s3) g1=>(s1,s2,s3) b1=>(s1,s2,s3) a1=>(s1,s2,s3)
			r2=>(s1) g2=>(s1) b2=>(s1) a2=>(s1) .. rn gn bn an

		   output data:
			r1s1 r1s2 r1s3 r2s1 .. rnsn
			g1s1 g1s2 d1s3 g2s1 .. gnsn
			b1s1 b1s2 b1s3 b2s1 .. bnsn
			a1s1 a1s2 a1s3 a2s1 .. ansn
		*/
		int width = m_dataWindow[2] - m_dataWindow[0] + 1;

		for (int i=0; i<i_numScanLines; ++i)
		{
			unsigned int *sample_offsets =
				(unsigned int *)malloc( width * sizeof(sample_offsets[0]) );

			int nsamples = 0;
			for(int j=0; j<width; j++)
			{
				nsamples += m_fb_sampleCount[i*width+j];
				sample_offsets[j] = nsamples;
			}

			/* Size of one line */
			int datasize = m_channelList->GetPixelSize() * nsamples;
			char *data = (char*) malloc( datasize );

			/* Reorder data */
			for (int n=0; n<m_channelList->NumChannels(); ++n)
			{
				size_t pixel_size = m_channelList->GetChannelPixelSize(n);
				size_t channel_alphabet_offset =
					m_channelList->GetChannelAlphabetOffset(n);

				char* destanation = 
					data +
					channel_alphabet_offset * nsamples;

				for (int j=0; j<width; j++)
				{
					const char* source =
						m_fb_base +
						i * m_fb_yStride +
						j * m_fb_xStride +
						m_dataWindow[0] * m_fb_xStride +
						n * m_fb_sampleStride;

					for( unsigned s=0; s<m_fb_sampleCount[i*width+j]; s++ )
					{
						const char* sample = *(const char**)source + s*pixel_size;

						memcpy(destanation, sample, pixel_size);
						destanation += pixel_size;
					}
				}
			}

			uint64_t pos = ftell(m_file);
			unsigned int k =
				m_offset_table_counter++/m_scanline_block->NumLinesInBlock();

			m_offset_table[k] = pos;

			m_scanline_block->StoreNextLine((char*)sample_offsets, data);
			m_scanline_block->WriteToFile();

			free(sample_offsets);
			free(data);
		}
		return IMF_ERROR_NOERROR;
	}
	// else

	/*
		Sort data in right order and write to file
		
		input data:
		 pixel data source is m_fb_base,
		 Pixel (x, y) is at address
		 base + x * xStride + y * yStride
		 
		 in following order
		 r1 g1 b1 a1 r2 g2 b2 a2 .. rn gn bn an
		
		output data:
		 r1 r2 .. rn g1 g2 .. gn b1 b2 .. bn a1 a2 .. an
	*/
	int width = m_dataWindow[2] - m_dataWindow[0] + 1;
	
	for (int i=0; i<i_numScanLines; ++i)
	{
		int datasize = m_channelList->GetPixelSize() * width;
		char *data = (char*) malloc( datasize );
		
		/* Reorder data */
		for (int j=0; j<width; j++)
		{
			for (int n=0; n<m_channelList->NumChannels(); ++n)
			{
				size_t pixel_size = m_channelList->GetChannelPixelSize(n);
				size_t channel_offset = m_channelList->GetChannelOffset(n);
				size_t channel_alphabet_offset =
					m_channelList->GetChannelAlphabetOffset(n);
				
				char* destanation = 
					data +
					channel_alphabet_offset * width +
					pixel_size * j;
				
				const char* source =
					m_fb_base +
					i * m_fb_yStride +
					j * m_fb_xStride +
					m_dataWindow[0] * m_fb_xStride +
					channel_offset;
				
				memcpy(destanation, source, pixel_size);
			}
		}

		/* Save position of current block to offset table */
		uint64_t  pos = ftell(m_file);
		unsigned int k =
			m_offset_table_counter++/m_scanline_block->NumLinesInBlock();

		m_offset_table[k] = pos;

		m_scanline_block->StoreNextLine(data);
		m_scanline_block->WriteToFile();
		
		free(data);
	}

	return IMF_ERROR_NOERROR;
}

