/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      mainc.c
 *  Author:         Tomas Dolak
 *  Date:           07.08.2024
 *  Description:    Implements Datalogger Application.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           main.c
 *  @author         Tomas Dolak
 *  @date           07.08.2024
 *  @brief          Implements Datalogger Application.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/

/* NXP Board Specific */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_clock.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/* FreeRTOS Includes. */
#include "semphr.h"

/* Application Includes */
#include "app_init.h"
#include "app_tasks.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
/*!
 * @brief 	Buffer for Static Stack of Mass Storage Task.
 */
static StackType_t mscTaskStack[MSC_STACK_SIZE];
/*!
 * @brief 	TCB (Task Control Block) - Metadata of Mass Storage Task.
 * @details Includes All The Information Needed to Manage The Task Such As Job Status,
 * 			Job Stack Pointer, Values of Variables During Context Switching.
 */
static StaticTask_t mscTaskTCB;

static StackType_t recordTaskStack[RECORD_STACK_SIZE];

static StaticTask_t recordTaskTCB;

usb_msc_struct_t g_msc;

TaskHandle_t mscTaskHandle 		= NULL;

TaskHandle_t recordTaskHandle 	= NULL;

SemaphoreHandle_t g_xSemRecord;

SemaphoreHandle_t g_xSemMassStorage;

/*******************************************************************************
 * Code
 ******************************************************************************/
 
/*!
 * @brief Application Entry Point.
 */
int main(void)
{

	g_xSemRecord = xSemaphoreCreateBinary();
	g_xSemMassStorage = xSemaphoreCreateBinary();

    if ((NULL == g_xSemRecord) || (NULL == g_xSemMassStorage))
    {
        PRINTF("ERR: Failed to Create Semaphores!\n");
        ERR_HandleError();
    }

    /* Launch Record Task With Dominance */
    xSemaphoreGive(g_xSemRecord);

    /* Initialize board hardware. */
	APP_InitBoard();

    recordTaskHandle = xTaskCreateStatic(
    			  record_task,       		/* Function That Implements The Task. 		*/
                  "record_task",          	/* Text Name For The Task. 					*/
				  MSC_STACK_SIZE,      		/* Number of Indexes In The xStack Array. 	*/
                  NULL,    					/* Parameter Passed Into The Task. 			*/
				  TASK_PRIO,				/* Priority at Which The Task Is Created. 	*/
				  &recordTaskStack[0],      /* Array To Use As The Task's Stack.		*/
                  &recordTaskTCB );
    if (NULL == recordTaskHandle)
    {
    	PRINTF("ERR: MSC Task Creation Failed!\r\n");
    	ERR_HandleError();
    }


#if (true == MSC_ENABLED)

    mscTaskHandle = xTaskCreateStatic(
    			  msc_task,       			/* Function That Implements The Task. 		*/
                  "msc_task",          		/* Text Name For The Task. 					*/
				  MSC_STACK_SIZE,      		/* Number of Indexes In The xStack Array. 	*/
                  NULL,    					/* Parameter Passed Into The Task. 			*/
				  TASK_PRIO - 1,			/* Priority at Which The Task Is Created. 	*/
				  &mscTaskStack[0],         /* Array To Use As The Task's Stack.		*/
                  &mscTaskTCB );
    if (NULL == mscTaskHandle)
    {
    	PRINTF("ERR: MSC Task Creation Failed!\r\n");
    	ERR_HandleError();
    }

#endif /* (true == MSC_ENABLED) */

    vTaskStartScheduler();
    while (1)
    {
    	;
    }

    return ERROR_OUT_OF_CYCLE;

}
