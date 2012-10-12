/*
 * mcsldevice.h
 *
 *  Created on: 2011-3-8
 *      Author: leiming
 */

#ifndef MCSLDEVICE_H_
#define MCSLDEVICE_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

#include "mcsltypes.h"

#define DEVICE_NAME_LEN	24	/*device name*/

#define MAX_MASTER		5	//max master devices
#define MAX_SLAVE		10		//max slave devices per master

/*注意与核心光标不同，核心键盘的产生是由核心光标建立产生的。所以一旦核心光标建立那么一定有一个挂在下面的从设备
 * 而核心键盘产生之后未必有物理键盘挂载
*/

typedef struct _master_pointer{
	int master_id;
	int dispatched;	//whether or not be dispatched
	int nr_slave;	//number of slaves
	int empty;	//points to empty position
	int slave_id[MAX_SLAVE];
	char master_name[DEVICE_NAME_LEN];		/*master device name*/
	char slave_names[MAX_SLAVE][DEVICE_NAME_LEN];	/*slave device name*/
//	Window attach_win;	/*attached wndow*/
}master_pointer;

typedef struct _pointer_device{
	int empty;	//points to empty position
	master_pointer master_pointers[MAX_MASTER];
}pointer_device;

/*
 * 键盘焦点必须依附于光标才能存在。先获取光标才能有键盘焦点。
 * 键盘焦点不支持tab切换
 * 核心keyboard设备id号一定是对应核心光标id号加1
 */
typedef struct _master_keyboard{
	int master_id;
	int nr_slave;	/*number of slaves*/
	int empty;		/*points to empty slave position*/
	int slave_id[MAX_SLAVE];
	char master_name[DEVICE_NAME_LEN];		/*master device name*/
	char slave_names[MAX_SLAVE][DEVICE_NAME_LEN];	/*slave device name*/
}master_keyboard;

typedef struct _keyboard_device{
	int empty;	/*points to empty master position*/
	master_keyboard master_keyboards[MAX_MASTER];	/*是与核心光标一一对应*/
}keyboard_device;


/*identify different pointers*/
#define NOPOINTER	0
#define POINTER1		1
#define POINTER2		2
#define POINTER3 	3
#define POINTER4		4
#define POINTER5		5
#define ANYPOINTER	 8

#define MAX_POINTERS		(POINTER5 + 1)

/*
 * 产生记录光标信息的库pointer_device
 *
 */
extern pointer_device *create_pointer_device(void);
/*
 *删除光标设备信息
 */
extern int delete_pointer_device(pointer_device *this);
/*
 *在光标设备信息里添加一个核心光标
 *@param1:核心光标ID
 *@param2:核心光标名称(可以为空)
 */
extern int add_master_pointer(pointer_device *this , int master_id , char *master_name);
/*
 *为一个核心光标添加物理光标
 *@param1:核心光标ID
 *@param2:物理光标ID
 *@param3:物理设备名称
 */
extern int add_slave_pointer(pointer_device *this , int master_id , int slave_id , char *slave_name);
/*
 * 删除一个核心光标
 */
extern int remove_master_pointer(pointer_device *this , int master_id);
/*
 * 删除一个物理光标。
 * @return:
 * 成功 >0 返回其依附的核心光标的序列号
 * 失败 -1 表示该slave_id不存在
 */
extern int remove_slave_pointer(pointer_device *this , int slave_id);
/*
 * 打印光标设备信息
 */
extern void tree_pointer_device(pointer_device *this);
/*get a cursor for a window*/
extern int get_cursor(pointer_device *this , Window win , int pointer_id);
/*dispatch a new cursor to a window which registered
 * @param0:光标设备信息
 * @param1:键盘设备信息
 * @param2:master_id
*/
extern int dispatch_cursor_keyboard(pointer_device *this , keyboard_device *that , int master_id);


/*判定不同的键盘。每个键盘标示与相应光标一一对应*/
#define NOKEYBOARD NOPOINTER
#define KEYBOARD1 POINTER1
#define KEYBOARD2 POINTER2
#define KEYBOARD3 POINTER3
#define KEYBOARD4 POINTER4
#define KEYBOARD5 POINTER5
#define ANYKEYBOARD ANYPOINTER

#define MAX_KEYBOARDS MAX_POINTERS

/*
 * 产生记录键盘信息keyboard_device
 *
 */
keyboard_device *create_keyboard_device(void);
/*
 *删除键盘设备信息
 */
extern int delete_keyboard_device(keyboard_device *this);
/*
 *在键盘设备信息里添加一个核心键盘
 *@param1:核心键盘ID
 *@param2:核心键盘名称
 */
extern int add_master_keyboard(keyboard_device *this , int master_id , char *master_name);
/*
 *为一个核心键盘添加物理键盘
 *@param1:核心键盘ID
 *@param2:物理键盘ID
 *@param3:物理键盘名称
 */
extern int add_slave_keyboard(keyboard_device *this , int master_id , int slave_id , char *slave_name);
/*
 * 删除一个核心键盘
 */
extern int remove_master_keyboard(keyboard_device *this , int master_id);
/*
 * 删除一个物理键盘。
 * @return:
 * 成功 >0 返回其依附的核心键盘的序列号
 * 失败 -1 表示该slave_id不存在
 */
extern int remove_slave_keyboard(keyboard_device *this , int slave_id);
/*
 * 打印键盘设备信息
 */
extern void tree_keyboard_device(keyboard_device *this);

/*get a keyboard for a window
 * @param0:client window
 * @param1:required keyboard_id
 * @return:
 * -1 失败
 * >0 核心键盘deviceid
*/
extern int get_keyboard(Window win , int keyboard_id);

/*dispatch a new keyboard to a window which registered
 * @param0:键盘设备信息
 * @param0:物理键盘的id
 * @param1:物理键盘的名字
 * 分配新插入的物理键盘设备号。原则是优先分配给已经有核心键盘但尚无物理键盘的窗口；
 * 当找不到此种情况时将其放入keyboard_store中备用
 * return: 0/-1
*/
extern int dispatch_phys_keyboard(keyboard_device *this , int phys_id , char *phys_name);
/*
 * 清除keyboard_clients之中拥有物理键盘id为phys_id的项目。其结果是使得该核心键盘之phys_id键盘为0
 * return: 0/-1
 */
extern int clear_phys_keyboard(int phys_id);

#endif /* MCSLDEVICE_H_ */
