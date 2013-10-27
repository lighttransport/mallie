/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2012, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

/*
	ImfCFile

	C interface to libexr
*/

#include "ImfCFile.h"
#include "exrfile.h"
#include "half.h"

#include <assert.h>

inline Header* header(ImfHeader* hdr)
{
    return (Header*)(hdr);
}

inline const Header* header(const ImfHeader* hdr)
{
    return (const Header*)(hdr);
}

inline EXRFile* exrfile(ImfOutputFile* out)
{
    return (EXRFile*)(out);
}

inline const EXRFile* exrfile(const ImfOutputFile* out)
{
    return (const EXRFile*)(out);
}

ImfHeader* ImfNewHeader(void)
{
	return (ImfHeader*) new Header;
}

void ImfDeleteHeader (ImfHeader* hdr)
{
	delete header (hdr);
}

int ImfHeaderInsertChannel(
	ImfHeader *hdr,
	const char name[],
	int type)
{
	return header(hdr)->InsertChannel( name, type );
}

void ImfHeaderSetDisplayWindow(
	ImfHeader *hdr,
	int xMin, int yMin,
	int xMax, int yMax)
{
	ImfHeaderSetBox2iAttribute(
		hdr,
		"displayWindow",
		xMin, yMin,
		xMax, yMax);
}

void ImfHeaderSetDataWindow(
	ImfHeader *hdr,
	int xMin, int yMin,
	int xMax, int yMax)
{
	ImfHeaderSetBox2iAttribute(
		hdr,
		"dataWindow",
		xMin, yMin,
		xMax, yMax);
}

void ImfHeaderSetPixelAspectRatio(
	ImfHeader *hdr,
	float pixelAspectRatio)
{
	ImfHeaderSetFloatAttribute(
		hdr,
		"pixelAspectRatio",
		pixelAspectRatio);
}

void ImfHeaderSetScreenWindowCenter(
	ImfHeader *hdr,
	float x, float y)
{
	ImfHeaderSetV2fAttribute(
		hdr,
		"screenWindowCenter",
		x, y);
}

void ImfHeaderSetScreenWindowWidth(
	ImfHeader *hdr,
	float width)
{
	ImfHeaderSetFloatAttribute(
		hdr,
		"screenWindowWidth",
		width);
}

void ImfHeaderSetLineOrder(
	ImfHeader *hdr,
	int lineOrder)
{
	unsigned char attribute_lineOrder = lineOrder;
	
	header(hdr)->SetAttribute(
		"lineOrder",
		"lineOrder",
		sizeof(attribute_lineOrder),
		(char*)&attribute_lineOrder);
}

void ImfHeaderSetCompression(
	ImfHeader *hdr,
	int compression)
{
	/*
		Only IMF_NO_COMPRESSION, IMF_ZIP_COMPRESSION and IMF_ZIPS_COMPRESSION
		are available at the moment (and the foreseeable future). 
	*/
	unsigned char attribute_compression;
	switch( compression )
	{
		case IMF_NO_COMPRESSION:
		case IMF_ZIP_COMPRESSION:
		case IMF_ZIPS_COMPRESSION:
			attribute_compression = compression;
			break;
		default:
			attribute_compression = IMF_ZIP_COMPRESSION;
			break;
	}

	header(hdr)->SetAttribute(
		"compression",
		"compression",
		sizeof(attribute_compression),
		(char*)&attribute_compression);
}

void ImfHeaderSetType (
	ImfHeader *hdr,
	int dataType)
{
	const char* type = "type";
	const char* version = "version";

	if( dataType == IMF_SCANLINEIMAGE )
	{
		ImfHeaderSetStringAttribute( hdr, type, "scanlineimage" );
	}
	else if( dataType == IMF_DEEPSCANLINE )
	{
		ImfHeaderSetStringAttribute( hdr, type, "deepscanline" );
	}

	ImfHeaderSetIntAttribute( hdr, version, 1 );
}

int ImfHeaderSetIntAttribute(
	ImfHeader *hdr,
	const char name[],
	int value)
{
	return header(hdr)->SetAttribute(
		name, "int", sizeof(value), (char*)&value);
}

int ImfHeaderSetFloatAttribute(
	ImfHeader *hdr,
	const char name[],
	float value)
{
	return header(hdr)->SetAttribute(
		name, "float", sizeof(value), (char*)&value);
}

int ImfHeaderSetStringAttribute(
	ImfHeader *hdr,
	const char name[],
	const char value[])
{
	return header(hdr)->SetAttribute(
		name, "string", strlen(value), (char*)value);
}

int ImfHeaderSetBox2iAttribute(
	ImfHeader *hdr,
	const char name[],
	int xMin, int yMin,
	int xMax, int yMax)
{
	int box2i[] = {xMin, yMin, xMax, yMax};
	
	return header(hdr)->SetAttribute(
		name, "box2i", sizeof(box2i), (char*)&box2i[0]);
}

int ImfHeaderSetBox2fAttribute (
	ImfHeader *hdr,
	const char name[],
	float xMin, float yMin,
	float xMax, float yMax)
{
	float box2f[] = {xMin, yMin, xMax, yMax};
	
	return header(hdr)->SetAttribute(
		name, "box2f", sizeof(box2f), (char*)&box2f[0]);
}

int ImfHeaderSetV2iAttribute(
	ImfHeader *hdr,
	const char name[],
	int x, int y)
{
	int v2i[] = {x, y};
	
	return header(hdr)->SetAttribute(
		name, "v2i", sizeof(v2i), (char*)&v2i[0]);
}

int ImfHeaderSetV2fAttribute(
	ImfHeader *hdr,
	const char name[],
	float x, float y)
{
	float v2f[] = {x, y};
	
	return header(hdr)->SetAttribute(
		name, "v2f", sizeof(v2f), (char*)&v2f[0]);
}

int ImfHeaderSetV3iAttribute (
	ImfHeader *hdr,
	const char name[],
	int x, int y, int z)
{
	int v3i[] = {x, y, z};
	
	return header(hdr)->SetAttribute(
		name, "v3i", sizeof(v3i), (char*)&v3i[0]);
}

int ImfHeaderSetV3fAttribute (
	ImfHeader *hdr,
	const char name[],
	float x, float y, float z)
{
	float v3f[] = {x, y, z};
	
	return header(hdr)->SetAttribute(
		name, "v3f", sizeof(v3f), (char*)&v3f[0]);
}

int ImfHeaderSetM44fAttribute (
	ImfHeader *hdr,
	const char name[],
	const float m[4][4])
{
	float m44f[] = {
		m[0][0], m[0][1], m[0][2], m[0][3],
		m[1][0], m[1][1], m[1][2], m[1][3],
		m[2][0], m[2][1], m[2][2], m[2][3],
		m[3][0], m[3][1], m[3][2], m[3][3] };
	
	return header(hdr)->SetAttribute(
		name, "m44f", sizeof(m44f), (char*)&m44f[0]);
}

ImfOutputFile* ImfOpenOutputFile(
	const char name[],
	const ImfHeader *hdr,
	int i_channels)
{
	Header *h = (Header*)header(hdr);

	/*
		Now, if we don't have any channels present in the header we
		add whatever is asked in i_channels.

		Note that for programs built with the standard OpenEXR API,
		there is of course no channels in the header since there are
		no funcitons to add channels in the header.
	*/
	if( h->NumChannels() == 0 )
	{
		unsigned channels[] = {
			IMF_WRITE_R, IMF_WRITE_G, IMF_WRITE_B, IMF_WRITE_A,
			IMF_WRITE_Y, IMF_WRITE_C, IMF_WRITE_Z, IMF_WRITE_ZB };

		const char *names[] = { "R", "G", "B", "A", "Y", "C", "Z", "ZBack" };

		int types[] = {
			IMF_PIXEL_HALF, IMF_PIXEL_HALF, IMF_PIXEL_HALF, IMF_PIXEL_HALF,
			IMF_PIXEL_HALF, IMF_PIXEL_HALF, IMF_PIXEL_FLOAT, IMF_PIXEL_FLOAT };

		assert( sizeof(names)/sizeof(names[0]) == 
			sizeof(channels)/sizeof(channels[0]) );

		for( unsigned i=0; i<sizeof(channels)/sizeof(channels[0]); i++ )
		{
			if( i_channels & channels[i] )
			{
				ImfHeaderInsertChannel(
					(ImfHeader*)hdr, names[i], types[i] );
			}
		}
	}


	EXRFile *file = new EXRFile( h );

	if ( file->OpenOutputFile(name) != IMF_ERROR_NOERROR)
	{
		return NULL;
	}
	
	return (ImfOutputFile*)file;
}

int ImfCloseOutputFile (ImfOutputFile *out)
{
	EXRFile *file = exrfile( out );
	
	file->CloseFile();
	delete file;
	
	return 0;
}

int ImfOutputSetFrameBuffer (
	ImfOutputFile *out,
	const char *base,
	size_t xStride,
	size_t yStride)
{
	return exrfile( out )->SetFBData(base, xStride, yStride);
}

int ImfOutputSetDeepFrameBuffer (
	ImfOutputFile *out,
	const unsigned int *sampleCount,
	const char **data,
	size_t xStride,
	size_t yStride,
	size_t sampleStride)
{
	return exrfile( out )->SetFBDeepData(
			sampleCount, data, xStride, yStride, sampleStride);
}

int ImfOutputWritePixels (
	ImfOutputFile *out,
	int numScanLines)
{
	return exrfile( out )->WriteFBPixels(numScanLines);
}

void ImfFloatToHalf (float f, ImfHalf *h)
{
	short half = FloatToHalf( *(int*)&f );
	h[0] = *(ImfHalf*)&half;
}

