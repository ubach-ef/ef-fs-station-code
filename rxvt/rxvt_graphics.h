#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <termios.h>

/***********************************************************************
 *
 * Stuff for rxvt special graphics mode
 *
 ***********************************************************************/
#define HORIZONTAL_ALIGNMENT 0x70
#define VERTICAL_ALIGNMENT   0x0f
#define RIGHT_TEXT           0x010
#define HCENTER_TEXT         0x020
#define LEFT_TEXT            0x030

/* Top alignment means the text is placed so that the specified point
 * is is at the top of the capital letters */
#define TOP_TEXT            0x01
/* Center alignment means the text is placed so that the specified point
 * is equidistant from the bottom of descenders and the top of the 
 * capital letters */
#define VCENTER_TEXT        0x02
/* Botton alignment means the text is placed so that the bottom of
 * descenders is on the specified point. */
#define BOTTOM_TEXT         0x03
/* Base alignment means the text is placed so that the bottom of
 * the characters with no descenders is on the specified point. */
#define BASE_TEXT           0x04
/* Caps_Center alignment means the text is placed so that the specified point
 * is equidistant from the bottom and tops of capital letters */
#define VCAPS_CENTER_TEXT   0x05







/***************************************************************************
 *
 * Prototypes for graphics_lib.c functions 
 *
 ***************************************************************************/
long CreateWin(int x, int y, int w, int h);
void StartLine(long id, int x, int y);
void ExtendLine(int x, int y);
void EndLine(void);
void StartPoint(long id, int x, int y);
void ExtendPoint(int x, int y);
void EndPoint(void);
void QueryWin(long id, int *width, int *height);
void StartText(long id, int x, int y,int mode, char *text);
void ClearWindow(long id);
void StartFillAreas(long id,int x1, int y1, int x2, int y2);
void AddFillAreas(int x1, int y1, int x2, int y2);
void EndFillAreas(void);
void ForeColor(int color);
void DefaultRendition(void);
void InitializeGraphics(int scroll_text_up);
void CloseGraphics(void);
void WaitForCarriageReturn(void);
