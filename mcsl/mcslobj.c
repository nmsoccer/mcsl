/*
 * mcslobj.c
 *
 *  Created on: 2011-3-21
 *      Author: leiming
 */
#include "mcslglobal.h"
#include "mcslobj.h"

#include "mcsltopwin.h"
#include "mcslbox.h"
#include "mcslbutton.h"
#include "mcslinput.h"
#include "mcsltool.h"

#include <X11/extensions/XInput2.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>

inner_store mcsl_store;

/*注册一个控件*/
int register_mcsl_cargo(mcslobj *source){
	mcsl_cargo *p;

	p = (mcsl_cargo *)malloc(sizeof(mcsl_cargo));
	p->content = NULL;
	p->next = NULL;

	mcsl_store.last->content = source;
	mcsl_store.last->next = p;
	mcsl_store.last = p;

	return 0;
}

/*根据传入的窗口ID获取对应的控件*/
mcslobj *fetch_mcsl_cargo(Window win){
	mcsl_cargo *p;

	p = &mcsl_store.head;

	while(p){
		if(p->content->basic_attr.win == win){
			return p->content;
		}

		p = p->next;
	}

	return NULL;
}


/*销毁仓库*/
int delete_mcsl_store(void){
	mcsl_cargo *p;
	mcsl_cargo *tmp;

	mcsl_graph *p_graph , *tmp_graph;

	if(mcsl_store.head.content){

		/*第一个一定是topwin*/
//		XUnloadFont(display , mcsl_store.head.content->basic_attr.font_info->fid);	/*删除字体*//
		free((struct _mcsltopwin *)mcsl_store.head.content);
	}

	p = mcsl_store.head.next;

	while(p){
		if(!p->next){
			break;
		}
		tmp = p->next;
//		if(p->content->flags.new_font){	/*如果不是继承字体 那么卸载字体*/
//			XUnloadFont(display , p->content->basic_attr.font_info->fid);
//		}
		if(p->content->malloc_attr.content){	/*回收控件的动态分配属性*/
			free(p->content->malloc_attr.content);
		}
		switch(p->content->type){	/*根据控件所属类型释放之*/
		case MCSL_TOPWIN:
			free((mcsltopwin *)p->content);
			break;
		case MCSL_BUTTON:
			free((mcslbutton *)p->content);
			break;
		case MCSL_LABEL:
			break;
		case MCSL_INPUT:
			free((mcslinput *)p->content);
			break;
		case MCSL_CANVAS:
			XFreeGC(display , p->content->basic_attr.gc);

			p_graph = &((mcslcanvas *)p->content)->mcsl_canvas_graphs.head_graph;	/*释放头结点的内容*/
			if(p_graph->drawing_points.nr_points > 0){
				free(p_graph->drawing_points.points);
			}
			if(p_graph->drawing_segs.nr_segs > 0){
				free(p_graph->drawing_segs.segs);
			}
			if(p_graph->drawing_rects.nr_rects > 0){
				free(p_graph->drawing_rects.rects);
			}
			if(p_graph->drawing_arcs.nr_arcs > 0){
				free(p_graph->drawing_arcs.arcs);
			}
			if(p_graph->filling_polygon.nr_points > 0){
				free(p_graph->filling_polygon.points);
			}
			if(p_graph->filling_rects.nr_rects > 0){
				free(p_graph->filling_rects.rects);
			}
			if(p_graph->filling_arcs.nr_arcs > 0){
				free(p_graph->filling_arcs.arcs);
			}

			p_graph = ((mcslcanvas *)p->content)->mcsl_canvas_graphs.head_graph.next;	/*释放每种颜色所画图像*/
			while(1){
				if(p_graph == NULL){
					break;
				}

				if(p_graph->drawing_points.nr_points > 0){
					free(p_graph->drawing_points.points);
				}
				if(p_graph->drawing_segs.nr_segs > 0){
					free(p_graph->drawing_segs.segs);
				}
				if(p_graph->drawing_rects.nr_rects > 0){
					free(p_graph->drawing_rects.rects);
				}
				if(p_graph->drawing_arcs.nr_arcs > 0){
					free(p_graph->drawing_arcs.arcs);
				}
				if(p_graph->filling_polygon.nr_points > 0){
					free(p_graph->filling_polygon.points);
				}
				if(p_graph->filling_rects.nr_rects > 0){
					free(p_graph->filling_rects.rects);
				}
				if(p_graph->filling_arcs.nr_arcs > 0){
					free(p_graph->filling_arcs.arcs);
				}

				tmp_graph = p_graph->next;
				free(p_graph);
				p_graph = tmp_graph;
			}

			free((mcslcanvas *)p->content);
			break;
		default:
			break;
		}


		free(p);
		p = tmp;
	}

	return 0;
}


///////////内调函数的实现///////////////////

/*处理显露事件*/
void cooked_expose(void *common , void *ev_data){
	int y;
	mcslobj *p = (mcslobj *)common;

	mcsl_graph *p_graph;

	switch(p->type){
	case MCSL_TOPWIN:
		if(p->flags.use_map){	/*使用背景图案*/
			XCopyArea(display , p->basic_attr.bg_map , p->basic_attr.win , mcsl_gc , 0 , 0 , p->coordinate.width , p->coordinate.height , 0 , 0);
		}
		break;
	case MCSL_BUTTON:
		if(p->flags.use_map){	/*使用背景图案*/
			XCopyArea(display , p->basic_attr.bg_map , p->basic_attr.win , mcsl_gc , 0 , 0 , p->coordinate.width , p->coordinate.height , 0 , 0);
		}else{
			XSetWindowBackground(display , p->basic_attr.win , p->basic_attr.bgcolor);
		}
		if(p->malloc_attr.content){	/*如果有文字内容*/
			mcsl_draw_string(p , p->malloc_attr.content , strlen(p->malloc_attr.content) , A_CENTER);
		}
		break;
	case MCSL_LABEL:
		if(p->flags.use_map){	/*使用背景图案*/
			XCopyArea(display , p->basic_attr.bg_map , p->basic_attr.win , mcsl_gc , 0 , 0 , p->coordinate.width , p->coordinate.height , 0 , 0);
		}
		if(p->malloc_attr.content){	/*如果有文字内容*/
			mcsl_draw_string(p , p->malloc_attr.content , strlen(p->malloc_attr.content) , A_LEFT);
		}
		break;

	case MCSL_INPUT:
		if(p->flags.use_map){	/*使用背景图案*/
			XCopyArea(display , p->basic_attr.bg_map , p->basic_attr.win , mcsl_gc , 0 , 0 , p->coordinate.width , p->coordinate.height , 0 , 0);
		}
		if(p->malloc_attr.content){	/*如果有文字内容*/
			mcsl_draw_string(p , p->malloc_attr.content , strlen(p->malloc_attr.content) , A_LEFT);
		}
		/*描绘插入光标*/
//		XDrawLine(display , p->basic_attr.win , p->basic_attr.gc , ((mcslinput *)p)->locate + 5 , 3 ,
//							((mcslinput *)p)->locate + 5 , p->coordinate.height - 5);
		break;

	case MCSL_CANVAS:
		/*将所有保存的图案重画*/
		p_graph = &((mcslcanvas *)p)->mcsl_canvas_graphs.head_graph;	/*重绘头结点的内容*/

		XSetForeground(display , p->basic_attr.gc , p_graph->pixel);	/*根据当前颜色设置前景色*/

		if(p_graph->drawing_points.nr_points > 0){
			XDrawPoints(display , p->basic_attr.win , p->basic_attr.gc , p_graph->drawing_points.points , p_graph->drawing_points.nr_points , CoordModeOrigin);
		}
		if(p_graph->drawing_segs.nr_segs > 0){
			XDrawSegments(display , p->basic_attr.win , p->basic_attr.gc , p_graph->drawing_segs.segs ,	p_graph->drawing_segs.nr_segs);
		}
		if(p_graph->drawing_rects.nr_rects > 0){
			XDrawRectangles(display , p->basic_attr.win , p->basic_attr.gc , p_graph->drawing_rects.rects ,	p_graph->drawing_rects.nr_rects);
		}
		if(p_graph->drawing_arcs.nr_arcs > 0){
			XDrawArcs(display , p->basic_attr.win , p->basic_attr.gc , p_graph->drawing_arcs.arcs ,	p_graph->drawing_arcs.nr_arcs);
		}
		if(p_graph->filling_polygon.nr_points > 0){
			XFillPolygon(display , p->basic_attr.win , p->basic_attr.gc , p_graph->filling_polygon.points , p_graph->filling_polygon.nr_points , Complex ,
					CoordModeOrigin);
		}
		if(p_graph->filling_rects.nr_rects > 0){
			XFillRectangles(display , p->basic_attr.win , p->basic_attr.gc , p_graph->filling_rects.rects ,	p_graph->filling_rects.nr_rects);
		}
		if(p_graph->filling_arcs.nr_arcs > 0){
			XFillArcs(display , p->basic_attr.win , p->basic_attr.gc , p_graph->filling_arcs.arcs ,	p_graph->filling_arcs.nr_arcs);
		}

		p_graph = ((mcslcanvas *)p)->mcsl_canvas_graphs.head_graph.next;	/*释放每种颜色所画图像*/
		while(1){
			if(p_graph == ((mcslcanvas *)p)->mcsl_canvas_graphs.empty_graph){
				break;
			}

			XSetForeground(display , p->basic_attr.gc , p_graph->pixel);	/*根据当前颜色设置前景色*/

			if(p_graph->drawing_points.nr_points > 0){
				XDrawPoints(display , p->basic_attr.win , p->basic_attr.gc , p_graph->drawing_points.points , p_graph->drawing_points.nr_points , CoordModeOrigin);
			}
			if(p_graph->drawing_segs.nr_segs > 0){
				XDrawSegments(display , p->basic_attr.win , p->basic_attr.gc , p_graph->drawing_segs.segs ,	p_graph->drawing_segs.nr_segs);
			}
			if(p_graph->drawing_rects.nr_rects > 0){
				XDrawRectangles(display , p->basic_attr.win , p->basic_attr.gc , p_graph->drawing_rects.rects ,	p_graph->drawing_rects.nr_rects);
			}
			if(p_graph->drawing_arcs.nr_arcs > 0){
				XDrawArcs(display , p->basic_attr.win , p->basic_attr.gc , p_graph->drawing_arcs.arcs ,	p_graph->drawing_arcs.nr_arcs);
			}
			if(p_graph->filling_polygon.nr_points > 0){
				XFillPolygon(display , p->basic_attr.win , p->basic_attr.gc , p_graph->filling_polygon.points , p_graph->filling_polygon.nr_points , Complex ,
						CoordModePrevious);
			}
			if(p_graph->filling_rects.nr_rects > 0){
				XFillRectangles(display , p->basic_attr.win , p->basic_attr.gc , p_graph->filling_rects.rects ,	p_graph->filling_rects.nr_rects);
			}
			if(p_graph->filling_arcs.nr_arcs > 0){
				XFillArcs(display , p->basic_attr.win , p->basic_attr.gc , p_graph->filling_arcs.arcs ,	p_graph->filling_arcs.nr_arcs);
			}

			p_graph = p_graph->next;
		}	/*end drawing & filling*/
		break;
	default:
		break;
	}

}
/*处理鼠标按键(左)*/
void cooked_button_press(void *common , void *ev_data){
	mcslobj *p = (mcslobj *)common;
	/*默认动作*/

	p->flags.button_pressed = 1;	/*pressed down*/

	switch(p->type){
	case MCSL_TOPWIN:
		break;
	case MCSL_BUTTON:
		if(p->flags.use_map){	/*要求使用背景图案*/
			XCopyArea(display , ((struct _mcslbutton *)p)->pressed_map , p->basic_attr.win , mcsl_gc , 0 , 0 , p->coordinate.width , p->coordinate.height , 0 , 0);
		}else{
			XSetWindowBackground(display , p->basic_attr.win , p->basic_attr.bgcolor);
		}
		if(p->malloc_attr.content){	/*如果有文字内容*/
			mcsl_draw_string(p , p->malloc_attr.content , strlen(p->malloc_attr.content) , A_CENTER);
		}

		break;
	default:
		break;
	}

	/*用户传入的回调函数*/
	if(p->user_interface.cb_button_press){
		p->user_interface.cb_button_press(p , p->user_interface.user_data[SIG_BUTTON_PRESS]);
	}
}

/*处理鼠标左键释放*/
void cooked_button_release(void *common , void *ev_data){
	mcslobj *p = (mcslobj *)common;
	/*默认动作*/

	p->flags.button_pressed = 0;	/*released*/

	switch(p->type){
	case MCSL_TOPWIN:
		/*do nothing*/
		break;
	case MCSL_BUTTON:
		if(p->flags.use_map){	/*要求使用背景图案*/
			XCopyArea(display , p->basic_attr.bg_map , p->basic_attr.win , mcsl_gc , 0 , 0 , p->coordinate.width , p->coordinate.height , 0 , 0);
		}else{
//			XSetWindowBackground(display , p->basic_attr.win , ((mcslbutton *)p)->pressed_color);
//			XFlush(display);
//			XUnMapWindow()
//			printf("hello\n");
		}
		if(p->malloc_attr.content){	/*如果有文字内容*/
			mcsl_draw_string(p , p->malloc_attr.content , strlen(p->malloc_attr.content) , A_CENTER);
		}

		break;

	default:
		break;
	}

	/*用户传入的回调函数*/
	if(p->user_interface.cb_button_release){
		p->user_interface.cb_button_release(p , p->user_interface.user_data[SIG_BUTTON_RELEASE]);
	}
}

/*鼠标中键按下*/
void cooked_button2_press(void *common , void *ev_data){
	mcslobj *p = (mcslobj *)common;
	/*默认动作*/

	p->flags.button2_pressed = 1;	/*pressed down*/

	switch(p->type){
	case MCSL_TOPWIN:
		break;
	case MCSL_BUTTON:
		if(p->flags.use_map){	/*要求使用背景图案*/
			XCopyArea(display , ((struct _mcslbutton *)p)->pressed_map , p->basic_attr.win , mcsl_gc , 0 , 0 , p->coordinate.width , p->coordinate.height , 0 , 0);
		}
		if(p->malloc_attr.content){	/*如果有文字内容*/
			mcsl_draw_string(p , p->malloc_attr.content , strlen(p->malloc_attr.content) , A_CENTER);
		}

		break;

	default:
		break;
	}

	/*用户传入的回调函数*/
	if(p->user_interface.cb_button2_press){
		p->user_interface.cb_button2_press(p , p->user_interface.user_data[SIG_BUTTON2_PRESS]);
	}
}

/*鼠标中键释放*/
void cooked_button2_release(void *common , void *ev_data){
	mcslobj *p = (mcslobj *)common;
	/*默认动作*/

	p->flags.button2_pressed = 0;	/*released*/

	switch(p->type){
	case MCSL_TOPWIN:
		/*do nothing*/
		break;
	case MCSL_BUTTON:
		if(p->flags.use_map){	/*要求使用背景图案*/
			XCopyArea(display , p->basic_attr.bg_map , p->basic_attr.win , mcsl_gc , 0 , 0 , p->coordinate.width , p->coordinate.height , 0 , 0);
		}
		if(p->malloc_attr.content){	/*如果有文字内容*/
			mcsl_draw_string(p , p->malloc_attr.content , strlen(p->malloc_attr.content) , A_CENTER);
		}

		break;

	default:
		break;
	}

	/*用户传入的回调函数*/
	if(p->user_interface.cb_button2_release){
		p->user_interface.cb_button2_release(p , p->user_interface.user_data[SIG_BUTTON2_RELEASE]);
	}
}

/*鼠标右键按下*/
void cooked_button3_press(void *common , void *ev_data){
	mcslobj *p = (mcslobj *)common;
	/*默认动作*/

	p->flags.button3_pressed = 1;	/*pressed down*/

	switch(p->type){
	case MCSL_TOPWIN:
		break;
	case MCSL_BUTTON:
		if(p->flags.use_map){	/*要求使用背景图案*/
			XCopyArea(display , ((struct _mcslbutton *)p)->pressed_map , p->basic_attr.win , mcsl_gc , 0 , 0 , p->coordinate.width , p->coordinate.height , 0 , 0);
		}
		if(p->malloc_attr.content){	/*如果有文字内容*/
			mcsl_draw_string(p , p->malloc_attr.content , strlen(p->malloc_attr.content) , A_CENTER);
		}

		break;

	default:
		break;
	}

	/*用户传入的回调函数*/
	if(p->user_interface.cb_button3_press){
		p->user_interface.cb_button3_press(p , p->user_interface.user_data[SIG_BUTTON3_PRESS]);
	}
}

/*鼠标右键释放*/
void cooked_button3_release(void *common , void *ev_data){
	mcslobj *p = (mcslobj *)common;
	/*默认动作*/

	p->flags.button3_pressed = 0;	/*released*/

	switch(p->type){
	case MCSL_TOPWIN:
		/*do nothing*/
		break;
	case MCSL_BUTTON:
		if(p->flags.use_map){	/*要求使用背景图案*/
			XCopyArea(display , p->basic_attr.bg_map , p->basic_attr.win , mcsl_gc , 0 , 0 , p->coordinate.width , p->coordinate.height , 0 , 0);
		}
		if(p->malloc_attr.content){	/*如果有文字内容*/
			mcsl_draw_string(p , p->malloc_attr.content , strlen(p->malloc_attr.content) , A_CENTER);
		}


		break;

	default:
		break;
	}

	/*用户传入的回调函数*/
	if(p->user_interface.cb_button3_release){
		p->user_interface.cb_button3_release(p , p->user_interface.user_data[SIG_BUTTON3_RELEASE]);
	}
}

/*键盘按下*/
void cooked_key_press(void *common , void *ev_data){
	int len;
	KeySym keysym;
	char key_str[10];
	XKeyEvent *k;

	XIDeviceEvent *xi_device_ev;
	mcslobj *p = (mcslobj *)common;
	/*默认动作*/
	switch(p->type){
	case MCSL_TOPWIN:
		/*do nothing*/
		break;
	case MCSL_BUTTON:
		if(p->flags.use_map){	/*要求使用背景图案*/
			XCopyArea(display , ((struct _mcslbutton *)p)->pressed_map , p->basic_attr.win , mcsl_gc , 0 , 0 , p->coordinate.width , p->coordinate.height , 0 , 0);
		}
		if(p->malloc_attr.content){	/*如果有文字内容*/
			mcsl_draw_string(p , p->malloc_attr.content , strlen(p->malloc_attr.content) , A_CENTER);
		}
		break;
	case MCSL_INPUT:	/*处理键盘输入*/
		if(p->flags.use_map){	/*使用背景图案*/
			XCopyArea(display , p->basic_attr.bg_map , p->basic_attr.win , mcsl_gc , 0 , 0 , p->coordinate.width , p->coordinate.height , 0 , 0);
		}
		/*获取键盘的按键*/
		memset(key_str , 0 , 10);
		xi_device_ev = (XIDeviceEvent *)ev_data;

		keysym = XKeycodeToKeysym(display , xi_device_ev->detail , 0);	/*vector max to 1*/
		if(keysym == NoSymbol){
			keysym = XKeycodeToKeysym(display , xi_device_ev->detail , 1);
			if(keysym == NoSymbol){
				break;
			}
		}
		/*根据按键内容分类处理*/
		if (((keysym > XK_KP_Space) && (keysym <= XK_KP_9)) || ((keysym > XK_space) && (keysym <= XK_asciitilde))){
			/*普通字符*/
			if(strlen(p->malloc_attr.content) >= ((mcslinput *)p)->size - 1){	/*如果输入框已满。要保留最末尾的0x0*/
				printf("full!\n");
				break;
			}

			XClearWindow(display , p->basic_attr.win);
			strcat(p->malloc_attr.content , XKeysymToString(keysym));

		}else{	/*其他字符或者特殊按键 in X11/keysymdef.h*/
			switch(keysym){
			case XK_space:	/*空格*/
			case XK_KP_Space:
				if(strlen(p->malloc_attr.content) >= ((mcslinput *)p)->size  - 1){	/*如果输入框已满。要保留最末尾的0x0*/
					printf("full!\n");
					break;
				}
				XClearWindow(display , p->basic_attr.win);
				strcat(p->malloc_attr.content , " ");
				break;

			case XK_BackSpace:	/*退格*/
				len = strlen(p->malloc_attr.content);
				if(len > 0){
					len--;
				}
				p->malloc_attr.content[len] = 0;
				XClearWindow(display , p->basic_attr.win);
				break;

			case XK_Delete:
				break;

			case XK_Return:	/*回车*/
			case XK_KP_Enter:
			case XK_Linefeed:
				XClearWindow(display , p->basic_attr.win);
				memset(p->malloc_attr.content , 0 , ((mcslinput *)p)->size);
				break;
			}

		}
		/*绘制文字*/
		if(p->malloc_attr.content){	/*如果有文字内容*/
			mcsl_draw_string(p , p->malloc_attr.content , strlen(p->malloc_attr.content) , A_LEFT);
		}

		break;
	default:
		break;
	}

	/*用户传入的回调函数*/
	if(p->user_interface.cb_key_press){
		p->user_interface.cb_key_press(p , p->user_interface.user_data[SIG_KEY_PRESS]);
	}
}

/*键盘释放*/
void cooked_key_release(void *common , void *ev_data){
	mcslobj *p = (mcslobj *)common;
	/*默认动作*/
	switch(p->type){
	case MCSL_TOPWIN:
		/*do nothing*/
		break;
	case MCSL_BUTTON:
		if(p->flags.use_map){	/*要求使用背景图案*/
			XCopyArea(display , p->basic_attr.bg_map , p->basic_attr.win , mcsl_gc , 0 , 0 , p->coordinate.width , p->coordinate.height , 0 , 0);
		}
		if(p->malloc_attr.content){	/*如果有文字内容*/
			mcsl_draw_string(p , p->malloc_attr.content , strlen(p->malloc_attr.content) , A_CENTER);
		}
		break;


	default:
		break;
	}

	/*用户传入的回调函数*/
	if(p->user_interface.cb_key_release){
		p->user_interface.cb_key_release(p , p->user_interface.user_data[SIG_KEY_RELEASE]);
	}
}

/*指针移动*/
void cooked_motion(void *common , void *ev_data){
	XIDeviceEvent *xi_device_ev;
	xi_device_ev = (XIDeviceEvent *)ev_data;

	mcslobj *p = (mcslobj *)common;
	/*默认动作*/
	switch(p->type){
	case MCSL_TOPWIN:
		/*do nothing*/
		break;
	case MCSL_BUTTON:

		p->basic_attr.cursor_x = (mcsl_u32)xi_device_ev->event_x;
		p->basic_attr.cursor_y = (mcsl_u32)xi_device_ev->event_y;
//		printf("it is:%d x %d\n" , p->basic_attr.cursor_x , p->basic_attr.cursor_y);
		break;

	default:
		p->basic_attr.cursor_x = (mcsl_u32)xi_device_ev->event_x;
		p->basic_attr.cursor_y = (mcsl_u32)xi_device_ev->event_y;
		break;
	}

	/*用户传入的回调函数*/
	if(p->user_interface.cb_motion){
		p->user_interface.cb_motion(p , p->user_interface.user_data[SIG_MOTION]);
	}
}

/*指针进入*/
void cooked_enter(void *common , void *ev_data){
	mcslobj *p = (mcslobj *)common;
	/*默认动作*/
	switch(p->type){
	case MCSL_TOPWIN:
		/*do nothing*/
		break;
	case MCSL_BUTTON:
//		printf("enter\n");
//		p->basic_attr.bordercolor = colors[BLACK];
		XSetWindowBorder(display , p->basic_attr.win , colors[MEDIUM_AQUAMARINE]);	/*光标进入按钮默认边框变色*/
		break;

	default:
		break;
	}

	/*用户传入的回调函数*/
	if(p->user_interface.cb_enter){
		p->user_interface.cb_enter(p , p->user_interface.user_data[SIG_ENTER]);
	}
}

/*指针移出*/
void cooked_leave(void *common , void *ev_data){
	mcslobj *p = (mcslobj *)common;
	/*默认动作*/
	switch(p->type){
	case MCSL_TOPWIN:
		/*do nothing*/
		break;
	case MCSL_BUTTON:
//		printf("leave\n");
		XSetWindowBorder(display , p->basic_attr.win , p->basic_attr.bordercolor);	/*光标离开按钮默认边框恢复*/
		break;

	default:
		break;
	}

	/*用户传入的回调函数*/
	if(p->user_interface.cb_leave){
		p->user_interface.cb_leave(p , p->user_interface.user_data[SIG_LEAVE]);
	}
}


////////////////////公共外部接口函数//////////////////////////////////////////
/*whether widget is pressed*/
int mcsl_ispressed(mcslobj *this , mcsl_u32 type_button){
	if(!this){
		return -1;
	}

	switch(type_button){
	case BUTTON1_PRESSED:
		return this->flags.button_pressed;
		break;
	case BUTTON2_PRESSED:
		return this->flags.button2_pressed;
		break;
	case BUTTON3_PRESSED:
		return this->flags.button3_pressed;
		break;
	default:
		return this->flags.button_pressed;
		break;
	}

}
/*get coordinate*/
int mcsl_get_coordinate(mcslobj *this , mcsl_u32 *x , mcsl_u32 *y , mcsl_u32 *width , mcsl_u32 *height , mcsl_u32 *border){
	if(!this){
		return -1;
	}

	if(x){
		*x = this->coordinate.x;
	}

	if(y){
		*y = this->coordinate.y;
	}

	if(width){
		*width = this->coordinate.width;
	}

	if(height){
		*height = this->coordinate.height;
	}

	if(border){
		*border = this->coordinate.border;
	}
	return 0;
}

/*fetch coordinate of cursor*/
int mcsl_fetch_cursor(mcslobj *this , mcsl_u32 *cursor_x , mcsl_u32 *cursor_y){
	if(!this || !cursor_x || !cursor_y){
		return -1;
	}

	*cursor_x = this->basic_attr.cursor_x;
	*cursor_y = this->basic_attr.cursor_y;

	return 0;
}

/*move this widget*/
int mcsl_move(mcslobj *this , mcsl_u32 x , mcsl_u32 y){
	if(!this){
		return -1;
	}

	this->coordinate.x = x;
	this->coordinate.y = y;

	return XMoveWindow(display , this->basic_attr.win , x , y);
}

/*resize the widget*/
int mcsl_resize(mcslobj *this , mcsl_u32 width , mcsl_u32 height){
	if(!this){
		return -1;
	}

	/*根据控件类型设置。如果使用背景图那么控件大小不能超过背景图的最大尺寸。如果不使用背景图那么可以随意设置*/
	switch(this->type){
	case MCSL_TOPWIN:
		if(this->flags.use_map){
			this->coordinate.width = (width>mcsl_pixmaps[MCSL_TOPWIN_PX].map_width)?mcsl_pixmaps[MCSL_TOPWIN_PX].map_width:width;
			this->coordinate.height = (height>mcsl_pixmaps[MCSL_TOPWIN_PX].map_height)?mcsl_pixmaps[MCSL_TOPWIN_PX].map_height:height;
		}else{
			this->coordinate.width = width;
			this->coordinate.height = height;
		}

	case MCSL_BUTTON:
		if(this->flags.use_map){
			this->coordinate.width = (width>mcsl_pixmaps[MCSL_BUTTON_PX].map_width)?mcsl_pixmaps[MCSL_BUTTON_PX].map_width:width;
			this->coordinate.height = (height>mcsl_pixmaps[MCSL_BUTTON_PX].map_height)?mcsl_pixmaps[MCSL_BUTTON_PX].map_height:height;
		}else{
			this->coordinate.width = width;
			this->coordinate.height = height;
		}
		break;
	default:
		this->coordinate.width = width;
		this->coordinate.height = height;
		break;
	}

	return XResizeWindow(display , this->basic_attr.win , this->coordinate.width , this->coordinate.height);
}

/*set color
 * 颜色格式为#RRGGBB的字符串。具体颜色查询颜色表
 * @param0:前景色RGB
 * @param1:背景色RGB
 * @param3:边框RGB
 */
int mcsl_set_color(mcslobj *this , char *fore_RGB , char *bg_RGB , char *border_RGB){
	Colormap map;
	XColor color;

	if(!this){
		return -1;
	}

	map = XDefaultColormap(display , screen_num);

	if(fore_RGB != NULL){
		memset(&color , 0 , sizeof(XColor));
		XParseColor(display , map , fore_RGB , &color);
		XAllocColor(display , map , &color);
		this->basic_attr.forecolor = color.pixel;
		XSetForeground(display , this->basic_attr.gc , color.pixel);	/*对于普通控件改变前景色将影响其他控件的前景色*/
	}

	if(bg_RGB != NULL){
		memset(&color , 0 , sizeof(XColor));
		XParseColor(display , map , bg_RGB , &color);
		XAllocColor(display , map , &color);
		this->basic_attr.bgcolor = color.pixel;
		this->flags.use_map = 0;	/*不再使用背景图 而使用背景色*/

		XSetWindowBackground(display , this->basic_attr.win , color.pixel);
	}

	if(border_RGB != NULL){
		memset(&color , 0 , sizeof(XColor));
		XParseColor(display , map , border_RGB , &color);
		XAllocColor(display , map , &color);
		this->basic_attr.bordercolor = color.pixel;
		XSetWindowBorder(display , this->basic_attr.win , color.pixel);
	}

	return 0;
}

/*change backgroud map*/
int mcsl_set_bgmap(mcslobj *this , char **bg_xpm){
	mcsl_u8 type;

	if(!this){
		return -1;
	}
/*
	switch(this->type){
	case MCSL_TOPWIN:
		type = MCSL_TOPWIN_PX;
		break;
	case MCSL_BUTTON:
		type = MCSL_BUTTON_PX;
		break;
	case MCSL_LABEL:
		type = MCSL_LABEL_PX;
		break;
	case MCSL_INPUT:
		type = MCSL_INPUT_PX;
		break;
	case MCSL_CANVAS:
		type = MCSL_CANVAS_PX;
		break;
	default:
		return -1;
	}
*/

	this->flags.use_map = 1;	/*使用背景图案*/

	/*set bgmap*/
	memset(&mcsl_xpmattr, 0, sizeof(mcsl_xpmattr));
	XpmCreatePixmapFromData(display, mcsl_topwin, bg_xpm , &this->basic_attr.bg_map , NULL, &mcsl_xpmattr);
	printf("it is:%d x %d\n" , mcsl_xpmattr.width , mcsl_xpmattr.height);

	return 0;
}

/*设置控件字体*/
int mcsl_set_font(mcslwidget *this , char *font_family , char *weights , char *slants , char *point_size){
	XFontStruct *font_info;
	font_info = load_font(font_family , weights , slants , point_size);

	if(font_info){	/*加载字体成功*/
		this->flags.new_font = 1;	/*使用新字体*/
		this->basic_attr.font_info = font_info;
		return 0;
	}

	return -1;

}


/*使控件可见*/
 int mcsl_visible(mcslwidget *this){
	 this->flags.visible = 1;
	 return XMapWindow(display , this->basic_attr.win);
 }
 /*隐藏控件*/
int mcsl_hidden(mcslwidget *this){
	 this->flags.visible = 0;
	 return XUnmapWindow(display , this->basic_attr.win);
}


/*为控件选取的鼠标ID*/
int mcsl_choose_pointer(mcslobj *this , mcsl_u8 pointer_id){
	int device_id;

	if(pointer_id > MAX_POINTERS){
		pointer_id = ANYPOINTER;
	}

	if(this->flags.chosen_keyboard){	/*在XI中特定键盘和光标不可兼得*/
		return -1;
	}

	this->flags.chosen_pointer = pointer_id;

	XIEventMask eventmask;
	unsigned char mask[2] = {0 , 0};

	device_id = get_cursor(p_device , this->basic_attr.win , pointer_id);

	if(device_id != -1){

		memset(mask , 0 , sizeof(mask));	/*reattach new cursor to a window registered*/
		eventmask.deviceid = device_id;

		eventmask.mask_len = sizeof(mask);
		eventmask.mask = mask;

		XISetMask(mask , XI_ButtonPress);
		XISetMask(mask , XI_ButtonRelease);
		XISetMask(mask , XI_KeyPress);
		XISetMask(mask , XI_KeyRelease);
		XISetMask(mask , XI_Motion);
		XISetMask(mask , XI_Enter);
		XISetMask(mask , XI_Leave);

		XISelectEvents(display , this->basic_attr.win , &eventmask , 1);

		/*set cursor shape when entering window appended:2012.5*/
		if(pointer_id != ANYPOINTER){
//			printf("I am here~ deamon\n");
			XIDefineCursor(display,device_id , this->basic_attr.win , XCreateFontCursor(display, cursor_shape[pointer_id]));
		}
		return 0;
	}

	return -1;
}

/*为控件选取的键盘ID*/
int mcsl_choose_keyboard(mcslobj *this , mcsl_u8 keyboard_id){
	int device_id;

	if(keyboard_id > MAX_KEYBOARDS){
		keyboard_id = ANYKEYBOARD;
	}

	if(this->flags.chosen_pointer){	/*在XI中特定键盘和光标不可兼得*/
		return -1;
	}

	this->flags.chosen_keyboard = keyboard_id;

	XIEventMask eventmask;
	unsigned char mask[2] = {0 , 0};

	device_id = get_keyboard(this->basic_attr.win , keyboard_id);

	if(device_id != -1){

		memset(mask , 0 , sizeof(mask));	/*reattach new cursor to a window registered*/
		eventmask.deviceid = device_id;

		eventmask.mask_len = sizeof(mask);
		eventmask.mask = mask;

		XISetMask(mask , XI_ButtonPress);
		XISetMask(mask , XI_ButtonRelease);
		XISetMask(mask , XI_KeyPress);
		XISetMask(mask , XI_KeyRelease);
		XISetMask(mask , XI_Motion);
		XISetMask(mask , XI_Enter);
		XISetMask(mask , XI_Leave);
		XISetMask(mask , XI_FocusIn);
		XISetMask(mask , XI_FocusOut);

		XISelectEvents(display , this->basic_attr.win , &eventmask , 1);
		return 0;
	}

	return -1;
}

//#define NOSIG				00000000
//#define PRESS_DOWN	00000001

/*根据信号链接用户回调函数*/
int mcsl_attach_callback(mcslobj *this , CALLBACK user_function , void *data , mcsl_u8 sig_type){
	if(!this){
		return -1;
	}
	if(!user_function){
		return -1;
	}

	switch(sig_type){
	case NOSIG:
		return -1;
		break;

	case SIG_BUTTON_PRESS:
		this->user_interface.user_data[SIG_BUTTON_PRESS] = data;
		this->user_interface.cb_button_press = user_function;
		break;

	case SIG_BUTTON_RELEASE:
		this->user_interface.user_data[SIG_BUTTON_RELEASE] = data;
		this->user_interface.cb_button_release = user_function;
		break;

	case SIG_BUTTON2_PRESS:
		this->user_interface.user_data[SIG_BUTTON2_PRESS] = data;
		this->user_interface.cb_button2_press = user_function;
		break;

	case SIG_BUTTON2_RELEASE:
		this->user_interface.user_data[SIG_BUTTON2_RELEASE] = data;
		this->user_interface.cb_button2_release = user_function;
		break;

	case SIG_BUTTON3_PRESS:
		this->user_interface.user_data[SIG_BUTTON3_PRESS] = data;
		this->user_interface.cb_button3_press = user_function;
		break;

	case SIG_BUTTON3_RELEASE:
		this->user_interface.user_data[SIG_BUTTON3_RELEASE] = data;
		this->user_interface.cb_button3_release = user_function;
		break;

	case SIG_KEY_PRESS:
		this->user_interface.user_data[SIG_KEY_PRESS] = data;
		this->user_interface.cb_key_press = user_function;
		break;

	case SIG_KEY_RELEASE:
		this->user_interface.user_data[SIG_KEY_RELEASE] = data;
		this->user_interface.cb_key_release = user_function;
		break;

	case SIG_MOTION:
		this->user_interface.user_data[SIG_MOTION] = data;
		this->user_interface.cb_motion = user_function;
		break;

	case SIG_ENTER:
		this->user_interface.user_data[SIG_ENTER] = data;
		this->user_interface.cb_enter = user_function;
		break;

	case SIG_LEAVE:
		this->user_interface.user_data[SIG_LEAVE] = data;
		this->user_interface.cb_leave = user_function;
		break;

	default:
		break;
	}

}

