#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include "rxvt.h"
#include "screen.h"
#include "command.h"
#include "xsetup.h"
#include "sbar.h"
#include "debug.h"
#include "rxvt_graphics.h"


extern Display	       *display;
extern Window	       vt_win;		/* vt100 window */
extern Window	       main_win;	/* parent window */
extern XFontStruct     *mainfont;

extern GC 	        gc;		/* GC for drawing text */
extern GC 	        rvgc;		/* GC for drawing text */
extern unsigned long	foreground;	/* foreground pixel value */
extern unsigned long	background;	/* background pixel value */
extern struct sbar_info sbar;

extern WindowInfo MyWinInfo;
extern ScreenInfo screens[NSCREENS];
extern ScreenInfo *cScreen;

Window win1 = None;
int win_x, win_y,win_h,win_w;
int graphics_up = 0;

extern int fore_color;
struct gr_command
{
  char command;
  int color;
  int ncoords;
  int *coords;
  unsigned char *text;
  struct gr_command *next;
};


struct rxvt_gr_win
{
  Window win;
  int win_x;
  int win_y;
  int win_w;
  int win_h;  
  struct gr_command *graphics;
  struct rxvt_gr_win *prev;
  struct rxvt_gr_win *next;
};

void Gr_CreateNewWindow(int nargs, int *args);
void Gr_do_graphics(char c, int nargs, int *args, unsigned char *text);
void Gr_DrawLine(struct rxvt_gr_win *gr_win, struct gr_command *data);
void Gr_DrawPts(struct rxvt_gr_win *gr_win, struct gr_command *data);
void Gr_ReturnGeom(struct rxvt_gr_win *gr_win, struct gr_command *data);
void Gr_scroll(int count);
void Gr_reset(void);
void Gr_DestroyWindow(struct rxvt_gr_win *gr_win);
void Gr_Redraw(struct rxvt_gr_win *gr_win);
void DoOneCommand(struct rxvt_gr_win *gr_win, struct gr_command *data);


struct rxvt_gr_win *gr_root = NULL;

void Gr_ReportButtonPress(int x, int y)
{
  struct rxvt_gr_win *gr_win, *target = NULL ;

  gr_win = gr_root;
  while((gr_win != NULL)&&(target == NULL))
    {
      if((x > gr_win->win_x)&&
	 (y > gr_win->win_y)&&
	 (x < gr_win->win_x + gr_win->win_w)&&
	 (y < gr_win->win_y + gr_win->win_h))
	target = gr_win;
    }
  
  if(target == NULL)
    return;
  
  x = 10000*(x - gr_win->win_x)/gr_win->win_w;
  y = 10000*(y - gr_win->win_y)/gr_win->win_h;
  cprintf("\033P%ld;%d;%d;\n",gr_win->win,x,y);
  return;

}
  
void Gr_ReportButtonRelease(int x, int y)
{
  struct rxvt_gr_win *gr_win, *target = NULL ;

  gr_win = gr_root;
  while((gr_win != NULL)&&(target == NULL))
    {
      if((x > gr_win->win_x)&&
	 (y > gr_win->win_y)&&
	 (x < gr_win->win_x + gr_win->win_w)&&
	 (y < gr_win->win_y + gr_win->win_h))
	target = gr_win;
    }
  
  if(target == NULL)
    return;
  
  x = 10000*(x - gr_win->win_x)/gr_win->win_w;
  y = 10000*(y - gr_win->win_y)/gr_win->win_h;
  cprintf("\033R%ld;%d;%d;\n",gr_win->win,x,y);
  return;


}

void Gr_do_graphics(char c, int nargs, int *args, unsigned char *text)
{
#ifdef GRAPHICS
  struct rxvt_gr_win *gr_win ;
  struct gr_command *newcom;
  struct gr_command *oldcom;
  long id;
  int i;

  if(c == 'W')
    {
      Gr_CreateNewWindow(nargs,args);
      return;
    }

  if(nargs>0)
    id = args[0];
  else 
    id = None;

  if((c == 'G')&&(id == None))
    {
      Gr_ReturnGeom(NULL,NULL);
      return;
    }

  if((Window)id == None)
    return;
  /* find the rxvt_gr_win struct */
  gr_win = gr_root;
  while((gr_win != NULL)&&(gr_win->win != (Window)id))
    gr_win = gr_win->next;

  if(gr_win == NULL)
    return;

  if(c == 'G')
    {
      Gr_ReturnGeom(gr_win,NULL);
      return;
    }

  /* record this new command */
  newcom = safemalloc(sizeof(struct gr_command),"gr_command");
  newcom->ncoords = nargs-1;
  newcom->coords = safemalloc(newcom->ncoords*sizeof(long),"coords");
  newcom->next = NULL;
  newcom->command = c;
  newcom->color = fore_color;
  for(i=0;i<newcom->ncoords;i++)
    newcom->coords[i] = args[i+1];
  if((c== 'T')&&(nargs >= 5))
    {
      newcom->text = safemalloc(args[4]*sizeof(char),"text");
      for(i=0;i<args[4];i++)
	{
	  newcom->text[i] = text[i];
	}
    }
  else
    newcom->text = 0;
  /* insert new command into command list */
  oldcom = gr_win->graphics;
  if(oldcom ==NULL)
    gr_win->graphics = newcom;
  else
    {
      while(oldcom->next != NULL)
	oldcom = oldcom->next;
      oldcom->next = newcom;
    }
  DoOneCommand(gr_win, newcom);
#endif
}


void Gr_CreateNewWindow(int nargs, int *args)
{
#ifdef GRAPHICS
  struct rxvt_gr_win *gr_win;
  Cursor cursor;

  if(nargs != 4)
    {
      printf("CreateWindow: needed 4 args. Got %d\n",nargs);
      return;
    }

  win_x = args[0]*MyWinInfo.pwidth/10000 + MARGIN;
  win_y = args[1]*MyWinInfo.pheight/10000 + MARGIN;
  win_w = args[2]*MyWinInfo.pwidth/10000;
  win_h = args[3]*MyWinInfo.pheight/10000;

  if((win_w >= 0)&&(win_h >= 0))
    {
      win1 = XCreateSimpleWindow(display,vt_win, 
				 win_x,win_y,win_w,win_h,
				 0,foreground,background);
      cursor = XCreateFontCursor(display,XC_left_ptr);
      XDefineCursor(display,win1,cursor);
      XMapWindow(display,win1);
      XSelectInput(display,win1,ExposureMask);
      graphics_up ++;
      gr_win = safemalloc(sizeof(struct rxvt_gr_win),"rxvt_gr_win");
      gr_win->win = win1;
      gr_win->win_x = win_x;
      gr_win->win_y = win_y;
      gr_win->win_w = win_w;
      gr_win->win_h = win_h;
      gr_win->prev = NULL;
      gr_win->next = gr_root;
      if(gr_win->next)
	gr_win->next->prev = gr_win;
      gr_root = gr_win;
      gr_win->graphics = NULL;
      cprintf("\033W%ld\n",(long)win1);
    }
#endif
}



void Gr_DrawLine(struct rxvt_gr_win *gr_win, struct gr_command *data)
{
#ifdef GRAPHICS
  int i,j;
  XPoint points[500];

  if((data->ncoords > 3)&&(gr_win))
    {
      j=0;
      for(i=0;i<data->ncoords;i+=2)
	{
	  points[j].x = data->coords[i]*gr_win->win_w/10000;
	  points[j].y = data->coords[i+1]*gr_win->win_h/10000;
	  j++;
	}
      XDrawLines(display,gr_win->win,gc,points,j,CoordModeOrigin);
    }
#endif
}

void Gr_DrawPts(struct rxvt_gr_win *gr_win, struct gr_command *data)
{
#ifdef GRAPHICS
  int i,j;

  XPoint points[500];

  if((data->ncoords > 3)&&(gr_win))
    {
      j=0;
      for(i=0;i<data->ncoords;i+=2)
	{
	  points[j].x = data->coords[i]*gr_win->win_w/10000;
	  points[j].y = data->coords[i+1]*gr_win->win_h/10000;
	  j++;
	}
      XDrawPoints(display,gr_win->win,gc,points,j,CoordModeOrigin);
    }
#endif
}

void Gr_ClearWindow(struct rxvt_gr_win *gr_win, struct gr_command *data)
{
#ifdef GRAPHICS
  struct gr_command *grcom,*grnext;

  if(gr_win == NULL)
    return;
  
  grcom = gr_win->graphics;
  
  while(grcom != NULL)
    {
      grnext = grcom->next;
      free(grcom->coords);
      if(grcom->text != (unsigned char *)0)
	free(grcom->text);
      free(grcom);
      grcom = grnext;
    }
  gr_win->graphics = NULL;
  XClearWindow(display,gr_win->win);
#endif
}

void Gr_DrawText(struct rxvt_gr_win *gr_win, struct gr_command *data)
{
#ifdef GRAPHICS
  int i,x1,y1,alignment;

  if((data->ncoords >= 4)&&(gr_win))
    {
      x1 = data->coords[0]*gr_win->win_w/10000;
      y1 = data->coords[1]*gr_win->win_h/10000;
      alignment = data->coords[2];
      if((alignment & HORIZONTAL_ALIGNMENT) == RIGHT_TEXT)
	x1 -= XTextWidth(mainfont,data->text,data->coords[3]);
      if((alignment & HORIZONTAL_ALIGNMENT) == HCENTER_TEXT)
	x1 -= (XTextWidth(mainfont,data->text,data->coords[3])>>1);

      if((alignment & VERTICAL_ALIGNMENT) == TOP_TEXT)
	y1 += mainfont->ascent;
      if((alignment & VERTICAL_ALIGNMENT) == BOTTOM_TEXT)
	y1 -= mainfont->descent;
      if((alignment & VERTICAL_ALIGNMENT) == VCENTER_TEXT)
	y1 = y1 - mainfont->descent+((mainfont->ascent+mainfont->descent)>>1);
      if((alignment & VERTICAL_ALIGNMENT) == VCAPS_CENTER_TEXT)
	y1 = y1 + (mainfont->ascent>>1);

      XDrawString(display,gr_win->win,gc,x1,y1,data->text,data->coords[3]);
    }
#endif
}


void Gr_ReturnGeom(struct rxvt_gr_win *gr_win, struct gr_command *data)
{
#ifdef GRAPHICS
  if(gr_win)
    cprintf("\033G%ld %ld %ld %ld %ld %ld %ld %ld %ld\n",gr_win->win,
	    gr_win->win_x,
	    gr_win->win_y,
	    gr_win->win_w,
	    gr_win->win_h,
	    MyWinInfo.fwidth,
	    MyWinInfo.fheight,
	    10000*MyWinInfo.fwidth/gr_win->win_w,
	    10000*MyWinInfo.fheight/gr_win->win_h);
  else
    cprintf("\033G%d 0 0 0 0 0 0 0 0\n");
#endif
}

void Gr_scroll(int count)
{
#ifdef GRAPHICS
  struct rxvt_gr_win *gr_win,*next;
  static int last_offset = 0;

  if((count == 0)&&(last_offset == MyWinInfo.offset))
    {
      return;
    }

  last_offset = MyWinInfo.offset;

  /* find the rxvt_gr_win struct */
  gr_win = gr_root;

  while(gr_win != NULL)
    {
      next = gr_win->next;
      gr_win->win_y -= count*MyWinInfo.fheight;
      if(gr_win->win_y + gr_win->win_h < 
	 -MyWinInfo.saved_lines * MyWinInfo.fheight)
	{
	  Gr_DestroyWindow(gr_win);
	}
      else
	{
	  XMoveWindow(display,gr_win->win,
		      gr_win->win_x,
		      gr_win->win_y+MyWinInfo.offset*MyWinInfo.fheight);
	}
      gr_win = next;
    }
#endif
}



void Gr_ClearScreen()
{
#ifdef GRAPHICS
  struct rxvt_gr_win *gr_win,*next;

  /* find the rxvt_gr_win struct */
  gr_win = gr_root;

  while(gr_win != NULL)
    {
      next = gr_win->next;
      if(gr_win->win_y + gr_win->win_h > 0)
	{
	  XResizeWindow(display,gr_win->win,
			gr_win->win_w, - gr_win->win_y);
	}
      gr_win = next;
    }
#endif
}

void Gr_FillArea(struct rxvt_gr_win *gr_win, struct gr_command *data)
{
#ifdef GRAPHICS
  int i,j;
  XRectangle rects[250];

  if((data->ncoords > 0)&&(gr_win))
    {
      j=0;
      for(i=0;i<data->ncoords;i+=4)
	{
	  rects[j].x = data->coords[i]*gr_win->win_w/10000;
	  rects[j].y = data->coords[i+1]*gr_win->win_h/10000;
	  rects[j].width = (data->coords[i+2]-data->coords[i]+1)*
	    gr_win->win_w/10000; 
	  rects[j].height = (data->coords[i+3]-data->coords[i+1]+1)*
	    gr_win->win_h/10000; 
	  j++;
	}
      XFillRectangles(display,gr_win->win,gc,rects,j);
    }
#endif
}


void Gr_reset(void)
{
#ifdef GRAPHICS
  struct rxvt_gr_win *gr_win, *next;

  /* find the rxvt_gr_win struct */
  gr_win = gr_root;
  while(gr_win != NULL)
    {
      next = gr_win->next;
      Gr_DestroyWindow(gr_win);
      gr_win = next;
    }
  graphics_up = 0;
#endif
}


void Gr_DestroyWindow(struct rxvt_gr_win *gr_win)
{
#ifdef GRAPHICS
  struct gr_command *grcom,*grnext;

  if(gr_win == NULL)
    return;

  grcom = gr_win->graphics;
  
  while(grcom != NULL)
    {
      grnext = grcom->next;
      free(grcom->coords);
      if(grcom->text != (unsigned char *)0)
	free(grcom->text);
      free(grcom);
      grcom = grnext;
    }
  XDestroyWindow(display,gr_win->win);
  if(gr_win->next != NULL)
    {
      gr_win->next->prev = gr_win->prev;
    }
  if(gr_win->prev != NULL)
    {
      gr_win->prev->next = gr_win->next;
    }
  else
    {
      gr_root = gr_win->next;
    }
  free(gr_win);
  graphics_up --;
#endif
}

void Gr_expose(Window win)
{
#ifdef GRAPHICS
  struct rxvt_gr_win *gr_win,*next;

  /* find the rxvt_gr_win struct */
  gr_win = gr_root;
  
  while(gr_win != NULL)
    {
      if(gr_win->win == win)
	{
	  Gr_Redraw(gr_win);
	  gr_win = NULL;
	}
      else
	gr_win = gr_win->next;
    }
#endif
}


void Gr_Redraw(struct rxvt_gr_win *gr_win)
{
#ifdef GRAPHICS
  struct gr_command *grcom;
  
  if(gr_win == NULL)
    return;

  grcom = gr_win->graphics;
  
  while(grcom != NULL)
    {
      DoOneCommand(gr_win, grcom);
      grcom = grcom->next;
    }
#endif
}


void DoOneCommand(struct rxvt_gr_win *gr_win, struct gr_command *data)
{
#ifdef GRAPHICS
#ifdef COLOR
  XGCValues newgcv;

  extern unsigned long pixel_colors[10];
  if(data->color != 0)
    {
      newgcv.foreground = pixel_colors[data->color];
      XChangeGC(display,gc,GCForeground,&newgcv);
    }
#endif      
  switch(data->command)
    {
    case 'L':
      Gr_DrawLine(gr_win,data);      
      break;
    case 'P':
      Gr_DrawPts(gr_win,data);      
      break;
    case 'T':
      Gr_DrawText(gr_win,data);
      break;
    case 'C':
      Gr_ClearWindow(gr_win,data);
    case 'F':
      Gr_FillArea(gr_win,data);
    }
#ifdef COLOR
  if(data->color != 0)
    {
      newgcv.foreground = pixel_colors[0];
      XChangeGC(display,gc,GCForeground,&newgcv);
    }
#endif

#endif
}

