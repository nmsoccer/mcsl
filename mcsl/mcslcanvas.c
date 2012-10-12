/*
 * mcslcanvas.c
 *
 *  Created on: 2011-4-14
 *      Author: leiming
 */
#include <math.h>
#include "mcslglobal.h"
#include "mcslcanvas.h"

#define MCSL_CANVAS_W	100
#define MCSL_CANVAS_H	100

#define MCSL_MAX_POINTS		(1024 * 4)	//最多画4k个点
#define MCSL_MAX_SEGS		512				//最多画0.5k条线段
#define MCSL_MAX_RECTS		512				//最多画0.5k个矩形
#define MCSL_MAX_ARCS			512				//最多画0.5k条弧


#define PI 3.1415

/*设置画笔的颜色。在画布中不推荐利用set_color设置前景色*/
static	int set_pen_color(mcslcanvas *this , char *pen_color);

/*利用画布画一个点*/
static	int draw_point(mcslcanvas *this , mcsl_u32 x , mcsl_u32 y);
/*利用画布画一条线段
*/
static	int draw_seg(mcslcanvas *this , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 dest_x , mcsl_u32 dest_y);
/*利用画布画一个矩形但不填充
 * @param1~@param2:矩形左上角位置
 * @param3~@param4:矩形对角线处点的位置
 */
static	int draw_rect(mcslcanvas *this , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 diagonal_x , mcsl_u32 diagonal_y);
/*利用画布画一个弧但不填充
 * @param1~@param2:弧所在的圆心
 * @param3~@param4:确定弧起始角度的射线。(该端点与圆心的距离确定半径)
 * @param5~@param6:确定弧终止角度的射线
 * 注意画的弧度始终是逆时针方向
 */
static	int draw_arc(mcslcanvas *this , mcsl_u32 x , mcsl_u32 y , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 dest_x , mcsl_u32  dest_y);

/*利用画布填充一个矩形
 * @param1~@param2:矩形左上角位置
 * @param3~@param4:矩形对角线处点的位置
 */
static	int fill_rect(mcslcanvas *this , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 diagonal_x , mcsl_u32 diagonal_y);
/*利用画布填充一段弧
 * @param1~@param2:弧所在的圆心
 * @param3~@param4:确定弧起始角度的射线。(该端点与圆心的距离确定半径)
 * @param5~@param6:确定弧终止角度的射线
 */
static	int fill_arc(mcslcanvas *this , mcsl_u32 x , mcsl_u32 y , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 dest_x , mcsl_u32  dest_y);

/*根据所画的点填充围成的多边形(单独一个点显然不能构成多边形的)
 *@param3:
 */
static	int fill_polygon(mcslcanvas *this , mcsl_u32 x , mcsl_u32 y , int shape);

/*
 * 产生一张画布
 * @param0:画布依附的父控件(如果为空表示依附顶层窗口)
 * @param1～2:画布大小。如果不传入那么用默认大小
 */
mcslcanvas *new_mcslcanvas(mcslwidget *parent , mcsl_u32 width , mcsl_u32 height){
	/*每张画布都有一个特有GC*/
	mcsl_u32 valuemask;
	XGCValues values;
	GC gc;

	Window parent_win;

	mcslcanvas *p = (mcslcanvas *)malloc(sizeof(mcslcanvas));
	memset(p , 0 , sizeof(mcslcanvas));	/*clear*/
	register_mcsl_cargo(&p->common);	/*注册*/

	parent_win = (parent == NULL)?mcsl_topwin:parent->basic_attr.win;

	/*public*/
	//type
	p->common.type = MCSL_CANVAS;
	//coordinate
	p->common.coordinate.x = 0;
	p->common.coordinate.y = 0;

	p->common.coordinate.width = (width == 0)?MCSL_CANVAS_W:width;
	p->common.coordinate.height = (height == 0)?MCSL_CANVAS_H:height;
	p->common.coordinate.border = 1;
	//flags
	p->common.flags.visible = 1;		/*可见*/
	p->common.flags.new_font = 0;	/*继承字体*/
	p->common.flags.use_map = 0;	/*do not use map*/
	p->common.flags.button_pressed = 0;
	p->common.flags.button2_pressed = 0;
	p->common.flags.button3_pressed = 0;
	p->common.flags.chosen_pointer = NOPOINTER;	/*开始没有选择光标*/
	p->common.flags.chosen_keyboard = NOKEYBOARD;
	//basic_attr
	p->common.basic_attr.bgcolor = colors[WHITE];
	p->common.basic_attr.forecolor = colors[BLACK];
	p->common.basic_attr.bordercolor = colors[CADET_BLUE];

	p->common.basic_attr.win = XCreateSimpleWindow(display , parent_win , p->common.coordinate.x , p->common.coordinate.y ,
			p->common.coordinate.width , p->common.coordinate.height , 1 , colors[CADET_BLUE] , colors[WHITE]);
	XSelectInput(display , p->common.basic_attr.win , ExposureMask | 	StructureNotifyMask);

	p->common.basic_attr.parent_win = parent_win;
	p->common.basic_attr.font_info = parent->basic_attr.font_info;
	/*Create GC*/
	valuemask = GCForeground | GCBackground;
	values.foreground = colors[BLACK];
	values.background = colors[WHITE];
	gc = XCreateGC(display , p->common.basic_attr.win , valuemask , &values);
	p->common.basic_attr.gc = gc;

	//malloc_attr
	p->common.malloc_attr.content = NULL;

	//private data
	memset(&p->mcsl_canvas_graphs , 0 , sizeof(mcsl_canvas_graph));

	p->mcsl_canvas_graphs.empty_graph = (mcsl_graph *)malloc(sizeof(mcsl_graph));
	memset(p->mcsl_canvas_graphs.empty_graph , 0 , sizeof(mcsl_graph));
	p->mcsl_canvas_graphs.empty_graph->next = NULL;

	p->mcsl_canvas_graphs.head_graph.pixel = 0;	/*默认的前景色为黑色*/
	p->mcsl_canvas_graphs.head_graph.red = 0;
	p->mcsl_canvas_graphs.head_graph.green = 0;
	p->mcsl_canvas_graphs.head_graph.blue = 0;
	p->mcsl_canvas_graphs.head_graph.next = p->mcsl_canvas_graphs.empty_graph;

	p->mcsl_canvas_graphs.current_graph = &p->mcsl_canvas_graphs.head_graph;

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
	p->set_pen_color = set_pen_color;
	p->draw_point = draw_point;
	p->draw_seg = draw_seg;
	p->draw_rect = draw_rect;
	p->draw_arc = draw_arc;
	p->fill_rect = fill_rect;
	p->fill_arc = fill_arc;
	p->fill_polygon = fill_polygon;

	return p;
}


////////////////////////PRIVATE FUNCTIONS///////////////////////////////////////////////////////////////
/*利用画布画一个点*/
/*设置画笔的颜色。在画布中不推荐利用set_color设置前景色*/
/*设置画笔的颜色。在画布中不推荐利用set_color设置前景色*/
static	int set_pen_color(mcslcanvas *this , char *pen_color){
	if(!this){
		return -1;
	}
	if(!pen_color){
		return -1;
	}
	Colormap map;
	XColor color;
	mcsl_graph *p;

	map = XDefaultColormap(display , screen_num);
	memset(&color , 0 , sizeof(XColor));
	XParseColor(display , map , pen_color , &color);
	XAllocColor(display , map , &color);
	this->common.basic_attr.forecolor = color.pixel;

	XSetForeground(display , this->common.basic_attr.gc , color.pixel);	/*改变使用的前景色*/

	/*查看该像素颜色是否被使用*/
	p = &this->mcsl_canvas_graphs.head_graph;
	while(1){
		if(p == this->mcsl_canvas_graphs.empty_graph){
			break;
		}

		if(p->pixel == color.pixel){
			this->mcsl_canvas_graphs.current_graph = p;	/*找到已经设置颜色的节点。将其设置为当前使用颜色*/
			return 0;
		}

		p = p->next;
	}

	/*没有找到已经设置该颜色的节点*/
	this->mcsl_canvas_graphs.current_graph = p;	//p == empty_graph
	p->pixel = color.pixel;
	p->red = color.red;
	p->green = color.green;
	p->blue = color.blue;

	p = (mcsl_graph *)malloc(sizeof(mcsl_graph));
	memset(p , 0 , sizeof(mcsl_graph));
	p->next = NULL;
	this->mcsl_canvas_graphs.empty_graph->next = p;
	this->mcsl_canvas_graphs.empty_graph = p;

	return 0;
}

/*利用画布画一个点*/
static	int draw_point(mcslcanvas *this , mcsl_u32 x , mcsl_u32 y){
	mcsl_u32 index;
	if(!this){
		return -1;
	}

	XDrawPoint(display , this->common.basic_attr.win , this->common.basic_attr.gc , x , y);
	/*将此点放入对应的颜色图形中保存*/
	index = this->mcsl_canvas_graphs.current_graph->drawing_points.nr_points;
	if(index > 0){
		this->mcsl_canvas_graphs.current_graph->drawing_points.points[index].x = x;
		this->mcsl_canvas_graphs.current_graph->drawing_points.points[index].y = y;
		index++;
		this->mcsl_canvas_graphs.current_graph->drawing_points.nr_points = index;
		return 0;
	}

	/*如果第一次使用该颜色画点需要分配内存*/
	if(index == 0){
		this->mcsl_canvas_graphs.current_graph->drawing_points.points = (XPoint *)malloc(sizeof(XPoint) * MCSL_MAX_POINTS);
		this->mcsl_canvas_graphs.current_graph->drawing_points.points[index].x = x;
		this->mcsl_canvas_graphs.current_graph->drawing_points.points[index].y = y;
		index++;
		this->mcsl_canvas_graphs.current_graph->drawing_points.nr_points = index;
		return 0;
	}
	return -1;	/*never run here*/
}
/*利用画布画一条线段
*/
static	int draw_seg(mcslcanvas *this , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 dest_x , mcsl_u32 dest_y){
	mcsl_u32 index;
	if(!this){
		return -1;
	}

	XDrawLine(display , this->common.basic_attr.win , this->common.basic_attr.gc , src_x , src_y , dest_x , dest_y);
	/*将此线段放入对应的颜色图形中保存*/
	index = this->mcsl_canvas_graphs.current_graph->drawing_segs.nr_segs;
	if(index > 0){
		this->mcsl_canvas_graphs.current_graph->drawing_segs.segs[index].x1 = src_x;
		this->mcsl_canvas_graphs.current_graph->drawing_segs.segs[index].y1 = src_y;
		this->mcsl_canvas_graphs.current_graph->drawing_segs.segs[index].x2 = dest_x;
		this->mcsl_canvas_graphs.current_graph->drawing_segs.segs[index].y2 = dest_y;
		index++;
		this->mcsl_canvas_graphs.current_graph->drawing_segs.nr_segs = index;
		return 0;
	}

	/*如果第一次使用该颜色画线段需要分配内存*/
	if(index == 0){
		this->mcsl_canvas_graphs.current_graph->drawing_segs.segs = (XSegment *)malloc(sizeof(XSegment) * MCSL_MAX_SEGS);
		this->mcsl_canvas_graphs.current_graph->drawing_segs.segs[index].x1 = src_x;
		this->mcsl_canvas_graphs.current_graph->drawing_segs.segs[index].y1 = src_y;
		this->mcsl_canvas_graphs.current_graph->drawing_segs.segs[index].x2 = dest_x;
		this->mcsl_canvas_graphs.current_graph->drawing_segs.segs[index].y2 = dest_y;
		index++;
		this->mcsl_canvas_graphs.current_graph->drawing_segs.nr_segs = index;
		return 0;
	}
	return -1;	/*never run here*/
}
/*利用画布画一个矩形但不填充
 * @param1~@param2:矩形左上角位置
 * @param3~@param4:矩形对角线处点的位置
 */
static	int draw_rect(mcslcanvas *this , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 diagonal_x , mcsl_u32 diagonal_y){
	mcsl_u32 index;
	mcsl_u32 start_x , start_y;
	mcsl_u32 width , height;
	if(!this){
		return -1;
	}

	/*找到所画矩形的起点。要求起点必须是左上角*/
	start_x = (src_x < diagonal_x)?src_x:diagonal_x;
	start_y = (src_y < diagonal_y)?src_y:diagonal_y;
	width = (src_x < diagonal_x)?diagonal_x - src_x:src_x - diagonal_x;
	height = (src_y < diagonal_y)?diagonal_y - src_y:src_y - diagonal_y;

	XDrawRectangle(display , this->common.basic_attr.win , this->common.basic_attr.gc , start_x , start_y , width , height);
	/*将此矩形放入对应的颜色图形中保存*/
	index = this->mcsl_canvas_graphs.current_graph->drawing_rects.nr_rects;
	if(index > 0){
		this->mcsl_canvas_graphs.current_graph->drawing_rects.rects[index].x = start_x;
		this->mcsl_canvas_graphs.current_graph->drawing_rects.rects[index].y = start_y;
		this->mcsl_canvas_graphs.current_graph->drawing_rects.rects[index].width = width;
		this->mcsl_canvas_graphs.current_graph->drawing_rects.rects[index].height = height;
		index++;
		this->mcsl_canvas_graphs.current_graph->drawing_rects.nr_rects = index;
		return 0;
	}

	/*如果第一次使用该颜色画矩形需要分配内存*/
	if(index == 0){
		this->mcsl_canvas_graphs.current_graph->drawing_rects.rects = (XRectangle *)malloc(sizeof(XRectangle) * MCSL_MAX_RECTS);
		this->mcsl_canvas_graphs.current_graph->drawing_rects.rects[index].x = start_x;
		this->mcsl_canvas_graphs.current_graph->drawing_rects.rects[index].y = start_y;
		this->mcsl_canvas_graphs.current_graph->drawing_rects.rects[index].width = width;
		this->mcsl_canvas_graphs.current_graph->drawing_rects.rects[index].height = height;
		index++;
		this->mcsl_canvas_graphs.current_graph->drawing_rects.nr_rects = index;
		return 0;
	}
	return -1;	/*never run here*/
}
/*利用画布画一个弧但不填充
 * @param1~@param2:弧所在的圆心
 * @param3~@param4:确定弧起始角度的射线。(该端点与圆心的距离确定半径)
 * @param5~@param6:确定弧终止角度的射线
 * 注意画的弧度始终是逆时针方向
 */
static	int draw_arc(mcslcanvas *this , mcsl_u32 x , mcsl_u32 y , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 dest_x , mcsl_u32  dest_y){
	mcsl_u32 index;
	mcsl_u32 radius;	/*半径*/
	double start_angle , dest_angle;	/*起始角度，结束角度*/
	double start_dist , dest_dist;	/*起点分别距离起始点以及终点的距离*/
	short short_start , short_dest;
	if(!this){
		return -1;
	}

	/*找到所画弧所在圆的半径*/
	start_dist = sqrt(pow((double)abs(src_x - x) , 2) + pow((double)abs(src_y - y) , 2));
	dest_dist = sqrt(pow((double)abs(dest_x - x) , 2) + pow((double)abs(dest_y - y) , 2));
	radius = (mcsl_u32)start_dist;
	printf("radius is:%d\n" , radius);

	/*设置起始角度*/
//	printf("ratio: %lf , asin: %lf" , abs(src_y - y)/start_dist , asin(abs(src_y - y)/start_dist));

	start_angle = asin(abs(src_y - y) / start_dist) * 180 / PI;		/*先取得最小正角*/
	start_angle = start_angle + (src_x > x?src_y >y?270:0:src_y > y?180:90);	/*根据起始点所在象限加上相应角度*/
	short_start = (short)start_angle;
	printf("start_angle is:%lf , %d\n" , start_angle , short_start);

	/*设置终止角度*/
	dest_angle = asin(abs(dest_y - y) / dest_dist) * 180 / PI;		/*先取得最小正角*/
	dest_angle = dest_angle + (dest_x > x?dest_y >y?270:0:dest_y > y?180:90);	/*根据起始点所在象限加上相应角度注意零点在左上角*/
	short_dest = (short)dest_angle;
	printf("dest_angle is:%lf , %d\n" , dest_angle , short_dest);

	XDrawArc(display , this->common.basic_attr.win , this->common.basic_attr.gc , x , y , radius * 2 , radius * 2 , short_start * 64 , short_dest * 64);
	/*将此弧放入对应的颜色图形中保存*/
	index = this->mcsl_canvas_graphs.current_graph->drawing_arcs.nr_arcs;
	if(index > 0){
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.arcs[index].x = x;
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.arcs[index].y = y;
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.arcs[index].width = radius * 2;
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.arcs[index].height = radius * 2;
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.arcs[index].angle1 = short_start * 64;
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.arcs[index].angle2 =short_dest * 64;
		index++;
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.nr_arcs = index;
		return 0;
	}

	/*如果第一次使用该颜色画矩形需要分配内存*/
	if(index == 0){
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.arcs = (XArc *)malloc(sizeof(XArc) * MCSL_MAX_ARCS);
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.arcs[index].x = x;
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.arcs[index].y = y;
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.arcs[index].width = radius;
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.arcs[index].height = radius;
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.arcs[index].angle1 = short_start;
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.arcs[index].angle2 =short_dest;
		index++;
		this->mcsl_canvas_graphs.current_graph->drawing_arcs.nr_arcs = index;
		return 0;
	}
	return -1;	/*never run here*/
}

/*利用画布填充一个矩形
 * @param1~@param2:矩形左上角位置
 * @param3~@param4:矩形对角线处点的位置
 */
static	int fill_rect(mcslcanvas *this , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 diagonal_x , mcsl_u32 diagonal_y){
	mcsl_u32 index;
	mcsl_u32 start_x , start_y;
	mcsl_u32 width , height;
	if(!this){
		return -1;
	}

	/*找到所画矩形的起点。要求起点必须是左上角*/
	start_x = (src_x < diagonal_x)?src_x:diagonal_x;
	start_y = (src_y < diagonal_y)?src_y:diagonal_y;
	width = (src_x < diagonal_x)?diagonal_x - src_x:src_x - diagonal_x;
	height = (src_y < diagonal_y)?diagonal_y - src_y:src_y - diagonal_y;

	XFillRectangle(display , this->common.basic_attr.win , this->common.basic_attr.gc , start_x , start_y , width , height);
	/*将此矩形放入对应的颜色图形中保存*/
	index = this->mcsl_canvas_graphs.current_graph->filling_rects.nr_rects;
	if(index > 0){
		this->mcsl_canvas_graphs.current_graph->filling_rects.rects[index].x = start_x;
		this->mcsl_canvas_graphs.current_graph->filling_rects.rects[index].y = start_y;
		this->mcsl_canvas_graphs.current_graph->filling_rects.rects[index].width = width;
		this->mcsl_canvas_graphs.current_graph->filling_rects.rects[index].height = height;
		index++;
		this->mcsl_canvas_graphs.current_graph->filling_rects.nr_rects = index;
		return 0;
	}

	/*如果第一次使用该颜色画矩形需要分配内存*/
	if(index == 0){
		this->mcsl_canvas_graphs.current_graph->filling_rects.rects = (XRectangle *)malloc(sizeof(XRectangle) * MCSL_MAX_RECTS);
		this->mcsl_canvas_graphs.current_graph->filling_rects.rects[index].x = start_x;
		this->mcsl_canvas_graphs.current_graph->filling_rects.rects[index].y = start_y;
		this->mcsl_canvas_graphs.current_graph->filling_rects.rects[index].width = width;
		this->mcsl_canvas_graphs.current_graph->filling_rects.rects[index].height = height;
		index++;
		this->mcsl_canvas_graphs.current_graph->filling_rects.nr_rects = index;
		return 0;
	}
	return -1;	/*never run here*/
}
/*利用画布填充一段弧
 * @param1~@param2:弧所在的圆心
 * @param3~@param4:确定弧起始角度的射线。(该端点与圆心的距离确定半径)
 * @param5~@param6:确定弧终止角度的射线
 */
static	int fill_arc(mcslcanvas *this , mcsl_u32 x , mcsl_u32 y , mcsl_u32 src_x , mcsl_u32 src_y , mcsl_u32 dest_x , mcsl_u32  dest_y){
	return 0;
}

/*根据所画的点填充围成的多边形(单独一个点显然不能构成多边形的)
 *@param3:
 */
static	int fill_polygon(mcslcanvas *this , mcsl_u32 x , mcsl_u32 y , int shape){
	return 0;
}

















