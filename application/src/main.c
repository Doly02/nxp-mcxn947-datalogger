/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      mainc.c
 *  Author:         Tomas Dolak
 *  Date:           07.08.2024
 *  Description:    Implements Data Logger Application.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           main.c
 *  @author         Tomas Dolak
 *  @date           07.08.2024
 *  @brief          Implements Data Logger Application.
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
/**
 * @brief 	Buffer for Static Stack of Mass Storage Task.
 */
static StackType_t g_xMscTaskStack[MSC_STACK_SIZE];

/**
 * @brief 	TCB (Task Control Block) - Meta Data of Mass Storage Task.
 * @details Includes All The Information Needed to Manage The Task Such As Job Status,
 * 			Job Stack Pointer, Values of Variables During Context Switching.
 */
static StaticTask_t g_xMscTaskTCB;

/**
 * @brief  Buffer For Static Stack Of Record Task.
 */
static StackType_t g_xRecordTaskStack[RECORD_STACK_SIZE];

/**
 * @brief  TCB (Task Control Block) - Meta Data of Record Task.
 * @details Includes All The Information Needed to Manage The Task Such As Job Status,
 *          Job Stack Pointer, Values of Variables During Context Switching.
 */
static StaticTask_t g_xRecordTaskTCB;


 /**
 * MISRA Deviation: Rule 8.4 [Required]
 * Suppress: External Object Has Definition Without Prior Declaration in This File.
 * Justification: Declaration of 'g_msc' is intentionally placed in 'app_tasks.h',
 * The Current File Only Provides The Definition.
 */
/*lint -e9075 */
/**
 * @brief Global USB MSC Structure.
 */
usb_msc_struct_t g_msc;
/*lint +e9075  */

/**
 * @brief USB Mass Storage Task Handle.
 */
TaskHandle_t g_xMscTaskHandle 	 = NULL;

/**
 * @brief Semaphore For USB Mass Storage Task Management.
 */
SemaphoreHandle_t g_xSemMassStorage;

/**
 * @brief Record Task Handle.
 */
TaskHandle_t g_xRecordTaskHandle = NULL;

/**
 * @brief Semaphore For Record Task Management.
 */
SemaphoreHandle_t g_xSemRecord;

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
