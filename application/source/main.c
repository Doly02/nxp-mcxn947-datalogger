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
static StackType_t g_xMscTaskStack[MSC_STACK_SIZE];
/*!
 * @brief 	TCB (Task Control Block) - Metadata of Mass Storage Task.
 * @details Includes All The Information Needed to Manage The Task Such As Job Status,
 * 			Job Stack Pointer, Values of Variables During Context Switching.
 */
static StaticTask_t g_xMscTaskTCB;

static StackType_t g_xRecordTaskStack[RECORD_STACK_SIZE];

static StaticTask_t g_xRecordTaskTCB;

/*!
 * @brief Global USB MSC Structure.
 *
 * MISRA Deviation: Rule 8.4
 * Justification: Declaration of 'g_msc' is intentionally placed in 'app_tasks.h', T
 * , the current file only provides the definition.
 */
/*lint -e9075 */
usb_msc_struct_t g_msc;
/*lint +e9075  */

TaskHandle_t g_xMscTaskHandle 	 = NULL;

TaskHandle_t g_xRecordTaskHandle = NULL;

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
#if (CONTROL_LED_ENABLED == true)
		LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */
    }

    /* Launch Record Task With Dominance */
    (void)xSemaphoreGive(g_xSemRecord);

    /* Initialize board hardware. */
	APP_InitBoard();

	g_xRecordTaskHandle = xTaskCreateStatic(
    			  record_task,       		/* Function That Implements The Task. 		*/
                  "record_task",          	/* Text Name For The Task. 					*/
				  MSC_STACK_SIZE,      		/* Number of Indexes In The xStack Array. 	*/
                  NULL,    					/* Parameter Passed Into The Task. 			*/
				  TASK_PRIO,				/* Priority at Which The Task Is Created. 	*/
				  &g_xRecordTaskStack[0],   /* Array To Use As The Task's Stack.		*/
                  &g_xRecordTaskTCB );
    if (NULL == g_xRecordTaskHandle)
    {
    	PRINTF("ERR: MSC Task Creation Failed!\r\n");
#if (CONTROL_LED_ENABLED == true)
		LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */
    	ERR_HandleError();
    }


#if (true == MSC_ENABLED)

    g_xMscTaskHandle = xTaskCreateStatic(
    			  msc_task,       			/* Function That Implements The Task. 		*/
                  "msc_task",          		/* Text Name For The Task. 					*/
				  MSC_STACK_SIZE,      		/* Number of Indexes In The xStack Array. 	*/
                  NULL,    					/* Parameter Passed Into The Task. 			*/
				  TASK_PRIO - 1,			/* Priority at Which The Task Is Created. 	*/
				  &g_xMscTaskStack[0],      /* Array To Use As The Task's Stack.		*/
                  &g_xMscTaskTCB );
    if (NULL == g_xMscTaskHandle)
    {
    	PRINTF("ERR: MSC Task Creation Failed!\r\n");
#if (CONTROL_LED_ENABLED == true)
		LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */
    	ERR_HandleError();
    }

#endif /* (true == MSC_ENABLED) */

    vTaskStartScheduler();
    while (true)
    {
    	;
    }

}
