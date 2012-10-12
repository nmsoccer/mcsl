/*
 * mcsltopwin.h
 *
 *  Created on: 2011-3-21
 *      Author: leiming
 */

#ifndef MCSLTOPWIN_H_
#define MCSLTOPWIN_H_

#include "mcslglobal.h"
#include "mcslobj.h"
#include "mcsl.h"


struct _mcsltopwin{
	PRIVATE
	//common
	mcslobj common;		/*所有控件的共同父辈。必须以此开头*/

	//private data

	PUBLIC
	//所有控件共有接口
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

	/*设置控件的字体。任何依附于该控件的子控件默认继承父控件字体。除非单独设置
	 * @param1:字体类型(CHARTER\COURIER\HELVETICA\SCHOOLBOOK\SYMBOL\TIMES)
	 * @param2:字体粗细(MEDIUM\BOLD)
	 * @param3:字体样式(ROMAN\ITALIC\OBLIC)
	 * @param4:字体大小(SIZE_DEFAULT\SIZE_8\10\12\14\18\24)
	 */
	int (*set_font)(mcslwidget *this , char *font_family , char *weights , char *slants , char *point_size);

	/*设置用户回调函数
	 * @param1:用户传入的回调函数
	 * @param2:用户需要的数据
	 * @param3:信号类型
	 */
	int (*attach_callback)(mcslwidget *this , CALLBACK user_function , void *data , mcsl_u8 sig_type);


	//	对于顶层窗口无效  int (*ispressed)(mcslwidget *this , mcsl_u32 type_button);		/*该控件上是否鼠标正被按下*/
	//	对于顶层窗口无效 	int (*visible)(mcslwidget *this);		/*使控件可见*/
	//	对于顶层窗口无效 	int (*hidden)(mcslwidget *this);		/*隐藏控件*/
	//	对于顶层窗口无效  int (*choose_pointer)(mcslwidget *this , mcsl_u8 pointer_id);	/*该控件选取的鼠标ID*/
	//  对于顶层窗口无效 int (*choose_keyboard)(mcslwidget *this , mcsl_u8 keyboard_id);	/*为控件选取的键盘ID*/

	//该控件独有的接口
	int (*set_app_name)(struct _mcsltopwin *this , char *name);	/*设定应用程序名称*/

};


typedef struct _mcsltopwin mcsltopwin;

#define MCSLTOPWIN(e) ((mcsltopwin *)e)

/*MCSL WIDGETS */
/*
 * 产生一个顶层窗口。每个应用程序只能有一个顶层窗口
 */
extern mcsltopwin *new_mcsltopwin(void);




#endif /* MCSLTOPWIN_H_ */
