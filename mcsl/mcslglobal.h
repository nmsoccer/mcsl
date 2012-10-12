/*
 * mcslglobal.h
 *
 *  Created on: 2011-3-17
 *      Author: leiming
 */

#ifndef MCSLGLOBAL_H_
#define MCSLGLOBAL_H_


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/xpm.h>

#include <X11/extensions/XInput2.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>

#include "mcsltypes.h"
#include "mcsldevice.h"

#define DEVELOP

#define PRIVATE	/*private*/
#define PUBLIC

#define MCSLDUMP_ERR(exp) {printf("%s failed\n" , #exp);}

#ifdef DEVELOP
#define PRINT(exp) {printf("%s\n" , #exp);}
#else
#define PRINT(exp)
#endif

/*DISPLAY*/
extern Display *display;
extern int screen_num;

/*TOPWIN*/
extern Window mcsl_topwin;

/*GC*/
extern mcsl_u32  mcsl_valuemask;
extern XGCValues mcsl_values;
extern GC mcsl_gc;

/* FONT*/
extern XFontStruct *mcsl_font;
extern char **mcsl_font_names;

/*Window Attributes*/
extern XWindowAttributes win_attr;
extern XSetWindowAttributes win_set_attr;

/*Cursor*/
#define MAX_CURSOR_SHAPE MAX_POINTERS	/*add 2012.5: SET SHAPE TO CURSOR*/
extern Cursor mcsl_cur;
extern unsigned int cursor_shape[MAX_CURSOR_SHAPE];
/*COLORS*/
#define MAX_COLORS	10

/*
 * COLORS系统内部使用。
 */
#define NOCOLOR		0	/*no color chosen*/
#define BLACK			1
#define WHITE			2
#define RED				3
#define GREEN			4
#define BLUE				5
#define YELLOW		6
#define ORANGE		7
#define CADET_BLUE	8
#define MEDIUM_AQUAMARINE	9
//#define CORAL			7
//#define CYAN				8
//#define GREY				9
//#define GRAY				10
//#define LIGHT_GREY 11
//#define LIGHT_GRAY	12
//#define MARGENTA	13
//#define MAROON		14
//#define AQUAMARINE	15
//#define BLUE_VIOLET	17

extern mcsl_u32 colors[MAX_COLORS];

/*Pointer Device Info*/
extern pointer_device *p_device;	//pointer_device info

/*PIXMAP 确定各个控件的基本贴图*/
#define MAX_PIXMAP 20

#define MCSL_TOPWIN_PX		0	/*顶层窗口背景*/
#define MCSL_BUTTON_PX		1	/*按钮默认背景*/
#define MCSL_BUTTON_P_PX	2	/*按钮被按下的背景*/
#define MCSL_LABEL_PX			3	/*标签默认背景*/
#define MCSL_INPUT_PX			4	/*输入框默认背景*/
#define MCSL_CANVAS_PX		5	/*画布*/

typedef struct _bgmap_info{
	Pixmap bg_map;
	mcsl_u32 map_width;	/*记录用作背景图的图片参数*/
	mcsl_u32 map_height;
}bgmap_info;


extern XpmAttributes mcsl_xpmattr;
extern bgmap_info mcsl_pixmaps[];

#endif /* GLOBAL_H_ */
