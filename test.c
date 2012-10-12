/*
 * test.c
 *
 *  Created on: 2011-3-17
 *      Author: leiming
 */

#include <math.h>
#include <stdio.h>
#include "mcsl/mcsl.h"

#include "no_press.xpm"
#include "pressed.xpm"

#define PI 3.14

mcsl_u32 c_src_x , c_src_y;
mcsl_u32 pre_x = 0;
mcsl_u32 pre_y = 0;
mcsl_u32 circle_x , circle_y;



void call_back(void *widget , void *data){
	mcslbutton *p = (mcslbutton *)widget;
	mcslcanvas *canvas = (mcslcanvas *)data;

	canvas->set_pen_color(MCSLCANVAS(canvas) , "#FF1493");

	p->fetch_cursor(MCSLWIDGET(p) , &c_src_x , &c_src_y);

}

void call_back2(void *widget , void *data){
	mcsl_u32 src_x , src_y;
	mcsl_u32 c_x, c_y;
	mcslbutton *p = (mcslbutton *)widget;

	if(p->ispressed(MCSLWIDGET(p) , BUTTON1_PRESSED)){
		p->get_coordinate(MCSLWIDGET(p) , &src_x , &src_y , NULL , NULL , NULL);
		p->fetch_cursor(MCSLWIDGET(p) , &c_x , &c_y);
		p->move(MCSLWIDGET(p) , c_x - c_src_x + src_x , c_y - c_src_y + src_y);
	}
}

void count_device(void *widget , void *data){
	printf("Now number of master device in system is:%d \n" , mcsl_count_device());
}




void draw_graphs(void *widget , void *data){
	mcsl_u32 x , y;
	mcslcanvas *p = (mcslcanvas *)widget;
	p->fetch_cursor(MCSLWIDGET(p) , &x , &y);
//	printf("S: x %d : y %d\n" , x , y);
	pre_x = x;
	pre_y = y;

}

void draw_graphs1(void *widget , void *data){
	mcsl_u32 x , y;
	mcslcanvas *p = (mcslcanvas *)widget;
	p->fetch_cursor(MCSLWIDGET(p) , &x , &y);
//	printf("D: x %d : y %d\n" ,x , y);
//	p->draw_arc(MCSLCANVAS(p) , circle_x , circle_y , pre_x , pre_y , x , y);
	p->fill_rect(MCSLCANVAS(p) , pre_x , pre_y , x , y);
}
/*
void draw_graphs2(void *widget , void *data){
	mcsl_u32 x , y;
	mcslcanvas *p = (mcslcanvas *)widget;
	if(p->ispressed(MCSLWIDGET(p) , BUTTON1_PRESSED)){
		p->fetch_cursor(MCSLWIDGET(p) , &x , &y);
		p->draw_point(MCSLCANVAS(p) , x , y);
	}
}
*/
void draw_graphs3(void *widget , void *data){
	mcsl_u32 x , y;
	mcslcanvas *p = (mcslcanvas *)widget;
	p->fetch_cursor(MCSLWIDGET(p) , &x , &y);
	printf("C: x %d : y %d\n" ,x , y);
	circle_x = x;
	circle_y = y;

}


int main(int argc , char **argv){
	printf("PI:%g\n" , PI);
	mcsl_init(argc , argv);

	mcsltopwin *topwin;
	topwin = new_mcsltopwin();

	mcslbutton *bt1;
	bt1 = new_mcslbutton(MCSLWIDGET(topwin) , "Soccer");
	bt1->choose_pointer(MCSLWIDGET(bt1) , POINTER1);

	mcslbutton *bt2;
	bt2 = new_mcslbutton(MCSLWIDGET(topwin) , "Boy");
	bt2->choose_pointer(MCSLWIDGET(bt2) , POINTER2);
	bt2->set_color(MCSLWIDGET(bt2) , NULL , "#778899" , NULL);
	bt2->set_pressed_color(MCSLBUTTON(bt2) , "#C0C0C0");
	mcslbutton *bt3;
	bt3 = new_mcslbutton(MCSLWIDGET(topwin) , NULL);
	bt3->choose_pointer(MCSLWIDGET(bt3) , POINTER1);
	bt3->set_bgmap(MCSLWIDGET(bt3) , no_press);
	bt3->set_pressed_map(MCSLBUTTON(bt3) , pressed);

	mcslbutton *bt4;
	bt4 = new_mcslbutton(MCSLWIDGET(topwin) , NULL);
//	bt4->choose_keyboard(MCSL_WIDGET(bt4) , KEYBOARD2);

	mcsllabel *label1;
	label1 = new_mcsllabel(MCSLWIDGET(topwin) , "Name:");

	mcslinput *input1;
	input1 = new_mcslinput(MCSLWIDGET(topwin) , "Soccer");
	input1->choose_keyboard(MCSLWIDGET(input1) , KEYBOARD1);

	mcslcanvas *canvas;
	canvas = new_mcslcanvas(MCSLWIDGET(topwin) , 150 , 150);
	canvas->choose_pointer(MCSLWIDGET(canvas) , ANYPOINTER);
	canvas->attach_callback(MCSLWIDGET(canvas) , draw_graphs , NULL , SIG_BUTTON_PRESS);
	canvas->attach_callback(MCSLWIDGET(canvas) , draw_graphs1 , NULL , SIG_BUTTON_RELEASE);
	canvas->attach_callback(MCSLWIDGET(canvas) , draw_graphs3 , NULL , SIG_BUTTON2_PRESS);
//	canvas->attach_callback(MCSLWIDGET(canvas) , draw_graphs2 , NULL , SIG_MOTION);


//	mcslinput *input2;
//	input2 = new_mcslinput(MCSLWIDGET(topwin) , "Boy");
//	input2->choose_keyboard(MCSLWIDGET(input2) , KEYBOARD2);
//	input2->set_size(input2 , 40);

	bt1->attach_callback(MCSLWIDGET(bt1) , count_device , NULL , SIG_BUTTON_PRESS);
	bt1->attach_callback(MCSLWIDGET(bt1) , call_back2 , NULL , SIG_MOTION);

//////////////////////////////////////

	mcslbox *box;
	box = new_mcslbox(VBOX_TOP , UN_HOMO , 30 , 0 , NULL , 0 , 0);
	box->attach(MCSLBOX(box) , MCSLWIDGET(topwin));


	mcslbox *box1;
	box1 = new_mcslbox(HBOX_LEFT , UN_HOMO , 20 , 30 , box , FLOAT_BOTTOM , 0);
	box1->append(MCSLBOX(box1) , MCSLWIDGET(bt1));
	box1->append(MCSLBOX(box1) , MCSLWIDGET(bt2));


	mcslbox *box2;
	box2 = new_mcslbox(HBOX_LEFT , HOMO , 20 , 30 , box1 , FLOAT_BOTTOM , 30);
	box2->append(MCSLBOX(box2) , MCSLWIDGET(bt3));
	box2->append(MCSLBOX(box2) , MCSLWIDGET(bt4));

	mcslbox *box3;
	box3 = new_mcslbox(HBOX_LEFT , UN_HOMO , 20 , 20 , box2 , FLOAT_BOTTOM , 30);
	box3->append(MCSLBOX(box3) , MCSLWIDGET(label1));
	box3->append(MCSLBOX(box3) , MCSLWIDGET(input1));
	box3->append(MCSLBOX(box3) , MCSLWIDGET(canvas));
//	box3->append(MCSLBOX(box3) , MCSLWIDGET(input2));

	mcsl_main();

	return 0;
}
