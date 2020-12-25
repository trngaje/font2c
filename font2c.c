/* 이 코드는 https://unix.stackexchange.com/questions/119236/bitmap-font-data-as-array-in-c/134167 에서 임의로 수정했음.
 * retroarch rgui 용 font 생성 코드입니다.
 */

#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype2/freetype/ftbitmap.h>

#include "options.h"
// 폰트 1글자에 대한 비트맵 사이즈
#if 0
#define WIDTH   8
#define BYTEWIDTH (WIDTH)/8
#define HEIGHT  8
#define BUFFERSIZE (WIDTH * HEIGHT / 8)
#else
//#define WIDTH   5
#define WIDTH 10
//#define BYTEWIDTH (WIDTH)/10
#define HEIGHT  10	
#define BUFFERSIZE ((WIDTH * HEIGHT + 7) / 8)
#endif

// 폰트를 array로 변환할때 임시로 변환하는 모노 비트맵 저장소
//static unsigned char image[HEIGHT][BYTEWIDTH];
static unsigned char image[BUFFERSIZE/*HEIGHT*WIDTH/8*/];

static FT_Library library;
static FT_Face face;
static FT_Error err;
static FT_Bitmap tempbitmap;


int g;

// 8의 배수 해상도에서는 적당하나 10x10 폰트에 대해서는 부적당함
static void to_bitmap( FT_Bitmap*  bitmap, FT_Int x, FT_Int y, FT_Int bitmap_left, FT_Int bitmap_top) {

    FT_Int  i, j, p, q;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows;

	int alignoffset=0;
	int alignoffsetx=0;
	
    //printf("[trngaje] width=%d, rows=%d, pitch=%d\n",  bitmap->width, bitmap->rows, bitmap->pitch);
#if 0
	if (bitmap->rows<10)
	{
		//printf("[trngaje]  width=%d, rows=%d, pitch=%d\n",  bitmap->width, bitmap->rows, bitmap->pitch);
		alignoffset = (10 - bitmap->rows); /* /2; */
		if (alignoffset < 1)
			alignoffset = 1;
	}
	
	if (g == 0x3002) 
		alignoffset--; 


	if (bitmap->width<10)
	{
		alignoffsetx = (10 - bitmap->width) /2; 
	}
#else
	
	if (g_opt.opt_no_align == 1)
	{
		alignoffsetx = 0;
		alignoffset = 0;
	}
	else
	{
		alignoffsetx = bitmap_left;
		if (bitmap_top<=10 && bitmap_top>=0)
			alignoffset = 10 - bitmap_top;
		else
			alignoffset = 0;
	}

#endif	
    for ( i = x, p = 0; i < x_max; i++, p++ ) {
        for ( j = y, q = 0; j < y_max; j++, q++ ) {
            if ( (i < 0) || (j < 0) || (i >= WIDTH || j >= HEIGHT) )
                continue;
			// 8개의 pixel 을 8bit (i >> 3) 단위로 정리
            //image[j][i >> 3] |= (bitmap->buffer[q * bitmap->width + p]) << (i & 7);
#if 1			
			if ((i+alignoffsetx >= WIDTH) || (j+alignoffset >= HEIGHT)) // boundary check
				continue;
			unsigned char rem = 1 << ((i+alignoffsetx + (j+alignoffset) * WIDTH) & 7);
			unsigned char offset  = (i+alignoffsetx + (j+alignoffset) * WIDTH) >> 3;
			if (bitmap->buffer[q * bitmap->width + p])
				image[offset] |= rem;	
#else			
			unsigned char rem = 1 << ((i+bitmap_left + (j+bitmap_top) * WIDTH) & 7);
			unsigned char offset  = (i+bitmap_left + (j+bitmap_top) * WIDTH) >> 3;
			if (bitmap->buffer[q * bitmap->width + p])
				image[offset] |= rem;				
#endif			
        }
    }
}

static void draw_glyph(unsigned short glyph, int *x, int *y) {
    FT_UInt  glyph_index;
    FT_GlyphSlot  slot = face->glyph;

	// glyph : unicode, glyph_index : 폰트내에 저장된 index(?), 존재하지 않으면 0이 return 됨
    glyph_index = FT_Get_Char_Index( face, glyph );
	
    if (glyph_index == 0)
	return;

    if ((err = FT_Load_Glyph( face, glyph_index, /*1*/ FT_LOAD_DEFAULT /*FT_LOAD_NO_SCALE | FT_LOAD_CROP_BITMAP | FT_LOAD_MONOCHROME*/))) {
        fprintf( stderr, "warning: failed FT_Load_Glyph 0x%x %d\n", glyph, err);
        return;
    }

    if ((err = FT_Render_Glyph( face->glyph, /*FT_RENDER_MODE_NORMAL*/FT_RENDER_MODE_MONO ))) {
        fprintf( stderr, "warning: failed FT_Render_Glyph 0x%x %d\n", glyph, err);
        return;
    }

    FT_Bitmap_New(&tempbitmap);
    FT_Bitmap_Convert( library, &slot->bitmap, &tempbitmap, 1);

	//printf("[trngaje] left=%d, top=%d\n", slot->bitmap_left, slot->bitmap_top);
    to_bitmap( &tempbitmap, *x, *y, slot->bitmap_left, slot->bitmap_top );

    FT_Bitmap_Done( library, &tempbitmap );

    //*x += slot->advance.x >> 6;
}

int main(int argc, char **argv) {
    char *filename;
    int x = 0, y = 0;
    //int g;

    memset (image, 0, BUFFERSIZE/*BYTEWIDTH*HEIGHT*/);

    if (argc < 2) {
        fprintf( stderr, "usage: font2c [font]\n");
        exit(1);
    }

    int c;
    int option_index = 0;

	while ((c = getopt_long(argc, argv, "i:o:t:W:H:s:e:KJCREnvh", longopts, &option_index)) != -1)
	{
		switch (c)
		{
			case 'i': g_opt.strInputName = optarg;
				break;

			case 'o': g_opt.strOutputName = optarg;
				break;

			case 't' : if (strcmp(optarg, "c") == 0)
						   g_opt.iOutputFontType = 1; // c header
					   else 
						   g_opt.iOutputFontType = 0; // bin
				break;
			case 'W' : g_opt.iInputFontWidth = atoi(optarg);
				break;
			case 'H' : g_opt.iInputFontHeight = atoi(optarg);
				break;
			case 's' : g_opt.iUnicode_start = (int)strtol(optarg, NULL, 0); // hex 값
				break;
			case 'e' : g_opt.iUnicode_end = (int)strtol(optarg, NULL, 0); // hex 값
				break;
			case 'K' : // kor : 0xac00 ~ 0xd7a3
				g_opt.iUnicode_start = 0xac00;
				g_opt.iUnicode_end = 0xd7a3;
				break;
			case 'J' : // CJK Symbols and Punctuation 0x3000 ~ 0x303f
						// jpn 0x3040 ~ 0x30ff
				g_opt.iUnicode_start = 0x3000;
				g_opt.iUnicode_end = 0x30ff;						
				break;
			case 'C' : // chn: 0x4e00 ~ 0x9fff
				g_opt.iUnicode_start = 0x4e00;
				g_opt.iUnicode_end = 0x9fff;			
				break;
			case 'R' : // rus: 0x400 ~ 0x45f
				g_opt.iUnicode_start = 0x400;
				g_opt.iUnicode_end = 0x45f;			
				break;
			case 'E' : // eng: 0 ~ 0xff
				g_opt.iUnicode_start = 0;
				g_opt.iUnicode_end = 0xff;			
				break;
			case 'n' : g_opt.opt_no_align = 1;
				break;
			case 'v' : g_opt.opt_verbose = 1;
				break;
			case 'h' : 
				break;
	
			default:
				printf("Unknown option. '%s'\n", longopts[option_index].name);
                exit(EXIT_FAILURE);
		}
	}

	printf("[trngaje] input=%s, output=%s, type=%d\n", g_opt.strInputName, g_opt.strOutputName, g_opt.iOutputFontType);
	printf("[trngaje] W=%d, H=%d, start=0x%x, end=0x%x\n", g_opt.iInputFontWidth, g_opt.iInputFontHeight, g_opt.iUnicode_start, g_opt.iUnicode_end);
	printf("[trngaje] no_align=%d, verbose=%d\n", g_opt.opt_no_align, g_opt.opt_verbose);
	
	// boundary check
	if (g_opt.iUnicode_start == 0 && g_opt.iUnicode_end == 0)
	{
		// kor : 0xac00 ~ 0xd7a3
		g_opt.iUnicode_start = 0xac00;
		g_opt.iUnicode_end = 0xd7a3;		
	}
	
	if (g_opt.iInputFontWidth == 0 || g_opt.iInputFontHeight == 0)
	{
		// default : 10x10
		g_opt.iInputFontWidth = 10;
		g_opt.iInputFontHeight = 10;
	}	
	
	if (g_opt.strInputName == NULL)
		exit(1);
	
    //filename = argv[1];
	filename = g_opt.strInputName;
	
    if ((err = FT_Init_FreeType( &library ))) {
        fprintf( stderr, "error: Init_Freetype failed %d\n", err);
        exit(1);
    }
	
    if ((err = FT_New_Face( library, filename, 0, &face ))) {
        fprintf( stderr, "error: FT_New_Face failed %d\n", err);
        exit(1);
    }

	err = FT_Set_Pixel_Sizes(
	  face,   /* handle to face object */
	  g_opt.iInputFontWidth,      /* pixel_width           */
	  g_opt.iInputFontHeight );

#if 0
	// 한국어 폰트
	// for Galmuri11.ttf, 10x10
	err = FT_Set_Pixel_Sizes(
          face,   /* handle to face object */
          14,      /* pixel_width           */
          14 ); 
#endif

#if 0
	// 한국어 폰트 -> 영어
	// for Galmuri11.ttf, 10x10
	err = FT_Set_Pixel_Sizes(
          face,   /* handle to face object */
          14,      /* pixel_width           */
          14 ); 
#endif

#if 0
	// 한국어 폰트 -> 영어
	// for Galmuri11.ttf, 8x8
	err = FT_Set_Pixel_Sizes(
          face,   /* handle to face object */
          20,      /* pixel_width           */
          16 ); 
#endif

#if 0
        // 영어 폰트
        // for Pixel12x10.ttf, 12x10
        err = FT_Set_Pixel_Sizes(
          face,   /* handle to face object */
          14,      /* pixel_width           */
          14 );
#endif


#if 0
	// 일본어 폰트
	// PixelMplus10-Regular.ttf, 10x10
	err = FT_Set_Pixel_Sizes(
          face,   /* handle to face object */
          10,      /* pixel_width           */
          10 ); 
#endif

#if 0
        // 러시아어 폰트
        // basis33.ttf, 5x10
        err = FT_Set_Pixel_Sizes(
          face,   /* handle to face object */
          12,      /* pixel_width           */
          12 );

#endif

#if 0
	// 중국어 폰트
	// for ARIALUNI.TTF, 10x10
	err = FT_Set_Pixel_Sizes(
          face,   /* handle to face object */
          12,      /* pixel_width           */
          12 ); 
#endif


#if 0
        // 러시아어 폰트
        // for ARIALUNI.TTF, 10x10
        err = FT_Set_Pixel_Sizes(
          face,   /* handle to face object */
          10,      /* pixel_width           */
          10 );
#endif

#if 0
	// 중국어 폰트
	// for msjh.ttc, 10x10
	err = FT_Set_Pixel_Sizes(
          face,   /* handle to face object */
          11,      /* pixel_width           */
          12 ); 
#endif

#if 0
    if (FT_HAS_COLOR(face))
       printf("[trngaje] FT_HAS_COLOR is true\n");
    else
       printf("[trngaje] FT_HAS_COLOR is false\n");
#endif
    if (FT_HAS_FIXED_SIZES(face))
    {
       //printf("[trngaje] FT_HAS_FIXED_SIZES is true\n");


       if ((err = FT_Select_Size(face, 0)) != 0)
       {
       }
    }
    else
    {
        //printf("[trngaje] FT_HAS_FIXED_SIZES is false\n");
/*
  FT_Set_Char_Size( FT_Face     face,
                    FT_F26Dot6  char_width,
                    FT_F26Dot6  char_height,
                    FT_UInt     horz_resolution,
                    FT_UInt     vert_resolution );
*/				
	    //if (err = FT_Set_Char_Size(face, 10, 10, 480, 320))
        //{
        //}
        //if ((err = FT_Set_Char_Size(face, face->available_sizes[0].width, face->available_sizes[0].height, Utils.window.Exten>
        //{
        //}
    }

#if 0
	중국어 폰트
	// Fiery_Turk.ttf : 10x10 
	err = FT_Set_Pixel_Sizes(
          face,   /* handle to face object */
          12,      /* pixel_width           */
          12 ); 
#endif

    //printf("static const unsigned char bitmap_chn_bin[] =\n");
	//printf("{\n");


	FILE *pFile_Output = NULL;
	
	if (g_opt.iOutputFontType == 0)	// bin
	{
		pFile_Output = fopen(g_opt.strOutputName, "wb");
	}
	else // c header
	{
		pFile_Output = fopen(g_opt.strOutputName, "w");
		fprintf(pFile_Output, "/* FontName = %s */ \n", g_opt.strInputName);	
		fprintf(pFile_Output, "/* generated by trngaje */ \n");		
		fprintf(pFile_Output, "\nstatic const unsigned char bitmap_bin[] =\n{");		
	}

	


    int i=0;	
	
	for (g = g_opt.iUnicode_start; g <= g_opt.iUnicode_end; g++) 
	{
		memset (image, 0, BUFFERSIZE/*BYTEWIDTH*HEIGHT*/);
        draw_glyph(g, &x, &y);

		if (g_opt.iOutputFontType == 0)	// bin
		{
			if (pFile_Output != NULL)
				fwrite(image, 1, BUFFERSIZE, pFile_Output);
		}
		else
		{
			if (i%16 == 0)
				fprintf(pFile_Output, "\n");
			//if (i%2 == 0)
				fprintf(pFile_Output, "\n\t");

			//out_xbm(BYTEWIDTH, HEIGHT);
			for (int y=0; y < BUFFERSIZE; y++) 
			{
				fprintf(pFile_Output, "0x%02X,", image[y]);
			}	
			
			fprintf(pFile_Output, "\t // 0x%x", g); 			
		}
		
		i++;
		
		if (g_opt.opt_verbose == 1)
		{
			// display font
			printf("\n");
			for (int jj = 0; jj < HEIGHT; jj++)
			{
				for (int ii = 0; ii < WIDTH; ii++)
				{
					unsigned char rem = 1 << ((ii + jj * WIDTH) & 7);
					unsigned char offset  = (ii + jj * WIDTH) >> 3;
					if ((image[offset] & rem) > 0)
						printf("#");
					else
						printf(" ");
				}
				printf("\n");
			}		
		}


    }

	if (g_opt.iOutputFontType != 0)	// c header
		fprintf(pFile_Output, "\n};\n");
	
    //out_xbm(BYTEWIDTH, HEIGHT);
	if (pFile_Output != NULL)
		fclose(pFile_Output);	
	
    FT_Done_Face( face );
    FT_Done_FreeType( library );

    return 0;

}
