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
GC mcsl_gc;

/*FONT*/
XFontStruct *mcsl_font;
char **mcsl_font_names;

/*Window Attributes*/
XWindowAttributes win_attr;
XSetWindowAttributes win_set_attr;

/*Cursor*/
Cursor mcsl_cur;

/*Colors*/
mcsl_u32 colors[MAX_COLORS];

/*XPM*/
Pixmap mcsl_pixmaps[MAX_PIXMAP];
XpmAttributes mcsl_xpmattr;


/*Pointer Device Info*/
pointer_device *p_device;	//pointer_device info


/*
 * LOCAL VAR
 * ########################
 */
static Window close_win;	/*close application Recommanded*/

static int opcode , f_event , f_error;	/*XI opcode*/

static int slave_id = 0;	/*new added slave_id not set*/
//static int device_id;	/*current device_id*/
static int seq_master = 1;	/*seq of master in pointer_device*/

static int nr_devices;	/*current number of devices*/
static XIDeviceInfo *devices;	/*device_info*/
static XIDeviceInfo *device;


static XIAddMasterInfo add_info;	/*Hierarchy Change Info*/
static XIRemoveMasterInfo remove_info;
static XIAttachSlaveInfo attach_info;
static XIDetachSlaveInfo detach_info;


int mcsl_init(int argc , char **argv){
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
		XAllocNamedColor(display , map , "CADET BLUE" , &screen_color , &exact_color);
		colors[CADET_BLUE] = screen_color.pixel;
		XAllocNamedColor(display , map , "CORAL" , &screen_color , &exact_color);
		colors[CORAL] = screen_color.pixel;
		XAllocNamedColor(display , map , "CYAN" , &screen_color , &exact_color);
		colors[CYAN] = screen_color.pixel;
		XAllocNamedColor(display , map , "GRAY" , &screen_color , &exact_color);
		colors[GRAY] = screen_color.pixel;
		XAllocNamedColor(display , map , "GREY" , &screen_color , &exact_color);
		colors[GREY] = screen_color.pixel;
		XAllocNamedColor(display , map , "LIGHT GRAY" , &screen_color , &exact_color);
		colors[LIGHT_GRAY] = screen_color.pixel;
		XAllocNamedColor(display , map , "LIGHT GREY" , &screen_color , &exact_color);
		colors[LIGHT_GREY] = screen_color.pixel;
		XAllocNamedColor(display , map , "MARGENTA" , &screen_color , &exact_color);
		colors[MARGENTA] = screen_color.pixel;
		XAllocNamedColor(display , map , "MAROON" , &screen_color , &exact_color);
		colors[MAROON] = screen_color.pixel;
		XAllocNamedColor(display , map , "AQUAMARINE" , &screen_color , &exact_color);
		colors[AQUAMARINE] = screen_color.pixel;
		XAllocNamedColor(display , map , "MEDIUM AQUAMARINE" , &screen_color , &exact_color);
		colors[MEDIUM_AQUAMARINE] = screen_color.pixel;
		XAllocNamedColor(display , map , "BLUE VIOLET" , &screen_color , &exact_color);
		colors[BLUE_VIOLET] = screen_color.pixel;


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

	mcsl_values.foreground = BlackPixel(display , screen_num);
	mcsl_values.background = WhitePixel(display , screen_num);
	mcsl_gc = XCreateGC(display , mcsl_topwin , mcsl_valuemask , &mcsl_values);


	/*
	 * Font
	 */
	mcsl_font = XLoadQueryFont(display , "9x15");



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

	devices = XIQueryDevice(display , XIAllDevices , &nr_devices);	/*initiate current pointer_device*/
	for(i=0; i<nr_devices; i++){
		device = &devices[i];

		switch(device->use){		/*Pointer device*/
		case XIMasterPointer:
			add_master_pointer(p_device , device->deviceid);
			break;
		case XIMasterKeyboard:
			break;
		case XISlavePointer:
			add_slave_pointer(p_device , device->attachment , device->deviceid);
			break;
		case XISlaveKeyboard:
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
	memset(&mcsl_xpmattr, 0, sizeof(mcsl_xpmattr));
	XpmCreatePixmapFromData(display, mcsl_topwin, mcsl_button_px , &mcsl_pixmaps[MCSL_BUTTON_PX] , NULL, &mcsl_xpmattr);	/*mcsl_button*/
	XpmCreatePixmapFromData(display, mcsl_topwin, mcsl_button_p_px , &mcsl_pixmaps[MCSL_BUTTON_P_PX], NULL, &mcsl_xpmattr);


}


/*
 * Deal with events
 */
void mcsl_main(void){
	int i;
	mcsl_u32 x0 , y0 , w0 , h0;
	mcsl_u32 x1 , y1 , w1 , h1;

	XEvent event;

	XGenericEventCookie *cookie;
	XIHierarchyEvent *xi_hierarchy_ev;
	XIDeviceEvent *xi_device_ev;


	XIEventMask eventmask;
	unsigned char mask[2] = {0 , 0};

	mcslobj *widget;
	mcslbox *box;
	mcsl_cargo *cargo;

	/*将所有注册的控件显示之*/
	cargo = &mcsl_store.head;
	while(cargo){

		widget = cargo->content;
		if(widget){	/*如果控件存在*/

			/*如果没有选择光标*/
			if(widget->flags.chosen_pointer == 0){
				widget->flags.chosen_pointer = ANYPOINTER;

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

//				delete_mcslbox(widget->basic_attr.container);	/*删除该控件之盒子*/

			}	/*end arrange box*/


			/*显示控件*/
			XMapWindow(display , widget->basic_attr.win);
		}
		cargo = cargo->next;
	}
	XMapWindow(display , close_win);

	/*
	 * handle event loop
	 */
	while(1){

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
							add_master_pointer(p_device , xi_hierarchy_ev->info[i].deviceid);	/*update pointer device*/

							if(slave_id){	/*attach new added slave to new master*/
								attach_info.type = XIAttachSlave;
								attach_info.deviceid = slave_id;
								attach_info.new_master = xi_hierarchy_ev->info[i].deviceid;
								XIChangeHierarchy(display , (XIAnyHierarchyChangeInfo *)&attach_info , 1);

								add_slave_pointer(p_device , xi_hierarchy_ev->info[i].deviceid , slave_id);	/*update p_device info*/
								dispatch_cursor(p_device , xi_hierarchy_ev->info[i].deviceid);
								tree_pointer_device(p_device);
								slave_id = 0;	/*clear slave_id*/
							}
							break;
						}	/*end reattach*/

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
							add_info.type = XIAddMaster;	/*create a new master*/
							add_info.name = "LeiMing";
							add_info.send_core = True;
							add_info.enable = True;

							XIChangeHierarchy(display , (XIAnyHierarchyChangeInfo *)&add_info , 1);
							slave_id = xi_hierarchy_ev->info[i].deviceid;
							break;
						}	/*end dealing with matching*/

					}	/*end for*/
					break;
				}

				/*Slave Removed*/
				if(xi_hierarchy_ev->flags & XISlaveRemoved){
					for(i=0; i<xi_hierarchy_ev->num_info; i++){/*searching id of removed device*/

						if(xi_hierarchy_ev->info[i].flags & XISlaveRemoved){
							seq_master = remove_slave_pointer(p_device , xi_hierarchy_ev->info[i].deviceid);	/*get seq_master of removed slave*/

							if(p_device->master_pointers[seq_master].nr_slave == 0){	/*if master slave attached to has no slave anymore,remove it*/

								remove_info.type = XIRemoveMaster;	/*remove master*/
								remove_info.deviceid = p_device->master_pointers[seq_master].master_id;
								remove_info.return_mode = XIAttachToMaster;
								remove_info.return_pointer = p_device->master_pointers[0].master_id;
								remove_info.return_keyboard = p_device->master_pointers[0].master_id + 1;
								XIChangeHierarchy(display , (XIAnyHierarchyChangeInfo *)&remove_info , 1);

								remove_master_pointer(p_device , p_device->master_pointers[seq_master].master_id);
							}
							tree_pointer_device(p_device);
							break;
						}
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

					delete_mcsl_store();
					delete_pointer_device(p_device);
					XFreeGC(display , mcsl_gc);
					XDestroyWindow(display , mcsl_topwin);
					XCloseDisplay(display);
					printf("Bye bye......\n");
				}else{
					widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/

					switch(xi_device_ev->detail){	/*确定按下的鼠标键*/
					case 2:
						widget->inner_interface.cook_button2_press(widget);	/*按下中键 调用对应的内调处理函数*/
						break;
					case 3:
						widget->inner_interface.cook_button3_press(widget);	/*按下右键 调用对应的内调处理函数*/
						break;
					default:
						widget->inner_interface.cook_button_press(widget);	/*按下左键 调用对应的内调处理函数*/
						break;
					}
				}
				break;

			case XI_ButtonRelease:
				xi_device_ev = (XIDeviceEvent *)cookie->data;

				widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/

				switch(xi_device_ev->detail){	/*确定释放的鼠标键*/
				case 2:
					widget->inner_interface.cook_button2_release(widget);	/*释放中键 调用对应的内调处理函数*/
					break;
				case 3:
					widget->inner_interface.cook_button3_release(widget);	/*释放右键 调用对应的内调处理函数*/
					break;
				default:
					widget->inner_interface.cook_button_release(widget);	/*释放左键 调用对应的内调处理函数*/
					break;
				}

				break;

			case XI_Motion:
				xi_device_ev = (XIDeviceEvent *)cookie->data;

				widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/
				widget->inner_interface.cook_motion(widget);	/*调用对应的内调处理函数*/
				break;

			case XI_Enter:
				xi_device_ev = (XIDeviceEvent *)cookie->data;

				widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/
				widget->inner_interface.cook_enter(widget);	/*调用对应的内调处理函数*/
				break;

			case XI_Leave:
				xi_device_ev = (XIDeviceEvent *)cookie->data;

				widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/
				widget->inner_interface.cook_leave(widget);	/*调用对应的内调处理函数*/
				break;

			case XI_KeyPress:
				xi_device_ev = (XIDeviceEvent *)cookie->data;

				widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/
				widget->inner_interface.cook_key_press(widget);	/*调用对应的内调处理函数*/
				break;

			case XI_KeyRelease:
				xi_device_ev = (XIDeviceEvent *)cookie->data;

				widget = fetch_mcsl_cargo(xi_device_ev->event);	/*根据窗口选择已经注册的控件头部*/
				widget->inner_interface.cook_key_release(widget);	/*调用对应的内调处理函数*/
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
				widget->inner_interface.cook_expose(widget);	/*调用对应的内调处理函数*/
			}
			break;

		case ConfigureNotify:
			break;

		case KeyPress:
			break;

		case ButtonPress:
			break;

		default:
			break;
		}

	}/*end while*/

}	/*end function*/
