/*
	This is needed for our half-to-float conversion code and is
	taken directly from:

	Fast Half Float Conversions by Jeroen van der Zijp
	November 2008 (Revised September 2010)

	To compile:

	gcc generate_half_to_float_tables.cpp
*/

#include <stdio.h>

short basetable[512];
unsigned char shifttable[512];

void generatetables(){
  unsigned int i;
  int e;
  for(i=0; i<256; ++i){
    e=i-127;
    if(e<-24){                  // Very small numbers map to zero
      basetable[i|0x000]=0x0000;
      basetable[i|0x100]=0x8000;
      shifttable[i|0x000]=24;
      shifttable[i|0x100]=24;
    }
    else if(e<-14){             // Small numbers map to denorms
      basetable[i|0x000]=(0x0400>>(-e-14));
      basetable[i|0x100]=(0x0400>>(-e-14)) | 0x8000;
      shifttable[i|0x000]=-e-1;
      shifttable[i|0x100]=-e-1;
    }
    else if(e<=15){             // Normal numbers just lose precision
      basetable[i|0x000]=((e+15)<<10);
      basetable[i|0x100]=((e+15)<<10) | 0x8000;
      shifttable[i|0x000]=13;
      shifttable[i|0x100]=13;
    }
    else if(e<128){             // Large numbers map to Infinity
      basetable[i|0x000]=0x7C00;
      basetable[i|0x100]=0xFC00;
      shifttable[i|0x000]=24;
      shifttable[i|0x100]=24;
    }
    else{                       // Infinity and NaN's stay Infinity and NaN's
      basetable[i|0x000]=0x7C00;
      basetable[i|0x100]=0xFC00;
      shifttable[i|0x000]=13;
      shifttable[i|0x100]=13;
    }
  }
}

int main()
{
	generatetables();

	fprintf( stdout, "unsigned short basetable[512] = {\n" );

	unsigned c = 0;
	for( unsigned i=0; i<64; i++ )
	{
		fprintf( stdout, "\t" );
		for( unsigned j=0; j<8; j++ )
		{
			fprintf( stdout, "0x%04x,", (unsigned short)(basetable[c++]) );
		}
		fprintf( stdout, "\n" );
	}
	fprintf( stdout, "};\n\n" );

	/* */
	fprintf( stdout, "unsigned short shifttable[512] = {\n" );
	c = 0;
	for( unsigned i=0; i<64; i++ )
	{
		fprintf( stdout, "\t" );
		for( unsigned j=0; j<8; j++ )
		{
			fprintf( stdout, "0x%03x,", (unsigned char)(shifttable[c++]) );
		}
		fprintf( stdout, "\n" );
	}
	fprintf( stdout, "};\n" );

	return 0;
}
