/*
 * mcsltool.h
 *
 *  Created on: 2011-3-18
 *      Author: leiming
 */

#ifndef MCSLTOOL_H_
#define MCSLTOOL_H_


/*
 * Font Set
 *
 */
/////////FONT FAMILY/////////////////
#define CHARTER	"Charter"
#define COURIER	"Courier"
#define HELVETICA	"Helvetica"
#define SCHOOLBOOK	"Schoolbook"
#define SYMBOL	"Symbol"
#define TIMES	"Times"
/////////Weights///////////////////////
#define MEDIUM	"Medium"
#define BOLD		"bold"
////////SLANTS////////////////////////
#define ROMAN	"-r-"
#define ITALIC	"-i-"
#define OBLIQUE "-o-"
////////POINT SIZES////////////////
#define SIZE_DEFAULT "0"
#define SIZE_8	"80"
#define SIZE_10 "100"
#define SIZE_12 "120"
#define SIZE_14 "140"
#define SIZE_18 "180"
#define SIZE_24 "240"


/*
 * load font which closest to the requried font
 * @return -1: fail
 * @return 0: success
 */
extern XFontStruct *load_font(char *font_family , char *weights , char *slants , char *point_size);


#define A_LEFT	0	/*左对齐*/
#define A_CENTER	1	/*居中对齐*/
/*
 * 根据传入的控件在其面板上绘制文字。
 * @param0:需要写文字的控件
 * @param1:字符串
 * @param2:字符数
 * @param3:对齐方式(A_LEFT/A_CENTER)
 */
extern int mcsl_draw_string(mcslobj *widget , char *string , int num , unsigned char align);

#endif /* MCSLTOOL_H_ */
