/*
 * mcslobj.h
 *
 *  Created on: 2011-3-20
 *      Author: leiming
 */

#ifndef MCSLOBJ_H_
#define MCSLOBJ_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/xpm.h>

#include "mcsltypes.h"
#include "mcslbox.h"
/*事件处理循环中内部调用函数.内调函数
 * @param0:控件公共头部
 * @param1:事件数据(对于XI事件是传入的相应事件数据event.xcookie.data *;对于普通事件是整个XEVent *)
*/
typedef void (*CALLIN)(void *common , void *ev_data);
/*用户传入的回调函数，由内调函数调用*/
typedef void (*CALLBACK)(void *mcslwidget , void *data);

/*控件种类*/
#define NR_MCSL_WIDGETS	10		/*控件的种类*/

#define MCSL_TOPWIN	1	/*顶层窗口*/
#define MCSL_BUTTON	2	/*按钮*/
#define MCSL_LABEL		3	/*标签*/
#define MCSL_INPUT		4	/*输入框*/
#define MCSL_CANVAS	5	/*画布*/


/*SIG TYPES*/
#define NOSIG				0

#define SIG_BUTTON_PRESS			1
#define SIG_BUTTON_RELEASE		2
#define SIG_BUTTON2_PRESS		3
#define SIG_BUTTON2_RELEASE	4
#define SIG_BUTTON3_PRESS		5
#define SIG_BUTTON3_RELEASE	6
#define SIG_KEY_PRESS			    7
#define SIG_KEY_RELEASE				8
#define SIG_MOTION					9
#define SIG_ENTER						10
#define SIG_LEAVE						11
#define MAX_SIG							12


struct _mcslobj{
	mcsl_u32 type;		/*type of this object*/

	struct _coordinate{	/*coordinate of obj*/
		mcsl_u32 x;
		mcsl_u32 y;

		mcsl_u32 width;
		mcsl_u32 height;
		mcsl_u32 border;	/*width of border*/
	}coordinate;

	struct _basic_attr{	/*basic attributes*/
		Window win;		/*控件本身窗口*/
		Window parent_win;	/*父窗口*/

		mcsl_u32 cursor_x;	/*光标在控件里的坐标*/
		mcsl_u32 cursor_y;

		mcsl_u32 bgcolor;
		mcsl_u32 forecolor;
		mcsl_u32 bordercolor;	/*border color*/

		Pixmap bg_map;	/*background map*/
		XFontStruct *font_info;	/*依附于该控件的字体类型*/

		GC gc;

		struct _mcslbox *container;	/*该控件拥有的容器。最多一个*/

	}basic_attr;

	struct _malloc_attr{	/*用于动态分配内存的属性*/
		char *content;	/*该控件包含的文字*/
	}malloc_attr;


	struct _flags{
		mcsl_u8 visible;		/*是否可见*/
		mcsl_u8 new_font;	/*如果为1表示自己加载的字体；如果为0表示继承父控件字体。默认为0*/
		mcsl_u8 use_map;	/*是否使用背景图案。默认为是。*/
		mcsl_u8 button_pressed;	/*鼠标左键是否被按下。默认为否*/
		mcsl_u8 button2_pressed;
		mcsl_u8 button3_pressed;
		mcsl_u8 chosen_pointer;		/*该控件选择的光标。默认为0表示没有选择。若最后仍没有选择那么默认选择ANYPOINTER*/
		mcsl_u8 chosen_keyboard; /*该控件选择的光标。默认为0表示没有选择。若最后仍没有选择那么默认不选择键盘(键盘不是必须的)*/
	}flags;


	/*
	 * if(win == destwin)
	 * p->inner_interface.xx(p);
	 */

	struct _inner_interface{	/*在事件处理循环中调用。由内部接口来调用用户传入的回调函数*/
		 CALLIN cook_expose;
		 CALLIN cook_button_press;/*default left button*/
		 CALLIN cook_button_release;
		 CALLIN cook_button2_press;
		 CALLIN cook_button2_release;
		 CALLIN cook_button3_press;
		 CALLIN cook_button3_release;
		 CALLIN cook_key_press;
		 CALLIN cook_key_release;
		 CALLIN cook_motion;
		 CALLIN cook_enter;	/*pointer enter or leave*/
		 CALLIN cook_leave;

	}inner_interface;


	struct _user_interface{	/*用户传入的回调函数.@param0:调用该回调函数的控件.@param1:如果需要，传入的数据*/
		void *user_data[MAX_SIG];		/*用户传入的数据指针每种信号可以传入相应的数据*/

		CALLBACK cb_button_press;	/*default left button*/
		CALLBACK cb_button_release;
		CALLBACK cb_button2_press;
		CALLBACK cb_button2_release;
		CALLBACK cb_button3_press;
		CALLBACK cb_button3_release;
		CALLBACK cb_key_press;
		CALLBACK cb_key_release;
		CALLBACK cb_motion;
		CALLBACK cb_enter;
		CALLBACK cb_leave;
	}user_interface;


};

typedef struct _mcslobj mcslobj;

/*内部使用mcslobj，外部使用mcslwidget*/
typedef mcslobj mcslwidget;

/*从任何一个控件的指针中获取其头部*/
#define MCSLWIDGET(any) (mcslwidget*)any


/*MCSLOBJSTORE 仓库。所有生成的控件的指针都要在此处注册*/

typedef struct _mcsl_cargo{
	mcslobj *content;
	struct _mcsl_cargo *next;
}mcsl_cargo;

typedef struct _inner_store{
	mcsl_cargo head;
	mcsl_cargo *last;	/*last始终指向一个未使用的cargo*/
}inner_store;

extern inner_store mcsl_store;

extern int register_mcsl_cargo(mcslobj *source); /*注册一个控件*/
extern mcslobj *fetch_mcsl_cargo(Window win);	/*根据传入的窗口ID获取对应的控件*/
extern int delete_mcsl_store(void);	/*销毁仓库*/

/*
 * 用于初始化内部调用函数、公共外部接口函数的声明
 */
//内调函数
extern void cooked_expose(void *common , void *ev_data);
extern void cooked_button_press(void *common , void *ev_data);/*default left button*/
extern void cooked_button_release(void *common , void *ev_data);
extern void cooked_button2_press(void *common , void *ev_data);
extern void cooked_button2_release(void *common , void *ev_data);
extern void cooked_button3_press(void *common , void *ev_data);
extern void cooked_button3_release(void *common , void *ev_data);
extern void cooked_key_press(void *common , void *ev_data);
extern void cooked_key_release(void *common , void *ev_data);
extern void cooked_motion(void *common , void *ev_data);
extern void cooked_enter(void *common , void *ev_data);	/*pointer enter or leave*/
extern void cooked_leave(void *common , void *ev_data);

//公共外部接口

#define BUTTON1_PRESSED	1	/*选择哪个按键按下*/
#define BUTTON2_PRESSED	2
#define BUTTON3_PRESSED	3
extern int mcsl_ispressed(mcslobj *this , mcsl_u32 type_button);	/*whether widget is pressed*/

extern int mcsl_get_coordinate(mcslobj *this , mcsl_u32 *x , mcsl_u32 *y , mcsl_u32 *width , mcsl_u32 *height ,	mcsl_u32 *border);	/*get coordinate*/
extern int mcsl_fetch_cursor(mcslobj *this , mcsl_u32 *cursor_x , mcsl_u32 *cursor_y);
extern int mcsl_move(mcslobj *this , mcsl_u32 x , mcsl_u32 y);/*move the widget*/
extern int mcsl_resize(mcslobj *this , mcsl_u32 width , mcsl_u32 height);	/*resize the widget*/
/*set color
 * 颜色格式为#RRGGBB的字符串。具体颜色查询颜色表
 * @param0:前景色RGB
 * @param1:背景色RGB
 * @param3:边框RGB
 */
extern int mcsl_set_color(mcslobj *this , char *fore_RGB , char *bg_RGB , char *border_RGB);
extern int mcsl_set_bgmap(mcslobj *this , char **bg_xpm);	/*change backgroud map*/
extern int mcsl_set_font(mcslwidget *this , char *font_family , char *weights , char *slants , char *point_size);	/*设置控件字体*/
extern int mcsl_visible(mcslwidget *this);		/*使控件可见*/
extern int mcsl_hidden(mcslwidget *this);		/*隐藏控件*/
extern int mcsl_choose_pointer(mcslobj *this , mcsl_u8 pointer_id);	/*为控件选取的鼠标ID*/
extern int mcsl_choose_keyboard(mcslobj *this , mcsl_u8 keyboard_id);	/*为控件选取的键盘ID*/
/*传入相应信号的回调函数*/
extern int mcsl_attach_callback(mcslobj *this , CALLBACK user_function , void *data , mcsl_u8 sig_type);





#endif /* MCSLOBJ_H_ */
