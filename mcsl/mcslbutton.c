/*
 * mcslbutton.c
 *
 *  Created on: 2011-3-24
 *      Author: leiming
 */
#include "mcslglobal.h"
#include "mcslbutton.h"

#define MCSL_BUTTON_W	 70
#define MCSL_BUTTON_H  35


/*设置按钮按下颜色*/
static int set_pressed_color(mcslbutton *this , char *pressed_RGB);
/*设置按钮按下背景图*/
static int set_pressed_map(mcslbutton *this , char **pressed_bg_xpm);

/*
 * 产生一个按钮
 * @param0:按钮依附的父控件(如果为空表示依附顶层窗口)
 * @param1:按钮上的文字(可以为空)
 */
mcslbutton *new_mcslbutton(mcslwidget *parent , char *str_content){
	Window parent_win;

	mcslbutton *p = (mcslbutton *)malloc(sizeof(mcslbutton));
	memset(p , 0 , sizeof(mcslbutton));	/*clear*/
	register_mcsl_cargo(&p->common);	/*注册*/

	parent_win = (parent == NULL)?mcsl_topwin:parent->basic_attr.win;

	/*public*/
	//type
	p->common.type = MCSL_BUTTON;
	//coordinate
	p->common.coordinate.x = 0;
	p->common.coordinate.y = 0;

	p->common.coordinate.width = MCSL_BUTTON_W;
	p->common.coordinate.height = MCSL_BUTTON_H;
	p->common.coordinate.border = 1;
	//flags
	p->common.flags.visible = 1;		/*可见*/
	p->common.flags.new_font = 0;	/*继承字体*/
	p->common.flags.use_map = 1;	/*use map*/
	p->common.flags.button_pressed = 0;
	p->common.flags.button2_pressed = 0;
	p->common.flags.button3_pressed = 0;
	p->common.flags.chosen_pointer = NOPOINTER;	/*开始没有选择光标*/
	p->common.flags.chosen_keyboard = NOKEYBOARD;
	//basic_attr
	p->common.basic_attr.bgcolor = colors[WHITE];
	p->common.basic_attr.forecolor = colors[CADET_BLUE];
	p->common.basic_attr.bordercolor = colors[CADET_BLUE];

	p->common.basic_attr.win = XCreateSimpleWindow(display , parent_win , p->common.coordinate.x , p->common.coordinate.y ,
			p->common.coordinate.width , p->common.coordinate.height , 1 , colors[CADET_BLUE] , colors[WHITE]);
	XSelectInput(display , p->common.basic_attr.win , ExposureMask | 	StructureNotifyMask);

	p->common.basic_attr.parent_win = parent_win;
	p->common.basic_attr.font_info = parent->basic_attr.font_info;
	p->common.basic_attr.gc = parent->basic_attr.gc;
	p->common.basic_attr.bg_map = mcsl_pixmaps[MCSL_BUTTON_PX].bg_map;

	//malloc_attr
	if(str_content){	/*如果有文本内容*/
		p->common.malloc_attr.content = (char *)malloc(strlen(str_content) + 1);
		memset(p->common.malloc_attr.content , 0 , strlen(str_content) + 1);
		strcpy(p->common.malloc_attr.content , str_content);
	}else{
		p->common.malloc_attr.content = NULL;
	}
	//private data
	p->pressed_color = colors[YELLOW];
	p->pressed_map = mcsl_pixmaps[MCSL_BUTTON_P_PX].bg_map;

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
	p->ispressed = mcsl_ispressed;
	p->get_coordinate = mcsl_get_coordinate;
	p->fetch_cursor = mcsl_fetch_cursor;
	p->move = mcsl_move;
	p->resize = mcsl_resize;
	p->set_color = mcsl_set_color;
	p->set_bgmap = mcsl_set_bgmap;
	p->visible = mcsl_visible;
	p->hidden = mcsl_hidden;
	p->choose_pointer = mcsl_choose_pointer;
	p->choose_keyboard = mcsl_choose_keyboard;
	p->attach_callback = mcsl_attach_callback;

	/*private*/
	p->set_pressed_color = set_pressed_color;
	p->set_pressed_map = set_pressed_map;

	return p;
}

//////////////////////////PRIVATE FUNCTION///////////////////////////
/*设置按钮按下颜色*/
static int set_pressed_color(mcslbutton *this , char *pressed_RGB){
	Colormap map;
	XColor color;

	if(!this){
		return -1;
	}

	map = XDefaultColormap(display , screen_num);
	memset(&color , 0 , sizeof(XColor));
	XParseColor(display , map , pressed_RGB , &color);
	XAllocColor(display , map , &color);
	this->pressed_color = color.pixel;


	return 0;

}

/*设置按钮按下背景图*/
static int set_pressed_map(mcslbutton *this , char **pressed_bg_xpm){
	if(!this){
		return -1;
	}

	memset(&mcsl_xpmattr, 0, sizeof(mcsl_xpmattr));
	XpmCreatePixmapFromData(display, mcsl_topwin, pressed_bg_xpm , &this->pressed_map , NULL, &mcsl_xpmattr);
	mcsl_pixmaps[MCSL_BUTTON_P_PX].map_width = mcsl_pixmaps[MCSL_BUTTON_PX].map_width;
	mcsl_pixmaps[MCSL_BUTTON_P_PX].map_height = mcsl_pixmaps[MCSL_BUTTON_PX].map_height;


	return 0;
}
