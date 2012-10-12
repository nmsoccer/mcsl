/*
 * mcslbox.c
 *
 *  Created on: 2011-3-28
 *      Author: leiming
 */
#include "mcsltypes.h"
#include "mcslglobal.h"
#include "mcslbox.h"


static	int attach(mcslbox *box , mcslobj *parent);	/*将容器添加到父控件中。一个控件最多一个容器*/

static	int nested(mcslbox *child_box , mcsl_u8 float_to , mcsl_u16 float_distance , mcslbox *parent_box);	/*嵌套容器 如果child_box为空等效于移除内层容器*/
static	int de_nest(mcslbox *parent_box);	/*将内层容器移除*/

static	int append(mcslbox *box , mcslobj *widget);	/*往容器里添加控件*/
static	int remove(mcslbox *box , mcslobj *widget);	/*从容器里移除控件*/


/*
 * 初始化一个容器。
 * @param0:布局类型。
 * @param1:是否控件大小齐整
 * @param2:控件组距离父控件的边距。具体情况由控件类型决定.
 * @param3:控件组中控件相隔距离
 * @param4:控件依附的父容器
 * @param5:相对于父容器内控件组的偏移方向
 * @param6:相对于父容器内控件组的偏移距离
 */
mcslbox *new_mcslbox(mcsl_u8 type , mcsl_u8 homogeneous , mcsl_u16 margin , mcsl_u16 space , mcslbox *parent_box ,
										mcsl_u8 float_to , mcsl_u16 float_distance){
	mcslbox *pbox;

	pbox = (mcslbox *)malloc(sizeof(mcslbox));
	memset(pbox , 0 , sizeof(mcslbox));

	pbox->type = type;
	pbox->homogeneous = homogeneous;
	pbox->margin = margin;
	pbox->space = space;

	if(parent_box){	/*如果父容器存在*/
		parent_box->float_to = float_to;
		parent_box->float_distance = float_distance;
		parent_box->in_box = pbox;	/*原来父容器内如果有内嵌的容器将被覆盖*/
		pbox->parent_widget = parent_box->parent_widget;	/*父容器的依赖控件也是子容器的依赖控件*/
	}else{
		pbox->parent_widget = NULL;
	}


	pbox->float_to = 0;
	pbox->float_distance = 0;

	pbox->head.widget = NULL;
	pbox->head.next = NULL;

	pbox->first = &pbox->head;
	pbox->last = pbox->first;	/*last始终指向一个空的box_content*/

	pbox->nr_widgets = 0;
	pbox->max_width = 0;
	pbox->max_height = 0;


	/*set public interface*/
	pbox->attach = attach;
	pbox->nested = nested;
	pbox->de_nest = de_nest;
	pbox->append = append;
	pbox->remove = remove;

	return pbox;
}


/*
 * 删除box
 * 递归删除盒子
 */
int delete_mcslbox(mcslbox *pbox){
	box_content *p;
	box_content *temp;

	if(pbox->parent_widget->basic_attr.container == pbox){
		pbox->parent_widget->basic_attr.container = NULL;
	}

	p = pbox->first->next;	/*删除一个盒子里的所有包含控件信息链*/
	while(p){
		temp = p->next;
		free(p);
		p = temp;
	}
	/*
	 * 如果有子盒子递归删除之
	 */
	if(pbox->in_box){
		delete_mcslbox(pbox->in_box);
	}

	free(pbox);

	return 0;
}



/*
 * 对box里的控件进行布局
 * @param0:将要布局的box
 * @param1~3:box可以利用的空间
 * @param4~7:返回设置控件组后可以利用的空间
 */
int arrange_mcslbox(mcslbox *box , mcsl_u32 s_x , mcsl_u32 s_y , mcsl_u32 s_width , mcsl_u32 s_height , mcsl_u32 *r_x ,
										mcsl_u32 *r_y , mcsl_u32 *r_width , mcsl_u32 *r_height){

	mcsl_u32 var_x , var_y , var_w , var_h;	/*跟踪容器组的位置*/
	mcsl_u32 x , y , width , height;				/*用于设定可能裁剪过后用于排列控件的新空间的坐标(完全用于安装控件)*/
	box_content *p;


	if(!box){
		return -1;
	}


	/*如果要求控件齐整*/
	if(box->homogeneous){
		p = box->first;
		while(p){
			if(!p->widget){
				break;
			}

			p->widget->coordinate.width = box->max_width;
			p->widget->coordinate.height = box->max_height;
			XResizeWindow(display , p->widget->basic_attr.win , (mcsl_u32)box->max_width , (mcsl_u32)box->max_height);

			p = p->next;
		}
	}	/*end homo*/



	/*拥有控件组*/
	x = s_x;
	y = s_y;
	width = s_width;
	height = s_height;

	switch(box->type){
	case VBOX_BOTTOM:
		/*容器内没有控件*/
		/*器内没有控件那么里层容器(如果有)的空间只受到margin影响(距离最下边距).*/
		if(box->nr_widgets == 0){

			*r_x = s_x;
			*r_y = s_y;
			*r_width = s_width;
			*r_height = s_height - box->margin;

			return 0;
		}


		/*容器具有控件 没有内层容器*/
		if(!box->in_box){
			*r_x = 0;	/*因为没有子容器所以返回的剩余空间为0*/
			*r_y = 0;
			*r_width = 0;
			*r_height = 0;

			/*计算控件组的总高度*/
			var_h = 0 + box->margin + (box->nr_widgets - 1) * box->space;
			p = box->first;
			while(p){
				if(!p->widget){
					break;
				}
				var_h += p->widget->coordinate.height;
				p = p->next;
			}

			x = s_x;	/*获取完全属于控件的空间*/
			y = s_y;
			width = s_width;
			height = s_height;

			var_y = y + s_height - var_h;/*第一个控件初始位置。*/
			goto v_arrange;
		}


		/*既有内层容器又有控件组.跳往专门安排水平控件组与内层容器的过程。因为无论靠左还是靠右方法都差不多*/
		goto arrange_vbox_container;
		break;


	case VBOX_TOP:
		/*容器内没有控件*/
		/*器内没有控件那么里层容器(如果有)的空间只受到margin影响(距离最上边距).*/
		if(box->nr_widgets == 0){

			*r_x = s_x;
			*r_y = s_y + box->margin;
			*r_width = s_width;
			*r_height = s_height - box->margin;

			return 0;
		}


		/*容器具有控件 没有内层容器*/
		if(!box->in_box){
			*r_x = 0;	/*因为没有子容器所以返回的剩余空间为0*/
			*r_y = 0;
			*r_width = 0;
			*r_height = 0;

			x = s_x;	/*获取完全属于控件的空间*/
			y = s_y;
			width = s_width;
			height = s_height;

			var_y = y + box->margin;/*第一个控件初始位置。此处margin认为距上边距离*/

			goto v_arrange;
		}


		/*既有内层容器又有控件组*/

arrange_vbox_container:
		switch(box->float_to){	/*内层容器相对控件组的偏移*/

		case FLOAT_TOP:	/*容器偏移在控件组上边、下边需要计算控件组的总高度*/
		case FLOAT_BOTTOM:
			/*计算控件组的总高度*/
			var_h = 0 + box->margin + (box->nr_widgets - 1) * box->space;

			p = box->first;
			while(p){
				if(!p->widget){
					break;
				}
				var_h += p->widget->coordinate.height;
				p = p->next;
			}

			if(box->float_to == FLOAT_TOP){	/*容器在控件组上边*/
				*r_x = s_x;
				*r_y = s_y;
				*r_width = s_width;
				*r_height = s_height - var_h - box->float_distance;

				x = s_x;	/*获取适用于安排控件的新空间*/
				y = s_y + *r_height + box->float_distance;
				width = s_width;
				height = var_h;

				/*计算第一个控件在控件组空间里的初始位置*/
				if(box->type == VBOX_TOP){/*VBOX_TOP*/
					var_y = y + box->margin;/*第一个控件初始位置。此处margin认为距上边距离*/
				}else{	/*VBOX_BOTTOM*/
					var_y = y;/*第一个控件初始位置。此处margin认为最末控件距下边距离*/
				}

			}else{	/*容器在控件组下边*/
				*r_x = s_x;
				*r_y = s_y + var_h + box->float_distance;
				*r_width = s_width;
				*r_height = s_height - var_h - box->float_distance;

				x = s_x;	/*获取适用于安排控件的新空间*/
				y = s_y;
				width = s_width;
				height = var_h;

				/*计算第一个控件在控件组空间里的初始位置*/
				if(box->type == VBOX_TOP){/*VBOX_TOP*/
					var_y = y + box->margin;/*第一个控件初始位置。此处margin认为距上边距离*/
				}else{	/*VBOX_BOTTOM*/
					var_y = y;/*第一个控件初始位置。此处margin认为最末控件距下边距离*/
				}

			}

			goto v_arrange;

		case FLOAT_LEFT:	/*如果容器在控件组左面*/
			*r_x = s_x;
			*r_y = s_y;
			*r_width = s_width - box->max_width - box->float_distance;
			*r_height = s_height;

			x = s_x + *r_width + box->float_distance;	/*获取控件组空间*/
			y = s_y;
			width = box->max_width;
			height = s_height;

			/*计算第一个控件在控件组空间里的初始位置*/
			if(box->type == VBOX_TOP){/*VBOX_TOP*/
				var_y = y + box->margin;/*第一个控件初始位置。此处margin认为距上边距离*/
			}else{	/*VBOX_BOTTOM*/
				var_h = 0 + box->margin + (box->nr_widgets - 1) * box->space;
				p = box->first;
				while(p){
					if(!p->widget){
						break;
					}
					var_h += p->widget->coordinate.height;
					p = p->next;
				}
				var_y = y + height - var_h;/*第一个控件初始位置。此处margin认为最末控件距下边距离*/
			}

			goto v_arrange;

		case FLOAT_RIGHT:	/*容器在控件组右面*/
			*r_x = s_x + box->max_width + box->float_distance;
			*r_y = s_y;
			*r_width = s_width - box->max_width - box->float_distance;
			*r_height = s_height;


			x = s_x;	/*获取完全属于控件的空间*/
			y = s_y;
			width = box->max_width;
			height = s_height;

			/*计算第一个控件在控件组空间里的初始位置*/
			if(box->type == VBOX_TOP){/*VBOX_TOP*/
				var_y = y + box->margin;/*第一个控件初始位置。此处margin认为距上边距离*/
			}else{	/*VBOX_BOTTOM*/
				var_h = 0 + box->margin + (box->nr_widgets - 1) * box->space;
				p = box->first;
				while(p){
					if(!p->widget){
						break;
					}
					var_h += p->widget->coordinate.height;
					p = p->next;
				}
				var_y = y + height - var_h;/*第一个控件初始位置。此处margin认为最末控件距下边距离*/
			}

			goto v_arrange;

		default:
			printf("error!\n");
			return -1;
			break;
		}	/*end switch box->float_to*/
v_arrange:	/*在完全属于控件的空间里从第一个控件开始竖直放置控件*/

		p = box->first;
		while(p){
			if(!p->widget){
				break;
			}

			var_x = x + (width - p->widget->coordinate.width) / 2;	/*每当新遇到一个控件时依据控件大小来更新y坐标*/

			p->widget->coordinate.x = var_x;	/*重新设置每个控件的位置*/
			p->widget->coordinate.y = var_y;
			XMoveWindow(display , p->widget->basic_attr.win , var_x , var_y);

			var_y += p->widget->coordinate.height + box->space;	/*在设定控件之后更新x坐标*/

			p = p->next;
		}

		return 0;


///////////////////////////////////////////////////////////////////////////

	case HBOX_RIGHT:
		/*容器内没有控件*/
		/*器内没有控件那么里层容器(如果有)的空间只受到margin影响(距离最右边距).*/
		if(box->nr_widgets == 0){

			*r_x = s_x;
			*r_y = s_y;
			*r_width = s_width  - box->margin;
			*r_height = s_height;

			return 0;
		}


		/*容器具有控件 没有内层容器*/
		if(!box->in_box){
			*r_x = 0;	/*因为没有子容器所以返回的剩余空间为0*/
			*r_y = 0;
			*r_width = 0;
			*r_height = 0;

			/*计算控件组的总长度*/
			var_w = 0 + box->margin + (box->nr_widgets - 1) * box->space;
			p = box->first;
			while(p){
				if(!p->widget){
					break;
				}
				var_w += p->widget->coordinate.width;
				p = p->next;
			}

			x = s_x;	/*获取完全属于控件的空间*/
			y = s_y;
			width = s_width;
			height = s_height;

			var_x = x + s_width - var_w;/*第一个控件初始位置。*/
			goto h_arrange;
		}


		/*既有内层容器又有控件组.跳往专门安排水平控件组与内层容器的过程。因为无论靠左还是靠右方法都差不多*/
		goto arrange_hbox_container;

		break;


	case HBOX_LEFT:
	default:
		/*容器内没有控件*/
		/*器内没有控件那么里层容器(如果有)的空间只受到margin影响(距离最左边距).*/
		if(box->nr_widgets == 0){

			*r_x = s_x + box->margin;
			*r_y = s_y;
			*r_width = s_width  - box->margin;
			*r_height = s_height;

			return 0;
		}

		/*容器具有控件 没有内层容器*/
		if(!box->in_box){
			*r_x = 0;	/*因为没有子容器所以返回的剩余空间为0*/
			*r_y = 0;
			*r_width = 0;
			*r_height = 0;

			x = s_x;	/*获取完全属于控件的空间*/
			y = s_y;
			width = s_width;
			height = s_height;

			var_x = x + box->margin;/*第一个控件初始位置。此处margin认为距左边距离*/

			goto h_arrange;
		}

		/*既有内层容器又有控件组*/

arrange_hbox_container:
		switch(box->float_to){	/*内层容器相对控件组的偏移*/

		case FLOAT_LEFT:	/*容器偏移在控件组左边、右边需要计算控件组的总宽度*/
		case FLOAT_RIGHT:
			/*计算控件组的总长度*/
			var_w = 0 + box->margin + (box->nr_widgets - 1) * box->space;

			p = box->first;
			while(p){
				if(!p->widget){
					break;
				}
				var_w += p->widget->coordinate.width;
				p = p->next;
			}

			if(box->float_to == FLOAT_LEFT){	/*容器在控件组左边*/
				*r_x = s_x;
				*r_y = s_y;
				*r_width = s_width - var_w - box->float_distance;	/*剩余宽度是总宽度减去控件组的宽度*/
				*r_height = s_height;

				x = s_x + *r_width + box->float_distance;	/*获取适用于安排控件的新空间*/
				y = s_y;
				width = var_w;
				height = s_height;
//				printf("box2 widgets_space:  x:%d , y: %d , width: %d , height: %d\n" , x , y , width , height);

				/*计算第一个控件在控件组空间里的初始位置*/
				if(box->type == HBOX_LEFT){/*HBOX_LEFT*/
					var_x = x + box->margin;/*第一个控件初始位置。此处margin认为距左边距离*/
				}else{	/*HBOX_RIGHT*/
					var_x = x;/*第一个控件初始位置。此处margin认为最末控件距右边距离*/
				}

			}else{	/*容器在控件组右边*/
				*r_x = s_x + var_w + box->float_distance;
				*r_y = s_y;
				*r_width = s_width - var_w - box->float_distance;
				*r_height = s_height;

				x = s_x;	/*获取适用于安排控件的新空间*/
				y = s_y;
				width = var_w;
				height = s_height;
//				printf("box2 widgets_space:  x:%d , y: %d , width: %d , height: %d\n" , x , y , width , height);

				/*计算第一个控件在控件组空间里的初始位置*/
				if(box->type == HBOX_LEFT){/*HBOX_LEFT*/
					var_x = x + box->margin;/*第一个控件初始位置。此处margin认为距左边距离*/
				}else{	/*HBOX_RIGHT*/
					var_x = x;/*第一个控件初始位置。此处margin认为最末控件距右边距离*/
				}

			}

			goto h_arrange;

		case FLOAT_TOP:	/*如果容器在控件组上面*/
			*r_x = s_x;
			*r_y = s_y;
			*r_width = s_width;
			*r_height = s_height - box->max_height - box->float_distance;

			x = s_x;	/*获取控件组空间*/
			y = s_y + *r_height + box->float_distance;
			width = s_width;
			height = box->max_height;
//			printf("box2 widgets_space:  x:%d , y: %d , width: %d , height: %d\n" , x , y , width , height);

			/*计算第一个控件在控件组空间里的初始位置*/
			if(box->type == HBOX_LEFT){/*HBOX_LEFT*/
				var_x = x + box->margin;/*第一个控件初始位置。此处margin认为距左边距离*/
			}else{	/*HBOX_RIGHT*/
				var_w = 0 + box->margin + (box->nr_widgets - 1) * box->space;
				p = box->first;
				while(p){
					if(!p->widget){
						break;
					}
					var_w += p->widget->coordinate.width;
					p = p->next;
				}
				var_x = x + width - var_w;/*第一个控件初始位置。此处margin认为最末控件距右边距离*/
			}

			goto h_arrange;

		case FLOAT_BOTTOM:	/*容器在控件组下面*/
			*r_x = s_x;
			*r_y = s_y + box->max_height + box->float_distance;
			*r_width = s_width;
			*r_height = s_height - box->max_height - box->float_distance;


			x = s_x;	/*获取完全属于控件的空间*/
			y = s_y;
			width = s_width;
			height = box->max_height;
//			printf("box2 widgets_space:  x:%d , y: %d , width: %d , height: %d\n" , x , y , width , height);

			/*计算第一个控件在控件组空间里的初始位置*/
			if(box->type == HBOX_LEFT){/*HBOX_LEFT*/
				var_x = x + box->margin;/*第一个控件初始位置。此处margin认为距左边距离*/
			}else{	/*HBOX_RIGHT*/
				var_w = 0 + box->margin + (box->nr_widgets - 1) * box->space;
				p = box->first;
				while(p){
					if(!p->widget){
						break;
					}
					var_w += p->widget->coordinate.width;
					p = p->next;
				}
				var_x = x + width - var_w;/*第一个控件初始位置。此处margin认为最末控件距右边距离*/
			}

			goto h_arrange;

		default:
			printf("error!\n");
			return -1;
			break;
		}	/*end switch box->float_to*/


h_arrange:		/*在完全属于控件的空间里从第一个控件开始水平放置控件*/
		p = box->first;
		while(p){
			if(!p->widget){
				break;
			}

			var_y = y + (height - p->widget->coordinate.height) / 2;	/*每当新遇到一个控件时依据控件大小来更新y坐标*/

			p->widget->coordinate.x = var_x;	/*重新设置每个控件的位置*/
			p->widget->coordinate.y = var_y;
			XMoveWindow(display , p->widget->basic_attr.win , var_x , var_y);

			var_x += p->widget->coordinate.width + box->space;	/*在设定控件之后更新x坐标*/

			p = p->next;
		}

		return 0;



	}	/*end switch box->type*/

}



/*
 * ###################################
 * 用来实现容器外部接口的内部函数实现
 */
/*将容器添加到父控件中。一个控件最多一个容器*/
static int attach(mcslbox *box , mcslobj *parent){
	mcslbox *p;
	if(!box){
		return -1;
	}

	box->parent_widget = parent;
	parent->basic_attr.container = box;

	/*将该容器的所有子容器的依赖控件都设置*/
	p = box->in_box;
	while(p){
		p->parent_widget = parent;
		p = p->in_box;
	}

	return 0;
}

/*将子容器嵌套入父容器
 * @param1:子容器相对于父容器控件组的浮动方向
 * @param2:子容器与父容器组的距离
 */
static int nested(mcslbox *child_box , mcsl_u8 float_to , mcsl_u16 float_distance , mcslbox *parent_box){
	if(!parent_box){
		return -1;
	}

	parent_box->float_to = float_to;
	parent_box->float_distance = float_distance;

	parent_box->in_box = child_box;	/*原来如果有内嵌的容器将被覆盖*/
	child_box->parent_widget = parent_box->parent_widget;	/*父容器的依赖控件也是子容器的依赖控件*/
	return 0;
}

/*将内层容器移除*/
static int de_nest(mcslbox *parent_box){
	if(!parent_box){
		return -1;
	}

	parent_box->in_box = NULL;
	return 0;
}

/*往容器里添加控件*/
static int append(mcslbox *box , mcslobj *widget){
	box_content *p;
	box_content *new;

	if(!box){
		return -1;
	}

	/*容器必须先依附于某控件才能添加控件*/
	if(!box->parent_widget){
		printf("box should be attached to widget first!\n");
		return -1;
	}

	/*如果容器依附的父控件与目标控件的父控件必须一致*/
	if(box->parent_widget->basic_attr.win != widget->basic_attr.parent_win){
		printf("box parent widget is not the same as the widget has\n");
		return -1;
	}

	p = box->first;	/*判定控件是否已经存在*/
	while(p){
		if(!p->widget){	/*如果box_content所包含控件为空表示已经到了最后一个*/
			break;
		}

		if(p->widget->basic_attr.win == widget->basic_attr.win){	/*已经存在控件*/
			return -1;
		}

		p = p->next;

	}

	/*控件不存在可以添加*/
	new = (box_content *)malloc(sizeof(box_content));
	new->widget = NULL;
	new->next = NULL;


	box->last->widget = widget;
	box->last->next = new;
	box->last = new;

	/*修改相关参数*/
	box->nr_widgets++;

	if(widget->coordinate.width > box->max_width){
		box->max_width = widget->coordinate.width;
	}

	if(widget->coordinate.height > box->max_height){
		box->max_height = widget->coordinate.height;
	}


	return 0;
}

/*从容器里移除控件*/
static int remove(mcslbox *box , mcslobj *widget){
	box_content *p;
	box_content *next;

	if(!box){
		return -1;
	}

	p = box->first;	/*判定控件是否存在*/
	while(p){
		if(!p->widget){	/*如果box_content所包含控件为空表示已经到了最后一个*/
			break;
		}

		if(p->widget->basic_attr.win == widget->basic_attr.win){	/*找到控件*/
			next = p->next;

			/*交换p与p->next的内容。清除p->next*/
			p->widget = p->next->widget;
			p->next = p->next->next;
			free(next);
			return 0;
		}

		p = p->next;

	}

	return -1;

}







