/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#ifndef __ImfCFile_h_
#define __ImfCFile_h_

/*
	ImfCFile

	This file describes the C interface to the EXR file format.
	
	This is the standard EXR C API but we added one additional entry
	that let us describe files with multiple channels: 

		ImfHeaderInsertChannel		
*/

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
	Interpreting unsigned shorts as 16-bit floating point numbers
*/

typedef unsigned short ImfHalf;

void ImfFloatToHalf (float f, ImfHalf *h);

/*
	RGBA pixel; memory layout must be the same as struct Imf::Rgba.
*/

struct ImfRgba
{
    ImfHalf	r;
    ImfHalf	g;
    ImfHalf	b;
    ImfHalf	a;
};

typedef struct ImfRgba ImfRgba;

/*
	The different write modes.
*/
#define IMF_WRITE_R 0x01
#define IMF_WRITE_G 0x02
#define IMF_WRITE_B 0x04
#define IMF_WRITE_A 0x08
#define IMF_WRITE_Y 0x10
#define IMF_WRITE_C 0x20
#define IMF_WRITE_Z 0x40
#define IMF_WRITE_ZB 0x80
#define IMF_WRITE_RGB 0x07
#define IMF_WRITE_RGBA 0x0f
#define IMF_WRITE_RGBAZZB 0xcf
#define IMF_WRITE_YC 0x30
#define IMF_WRITE_YA 0x18
#define IMF_WRITE_YCA 0x38


/*
	Magic number
*/

#define IMF_MAGIC 20000630

/*
	Version number
*/

#define IMF_VERSION_NUMBER 2

/*
	Line order
*/

#define IMF_INCREASING_Y 0
#define IMF_DECREASING_Y 1
#define IMF_RAMDOM_Y 2

/*
	Compression types
*/

#define IMF_NO_COMPRESSION 0
#define IMF_RLE_COMPRESSION 1
#define IMF_ZIPS_COMPRESSION 2
#define IMF_ZIP_COMPRESSION 3
#define IMF_PIZ_COMPRESSION	4
#define IMF_PXR24_COMPRESSION 5
#define IMF_B44_COMPRESSION 6
#define IMF_B44A_COMPRESSION 7

/*
	Data type
*/
#define IMF_SCANLINEIMAGE 0
#define IMF_DEEPSCANLINE 2

/*
	Pixel types
*/

#define IMF_PIXEL_UINT 0
#define IMF_PIXEL_HALF 1
#define IMF_PIXEL_FLOAT 2

/*
	Error codes
*/

#define IMF_ERROR_NOERROR 0
#define IMF_ERROR_ERROR	1
#define IMF_ERROR_NOT_SUPPORTED	2

/*
	File header
*/

struct ImfHeader;
typedef struct ImfHeader ImfHeader;

ImfHeader* ImfNewHeader(void);

void ImfDeleteHeader (ImfHeader *hdr);

int ImfHeaderInsertChannel(
	ImfHeader *hdr,
	const char name[],
	int type);

void ImfHeaderSetDisplayWindow (
	ImfHeader *hdr,
	int xMin, int yMin,
	int xMax, int yMax);

void ImfHeaderSetDataWindow (
	ImfHeader *hdr,
	int xMin, int yMin,
	int xMax, int yMax);

void ImfHeaderSetPixelAspectRatio (
	ImfHeader *hdr,
	float pixelAspectRatio);

void ImfHeaderSetScreenWindowCenter (
	ImfHeader *hdr,
	float x, float y);

void ImfHeaderSetScreenWindowWidth (
	ImfHeader *hdr,
	float width);

void ImfHeaderSetLineOrder ( 
	ImfHeader *hdr,
	int lineOrder);

/*
	ImfHeaderSetCompression
	
	Sets compression attribute to header

	Only IMF_NO_COMPRESSION and IMF_ZIP_COMPRESSION are available at the moment 
*/
void ImfHeaderSetCompression (
	ImfHeader *hdr,
	int compression);

void ImfHeaderSetType (
	ImfHeader *hdr,
	int dataType);

int ImfHeaderSetIntAttribute (
	ImfHeader *hdr,
	const char name[],
	int value);

int ImfHeaderSetFloatAttribute (
	ImfHeader *hdr,
	const char name[],
	float value);

int ImfHeaderSetStringAttribute (
	ImfHeader *hdr,
	const char name[],
	const char value[]);

int ImfHeaderSetBox2iAttribute (
	ImfHeader *hdr,
	const char name[],
	int xMin, int yMin,
	int xMax, int yMax);

int ImfHeaderSetBox2fAttribute (
	ImfHeader *hdr,
	const char name[],
	float xMin, float yMin,
	float xMax, float yMax);

int ImfHeaderSetV2iAttribute (
	ImfHeader *hdr,
	const char name[],
	int x, int y);

int ImfHeaderSetV2fAttribute (
	ImfHeader *hdr,
	const char name[],
	float x, float y);

int ImfHeaderSetV3iAttribute (
	ImfHeader *hdr,
	const char name[],
	int x, int y, int z);

int ImfHeaderSetV3fAttribute (
	ImfHeader *hdr,
	const char name[],
	float x, float y, float z);

int ImfHeaderSetM44fAttribute (
	ImfHeader *hdr,
	const char name[],
	const float m[4][4]);

/*
	Output functions.
*/

struct ImfOutputFile;
typedef struct ImfOutputFile ImfOutputFile;

ImfOutputFile* ImfOpenOutputFile (
	const char name[],
	const ImfHeader *hdr,
	int channels = 0);

int ImfCloseOutputFile (ImfOutputFile *out);

/*
	ImfOutputSetFrameBuffer

	Define a frame buffer as the pixel data source:
	Pixel (x, y) is at address
	base + x * xStride + y * yStride
*/
int ImfOutputSetFrameBuffer (
	ImfOutputFile *out,
	const char *base,
	size_t xStride,
	size_t yStride);

/*
	ImfOutputSetDeepFrameBuffer

	Define a deep frame buffer as a pixel data source:
	- sampleCount is a table with number of samples for each pixel
	- data is a table with a pointer to samples.
	  Pointer to samples of n-th channel of pixel (x, y) is at address
	  data + x * xStride + y * yStride + n*sampleStride
*/
int ImfOutputSetDeepFrameBuffer (
	ImfOutputFile *out,
	const unsigned int *sampleCount,
	const char **data,
	size_t xStride,
	size_t yStride,
	size_t sampleStride);

int ImfOutputWritePixels (
	ImfOutputFile *out,
	int numScanLines);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

