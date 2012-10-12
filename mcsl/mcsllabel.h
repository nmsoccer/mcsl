/*
 * msllabel.h
 *
 *  Created on: 2011-4-12
 *      Author: leiming
 */

#ifndef MCSLLABEL_H_
#define MCSLLABEL_H_

#include "mcslglobal.h"
#include "mcslobj.h"

struct _mcsllabel{
	PRIVATE
	//common
	mcslobj common;		/*所有控件的共同父辈。必须以此开头*/


	//private data


	PUBLIC
	//所有控件共有接口
	/*该控件上是否鼠标正被按下*/
	int (*ispressed)(mcslwidget *this , mcsl_u32 type_button);

	/*get coordinate*/
	int (*get_coordinate)(mcslwidget *this , mcsl_u32 *x , mcsl_u32 *y , mcsl_u32 *width , mcsl_u32 *height ,
			mcsl_u32 *border);

	/*fetch coordinate of cursor*/
	int (*fetch_cursor)(mcslobj *this , mcsl_u32 *cursor_x , mcsl_u32 *cursor_y);

	/*move the widget*/
	int (*move)(mcslwidget *this , mcsl_u32 x , mcsl_u32 y);

	/*resize the widget*/
	int (*resize)(mcslwidget *this , mcsl_u32 width , mcsl_u32 height);

	/*set color
	 * 颜色格式为#RRGGBB的字符串。具体颜色查询颜色表
	 * @param0:前景色RGB
	 * @param1:背景色RGB
	 * @param3:边框RGB
	 */
	int (*set_color)(mcslwidget *this , char *fore_RGB , char *bg_RGB , char *border_RGB);

	/*change backgroud map*/
	int (*set_bgmap)(mcslwidget *this , char **bg_xpm);

	/*使控件可见/隐藏*/
	int (*visible)(mcslwidget *this);
	int (*hidden)(mcslwidget *this);

	/*设置控件的字体。任何依附于该控件的子控件默认继承父控件字体。除非单独设置
	 * @param1:字体类型(CHARTER\COURIER\HELVETICA\SCHOOLBOOK\SYMBOL\TIMES)
	 * @param2:字体粗细(MEDIUM\BOLD)
	 * @param3:字体样式(ROMAN\ITALIC\OBLIC)
	 * @param4:字体大小(SIZE_DEFAULT\SIZE_8\10\12\14\18\24)
	 */
	int (*set_font)(mcslwidget *this , char *font_family , char *weights , char *slants , char *point_size);

	/*该控件选取的鼠标ID*/
	int (*choose_pointer)(mcslwidget *this , mcsl_u8 pointer_id);
	/*为控件选取的键盘ID*/
	int (*choose_keyboard)(mcslwidget *this , mcsl_u8 keyboard_id);
	/*设置用户回调函数
	 * @param1:用户传入的回调函数
	 * @param2:信号类型
	 */
	int (*attach_callback)(mcslwidget *this , CALLBACK user_function , void *data , mcsl_u8 sig_type);


	//MCSLLABEL独有的接口
	/*
	 * 设置标签的内容
	 */
	int (*set_label)(struct _mcsllabel *this , char *label);


};

typedef struct _mcsllabel mcsllabel;




/*
 * 产生一个标签
 * @param0:输入框依附的父控件(如果为空表示依附顶层窗口)
 * @param1:标签的文字
 */
extern mcsllabel *new_mcsllabel(mcslwidget *parent , char *label);

#define MCSLLABEL(e) ((mcsllabel *)e)

#endif /* MSLLABEL_H_ */
