/*
 * mcsllabel.c
 *
 *  Created on: 2011-4-12
 *      Author: leiming
 */
#include "mcslglobal.h"
#include "mcsllabel.h"

#define MCSL_LABEL_W		50
#define MCSL_LABEL_H		35

/*
 * 设置标签的内容
 */
static int set_label(mcsllabel *this , char *label);

/*
 * 产生一个标签
 * @param0:输入框依附的父控件(如果为空表示依附顶层窗口)
 * @param1:标签的文字
 */
mcsllabel *new_mcsllabel(mcslwidget *parent , char *label){
	Window parent_win;

	mcsllabel *p = (mcsllabel *)malloc(sizeof(mcsllabel));
	memset(p , 0 , sizeof(mcsllabel));	/*clear*/
	register_mcsl_cargo(&p->common);	/*注册*/

	parent_win = (parent == NULL)?mcsl_topwin:parent->basic_attr.win;

	/*public*/
	//type
	p->common.type = MCSL_LABEL;

	//malloc_attr
	if(label){	/*如果有文本内容*/
		p->common.malloc_attr.content = (char *)malloc(strlen(label) + 1);
		memset(p->common.malloc_attr.content , 0 , strlen(label) + 1);
		strcpy(p->common.malloc_attr.content , label);
		printf("it is:%s\n" , p->common.malloc_attr.content);
	}else{
		p->common.malloc_attr.content = NULL;
	}

	//coordinate
	p->common.coordinate.x = 0;
	p->common.coordinate.y = 0;

	if(label){	/*如果有标签文本那么宽度由文本宽度决定*/
		p->common.coordinate.width = XTextWidth(parent->basic_attr.font_info , label , strlen(label) + 1) + 10;
		printf("label is:%d\n" , p->common.coordinate.width);
	}else{	/*没有文本那么使用默认宽度*/
		p->common.coordinate.width = MCSL_LABEL_W;
	}
	p->common.coordinate.height = MCSL_LABEL_H;
	p->common.coordinate.border = 0;
	//flags
	p->common.flags.visible = 1;		/*可见*/
	p->common.flags.new_font = 0;	/*继承字体*/
	p->common.flags.use_map = 0;	/*do not use map*/
	p->common.flags.button_pressed = 0;
	p->common.flags.button2_pressed = 0;
	p->common.flags.button3_pressed = 0;
	p->common.flags.chosen_pointer = NOPOINTER;	/*开始没有选择光标*/
	//basic_attr
	p->common.basic_attr.bgcolor = colors[WHITE];
	p->common.basic_attr.forecolor = colors[CADET_BLUE];
	p->common.basic_attr.bordercolor = colors[WHITE];

	p->common.basic_attr.win = XCreateSimpleWindow(display , parent_win , p->common.coordinate.x , p->common.coordinate.y ,
			p->common.coordinate.width , p->common.coordinate.height , 1 , colors[WHITE] , colors[WHITE]);
	XSelectInput(display , p->common.basic_attr.win , ExposureMask | 	StructureNotifyMask);

	p->common.basic_attr.parent_win = parent_win;
	p->common.basic_attr.font_info = parent->basic_attr.font_info;
	p->common.basic_attr.gc = parent->basic_attr.gc;
//	p->common.basic_attr.bg_map = mcsl_pixmaps[MCSL_LABEL].bg_map;

	//private data


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

	//MCSLLABEL独有的接口
	/*
	 * 设置标签的内容
	 */
	p->set_label = set_label;


	return p;
}



/*
 * 设置标签的内容
 */
static int set_label(mcsllabel *this , char *label){
	if((this == NULL) || (label == NULL)){
		return -1;
	}

	/*清除原来文字*/
	XClearWindow(display ,this->common.basic_attr.win);
	free(this->common.malloc_attr.content);
	this->common.malloc_attr.content = (char *)malloc(strlen(label) + 1);
	memset(this->common.malloc_attr.content , 0 , strlen(label) + 1);
	strcpy(this->common.malloc_attr.content , label);

	/*调整大小*/
	this->common.coordinate.width = XTextWidth(this->common.basic_attr.font_info , label , strlen(label) + 1) + 10;
	XResizeWindow(display , this->common.basic_attr.win , this->common.coordinate.width , this->common.coordinate.height);


}
