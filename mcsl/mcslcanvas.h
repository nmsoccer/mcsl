/*
 * mcslcanvas.h
 *
 *  Created on: 2011-4-14
 *      Author: leiming
 */

#ifndef MCSLCANVAS_H_
#define MCSLCANVAS_H_

#include "mcslglobal.h"
#include "mcslobj.h"


typedef struct _mcsl_points{
	mcsl_u32 nr_points;
	XPoint *points;
}mcsl_points;

typedef struct _mcsl_segs{
	mcsl_u32 nr_segs;
	XSegment *segs;
}mcsl_segs;

typedef struct _mcsl_rects{
	mcsl_u32 nr_rects;
	XRectangle *rects;
}mcsl_rects;

typedef struct _mcsl_arcs{
	mcsl_u32 nr_arcs;
	XArc *arcs;
}mcsl_arcs;

typedef struct _mcsl_graph{	/*每一种颜色画出的图形*/
	mcsl_u32	pixel;	/*像素*/
	mcsl_u16	red;		/*对应的RGB*/
	mcsl_u16	green;
	mcsl_u16 blue;
	mcsl_points drawing_points;	/*存储已画出的图案*/
	mcsl_segs	drawing_segs;
	mcsl_rects	drawing_rects;
	mcsl_arcs		drawing_arcs;
	mcsl_points	filling_polygon;		/*存储填充图案*/
	mcsl_rects	filling_rects;
	mcsl_arcs		filling_arcs;
	struct _mcsl_graph *next;
}mcsl_graph;

typedef struct _mcsl_canvas_graph{
	mcsl_graph head_graph;
	mcsl_graph *empty_graph;		/*指向未设置但已分配内存的mcsl_graph*/
	mcsl_graph *current_graph;	/*当前正在使用的graph*/
}mcsl_canvas_graph;



struct _mcslcanvas{
	PRIVATE
	//common
	mcslobj common;		/*所有控件的共同父辈。必须以此开头*/


	//private data
	mcsl_canvas_graph mcsl_canvas_graphs;	/*存储画布用所有颜色所画的图案*/

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

	//控件独有的接口
	/*设置画笔的颜色。在画布中不推荐利用set_color设置前景色*/
	int (*set_pen_color)(struct _mcslcanvas *this , char *pen_color);

	/*利用画布画一个点*/
	int (*draw_point)(struct _mcslcanvas *this , mcsl_u32 x , mcsl_u32 y);
	/*利用画布画一条线段
	*/
	int (*draw_seg)(struct _mcslcanvas *this , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 dest_x , mcsl_u32 dest_y);
	/*利用画布画一个矩形但不填充
	 * @param1~@param2:矩形左上角位置
	 * @param3~@param4:矩形对角线处点的位置
	 */
	int (*draw_rect)(struct _mcslcanvas *this , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 diagonal_x , mcsl_u32 diagonal_y);
	/*利用画布画一个弧但不填充
	 * @param1~@param2:弧所在的圆心
	 * @param3~@param4:确定弧起始角度的射线。(该端点与圆心的距离确定半径)
	 * @param5~@param6:确定弧终止角度的射线
	 * 注意画的弧度始终是逆时针方向
	 */
	int (*draw_arc)(struct _mcslcanvas *this , mcsl_u32 x , mcsl_u32 y , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 dest_x , mcsl_u32  dest_y);

	/*利用画布填充一个矩形
	 * @param1~@param2:矩形左上角位置
	 * @param3~@param4:矩形对角线处点的位置
	 */
	int (*fill_rect)(struct _mcslcanvas *this , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 diagonal_x , mcsl_u32 diagonal_y);
	/*利用画布填充一段弧
	 * @param1~@param2:弧所在的圆心
	 * @param3~@param4:确定弧起始角度的射线。(该端点与圆心的距离确定半径)
	 * @param5~@param6:确定弧终止角度的射线
	 */
	int (*fill_arc)(struct _mcslcanvas *this , mcsl_u32 x , mcsl_u32 y , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 dest_x , mcsl_u32  dest_y);

	/*根据所画的点填充围成的多边形(单独一个点显然不能构成多边形的)
	 *@param3:
	 */
	int (*fill_polygon)(struct _mcslcanvas *this , mcsl_u32 x , mcsl_u32 y , int shape);

};

typedef struct _mcslcanvas mcslcanvas;




/*
 * 产生一张画布
 * @param0:画布依附的父控件(如果为空表示依附顶层窗口)
 * @param1～2:画布大小。如果不传入那么用默认大小
 */
extern mcslcanvas *new_mcslcanvas(mcslwidget *parent , mcsl_u32 width , mcsl_u32 height);





#define MCSLCANVAS(e) ((mcslcanvas *)e)

#endif /* MCSLCANVAS_H_ */
