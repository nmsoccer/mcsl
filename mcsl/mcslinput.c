/*
 * mcslinput.c
 *
 *  Created on: 2011-4-10
 *      Author: leiming
 */
#include "mcslglobal.h"
#include "mcslinput.h"

#define MCSL_INPUT_SIZE	30	/*默认输入框包括的最大字符数目 (包括0x0)*/
//#define MCSL_INPUT_W	 100
#define MCSL_INPUT_H  28
//#define MCSL_INPUT_H  40


//MCSLINPUT独有的接口
/*
 * 设置input可以容纳的最大字符数
 */
static int set_size(mcslinput *this , int size);
/*
 * 获取input可以容纳的最大字符数
 */
static int get_size(mcslinput *this);
/*
 * 获取input的内容
 */
static char *get_text(mcslinput *this);


/*
 * 产生一个输入框
 * @param0:输入框依附的父控件(如果为空表示依附顶层窗口)
 * @param1:输入框上的默认文字(可以为空)
 */
mcslinput *new_mcslinput(mcslwidget *parent , char *str_content){
	char string[MCSL_INPUT_SIZE];	/*用于估算字符长度*/
	memset(string , 'X' , MCSL_INPUT_SIZE - 1);
	string[MCSL_INPUT_SIZE] = 0;
	Window parent_win;

	mcslinput *p = (mcslinput *)malloc(sizeof(mcslinput));
	memset(p , 0 , sizeof(mcslinput));	/*clear*/
	register_mcsl_cargo(&p->common);	/*注册*/

	parent_win = (parent == NULL)?mcsl_topwin:parent->basic_attr.win;

	/*public*/
	//type
	p->common.type = MCSL_INPUT;

	//coordinate
	p->common.coordinate.x = 0;
	p->common.coordinate.y = 0;

	p->common.coordinate.width = XTextWidth(parent->basic_attr.font_info , string , MCSL_INPUT_SIZE - 1);
	printf("width is: %d\n" , p->common.coordinate.width);
	p->common.coordinate.height = MCSL_INPUT_H;
	printf("width is: %d\n" , p->common.coordinate.height);
	p->common.coordinate.border = 1;
	//flags
	p->common.flags.visible = 1;		/*可见*/
	p->common.flags.new_font = 0;	/*继承字体*/
	p->common.flags.use_map = 0;	/*do not use map*/
	p->common.flags.button_pressed = 0;
	p->common.flags.button2_pressed = 0;
	p->common.flags.button3_pressed = 0;
	p->common.flags.chosen_pointer = NOPOINTER;	/*开始没有选择光标*/
	p->common.flags.chosen_keyboard = NOKEYBOARD;	/*默认是任意键盘*/
	//basic_attr
	p->common.basic_attr.bgcolor = colors[WHITE];
	p->common.basic_attr.forecolor = colors[BLACK];
	p->common.basic_attr.bordercolor = colors[CADET_BLUE];

	p->common.basic_attr.win = XCreateSimpleWindow(display , parent_win , p->common.coordinate.x , p->common.coordinate.y ,
			p->common.coordinate.width , p->common.coordinate.height , 1 , colors[CADET_BLUE] , colors[WHITE]);
	XSelectInput(display , p->common.basic_attr.win , ExposureMask | 	StructureNotifyMask);

	p->common.basic_attr.parent_win = parent_win;
	p->common.basic_attr.font_info = parent->basic_attr.font_info;
	p->common.basic_attr.gc = parent->basic_attr.gc;
//	p->common.basic_attr.bg_map = mcsl_pixmaps[MCSL_BUTTON_PX].bg_map;

	//private data
	p->size = MCSL_INPUT_SIZE;

	//malloc_attr
	if(str_content){	/*如果有文本内容 必须 < size*/
		p->common.malloc_attr.content = (char *)malloc(p->size);
		memset(p->common.malloc_attr.content , 0 , p->size);
		strcpy(p->common.malloc_attr.content , str_content);
		printf("it is:%s\n" , p->common.malloc_attr.content);
	}else{
		p->common.malloc_attr.content = NULL;
	}


	//private data
	if(p->common.malloc_attr.content){
		p->locate = XTextWidth(p->common.basic_attr.font_info , p->common.malloc_attr.content , strlen(p->common.malloc_attr.content));
	}else{
		p->locate = 0;
	}

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

	//MCSLINPUT独有的接口
	/*
	 * 设置input可以容纳的最大字符数
	 */
	p->set_size = set_size;
	/*
	 * 获取input可以容纳的最大字符数
	 */
	p->get_size = get_size;
	/*
	 * 获取input的内容
	 */
	p->get_text = get_text;

	return p;
}


/////////////////////////////////////////////////////////////////////


//MCSLINPUT独有的接口
/*
 * 设置input可以容纳的最大字符数
 * 如果size<=0 不做设置
 * return: success >0
 * error -1
 */
static int set_size(mcslinput *this , int size){
	if((!this) || (size <= 0)){
		return -1;
	}

	this->size = size;
	/*原来的内容作废*/
	XClearWindow(display ,this->common.basic_attr.win);
	free(this->common.malloc_attr.content);
	this->common.malloc_attr.content = (char *)malloc(size);
	memset(this->common.malloc_attr.content , 'X' , size - 1);

	/*调整大小*/
	this->common.coordinate.width = XTextWidth(this->common.basic_attr.font_info , this->common.malloc_attr.content , size - 1);
	XResizeWindow(display , this->common.basic_attr.win , this->common.coordinate.width , this->common.coordinate.height);
	/*清空文本区*/
	memset(this->common.malloc_attr.content , 0 , size);

	return 0;
}
/*
 * 获取input可以容纳的最大字符数
 * return: >=0 success
 * -1: error
 */
static int get_size(mcslinput *this){
	if(!this){
		return -1;
	}

	return this->size;
}
/*
 * 获取input的内容
 */
static char *get_text(mcslinput *this){
	if(!this){
		return NULL;
	}

	return this->common.malloc_attr.content;
}


