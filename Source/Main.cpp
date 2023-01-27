#include <Windows.h>
#include <cassert>
#include <string.h>

#undef __STDC_LIB_EXT1__
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "External/stb_image_write.h"
#include "FileIO.cpp"
#define STB_IMAGE_IMPLEMENTATION
#include "External/stb_image.h"
#define PACK_TIGHTLY 1

static const int MAGIC = *(int*)"edgy";

static void DoFont(char* FontName);
static void DoPng(char* FilePath);
static char* GetFileNameFromPath(char* Path);

struct glyph_render_info
{
    int X0, Y0, X1, Y1;
    float UVX0, UVY0, UVX1, UVY1;
    float TopGap, LeftGap;
    int CellWidth;
    int CellHeight;
};

struct kerning_pair
{
    unsigned short First, Second;
    int KernAmount;
};

struct edgy_font_file
{
    int Magic;
    int Height;
    int TextureWidth;
    int TextureHeight;
    int GlyphCount;
    int PairCount;
    int GlyphInfoStartOffset;
    int ImageStartOffset;
    int KerningPairsOffset;
    int SpaceWidth;
    glyph_render_info* GlyphInfo;
    unsigned int* Pixels;
    kerning_pair* Pairs;
};

struct edgy_image_file
{
	int Magic;
	int Width;
	int Height;
	unsigned char* Pixels;
};

int
main(int Argc, char** Argv)
{
    if(Argc >= 3 && strcmp(Argv[1], "-font") == 0)
    {
        // NOTE: at this point we assume that 3rd argument is filepath
		char* FontName = Argv[2];
        printf("%s\n", FontName);
		if (strlen(FontName) > 996)
		{
			printf("Too big font name: %s\n", FontName);
			return -1;
		}
		DoFont(FontName);
    }
	else if(Argc >= 3 && strcmp(Argv[1], "-png") == 0)
	{
		char* FileName = Argv[2];
		if (strlen(FileName) > 996)
		{
			printf("Too big file name: %s\n", FileName);
			return -1;
		}
        DoPng(FileName);   
	}
    else
    {
        printf("Wrong usage!\nExample:\nprogram.exe -font \"[fontname]\"\nprogram.exe -png \"[filename]\"");
        printf("Path can not contain \\(backslash)");
        return -1;
    }

    return 0;
}

static void
DoFont(char* FontName)
{
    HFONT Font = CreateFontA(-72, 0, 0, 0,
                             FW_REGULAR,
                             FALSE,
                             FALSE,
                             FALSE,
                             ANSI_CHARSET,
                             OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS,
                             ANTIALIASED_QUALITY,
                             FF_DONTCARE,
                             FontName);
    assert(Font != NULL);

    HDC DeviceContext = CreateCompatibleDC(GetDC(0));
    assert(DeviceContext != NULL);
    HFONT DefaultFont = (HFONT)SelectObject(DeviceContext, Font);

    int BitmapWidth  = 2048;
    int BitmapHeight = 256;

    BITMAPINFO BitmapInfo = {};
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    unsigned int* BitmapMem = NULL;
    HBITMAP Bitmap = CreateDIBSection(DeviceContext, &BitmapInfo, DIB_RGB_COLORS, (void**)&BitmapMem, NULL, 0);
    assert(Bitmap != NULL);
    HGDIOBJ OldBitmap = SelectObject(DeviceContext, Bitmap);
    SetTextColor(DeviceContext, RGB(255, 255, 255));
    SetBkMode(DeviceContext, TRANSPARENT);
    SetBkColor(DeviceContext, RGB(0, 0, 0));

    TEXTMETRIC Metrics = {};
    GetTextMetrics(DeviceContext, &Metrics);

    int PairCount = GetKerningPairsA(DeviceContext, 100000, NULL);
    kerning_pair* Pairs = NULL;
    if (PairCount > 0)
    {
        Pairs = (kerning_pair*)malloc(PairCount * sizeof(kerning_pair));
        GetKerningPairsA(DeviceContext, 100000, (KERNINGPAIR*)Pairs);
    }

    glyph_render_info* Glyphs = (glyph_render_info*)malloc(94 * sizeof(glyph_render_info));
    int CursorX = 0, CursorY = 0;
    for (int I = 0; I < 94; ++I)
    {
        char Char = (char)(I + 33);
        SIZE Size = {};
        GetTextExtentPoint32A(DeviceContext, &Char, 1, &Size);
        TextOutA(DeviceContext, CursorX, CursorY, &Char, 1);
        // Calculate glyph bounding box
        int X0 = 10000, Y0 = 10000, X1 = -10000, Y1 = -10000;
        for (int Y = CursorY; Y < CursorY + Size.cy + 1; ++Y)
        {
            // NOTE: Size.cx + 10 is because size.cx is not really the size in pixels?
            for (int X = CursorX; X < CursorX + Size.cx + 10; ++X)
            {
                unsigned int Pixel = BitmapMem[Y * BitmapWidth + X];
                if (Pixel != 0)
                {
                    if (X < X0)
                    {
                        X0 = X;
                    }
                    if (X > X1)
                    {
                        X1 = X;
                    }
                    if (Y < Y0)
                    {
                        Y0 = Y;
                    }
                    if (Y > Y1)
                    {
                        Y1 = Y;
                    }
                }
            }
        }
        int BWidth = X1 - X0 + 1;
        int BHeight = Y1 - Y0 + 1;
        float TopGap = (float)(Y0 - CursorY);
        float LeftGap = (float)(X0 - CursorX);
#if PACK_TIGHTLY
        int Advance = X1 - CursorX + 2;
#else
        int Advance = Size.cx;
#endif
        CursorX += Advance;
        if (CursorX + Advance >= BitmapWidth)
        {
            CursorX = 0;
            CursorY += Size.cy;
            assert(CursorY <= BitmapHeight);
        }
        glyph_render_info Glyph = { X0, Y0, X1, Y1, (float)X0 / (float)BitmapWidth, (float)Y0 / (float)BitmapHeight, (float)(X1 + 1) / (float)BitmapWidth, (float)(Y1 + 1) / (float)BitmapHeight, TopGap, LeftGap };
        Glyph.CellWidth = Size.cx;
        Glyph.CellHeight = Size.cy;
        Glyphs[I] = Glyph;
    }
    char SpaceChar = ' ';
    SIZE SpaceSize = {};
    GetTextExtentPoint32A(DeviceContext, &SpaceChar, 1, &SpaceSize);

    // Make alpha channel same as same as r,g,b channels
    for(int I = 0; I < BitmapHeight * BitmapWidth; ++I)
    {
        BitmapMem[I] = BitmapMem[I] | BitmapMem[I] << 24;
    }

    int GlyphCount = 94;
    int GlyphInfoStartOffset = 10 * sizeof(int);
    int ImageStartOffset = GlyphInfoStartOffset + GlyphCount * sizeof(glyph_render_info);
    int KerningPairsOffset = ImageStartOffset + BitmapWidth * BitmapHeight * sizeof(unsigned int);
    int SpaceWidth = SpaceSize.cx;
    char FileName[1000];
    sprintf_s(FileName, "%s.edgy", GetFileNameFromPath(FontName));
    BeginFileWrite(FileName);
    AddToFile(&MAGIC, sizeof(int));
    AddToFile(&Metrics.tmHeight, sizeof(int));
    AddToFile(&BitmapWidth, sizeof(int));
    AddToFile(&BitmapHeight, sizeof(int));
    AddToFile(&GlyphCount, sizeof(int));
    AddToFile(&PairCount, sizeof(int));
    AddToFile(&GlyphInfoStartOffset, sizeof(int));
    AddToFile(&ImageStartOffset, sizeof(int));
    AddToFile(&KerningPairsOffset, sizeof(int));
    AddToFile(&SpaceSize, sizeof(int));
    AddToFile(Glyphs, GlyphCount * sizeof(glyph_render_info));
    AddToFile(BitmapMem, BitmapWidth * BitmapHeight * sizeof(unsigned int));
    AddToFile(Pairs, PairCount * sizeof(kerning_pair));
    EndFileWrite();
    printf("Saved font to file %s\n", FileName);
}

static void
DoPng(char* FilePath)
{
    int X, Y, N;
    unsigned char* Data = stbi_load(FilePath, &X, &Y, &N, 0);
	if(Data == NULL)
	{
		printf("Failed to load file %s\n", FilePath);
		return;
	}
	char FileName[1000];
	sprintf_s(FileName, "%s.edgy", GetFileNameFromPath(FilePath));
	BeginFileWrite(FileName);
	AddToFile(&MAGIC, sizeof(int));
	AddToFile(&X, sizeof(int));
	AddToFile(&Y, sizeof(int));
    AddToFile(Data, sizeof(int) * X * Y);
	EndFileWrite();
    printf("Saved image to file %s\n", FileName);
}

static char*
GetFileNameFromPath(char* Path)
{
    char* Last = strrchr(Path, '/');
    if (Last != NULL)
    {
        return Last + 1;
    }
    return Path;
}