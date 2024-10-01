// header for SSD1306 OLED display - VERSION 1.2 (April 14th 2023)
// Simon Walker, char map by Chelsea Walker
// Device variant changes - Diarmid Rendell
// working as of May 19/2022
// April 14th - Added enum for display orientation (tested only on 128 x 32 devices)

// private helpers
//int SSD1306_Command8 (unsigned char command);
//int SSD1306_Command16 (unsigned char commandA, unsigned char commandB);
//void SSD1306_Data (unsigned char * data, unsigned int iCount);

#ifndef _SSD1306_ADDRESS
#define _SSD1306_ADDRESS 0x3C
#endif

// comment in/out the appropriate size of your display! ****
#ifndef _SSD1306_DisplaySize128x32
#define _SSD1306_DisplaySize128x32
#endif
//#ifndef _SSD1306_DisplaySize128x64
//#define _SSD1306_DisplaySize128x64
//#endif
// comment in/out the appropriate size of your display! ****

// screen orientation
typedef enum 
{
  _SSD1306_OR_UP,
  _SSD1306_OR_Down
} _SSD1306_Orientation;

// management
void _SSD1306_DispInit (_SSD1306_Orientation screen_dir);
void _SSD1306_Noise (void);
void _SSD1306_Clear (void);
void _SSD1306_Render (void);
int _SSD1306_IsDirty (void);
void _SSD1306_DisplayOn (void);
void _SSD1306_DisplayOff (void);
int _SSD1306_SetInverse (int IsInverse);

// string
void _SSD1306_CharXY (unsigned char iX, unsigned char iY, char disp);
void _SSD1306_StringXY (unsigned char iX, unsigned char iY, char * pStr);

// graphics
void _SSD1306_SetPixel (int iX, int iY);
void _SSD1306_Line (int iXS, int iYS, int iXE, int iYE);
void _SSD1306_Circle (int iXS, int iYS, float fRad);

// requires the page data to be in flash
void _SSD1306_SetPage (int page, unsigned char * buff);
