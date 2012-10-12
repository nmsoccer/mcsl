/*
 * mcsltool.c
 *
 *  Created on: 2011-3-18
 *      Author: leiming
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

#include "mcslglobal.h"
#include "mcsl.h"
#include "mcsltool.h"

/*
 * load font which closest to the requried font
 */
XFontStruct *load_font(char *font_family , char *weights , char *slants , char *point_size){
	int real_nr;
	int head;	/*head<->tail > a segmen*/
	int tail;

	XFontStruct *font;
	char **font_names;
	char font_name[40];

	memset(font_name , 0 , sizeof(font_name));
	font_name[0] = '*';
	head = 1;
	tail = 1;

	/*validate font_family*/
	if(!font_family){
		strcpy(&font_name[head] , "Courier");
	}else{
		strcpy(&font_name[head] , font_family);
	}
	tail = strlen(font_name);
	font_name[tail] = '*';
	font_names = XListFonts(display , font_name , 10 , &real_nr);

	if(real_nr == 0){/*No such font_family Set Default Font_Family*/
		memset(&font_name[head] , 0 , strlen(&font_name[head]));
		strcpy(&font_name[head] , "Courier");
		tail = strlen(font_name);
		font_name[tail] = '*';
		font_names = XListFonts(display , font_name , 10 , &real_nr);
	}


	if(real_nr == 0){	/*still error return*/
		printf("font_family error! %s\n" , font_name);
		return NULL;
	}

	/*validate font_weights*/
	head = strlen(font_name);
	if(!weights){
		strcpy(&font_name[head] , "Medium");
	}else{
		strcpy(&font_name[head] , weights);
	}
	tail = strlen(font_name);
	font_name[tail] = '*';
	font_names = XListFonts(display , font_name , 10 , &real_nr);

	if(real_nr == 0){/*No such weights*/
		memset(&font_name[head] , 0 , strlen(&font_name[head]));
		strcpy(&font_name[head] , "Medium");
		tail = strlen(font_name);
		font_name[tail] = '*';
		font_names = XListFonts(display , font_name , 10 , &real_nr);
	}


	if(real_nr == 0){	/*still error return*/
		printf("weights error!\n");
		return NULL;
	}

	/*validate slants*/
	head = strlen(font_name);
	if(!slants){
		strcpy(&font_name[head] , "r");
	}else{
		strcpy(&font_name[head] , slants);
	}
	tail = strlen(font_name);
	font_name[tail] = '*';
	font_names = XListFonts(display , font_name , 10 , &real_nr);

	if(real_nr == 0){/*No such weights*/
		memset(&font_name[head] , 0 , strlen(&font_name[head]));
		strcpy(&font_name[head] , "r");
		tail = strlen(font_name);
		font_name[tail] = '*';
		font_names = XListFonts(display , font_name , 10 , &real_nr);
	}

	if(real_nr == 0){	/*still error return*/
		printf("slants error!\n");
		return NULL;
	}

	/*validate point_size*/
	head = strlen(font_name);
	if(!point_size){
		strcpy(&font_name[head] , "0");
	}else{
		strcpy(&font_name[head] , point_size);
	}
	tail = strlen(font_name);
	font_name[tail] = '*';
	font_names = XListFonts(display , font_name , 10 , &real_nr);

	if(real_nr == 0){/*TRY SIZE_DEFAULT*/
		memset(&font_name[head] , 0 , strlen(&font_name[head]));
		strcpy(&font_name[head] , "0");
		tail = strlen(font_name);
		font_name[tail] = '*';
		font_names = XListFonts(display , font_name , 10 , &real_nr);
	}


	if(real_nr == 0){/*TRY SIZE_8*/
		memset(&font_name[head] , 0 , strlen(&font_name[head]));
		strcpy(&font_name[head] , "80");
		tail = strlen(font_name);
		font_name[tail] = '*';
		font_names = XListFonts(display , font_name , 10 , &real_nr);
	}


	if(real_nr == 0){/*TRY SIZE_12*/
		memset(&font_name[head] , 0 , strlen(&font_name[head]));
		strcpy(&font_name[head] , "120");
		tail = strlen(font_name);
		font_name[tail] = '*';
		font_names = XListFonts(display , font_name , 10 , &real_nr);

	}

	if(real_nr == 0){/*TRY SIZE_14*/
		memset(&font_name[head] , 0 , strlen(&font_name[head]));
		strcpy(&font_name[head] , "140");
		tail = strlen(font_name);
		font_name[tail] = '*';
		font_names = XListFonts(display , font_name , 10 , &real_nr);
	}


	if(real_nr == 0){/*TRY SIZE_18*/
		memset(&font_name[head] , 0 , strlen(&font_name[head]));
		strcpy(&font_name[head] , "180");
		tail = strlen(font_name);
		font_name[tail] = '*';
		font_names = XListFonts(display , font_name , 10 , &real_nr);
	}


	if(real_nr == 0){/*TRY SIZE_24*/
		memset(&font_name[head] , 0 , strlen(&font_name[head]));
		strcpy(&font_name[head] , "240");
		tail = strlen(font_name);
		font_name[tail] = '*';
		font_names = XListFonts(display , font_name , 10 , &real_nr);
	}

	if(real_nr == 0){	/*still error return*/
		printf("point_size error!\n , %s" , font_name);
		return NULL;
	}

//	printf("last font_name is:%s PK %s\n" , font_name , font_names[0]);

	if((font = XLoadQueryFont(display , font_name)) != NULL){
		printf("load font %s success \n" , font_names[0]);
		XFreeFontNames(font_names);

		return font;
	}

	return NULL;
}



/*
 * 根据传入的控件在其面板上绘制文字。
 * @param0:需要写文字的控件
 * @param1:字符串
 * @param2:字符数
 * @param3:对齐方式(A_LEFT/A_CENTER)
 * return:字符串的起始y坐标
 */
int mcsl_draw_string(mcslobj *widget , char *string , int num , unsigned char align){
	int x , y;
	int number;
	int text_height;
	int text_width;

	if(mcsl_font){
		text_width = XTextWidth(widget->basic_attr.font_info , string , num);
	}else{
		printf("mcsl_font is empty!\n");
	}

	text_height = widget->basic_attr.font_info->ascent + widget->basic_attr.font_info->descent;

	switch(align){
	case A_LEFT:
		x = 0;
		break;
	case A_CENTER:
	default:
		x = (widget->coordinate.width - text_width) / 2;
		break;
	}
	y = (widget->coordinate.height - text_height) / 2 * 2;
//	y += 20;

	XDrawString(display , widget->basic_attr.win , widget->basic_attr.gc , x , y , string , num);

	return y;

}

int mcsl_draw_rect();

