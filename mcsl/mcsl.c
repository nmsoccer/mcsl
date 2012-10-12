/*
 * mcsl.c
 *
 *  Created on: 2011-3-17
 *      Author: leiming
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/xpm.h>

#include <X11/extensions/XInput2.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>

#include <stdio.h>

#include "mcslglobal.h"
#include "mcsl.h"
#include "mcsldevice.h"
#include "mcsltool.h"

#include "res/mcsl_button_px.xpm"
#include "res/mcsl_button_p_px.xpm"


/*
 * GLOBAL VAR
 * ########################
 */

/*Display*/
Display *display;
int screen_num;

/*TOPWIN*/
Window mcsl_topwin;

/*GC*/
mcsl_u32 mcsl_valuemask;
XGCValues mcsl_values;
GC mcsl_gc;	/*用于普通控件*/

/*FONT*/
XFontStruct *mcsl_font;
char **mcsl_font_names;

/*Window Attributes*/
XWindowAttributes win_attr;
XSetWindowAttributes win_set_attr;

/*Cursor*/
Cursor mcsl_cur;
unsigned int cursor_shape[MAX_CURSOR_SHAPE];
/*Colors*/
mcsl_u32 colors[MAX_COLORS];

/*XPM*/
bgmap_info mcsl_pixmaps[MAX_PIXMAP];
XpmAttributes mcsl_xpmattr;


/*Pointer & Keyboard Device Info*/
pointer_device *p_device;	//pointer_device info
keyboard_device *k_device;	//keyboard_device info

/*
 * LOCAL VAR
 * ########################
 */
static Window close_win;	/*close application Recommanded*/

static int opcode , f_event , f_error;	/*XI opcode*/

static int slave_id = 0;	/*new added slave_id not set*/
static int seq_master = 1;	/*seq of master in pointer_device*/
static int confirm_create_master = 1;	/*标志位。是否真正创建master*/

static int nr_devices;	/*current number of devices*/
static XIDeviceInfo *devices;	/*device_info*/
static XIDeviceInfo *device;


static XIAddMasterInfo add_info;	/*Hierarchy Change Info*/
static XIRemoveMasterInfo remove_info;
static XIAttachSlaveInfo attach_info;
static XIDetachSlaveInfo detach_info;


int mcsl_init(int argc , char **argv){
	printf("Hello , Welcom to MCSL world~\n");
	int i;

	display = NULL;
	screen_num = 0;

	memset(colors , 0 , sizeof(colors));


	/*Open display*/
	display = XOpenDisplay(NULL);
	if(!display){
		MCSLDUMP_ERR(Open Display);
		return -1;
	}
	screen_num = DefaultScreen(display);

	/*
	 * Get HINTS
	 */
	XSizeHints *size_hints;
	XWMHints *wm_hints;
	XClassHint *class_hint;

	size_hints = XAllocSizeHints();
	if(!size_hints){
		MCSLDUMP_ERR(Allocate SizeHints);
		return -1;
	}

	wm_hints = XAllocWMHints();
	if(!wm_hints){
		MCSLDUMP_ERR(Allocate WMHints);
		return -1;
	}

	class_hint = XAllocClassHint();
	if(!class_hint){
		MCSLDUMP_ERR(Allocate ClassHint);
		return -1;
	}

	/*XI Extension*/
	int major , minor;
	XIEventMask eventmask;
	unsigned char mask[2] = {0 , 0};

	if(!XQueryExtension(display , "XInputExtension" , &opcode , &f_event , &f_error)){
		MCSLDUMP_ERR(X Input extension not available);
		return -1;
	}
	printf("Support XI opcode: %d\n" , opcode);


	major = 2;
	minor = 0;
	if(XIQueryVersion(display , &major , &minor) == BadRequest){
		MCSLDUMP_ERR(XI2 not available);
	}
	printf("Server supports %d.%d\n" , major , minor);


	/*
		 * Set Colors
		 */
		Colormap map;
		XColor screen_color , exact_color;

		colors[BLACK] = XBlackPixel(display , screen_num);
		colors[WHITE] = XWhitePixel(display , screen_num);

		map = XDefaultColormap(display , screen_num);

		XAllocNamedColor(display , map , "RED" , &screen_color , &exact_color);
		colors[RED] = screen_color.pixel;
		XAllocNamedColor(display , map , "GREEN" , &screen_color , &exact_color);
		colors[GREEN] = screen_color.pixel;
		XAllocNamedColor(display , map , "BLUE" , &screen_color , &exact_color);
		colors[BLUE] = screen_color.pixel;
		XAllocNamedColor(display , map , "YELLOW" , &screen_color , &exact_color);
		colors[YELLOW] = screen_color.pixel;
		XAllocNamedColor(display , map , "ORANGE" , &screen_color , &exact_color);
		colors[ORANGE] = screen_color.pixel;
		XAllocNamedColor(display , map , "CADET BLUE" , &screen_color , &exact_color);
		colors[CADET_BLUE] = screen_color.pixel;
		XAllocNamedColor(display , map , "MEDIUM AQUAMARINE" , &screen_color , &exact_color);
		colors[MEDIUM_AQUAMARINE] = screen_color.pixel;

		/*
		 * set cursor_shape
		 */
		for(i=1; i<MAX_CURSOR_SHAPE; i++){
			cursor_shape[i] = XC_clock + (i-1) *8 ;	/*refer cursorfont.h*/
		}
		cursor_shape[POINTER1] = XC_dot;
		cursor_shape[POINTER2] = XC_clock;
		cursor_shape[POINTER3] = XC_cross;
		cursor_shape[POINTER4] = XC_coffee_mug;
		cursor_shape[POINTER5] = XC_boat;

	/*
	 * Create mcsl_topwin and select input
	 */
	mcsl_topwin = XCreateSimpleWindow(display , RootWindow(display , screen_num) , 0 , 0 , DisplayWidth(display , screen_num) / 2 ,
			DisplayHeight(display , screen_num) / 2 , 1 , colors[BLACK] , colors[WHITE]);

	XSelectInput(display , mcsl_topwin , ExposureMask | StructureNotifyMask | ButtonPressMask);

	eventmask.deviceid = XIAllDevices;
	eventmask.mask_len = sizeof(mask);
	eventmask.mask = mask;

	XISetMask(mask , XI_HierarchyChanged);
	XISetMask(mask , XI_ButtonPress);
	XISetMask(mask , XI_ButtonRelease);
//	XISetMask(mask , XI_Motion);
//	XISetMask(mask , XI_Enter);
//	XISetMask(mask , XI_Leave);
	XISelectEvents(display , mcsl_topwin , &eventmask , 1);

	/*
	 * Create GC
	 */
	mcsl_valuemask = GCForeground | GCBackground;

	mcsl_values.foreground = colors[CADET_BLUE];
	mcsl_values.background = colors[WHITE];
	mcsl_gc = XCreateGC(display , mcsl_topwin , mcsl_valuemask , &mcsl_values);


	/*
	 * Font
	 */
//	mcsl_font = XLoadQueryFont(display , "9x15");	/*默认字体*/
	mcsl_font = load_font(HELVETICA , MEDIUM , ROMAN , SIZE_DEFAULT);
	if(mcsl_font == NULL){
		printf("load font error!\n");
	}


	/*
	 * Set Hints
	 */
	size_hints->flags = PPosition | PSize | PMinSize;
	size_hints->min_width = 50;
	size_hints->min_height = 50;

	wm_hints->initial_state = NormalState;
	wm_hints->input = True;

	class_hint->res_name = "MCSL";
	class_hint->res_class = "MCSL";

	XSetWMProperties(display , mcsl_topwin , NULL , NULL , argv , argc , size_hints , wm_hints , class_hint);

	/*Create close_window*/
	close_win = XCreateSimpleWindow(display , mcsl_topwin , 0 , 0 , 10 , 10 , 1 , colors[BLACK] , colors[BLACK]);
	XSelectInput(display , mcsl_topwin , ExposureMask | StructureNotifyMask);

	memset(mask , 0 , sizeof(mask));
	eventmask.deviceid = XIAllMasterDevices;
	eventmask.mask_len = sizeof(mask);
	eventmask.mask = mask;

	XISetMask(mask , XI_ButtonPress);
	XISelectEvents(display , close_win , &eventmask , 1);

	/*Create pointer_device*/
	p_device = create_pointer_device();
	k_device = create_keyboard_device();
	devices = XIQueryDevice(display , XIAllDevices , &nr_devices);	/*initiate current pointer_device*/
	for(i=0; i<nr_devices; i++){
		device = &devices[i];

		switch(device->use){		/*Pointer device*/
		case XIMasterPointer:
			add_master_pointer(p_device , device->deviceid , device->name);
			break;
		case XIMasterKeyboard:
			add_master_keyboard(k_device , device->deviceid , device->name);
			break;
		case XISlavePointer:
			add_slave_pointer(p_device , device->attachment , device->deviceid , device->name);
			break;
		case XISlaveKeyboard:
			add_slave_keyboard(k_device , device->attachment , device->deviceid , device->name);
			break;
		case XIFloatingSlave:
			break;
		default:
			break;
		}

	}
	XIFreeDeviceInfo(devices);
	tree_pointer_device(p_device);


	/*Inite mcsl_store*/
	mcsl_store.head.content = NULL;
	mcsl_store.head.next = NULL;
	mcsl_store.last = &mcsl_store.head;

	/*Create Pixmap*/
	/*USE XPM*/
	memset(mcsl_pixmaps , 0 , sizeof(mcsl_pixmaps));

	/*button*/
	memset(&mcsl_xpmattr, 0, sizeof(mcsl_xpmattr));
	XpmCreatePixmapFromData(display, mcsl_topwin, mcsl_button_px , &mcsl_pixmaps[MCSL_BUTTON_PX].bg_map , NULL, &mcsl_xpmattr);
	mcsl_pixmaps[MCSL_BUTTON_PX].map_width = mcsl_xpmattr.width;
	mcsl_pixmaps[MCSL_BUTTON_PX].map_height = mcsl_xpmattr.height;
	printf("it is:%d x %d\n" , mcsl_xpmattr.width , mcsl_xpmattr.height);

	memset(&mcsl_xpmattr, 0, sizeof(mcsl_xpmattr));
	XpmCreatePixmapFromData(display, mcsl_topwin, mcsl_button_p_px , &mcsl_pixmaps[MCSL_BUTTON_P_PX].bg_map, NULL, &mcsl_xpmattr);
	mcsl_pixmaps[MCSL_BUTTON_P_PX].map_width = mcsl_pixmaps[MCSL_BUTTON_PX].map_width;
	mcsl_pixmaps[MCSL_BUTTON_P_PX].map_height = mcsl_pixmaps[MCSL_BUTTON_PX].map_height;

}


/*
 * Deal with events
 */
void mcsl_main(void){
	int i , j , k;

	mcsl_u32 x0 , y0 , w0 , h0;
	mcsl_u32 x1 , y1 , w1 , h1;

	XEvent event;

	XGenericEventCookie *cookie;
	XIHierarchyEvent *xi_hierarchy_ev;
	XIDeviceEvent *xi_device_ev;

	char pointer_name[DEVICE_NAME_LEN];
	char keyboard_name[DEVICE_NAME_LEN];

	XIEventMask eventmask;
	unsigned char mask[2] = {0 , 0};

	mcslobj *widget;
	mcslbox *box;
	mcsl_cargo *cargo;

	memset(pointer_name , 0 , DEVICE_NAME_LEN);
	memset(keyboard_name , 0 , DEVICE_NAME_LEN);

	/*将所有注册的控件显示之*/
	cargo = &mcsl_store.head;
	while(cargo){

		widget = cargo->content;
		if(widget){

			/*如果没有选择光标同时也没有选择键盘那么默认选择任意光标*/
			if((widget->flags.chosen_pointer == NOPOINTER) && (widget->flags.chosen_keyboard == NOKEYBOARD)){
				if(widget->type == MCSL_INPUT){	/*如果是input控件默认任意键盘*/
					widget->flags.chosen_keyboard = ANYKEYBOARD;
				}else{	/*其他控件默认任何鼠标*/
					widget->flags.chosen_pointer = ANYPOINTER;
				}
				memset(mask , 0 , sizeof(mask));	/*reattach new cursor to a window registered*/
				eventmask.deviceid = XIAllMasterDevices;

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
				XISelectEvents(display , widget->basic_attr.win , &eventmask , 1);
			}


			/*如果该控件具有容器*/
			if(widget->basic_attr.container){	/*逐次安排该容器里的控件*/
 			box = widget->basic_attr.container;
				x0 = 0;
				y0 = 0;
				w0 = widget->coordinate.width;
 			h0 = widget->coordinate.height;

				while(box){	/*arrange box*/
					arrange_mcslbox(box , x0 , y0 , w0 , h0 , &x1 , &y1 , &w1 , &h1);

					x0 = x1;
					y0 = y1;
					w0 = w1;
					h0 = h1;
					box = box->in_box;
				}	/*end while*/

				delete_mcslbox(widget->basic_attr.container);	/*删除该控件之盒子*/

			}	/*end arrange box*/

			/*如果设置为控件可见*/
			if(widget->flags.visible){
				XMapWindow(display , widget->basic_attr.win);
			}


		}
		cargo = cargo->next;
	}

	XMapWindow(display , close_win);

	/*
	 * handle event loop
	 */
	while(1){
start_deal_event:
		XNextEvent(display , &event);


		if(event.xcookie.type == GenericEvent && event.xcookie.extension == opcode && XGetEventData(display , &event.xcookie)){
			cookie = &event.xcookie;

			switch(cookie->evtype){

			/*Hierarchy Changed*/
			case XI_HierarchyChanged:
				xi_hierarchy_ev = (XIHierarchyEvent *)cookie->data;

				/*Master Added*/
				if(xi_hierarchy_ev->flags & XIMasterAdded){

					for(i=0; i<xi_hierarchy_ev->num_info; i++){/*searching id of added master*/

						if(xi_hierarchy_ev->info[i].flags & XIMasterAdded){	/*find master_id*/

							if(!confirm_create_master){	/*键盘插入时发现提前有该键盘衍生光标插入，那么会将该位置0.表示不需要真正创建*/
								printf("remove keyboard pointer!\n");
								remove_info.type = XIRemoveMaster;	/*remove master*/
								remove_info.deviceid = xi_hierarchy_ev->info[i].deviceid;
								remove_info.return_mode = XIRemoveMaster;
								XIChangeHierarchy(display , (XIAnyHierarchyChangeInfo *)&remove_info , 1);
								confirm_create_master = 1;
								break;
							}

							/*confirm_create_master == 1 表示的确为真正的光标。衍生光标信息不会录入pointer_device之中*/
							add_master_pointer(p_device , xi_hierarchy_ev->info[i].deviceid , "LeiMing");
							add_master_keyboard(k_device , xi_hierarchy_ev->info[i].deviceid + 1 , "LeiMing");	/*同时加入核心键盘信息*/
							if(slave_id){	/*attach new added slave to new master*/
								attach_info.type = XIAttachSlave;
								attach_info.deviceid = slave_id;
								attach_info.new_master = xi_hierarchy_ev->info[i].deviceid;
								XIChangeHierarchy(display , (XIAnyHierarchyChangeInfo *)&attach_info , 1);

								add_slave_pointer(p_device , xi_hierarchy_ev->info[i].deviceid , slave_id , pointer_name);	/*update p_device info*/
								dispatch_cursor_keyboard(p_device , k_device , xi_hierarchy_ev->info[i].deviceid);
								slave_id = 0;
							}

							break;
						}	/*end if:find match master*/

					}	/*end for*/
					break;
				}


				/*Master Removed*/
				if(xi_hierarchy_ev->flags & XIMasterRemoved){
					break;
				}

				/*Slave Added*/
				if(xi_hierarchy_ev->flags & XISlaveAdded){

					for(i=0; i<xi_hierarchy_ev->num_info; i++){/*searching id of added device*/

						if(xi_hierarchy_ev->info[i].flags & XISlaveAdded){	/*match*/
							printf("added!\n");
							/*根据添加的设备id找到该设备是从光标设备还是从键盘设备*/
							devices = XIQueryDevice(display , XIAllDevices , &nr_devices);
							for(j=0; j<nr_devices; j++){
								device = &devices[j];
								if(device->deviceid != xi_hierarchy_ev->info[i].deviceid){
									continue;
								}
								/*找到了对应的设备id*/
								switch(device->use){
								case XISlavePointer:		/*如果是添加的物理光标。那么增加一个新的核心设备*/
									printf("it is slave pointer!\n");

									/*如果该从光标设备是由从键盘的插入产生的衍生光标那么不作处理
									 */
									if(strcmp(device->name , keyboard_name) == 0){
										printf("this pointer is generated by keyboard! do nothing\n");
										break;
									}
									/*是真正的物理光标那么产生一个新的核心光标。但是也有可能先于键盘产生的衍生光标*/
									printf("it is a real physical pointer!\n");
									memset(pointer_name , 0 , DEVICE_NAME_LEN);
									strcpy(pointer_name , device->name);

									/*首先寻找pointer_device看是否存在没有从光标的核心光标，如果有将此光标加入，不再建立核心光标*/
									for(j=0; j<MAX_MASTER; j++){
										if((p_device->master_pointers[j].master_id != 0) && (p_device->master_pointers[j].nr_slave == 0)){
											printf("attach to a exiested master pointer!\n");
											attach_info.type = XIAttachSlave;
											attach_info.deviceid = xi_hierarchy_ev->info[i].deviceid;
											attach_info.new_master = p_device->master_pointers[j].master_id;
											XIChangeHierarchy(display , (XIAnyHierarchyChangeInfo *)&attach_info , 1);

											add_slave_pointer(p_device , p_device->master_pointers[j].master_id , xi_hierarchy_ev->info[i].deviceid , pointer_name);	/*update p_device info*/
											goto start_deal_event;
										}
									}

									add_info.type = XIAddMaster;	/*create a new master*/
									add_info.name = "LeiMing";
									add_info.send_core = True;
									add_info.enable = True;

									XIChangeHierarchy(display , (XIAnyHierarchyChangeInfo *)&add_info , 1);
									slave_id = xi_hierarchy_ev->info[i].deviceid;
									break;
								case XISlaveKeyboard:
									printf("it is slave keyboard!\n");
									/*将此键盘名称复制到keyboard_name暂时保存。因为插入一个物理键盘会同时
									 * 产生一个与其同名的衍生光标
									 */
									memset(keyboard_name , 0 , DEVICE_NAME_LEN);
									strcpy(keyboard_name , device->name);

									dispatch_phys_keyboard(k_device , device->deviceid , device->name);

									/*如果由键盘插入产生的衍生光标先于键盘产生，那么已经产生了一个无效的核心设备
									 * 找到该核心设备(如果有)。删除之。
									 */
									if(strcmp(keyboard_name , pointer_name) == 0){

										/*第一步检查此时衍生光标是否已经成功建立，如果已经成功建立删除之*/
										for(i=1; i<MAX_MASTER; i++){	/*此处i,j可以有新的作用了*/
											if(p_device->master_pointers[i].master_id == 0){
												continue;
											}
											for(j=0; j<MAX_SLAVE; j++){
												/*如果找到某核心设备的从设备是该键盘插入产生的。表示在键盘插入时其衍生的
												 * 光标已经成功建立一套核心光标而且可能已经分配给一系列窗口。此时将该核心设备保留
												 * 但删除其从光标与(因为它是无效的)等待真实的物理光标插入。
												 */
												if(strcmp(p_device->master_pointers[i].slave_names[j] , keyboard_name) == 0){
													printf("Find ERROR MASTER POINTER\n");
													/*删除从光标设备*/
													remove_slave_pointer(p_device , p_device->master_pointers[i].slave_id[j]);
//													tree_pointer_device(p_device);
													goto deal_slave_keyboard;
												}
											}	/*end search slave per master*/
										}	/*end search all master pointers*/

										/*此处没有发现已经建立的核心光标。说明还没有收到XICreateMaster事件，因此我们阻止核心光标建立*/
										confirm_create_master = 0;

									}	/*end 删除衍生光标*/

deal_slave_keyboard:
//									tree_keyboard_device(k_device);

									break;
								default:
									break;
								}	/*end switch*/

								XIFreeDeviceInfo(devices);
								break;

							}	/*end searching type of slave device*/

							break;
						}	/*end matching*/

					}	/*end for*/
					break;
				}

				/*Slave Removed*/
				if(xi_hierarchy_ev->flags & XISlaveRemoved){
					for(i=0; i<xi_hierarchy_ev->num_info; i++){/*searching id of removed device*/

						if(xi_hierarchy_ev->info[i].flags & XISlaveRemoved){
							printf("removed %d!\n" , xi_hierarchy_ev->info[i].deviceid);

							/*根据删除的设备id找到该设备是从光标设备还是从键盘设备*/

							/*首先检测是否是键盘*/
							if(remove_slave_keyboard(k_device , xi_hierarchy_ev->info[i].deviceid) != -1){
								/*删除从键盘不会删除整个核心设备。即使核心键盘之从设备数目为0*/
								clear_phys_keyboard(xi_hierarchy_ev->info[i].deviceid);
								printf("remove slave keyboard success!\n");

								break;
							}

							/*上面表明不是从键盘*/
							if((seq_master = remove_slave_pointer(p_device , xi_hierarchy_ev->info[i].deviceid)) == -1){
								/*表明是删除的键盘产生的衍生光标。该光标未在p_device中注册。不做处理*/
								printf("remove keyboard pointer success!\n");
								break;
							}

							/*表明是真正的物理光标*/
							printf("it is a real physical pointer!\n");
							/*如果该光标对应的核心设备不再有从设备那么删除该核心设备*/
							if(p_device->master_pointers[seq_master].nr_slave == 0){

								remove_info.type = XIRemoveMaster;	/*remove master*/
								remove_info.deviceid = p_device->master_pointers[seq_master].master_id;
								remove_info.return_mode = XIAttachToMaster;
								remove_info.return_pointer = p_device->master_pointers[0].master_id;
								remove_info.return_keyboard = p_device->master_pointers[0].master_id + 1;
								XIChangeHierarchy(display , (XIAnyHierarchyChangeInfo *)&remove_info , 1);

								remove_master_keyboard(k_device , p_device->master_pointers[seq_master].master_id + 1);
								remove_master_pointer(p_device , p_device->master_pointers[seq_master].master_id);
							}

//							tree_pointer_device(p_device);
//							tree_keyboard_device(k_device);
							break;
						}	/*end if:find id*/
					}
					break;
				}

				break;

			case XI_DeviceChanged:
				printf("deviceChanged!");
				break;
			case XI_ButtonPress:
				xi_device_ev = (XIDeviceEvent *)cookie->data;

				if(xi_device_ev->event == close_win){	/*close application 这个窗口是特殊窗口，它并未在商店里注册*/
					remove_info.type = XIRemoveMaster;	/*只保留一个鼠标*/
					remove_info.return_mode = XIAttachToMaster;
					remove_info.return_pointer = p_device->master_pointers[0].master_id;
					remove_info.return_keyboard = p_device->master_pointers[0].master_id + 1;
					for(i=1; i<MAX_MASTER; i++){	/*only maitain one cursor*/
						if(p_device->master_pointers[i].master_id){
							remove_info.deviceid = p_device->master_pointers[i].master_id;
							XIChangeHierarchy(display , (XIAnyHierarchyChangeInfo *)&remove_info , 1);
						}
					}

					printf("ready to exit...!\n");
					delete_keyboard_device(k_device);
					delete_pointer_device(p_device);
					printf("destroy device info scucess!\n");
					delete_mcsl_store();
					printf("destroy widget store scucess!\n");
					XFreeGC(display , mcsl_gc);
					XDestroyWindow(display , mcsl_topwin);
					XCloseDisplay(display);
					printf("Bye bye......\n");
				}else{
					widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/

					switch(xi_device_ev->detail){	/*确定按下的鼠标键*/
					case 2:
						widget->inner_interface.cook_button2_press(widget , xi_device_ev);	/*按下中键 调用对应的内调处理函数*/
						break;
					case 3:
						widget->inner_interface.cook_button3_press(widget , xi_device_ev);	/*按下右键 调用对应的内调处理函数*/
						break;
					default:
						widget->inner_interface.cook_button_press(widget , xi_device_ev);	/*按下左键 调用对应的内调处理函数*/
						break;
					}
				}
				break;

			case XI_ButtonRelease:
				xi_device_ev = (XIDeviceEvent *)cookie->data;

				widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/

				switch(xi_device_ev->detail){	/*确定释放的鼠标键*/
				case 2:
					widget->inner_interface.cook_button2_release(widget , xi_device_ev);	/*释放中键 调用对应的内调处理函数*/
					break;
				case 3:
					widget->inner_interface.cook_button3_release(widget , xi_device_ev);	/*释放右键 调用对应的内调处理函数*/
					break;
				default:
					widget->inner_interface.cook_button_release(widget , xi_device_ev);	/*释放左键 调用对应的内调处理函数*/
					break;
				}

				break;

			case XI_Motion:
				xi_device_ev = (XIDeviceEvent *)cookie->data;

				widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/
				widget->inner_interface.cook_motion(widget , xi_device_ev);	/*调用对应的内调处理函数*/
				break;

			case XI_Enter:
				xi_device_ev = (XIDeviceEvent *)cookie->data;

				widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/
				widget->inner_interface.cook_enter(widget , xi_device_ev);	/*调用对应的内调处理函数*/
				break;

			case XI_Leave:
				xi_device_ev = (XIDeviceEvent *)cookie->data;

				widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/
				widget->inner_interface.cook_leave(widget , xi_device_ev);	/*调用对应的内调处理函数*/
				break;

			case XI_KeyPress:
				xi_device_ev = (XIDeviceEvent *)cookie->data;

				widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/
				widget->inner_interface.cook_key_press(widget , xi_device_ev);	/*调用对应的内调处理函数*/
				break;

			case XI_KeyRelease:
				xi_device_ev = (XIDeviceEvent *)cookie->data;

				widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/
				widget->inner_interface.cook_key_release(widget , xi_device_ev);	/*调用对应的内调处理函数*/
				break;

			default:
				break;
			}

			XFreeEventData(display , &event.xcookie);
			continue;
		}



		/*General Event*/
		switch(event.type){
		case ConfigureRequest:
			printf("configure request from server when client move&resize window while do nothing!\n");
			break;

		case Expose:
			if(event.xexpose.count == 0){
				widget = fetch_mcsl_cargo(event.xexpose.window);	/*根据窗口选择已经注册的控件头部*/
				widget->inner_interface.cook_expose(widget , &event);	/*调用对应的内调处理函数*/
			}
			break;

		case ConfigureNotify:
			break;

		default:
			break;
		}

	}/*end while*/

}	/*end function*/




/*
 * exit app
 */
void mcsl_exit(void){
	printf("ready to exit...!\n");
	delete_keyboard_device(k_device);
	delete_pointer_device(p_device);
	printf("destroy device info scucess!\n");
	delete_mcsl_store();
	printf("destroy widget store scucess!\n");
	XFreeGC(display , mcsl_gc);
	XDestroyWindow(display , mcsl_topwin);
	XCloseDisplay(display);
	printf("Bye bye......\n");

}

/*
 * get number of master device in the system
 */
int mcsl_count_device(void){
	int number , i;

	for(number=0,i=0; i<MAX_MASTER; i++){

		if(p_device->master_pointers[i].master_id != 0){	/*add number when finding master*/
			number++;
		}

	}



	return number;
}



