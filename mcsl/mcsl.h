/*
 * mcsl.h
 *
 *  Created on: 2011-3-17
 *      Author: leiming
 */

#ifndef MCSL_H_
#define MCSL_H_

#include "mcsltypes.h"
#include "mcslbox.h"
#include "mcsltopwin.h"
#include "mcslbutton.h"
#include "mcslinput.h"
#include "mcsllabel.h"
#include "mcslcanvas.h"



/*
 * Programmer Needs thes Macro
 */

/*
 * Init enviroment.
 * First called
 */
extern int mcsl_init(int argc , char **argv);

/*
 * Deal with events
 */
extern void mcsl_main(void);

/*
 * exit app
 */
extern void mcsl_exit(void);

///////////////SPECIAL FUNCTONS/////////////////////
/*
 * get number of master device in the system
 */
extern int mcsl_count_device(void);


#endif /* MCSL_H_ */
