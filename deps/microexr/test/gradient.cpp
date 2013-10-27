#include "ImfCFile.h"

#include <stdlib.h>
#include <assert.h>

#define XRES 1024
#define YRES 512

int main()
{
	const char *software = "microexr";
	const char *comment = "a simple test case for the microexr library";

	ImfHeader *header = ImfNewHeader();

	ImfHeaderSetCompression(header, IMF_ZIP_COMPRESSION );
	ImfHeaderSetStringAttribute( header, "software", software );
	ImfHeaderSetStringAttribute( header, "comment", comment );
	ImfHeaderSetDataWindow( header, 0, 0, XRES-1, YRES-1 );
	ImfHeaderSetDisplayWindow( header, 0, 0, XRES-1, YRES-1 );
	ImfHeaderSetPixelAspectRatio( header, 1.0f );
	ImfHeaderSetScreenWindowCenter(header, 0.0, 0.0);
	ImfHeaderSetScreenWindowWidth(header, 1.0);
	
	ImfHeaderSetLineOrder( header, IMF_INCREASING_Y );
	
	ImfOutputFile *exr = ImfOpenOutputFile( "gradient.exr", header, IMF_WRITE_RGBA );

	if( !exr )
	{
		perror( "gradient: unable to open 'gradient.exr' for writing" );
		exit( 1 );
	}

	/* Write a simple gradient in the framebuffer. */
	ImfHalf *fb = new ImfHalf[XRES*4];

	for( unsigned j=0; j<YRES; j++ )
	{
		float g = float(j) / float(YRES-1);
		assert( g<=1.0f );

		ImfHalf *current_pixel = fb;

		for( unsigned i=0; i<XRES; i++ )
		{
			float r = float(i) / float(XRES-1);
	
			assert( r<=1.0f );

			ImfFloatToHalf( r, current_pixel + 0 );
			ImfFloatToHalf( g, current_pixel + 1 );
			ImfFloatToHalf( 0.5f, current_pixel + 2 );
			ImfFloatToHalf( 1.0f, current_pixel + 3 );

			current_pixel += 4;
		}

		ImfOutputSetFrameBuffer( exr, (const char *)fb, 4*sizeof(ImfHalf), 0 );
		ImfOutputWritePixels( exr, 1 );
	}

	ImfCloseOutputFile( exr );
	ImfDeleteHeader( header );

	delete [] fb;

	exit( 0 );
}
