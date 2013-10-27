#include "ImfCFile.h"

#include <stdlib.h>
#include <assert.h>
#include <math.h>


#define XRES 1024
#define YRES 512

int main()
{
	const char *software = "microexr";
	const char *comment = "a simple test case for the microexr library";

	ImfHeader *header = ImfNewHeader();

	ImfHeaderSetType(header, IMF_DEEPSCANLINE);

	ImfHeaderSetCompression(header, IMF_ZIPS_COMPRESSION );
	ImfHeaderSetStringAttribute( header, "software", software );
	ImfHeaderSetStringAttribute( header, "comment", comment );
	ImfHeaderSetDataWindow( header, 0, 0, XRES-1, YRES-1 );
	ImfHeaderSetDisplayWindow( header, 0, 0, XRES-1, YRES-1 );
	ImfHeaderSetPixelAspectRatio( header, 1.0f );
	ImfHeaderSetScreenWindowCenter(header, 0.0, 0.0);
	ImfHeaderSetScreenWindowWidth(header, 1.0);

	ImfHeaderSetLineOrder( header, IMF_INCREASING_Y );

	ImfOutputFile *exr =
		ImfOpenOutputFile( "deepgradient.exr", header, IMF_WRITE_RGBAZZB );

	if( !exr )
	{
		perror( "gradient: unable to open 'gradient.exr' for writing" );
		exit( 1 );
	}

	/* Write a simple gradient in a framebuffer. */
	void **fb = new void*[XRES*6];
	/* Fast access to half and float samples of the framebuffer */
	ImfHalf **half_fb = (ImfHalf **)fb;
	float **float_fb = (float **)fb;

	unsigned int *samples = new unsigned int[XRES];

	for(unsigned i=0; i<XRES; i++)
	{
		samples[i]=1;
		fb[i*6 + 0] = new ImfHalf[samples[i]];
		fb[i*6 + 1] = new ImfHalf[samples[i]];
		fb[i*6 + 2] = new ImfHalf[samples[i]];
		fb[i*6 + 3] = new ImfHalf[samples[i]];
		fb[i*6 + 4] = new float[samples[i]];
		fb[i*6 + 5] = new float[samples[i]];
	}

	for( unsigned j=0; j<YRES; j++ )
	{
		float g = float(j) / float(YRES-1);
		assert( g<=1.0f );

		for( unsigned i=0; i<XRES; i++ )
		{
			float r = float(i) / float(XRES-1);

			for(unsigned s=0; s<samples[i]; s++)
			{
				ImfFloatToHalf(  r,  half_fb[i*6 + 0]+s );
				ImfFloatToHalf(  g,  half_fb[i*6 + 1]+s );
				ImfFloatToHalf(0.5f, half_fb[i*6 + 2]+s );
				ImfFloatToHalf(0.9f, half_fb[i*6 + 3]+s );

				float z = (g-0.5)*(g-0.5) + (r-0.5)*(r-0.5);
				z = 10*sqrt(z);
				z = 10-z;

				float_fb[i*6 + 4][s] = z;
				float_fb[i*6 + 5][s] = z+0.01;
			}
		}

		ImfOutputSetDeepFrameBuffer(
				exr, samples, (const char **)fb, 6*sizeof(fb), 0, sizeof(fb) );
		ImfOutputWritePixels( exr, 1 );
	}

	ImfCloseOutputFile( exr );
	ImfDeleteHeader( header );

	delete [] samples;
	for(unsigned i=0; i<XRES; i++)
	{
		delete half_fb[i*6 + 0];
		delete half_fb[i*6 + 1];
		delete half_fb[i*6 + 2];
		delete half_fb[i*6 + 3];
		delete float_fb[i*6 + 4];
		delete float_fb[i*6 + 5];
	}
	delete [] fb;

	exit( 0 );
}
