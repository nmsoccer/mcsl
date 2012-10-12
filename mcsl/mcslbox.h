/*
 * mcslbox.h
 *
 *  Created on: 2011-3-28
 *      Author: leiming
 */

#ifndef MCSLBOX_H_
#define MCSLBOX_H_

#include "mcsltypes.h"
#include "mcslobj.h"

/*
 * 专门用于布局的容器。
 * 一个控件只能拥有一个容器；容器可以嵌套。
 *
 * 一个容器的空间可以看成由控件组拥有空间与内层容器空间和控件组与容器间隔三要素共同组成。
 * 以HBOX_LEFT类型为例:
 * margin 始终是指第一个控件距离左边框的距离。
 *特殊情况是当控件不存在而内层容器存在时表示内层容器偏移的距离。
 *
 * 1.控件存在而内层容器不存在:
 * 	   控件将会使用父容器的全部空间。其距离边框的像素由margin决定。
 * 		type=HBOX_LEFT那么margin表示距离第一个控件距离左边框像素。此时float_to , float_distance无效
 *
 * 2.控件不存在而内层容器存在:
 * 		内层容器拥有空间由type与margin共同决定。
 *    type=HBOX_LEFT那么内层容器空间为父容器宽度减去margin后所剩余空间。此时float_to , float_distance无效
 *
 * 3.控件与内层容器均存在时:
 *   	不同类型的父容器之控件组位于其名字相应的位置时margin才起明显作用。其他三种相对位置均贴边。
 * 		tpe=HBOX_LEFT
 *		控件组在内层容器左方：受margin影响。
 *		控件组在内层容器右方：控件组贴父容器最右方。
 *		控件组在内层容器下方：控件组贴父容器最下方。
 *		控件组在内层容器上方：控件组贴父容器最上方。
 *
 * 		tpe=HBOX_RIGHT
 *		控件组在内层容器右方：受margin影响。
 *		控件组在内层容器左方：控件组贴父容器最左方。
 *		控件组在内层容器下方：控件组贴父容器最下方。
 *		控件组在内层容器上方：控件组贴父容器最上方。

 * 		tpe=VBOX_TOP
 *		内层容器在控件组上方：受margin影响。
 *		控件组在内层容器下方：控件组贴父容器最下方。
 *		控件组在内层容器左方：控件组贴父容器最左方。
 *		控件组在内层容器右方：控件组贴父容器最右方。

 * 		tpe=VBOX_BOTTOM
 *		内层容器在控件组下方：受margin影响。
 *		控件组在内层容器上方：控件组贴父容器最上方。
 *		控件组在内层容器左方：控件组贴父容器最左方。
 *		控件组在内层容器右方：控件组贴父容器最右方。

 */

/*容器类型*/
#define HBOX_LEFT		0	/*横排左列。margin代表的是控件距离控件组所拥有空间的距离。容器在控件*/



#define HBOX_RIGHT	1	/*从右排列*/
#define VBOX_TOP		2	/*纵向从上开始排*/
#define VBOX_BOTTOM	3	/*从下开始排*/

#define HOMO			1
#define UN_HOMO		0

/*内部容器相对父容器控件的浮动方向*/
#define FLOAT_LEFT			0	/*浮动在控件组的左边*/
#define FLOAT_RIGHT		1	/*浮动在控件组的右边*/
#define FLOAT_TOP			2	/*浮动在控件组的上面*/
#define FLOAT_BOTTOM 	3	/*浮动在控件组的下面*/

typedef struct _box_content{
	struct _mcslobj *widget;
	struct _box_content *next;
}box_content;


struct _mcslbox{
	/*private data*/
	mcsl_u8 type;		/*类型 横排或者纵排*/
	mcsl_u8 homogeneous;		/*是否均匀排列(控件大小一样)*/
	mcsl_u16 margin;	/*最左/右/上/下距离边界的像素*/
	mcsl_u16 space;	/*控件之间间隔的像素*/


	struct _mcslobj *parent_widget;	/*该容器依附的控件. 这里出现了box与obj相互引用的情况。不能使用mcslobj * 而是struct _mcslobj **/

	mcsl_u8 float_to;		/*内部容器浮动于父容器拥有控件的方向*/
	mcsl_u16 float_distance;	/*内部容器与父容器控件组的距离*/
	struct _mcslbox *in_box;	/*内部的容器，可以没有。如果有只能有一个*/

	box_content head;	/*放入该容器中的控件，以链表的形式存在*/
	box_content *first;
	box_content *last;

	mcsl_u8 nr_widgets;	/*所拥有的控件数目*/
	mcsl_u16 max_width;	/*控件的最宽度*/
	mcsl_u16 max_height;	/*控件的最高度*/

	/*public interface*/
	int (*attach)(struct _mcslbox *box , struct _mcslobj *parent);	/*将容器添加到父控件中。一个控件最多一个容器*/

	/*将子容器嵌套入父容器
	 * @param1:子容器相对于父容器控件组的浮动方向
	 * @param2:子容器与父容器组的距离
	 */
	int (*nested)(struct _mcslbox *child_box , mcsl_u8 float_to , mcsl_u16 float_distance , struct _mcslbox *parent_box);

	int (*de_nest)(struct _mcslbox *parent_box);	/*将内层容器移除*/

	int (*append)(struct _mcslbox *box , struct _mcslobj *widget);	/*往容器里添加控件*/
	int (*remove)(struct _mcslbox *box , struct _mcslobj *widget);	/*从容器里移除控件*/
};

typedef struct _mcslbox mcslbox;

#define MCSLBOX(e) ((mcslbox *)e)

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
										mcsl_u8 float_to , mcsl_u16 float_distance);


/*
 * 删除box
 * 递归删除嵌套盒子
 */
int delete_mcslbox(mcslbox *pbox);



/*
 * 对box里的控件进行布局
 * @param0:将要布局的box
 * @param1~3:box可以利用的空间
 * @param4~7:返回设置控件组后可以利用的空间
 */
int arrange_mcslbox(mcslbox *box , mcsl_u32 x , mcsl_u32 y , mcsl_u32 width , mcsl_u32 height , mcsl_u32 *r_x ,
										mcsl_u32 *r_y , mcsl_u32 *r_width , mcsl_u32 *r_height);


















#endif /* MCSLBOX_H_ */
