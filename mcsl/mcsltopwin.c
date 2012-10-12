/*
 * mcsltopwin.c
 *
 *  Created on: 2011-3-23
 *      Author: leiming
 */
#include "mcslglobal.h"
#include "mcsltopwin.h"

#define MCSL_TOPWIN_W	 (DisplayWidth(display , screen_num) / 2)
#define MCSL_TOPWIN_H  (DisplayHeight(display , screen_num) / 2)

/*设定应用程序名称*/
static int set_app_name(struct _mcsltopwin *this , char *name);

/*
 * 产生一个顶层窗口。每个应用程序只能有一个顶层窗口
 */
struct _mcsltopwin *new_mcsltopwin(void){
	mcsltopwin *p = (mcsltopwin *)malloc(sizeof(mcsltopwin));

	memset(p , 0 , sizeof(mcsltopwin));	/*clear*/

	register_mcsl_cargo(&p->common);	/*注册*/

	/*public*/
	//type
	p->common.type = MCSL_TOPWIN;
	//coordinate
	p->common.coordinate.x = 0;
	p->common.coordinate.y = 0;
	p->common.coordinate.width = MCSL_TOPWIN_W;
	p->common.coordinate.height = MCSL_TOPWIN_H;
	p->common.coordinate.border = 1;
	//flags
	p->common.flags.visible = 1;		/*可见*/
	p->common.flags.new_font = 0;	/*继承字体*/
	p->common.flags.use_map = 0;	/*do not use map*/
	p->common.flags.button_pressed = 0;
	p->common.flags.button2_pressed = 0;
	p->common.flags.button3_pressed = 0;
	p->common.flags.chosen_pointer = ANYPOINTER;	/*所有光标*/
	//basic_attr
	p->common.basic_attr.win = mcsl_topwin;
	p->common.basic_attr.parent_win = RootWindow(display , screen_num);
	p->common.basic_attr.bgcolor = colors[WHITE];
	p->common.basic_attr.forecolor = colors[BLACK];
	p->common.basic_attr.bordercolor = colors[BLACK];
	p->common.basic_attr.font_info = mcsl_font;
	p->common.basic_attr.gc = mcsl_gc;
	p->common.basic_attr.bg_map = 0;
	//malloc_attr
	p->common.malloc_attr.content = NULL;
	//inner_interface
	p->common.inner_interface.cook_expose = cooked_expose;
	p->common.inner_interface.cook_button_press = cooked_button_press;
	p->common.inner_interface.cook_button_release = cooked_button_release;
	p->common.inner_interface.cook_button2_press = cooked_button2_press;
	p->common.inner_interface.cook_button2_release = cooked_button2_release;
	p->common.inner_interface.cook_button3_press = cooked_button3_press;
	p->common.inner_interface.cook_button3_release = cooked_button3_release;
	p->common.inner_interface.cook_key_press = cooked_key_press;
	p->common.inner_interface.cook_key_release = cooked_key_release;
	p->common.inner_interface.cook_motion = cooked_motion;
	p->common.inner_interface.cook_enter = cooked_enter;
	p->common.inner_interface.cook_leave = cooked_leave;

	//public interface
	p->get_coordinate = mcsl_get_coordinate;
	p->fetch_cursor = mcsl_fetch_cursor;
	p->move = mcsl_move;
	p->resize = mcsl_resize;
	p->set_color = mcsl_set_color;
	p->set_bgmap = mcsl_set_bgmap;
	p->set_font = mcsl_set_font;
	p->attach_callback = mcsl_attach_callback;

	/*private*/
	p->set_app_name = set_app_name;

	return p;
}

//////////////////////////////////////////////////////////////
/*设定应用程序名称*/
static int set_app_name(struct _mcsltopwin *this , char *name){
	return XStoreName(display , this->common.basic_attr.win , name);
}
