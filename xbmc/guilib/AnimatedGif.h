/*!
\file AnimatedGif.h
\brief
*/


// ****************************************************************************
//
// WINIMAGE.H : Generic classes for raster images (MSWindows specialization)
//
//  Content: Class declarations of:
//  - class CAnimatedGif             : Storage class for single images
//  - class CAnimatedGifSet          : Storage class for sets of images
//  - class C_AnimationWindow   : Window Class to display animations
//
//  (Includes declarations of routines to Load and Save BMP files and to load
// GIF files into these classes).
//
//  --------------------------------------------------------------------------
//
// Copyright � 2000, Juan Soulie <jsoulie@cplusplus.com>
//
// Permission to use, copy, modify, distribute and sell this software or any
// part thereof and/or its documentation for any purpose is granted without fee
// provided that the above copyright notice and this permission notice appear
// in all copies.
//
// This software is provided "as is" without express or implied warranty of
// any kind. The author shall have no liability with respect to the
// infringement of copyrights or patents that any modification to the content
// of this file or this file itself may incur.
//
// ****************************************************************************




#pragma pack(1)

struct COLOR {unsigned char b,g,r,x;};	// Windows GDI expects 4bytes per color
#define ALIGN sizeof(int)         ///< Windows GDI expects all int-aligned

#define LZW_MAXBITS   12
#define LZW_SIZETABLE (1<<LZW_MAXBITS)

/*!
 \ingroup textures
 \brief
 */
typedef struct tagGUIRGBQUAD
{
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
}
GUIRGBQUAD;

/*!
 \ingroup textures
 \brief
 */
typedef struct tagGUIBITMAPINFOHEADER
{
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
}
GUIBITMAPINFOHEADER;

/*!
 \ingroup textures
 \brief
 */
#ifdef HAS_XBOX_D3D
typedef struct tagGUIBITMAPINFO
{
    GUIBITMAPINFOHEADER    bmiHeader;
    GUIRGBQUAD						 bmiColors[1];
} GUIBITMAPINFO;
#else
  typedef tagBITMAPINFO GUIBITMAPINFO;
#endif

#pragma pack()


// ****************************************************************************
// * CAnimatedGif                                                                  *
// *    Storage class for single images                                       *
// ****************************************************************************
/*!
 \ingroup textures
 \brief Storage class for single images
 */
class CAnimatedGif
{
public:
  CAnimatedGif();
  virtual ~CAnimatedGif();

  // standard members:
  int Width, Height;   ///< Dimensions in pixels
  int BPP;        // Bits Per Pixel
  char* Raster;       ///< Bits of Raster Data (Byte Aligned)
  COLOR* Palette;      ///< Color Map
  int BytesPerRow;    ///< Width (in bytes) including alignment!
  int Transparent;    ///< Index of Transparent color (-1 for none)

  // Extra members for animations:
  int nLoops;
  int xPos, yPos;     ///< Relative Position
  int Delay;       ///< Delay after image in 1/1000 seconds.
  int Transparency;    ///< Animation Transparency.
  // Windows GDI specific:
  GUIBITMAPINFO* pbmi;        ///< BITMAPINFO structure

  // constructor and destructor:

  // operator= (object copy)
  CAnimatedGif& operator= (CAnimatedGif& rhs);

  /// \brief Image initializer (allocates space for raster and palette):
  void Init (int iWidth, int iHeight, int iBPP, int iLoops = 0);

  inline char& Pixel (int x, int y) { return Raster[y*BytesPerRow + x];}

#ifndef _XBOX
  // Windows GDI Specific function to paint the image on a DC:
  int GDIPaint (HDC hdc, int xDest, int yDest);
#endif

};

// ****************************************************************************
// * CAnimatedGifSet                                                               *
// *    Storage class for sets of images                                      *
// ****************************************************************************
/*!
 \ingroup textures
 \brief Storage class for sets of images
 */
class CAnimatedGifSet
{
public:

  // constructor and destructor:
  CAnimatedGifSet();
  virtual ~CAnimatedGifSet();

  int FrameWidth, FrameHeight; ///< Dimensions of ImageSet in pixels.
  int nLoops;          // Number of Loops (0 = infinite)

  std::vector<CAnimatedGif*> m_vecimg;        ///< Images' Vector.

  void AddImage (CAnimatedGif*);   ///< Append new image to vector (push_back)

  int GetImageCount() const;
  // File Formats:
  int LoadGIF (const char* szFile);

  void Release();
protected:
  unsigned char getbyte(FILE *fd);
};

#pragma pack()
