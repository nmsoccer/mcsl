/*
 * mcsldevice.c
 *
 *  Created on: 2011-3-8
 *      Author: leiming
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mcslglobal.h"
#include "mcsldevice.h"
#include "mcsl.h"

#include <X11/extensions/XInput2.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>

/*registered windows*/
#define MAX_WINDOWS	10
typedef struct _register_windows{
	int ready_to_dispatch;	/*points to window not dispatched*/
	int end;	/*end of array*/
	Window windows[MAX_WINDOWS];
}register_windows;

static register_windows reg_win;


/*pointer_ids store which window owns which pointer*/

struct _client{
	Window client_win;
	struct _client *next;
};


struct _id_info{
		int deviceid;	/*actual deviceid <-> pointer_id*/
		struct _client *last_client;	/*指向最后一个客户*/
		struct _client client;
};

static struct _id_info pointer_clients[MAX_POINTERS];

/*keyboard_ids store which window owns which keyboard*/

struct _key_id_info{	/*除了初始核心设备之外，其他的核心键盘只挂一个从键盘*/
	int master_id;	/*键盘核心id*/
	int phys_id;		/*实际的物理键盘id*/
	struct _client *last_client;	/*指向最后一个客户*/
	struct _client client;
};

static struct _key_id_info keyboard_clients[MAX_KEYBOARDS];

/*
 * 当有新键盘插入时，如果没有找到合适的keyboard需求那么放入此中，等待使用
 */
static struct _keyboard_store{
	int keyboard_id[MAX_SLAVE];
	char keyboard_name[MAX_SLAVE][DEVICE_NAME_LEN];
	int head;
	int tail;
	int nr_ids;
}keyboard_store;


static XIEventMask eventmask;
static unsigned char mask[2] = {0 , 0};


/*
 * 将键盘设备号放入store之中
 */
static int store_keyboard(int keyboard_id , char *keyboard_name){
	keyboard_store.keyboard_id[keyboard_store.tail] = keyboard_id;
	memset(keyboard_store.keyboard_name[keyboard_store.tail] , 0 , DEVICE_NAME_LEN);
	strcpy(keyboard_store.keyboard_name[keyboard_store.tail] , keyboard_name);
	keyboard_store.tail = (keyboard_store.tail + 1) % (MAX_SLAVE);
	keyboard_store.nr_ids ++;
	return 0;
}

/*
 * 从store中取出一个键盘
 * return: >0 成功
 * -1:没有键盘
 */
static int fetch_keyboard(void){
	int id;

	if(keyboard_store.nr_ids){
		id = keyboard_store.keyboard_id[keyboard_store.head];
		keyboard_store.head = (keyboard_store.head + 1) % (MAX_SLAVE);
		keyboard_store.nr_ids--;

		return id;
	}

	return -1;
}


/////////////////////////////////////////////////////////////////////////////////

/*
 * 产生记录光标信息的库pointer_device
 *
 */
pointer_device *create_pointer_device(void){
	pointer_device *p;

	p = (pointer_device *)malloc(sizeof(pointer_device));
	memset(p , 0 , sizeof(pointer_device));

	memset(&reg_win , 0 , sizeof(reg_win));	/*set reg_win*/
	memset(&pointer_clients , 0 , sizeof(pointer_clients));

	return p;
}

/*
 *删除光标设备信息
 */
int delete_pointer_device(pointer_device *this){
	if(this){
		free(this);
		return 0;
	}
	return -1;
}

/*
 *在光标设备信息里添加一个核心光标
 *@param1:核心光标ID
 *@param2:核心光标名称(可以为空)
 */
int add_master_pointer(pointer_device *this , int master_id , char *master_name){
	int i;

	if(this->empty == -1){	/*full*/
		return -1;
	}

	this->master_pointers[this->empty].master_id = master_id;
	this->master_pointers[this->empty].empty = 0;
	this->master_pointers[this->empty].dispatched = 0;

	if(master_name){
		strcpy(this->master_pointers[this->empty].master_name , master_name);
	}


	for(i=0; i<MAX_MASTER; i++){	/*set next empty location*/
		if(this->master_pointers[i].master_id == 0){
			this->empty = i;
			return 0;
		}
	}

	this->empty = -1;	/*full*/
	return 0;

}

/*
 *为一个核心光标添加物理光标
 *@param1:核心光标ID
 *@param2:物理光标ID
 *@param3:物理设备名称
 */
int add_slave_pointer(pointer_device *this , int master_id , int slave_id , char *slave_name){
	int i;
	int j;

	for(i=0; i<MAX_MASTER; i++){

		if(this->master_pointers[i].master_id == master_id){	/*matching master*/
			if(this->master_pointers[i].empty == -1){	/*slave full*/
				return -1;
			}

			this->master_pointers[i].slave_id[this->master_pointers[i].empty] = slave_id;	/*set slave_id*/
			if(slave_name){
				strcpy(this->master_pointers[i].slave_names[this->master_pointers[i].empty] , slave_name);
			}
			this->master_pointers[i].nr_slave++;

			for(j=0; j<MAX_SLAVE; j++){	/*searching next empty location*/
				if(this->master_pointers[i].slave_id[j] == 0){
					this->master_pointers[i].empty = j;
					return 0;
				}
			}
			this->master_pointers[i].empty = -1;	/*full*/
			return 0;
		}	/*end if*/

	}	/*end for*/

	return -1;	/*can not find mating master_id*/

}

/*
 * 删除一个核心光标
 */
int remove_master_pointer(pointer_device *this , int master_id){
	if(!this){
		return -1;
	}
	int i;

	for(i=0; i<MAX_MASTER; i++){
		if(this->master_pointers[i].master_id == master_id){
			memset(&(this->master_pointers[i]) , 0 , sizeof(master_pointer));
			this->empty = i;
			return 0;
		}
	}

	return -1;
}

/*
 * 删除一个物理光标。
 * @return:
 * 成功 >0 返回其依附的核心光标的序列号
 * 失败 -1 表示该slave_id不存在
 */
int remove_slave_pointer(pointer_device *this , int slave_id){
	if(!this){
		return -1;
	}
	int i , j;

	for(i=0; i<MAX_MASTER; i++){	/*each master*/
		for(j=0; j<MAX_SLAVE; j++){	/*each slave*/
			if(this->master_pointers[i].slave_id[j] == slave_id){
				this->master_pointers[i].slave_id[j] = 0;
				memset(this->master_pointers[i].slave_names[j] , 0 , DEVICE_NAME_LEN);
				this->master_pointers[i].nr_slave--;
				this->master_pointers[i].empty = j;
				return i;	/*return master_id */
			}
		}	/*end slave*/

	}	/*end master*/

	return -1;

}

/*
 * 打印光标设备信息
 */
void tree_pointer_device(pointer_device *this){
	int i , j;
	pointer_device *p_device = this;

	for(i=0; i<MAX_MASTER; i++){
		if(p_device->master_pointers[i].master_id != 0){
			printf("master_point id:%d %s\n" , p_device->master_pointers[i].master_id , p_device->master_pointers[i].master_name);
			for(j=0; j<MAX_SLAVE; j++){
				if(p_device->master_pointers[i].slave_id[j] != 0){
					printf("<< slave_point id:%d %s\n" ,  p_device->master_pointers[i].slave_id[j] , p_device->master_pointers[i].slave_names[j]);
				}
			}

			printf("\n");
		}

	}

}



/*get a cursor for a window
 * @param1:client window
 * @param2:required pointer_id
 * @return master_id
*/
int get_cursor(pointer_device *this , Window win , int pointer_id){
	int i;

	if(pointer_id == ANYPOINTER){	/*任意指针*/
		return XIAllMasterDevices;
	}

	if(pointer_id < POINTER1 || pointer_id > POINTER5){
		return -1;
	}

	if(pointer_clients[pointer_id].deviceid != 0){	/*之前已经有窗口获取该指针*/
		return pointer_clients[pointer_id].deviceid;
	}

	if(pointer_clients[pointer_id].last_client != NULL){	/*之前已经有窗口注册该指针但还没有分配设备*/
		/*新产生一个客户挂在需求链的末尾*/
		pointer_clients[pointer_id].last_client->next = (struct _client *)malloc(sizeof(struct _client));
		pointer_clients[pointer_id].last_client = pointer_clients[pointer_id].last_client->next;

		pointer_clients[pointer_id].last_client->client_win = win;
		pointer_clients[pointer_id].last_client->next = NULL;

		return -1;
	}

	for(i=0; i<MAX_MASTER; i++){
		/*没有被分配的设备只存在于初始设备。以后的设备都将在插入时被分配，即使没有需求窗口*/
		if((this->master_pointers[i].master_id != 0) && (this->master_pointers[i].dispatched == 0)){	/*该设备还没有被分配*/
			printf("find one device not dispatched!\n");
			this->master_pointers[i].dispatched = 1;	/*set dispatched*/
//			this->master_pointers[i].attach_win = win;

			/*更新指针的客户窗口链*/
			pointer_clients[pointer_id].deviceid = this->master_pointers[i].master_id;	/*将指针ID对应设备的实际ID*/
			pointer_clients[pointer_id].last_client = &pointer_clients[pointer_id].client;	/*这是注册该设备客户链的第一个也是最后一个*/

			pointer_clients[pointer_id].client.client_win = win;	/*这是该指针设备的第一个客户窗口*/
			pointer_clients[pointer_id].client.next = NULL;	/*还没有下一个客户窗口*/


			/*初始化第一套核心键盘*/
			if(i == 0){	/*表示这是最初的一套核心设备。它们最先存在不能被删除*/
				keyboard_clients[pointer_id].master_id = this->master_pointers[i].master_id + 1;
				keyboard_clients[pointer_id].phys_id = -1;		/*这是唯一一个设置为-1的值。表示系统第一套核心键盘有物理设备但不知具体设备号*/
			}


			return this->master_pointers[i].master_id;
		}
	}

	/*找不到还没有分配的设备,那么将该窗口注册，等待分配新插入设备，同时将该窗口放入该指针ID对应的第一项目中*/
	reg_win.windows[reg_win.end] = win;
	reg_win.end++;

	pointer_clients[pointer_id].deviceid = 0;
	pointer_clients[pointer_id].last_client = &pointer_clients[pointer_id].client;	/*这是注册该设备客户链的第一个但不一定是最后一个*/
	pointer_clients[pointer_id].client.client_win = win;
	pointer_clients[pointer_id].client.next = NULL;

	return -1;
}

/*dispatch a new cursor to a window which registered
 * @param0:光标设备信息
 * @param1:键盘设备信息
 * @param2:master_id
 * return 0/-1
*/
int dispatch_cursor_keyboard(pointer_device *this , keyboard_device *that , int master_id){
	int i , j;
	int keyboard_id;
	Window win;
	struct _client *p;
	XIAttachSlaveInfo attach_info;

	printf("ready to dispatch!\n");

	if(reg_win.windows[reg_win.ready_to_dispatch] == 0){	/*no window registers*/
		/*没有窗口需求光标那么根据键盘需求来找到有需求的keyboard_id之后再设置相应的pointer_id*/
		for(i=0; i<MAX_KEYBOARDS; i++){
			/*没有核心键盘但已经有窗口在keyboard_clients之中注册*/
			if((keyboard_clients[i].master_id == 0) && (keyboard_clients[i].client.client_win != 0)){
				pointer_clients[i].deviceid = master_id;

				for(j = 0; j<MAX_MASTER; j++){	/*更新设备信息库*/
					if(this->master_pointers[j].master_id == master_id){
						this->master_pointers[j].dispatched = 1;
						break;
					}
				}

				goto dispatch_keyboard;
			}	/*end if: find correct result*/
		}	/*end for*/

		/*如果没有找到上面的理想情况那么在pointer_clients之中找一个空缺位置填入*/
		for(i=0; i<MAX_POINTERS; i++){

			if(pointer_clients[i].deviceid == 0){
				pointer_clients[i].deviceid = master_id;


				for(j = 0; j<MAX_MASTER; j++){	/*更新设备信息库*/
					if(this->master_pointers[j].master_id == master_id){
						this->master_pointers[j].dispatched = 1;
						break;
					}
				}

				goto dispatch_keyboard;

			}	/*end if: find empty item*/

		}	/*end for*/

		/*never run here*/
		return -1;
	}


dispatch_pointer:
	/*有窗口等待获取设备*/
	/*将新获得的核心光标设备id和pointer_clients之中某个pointer_id联系起来*/
	win = reg_win.windows[reg_win.ready_to_dispatch];	/*获取待分配设备的窗口*/
	reg_win.ready_to_dispatch++;

	for(i = 0; i<MAX_MASTER; i++){	/*更新设备信息库*/
		if(this->master_pointers[i].master_id == master_id){
			this->master_pointers[i].dispatched = 1;
			break;
		}
	}

	for(i=0; i<MAX_POINTERS; i++){	/*在指针表中查找注册相应指针的窗口*/
		if(pointer_clients[i].deviceid){
			continue;
		}

		if(pointer_clients[i].client.client_win != win){
			continue;
		}

		/*match*/
		pointer_clients[i].deviceid = master_id;
		p = &pointer_clients[i].client;

		break;
	}

	memset(mask , 0 , sizeof(mask));	/*reattach new cursor to a window registered*/
	eventmask.deviceid = master_id;

	eventmask.mask_len = sizeof(mask);
	eventmask.mask = mask;

	XISetMask(mask , XI_DeviceChanged);
	XISetMask(mask , XI_ButtonPress);
	XISetMask(mask , XI_ButtonRelease);
	XISetMask(mask , XI_KeyPress);
	XISetMask(mask , XI_KeyRelease);
	XISetMask(mask , XI_Motion);
	XISetMask(mask , XI_Enter);
	XISetMask(mask , XI_Leave);

	while(p){	/*为所有注册该指针的窗口设置*/
		printf("dispatch a pointer!\n");
		win = p->client_win;
		XISelectEvents(display , win , &eventmask , 1);

		/*set cursor shape when entering window appended:2012.5*/
		XIDefineCursor(display, master_id , win , XCreateFontCursor(display,cursor_shape[i]));

		p = p->next;
	}

	//////////////////设置对应的键盘信息//////////

dispatch_keyboard:

	keyboard_clients[i].master_id = pointer_clients[i].deviceid + 1;
	p = &keyboard_clients[i].client;

	/*如果有store里有可供使用的键盘那么使用之*/
	if(keyboard_store.nr_ids){
		printf("store is used!\n");
		add_slave_keyboard(that , keyboard_clients[i].master_id , keyboard_store.keyboard_id[keyboard_store.head] ,
											keyboard_store.keyboard_name[keyboard_store.head]);	/*update p_device info*/
		/*将获取的物理键盘添加到新产生的核心键盘中*/
		attach_info.type = XIAttachSlave;
		attach_info.deviceid = keyboard_store.keyboard_id[keyboard_store.head];
		attach_info.new_master = keyboard_clients[i].master_id;
		XIChangeHierarchy(display , (XIAnyHierarchyChangeInfo *)&attach_info , 1);

		keyboard_clients[i].phys_id = keyboard_store.keyboard_id[keyboard_store.head];

		printf("keyboard %d attach to %d\n" , keyboard_clients[i].phys_id , keyboard_clients[i].master_id);

		keyboard_store.head = (keyboard_store.head + 1) % (MAX_SLAVE);
		keyboard_store.nr_ids--;

	}else{
		printf("no free keyboard id!\n");
	}

	memset(mask , 0 , sizeof(mask));	/*reattach new cursor to a window registered*/
	eventmask.deviceid = keyboard_clients[i].master_id;
	eventmask.mask_len = sizeof(mask);
	eventmask.mask = mask;

	XISetMask(mask , XI_DeviceChanged);
	XISetMask(mask , XI_ButtonPress);
	XISetMask(mask , XI_ButtonRelease);
	XISetMask(mask , XI_KeyPress);
	XISetMask(mask , XI_KeyRelease);
	XISetMask(mask , XI_Motion);
	XISetMask(mask , XI_Enter);
	XISetMask(mask , XI_Leave);

	while(p){
		if(p->client_win == 0){
			break;
		}
		win = p->client_win;
		XISelectEvents(display , win , &eventmask , 1);

		p = p->next;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////


/*
 * 产生记录键盘信息keyboard_device
 *
 */
keyboard_device *create_keyboard_device(void){
	int i;
	keyboard_device *p;

	p = (keyboard_device *)malloc(sizeof(keyboard_device));
	memset(p , 0 , sizeof(keyboard_device));

	memset(&keyboard_clients , 0 , sizeof(keyboard_clients));
	for(i=0; i<MAX_KEYBOARDS; i++){
		keyboard_clients[i].last_client = &keyboard_clients[i].client;
	}

	memset(&keyboard_store , 0 , sizeof(keyboard_store));

	return p;
}

/*
 *删除键盘设备信息
 */
int delete_keyboard_device(keyboard_device *this){
	if(this){
		free(this);
		return 0;
	}
	return -1;
}

/*
 *在键盘设备信息里添加一个核心键盘
 *@param1:核心键盘ID
 *@param2:核心键盘名称
 */
int add_master_keyboard(keyboard_device *this , int master_id , char *master_name){
	int i;

	if(this->empty == -1){	/*full*/
		return -1;
	}

	this->master_keyboards[this->empty].master_id = master_id;
	if(master_name){
		strcpy(this->master_keyboards[this->empty].master_name , master_name);
	}
	this->master_keyboards[this->empty].empty = 0;	/*还没有slave keyboard*/

	for(i=0; i<MAX_MASTER; i++){	/*set next empty location*/
		if(this->master_keyboards[i].master_id == 0){
			this->empty = i;
			return 0;
		}
	}

	this->empty = -1;	/*full*/
	return 0;

}


/*
 *为一个核心键盘添加物理键盘
 *@param1:核心键盘ID
 *@param2:物理键盘ID
 *@param3:物理键盘名称
 */
int add_slave_keyboard(keyboard_device *this , int master_id , int slave_id , char *slave_name){
	int i;
	int j;

	for(i=0; i<MAX_MASTER; i++){

		if(this->master_keyboards[i].master_id == master_id){	/*matching master*/
			if(this->master_keyboards[i].empty == -1){	/*slave full*/
				return -1;
			}

			this->master_keyboards[i].slave_id[this->master_keyboards[i].empty] = slave_id;	/*set slave_id*/
			if(slave_name){
				strcpy(this->master_keyboards[i].slave_names[this->master_keyboards[i].empty] , slave_name);
			}
			this->master_keyboards[i].nr_slave++;

			for(j=0; j<MAX_SLAVE; j++){	/*searching next empty location*/
				if(this->master_keyboards[i].slave_id[j] == 0){
					this->master_keyboards[i].empty = j;
					return 0;
				}
			}
			this->master_keyboards[i].empty = -1;	/*full*/
			return 0;
		}	/*end if*/

	}	/*end for*/

	return -1;	/*can not find mating master_id*/

}


/*
 * 删除一个核心键盘
 */
int remove_master_keyboard(keyboard_device *this , int master_id){
	if(!this){
		return -1;
	}
	int i;

	for(i=0; i<MAX_MASTER; i++){
		if(this->master_keyboards[i].master_id == master_id){
			memset(&(this->master_keyboards[i]) , 0 , sizeof(master_keyboard));
			this->empty = i;
			return 0;
		}
	}

	return -1;
}


/*
 * 删除一个物理键盘。
 * @return:
 * 成功 >0 返回其依附的核心键盘的序列号
 * 失败 -1 表示该slave_id不存在
 */
int remove_slave_keyboard(keyboard_device *this , int slave_id){
	if(!this){
		return -1;
	}
	int i , j;

	for(i=0; i<MAX_MASTER; i++){	/*each master*/
		for(j=0; j<MAX_SLAVE; j++){	/*each slave*/
			if(this->master_keyboards[i].slave_id[j] == slave_id){
				this->master_keyboards[i].slave_id[j] = 0;
				memset(this->master_keyboards[i].slave_names[j] , 0 , DEVICE_NAME_LEN);
				this->master_keyboards[i].nr_slave--;
				this->master_keyboards[i].empty = j;
				return i;	/*return master_id */
			}
		}	/*end slave*/

	}	/*end master*/

	return -1;

}


/*
 * 打印键盘设备信息
 */
void tree_keyboard_device(keyboard_device *this){
	int i , j;
	keyboard_device *p_device = this;

	for(i=0; i<MAX_MASTER; i++){
		if(p_device->master_keyboards[i].master_id != 0){
			printf("master_keyboard id:%d %s\n" , p_device->master_keyboards[i].master_id , p_device->master_keyboards[i].master_name);

			for(j=0; j<MAX_SLAVE; j++){
				if(p_device->master_keyboards[i].slave_id[j] != 0){
					printf("<< slave_keyboard id:%d %s\n" , p_device->master_keyboards[i].slave_id[j] , p_device->master_keyboards[i].slave_names[j]);
				}
			}

			printf("\n");
		}

	}	/*end for*/

}


/*get a keyboard for a window
 * @param0:client window
 * @param1:required keyboard_id
 * @return:
 * -1 失败
 * >0 核心键盘deviceid
*/
int get_keyboard(Window win , int keyboard_id){
	struct _client *p_client;

	if(keyboard_id == ANYKEYBOARD){	/*任意键盘*/
		return XIAllMasterDevices;
	}

	if(keyboard_id < KEYBOARD1 || keyboard_id > KEYBOARD5){
		return -1;
	}


	/*该核心键盘存在*/
	if(keyboard_clients[keyboard_id].master_id > 0){
		return keyboard_clients[keyboard_id].master_id;
	}else{	/*核心键盘id不存在*/

		printf("no master keyboard existed!\n");

		/*如果对应核心光标也不存在那么表示相应的核心设备还没有建立.将客户窗口挂在链上。等待核心设备的建立*/

		p_client = (struct _client *)malloc(sizeof(struct _client));
		p_client->client_win = 0;
		p_client->next = NULL;

		keyboard_clients[keyboard_id].phys_id = 0;
		keyboard_clients[keyboard_id].last_client->client_win = win;
		keyboard_clients[keyboard_id].last_client->next = p_client;
		keyboard_clients[keyboard_id].last_client = p_client;

		return -1;
	}

}

/*dispatch a new keyboard to a window which registered
 * @param0:键盘设备信息
 * @param0:物理键盘的id
 * @param1:物理键盘的名字
 * 分配新插入的物理键盘设备号。原则是优先分配给已经有核心键盘但尚无物理键盘的窗口；
 * 当找不到此种情况时将其放入keyboard_store中备用
 * return: 0/-1
*/
int dispatch_phys_keyboard(keyboard_device *this , int phys_id , char *phys_name){
	int i;
	XIAttachSlaveInfo attach_info;

	for(i=0; i<MAX_KEYBOARDS; i++){
		/*找到具有核心键盘但无从键盘的id*/
		if((keyboard_clients[i].master_id !=0) && (keyboard_clients[i].phys_id == 0)){
			/*将该物理键盘添加到核心键盘*/
			attach_info.type = XIAttachSlave;
			attach_info.deviceid = phys_id;
			attach_info.new_master = keyboard_clients[i].master_id;
			XIChangeHierarchy(display , (XIAnyHierarchyChangeInfo *)&attach_info , 1);

			add_slave_keyboard(this , keyboard_clients[i].master_id , phys_id , phys_name);	/*update p_device info*/
			return 0;
		}
	}

	/*没有找到无从设备的核心设备。放入store中*/

	store_keyboard(phys_id , phys_name);

	return -1;
}

/*
 * 清除keyboard_clients之中拥有物理键盘id为phys_id的项目。其结果是使得该核心键盘之phys_id键盘为0
 * return: 0/-1
 */
int clear_phys_keyboard(int phys_id){
	int i;

	for(i=0; i<MAX_KEYBOARDS; i++){
		if(keyboard_clients[i].phys_id == phys_id){
			keyboard_clients[i].phys_id =0;
			return 0;
		}
	}
	return -1;
}
