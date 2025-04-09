/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      tasks.c
 *  Author:         Tomas Dolak
 *  Date:           14.09.2024
 *  Description:    Includes Implementation of Task For FreeRTOS.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           tasks.c
 *  @author         Tomas Dolak
 *  @date           14.09.2024
 *  @brief          Includes Implementation of Task For FreeRTOS.
 * ****************************/

#ifndef APP_TASKS_H_
#define APP_TASKS_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

/* FreeRTOS Include */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

/* Mass Storage Includes */
#include <disk.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TASK_PRIO		(configMAX_PRIORITIES - 1) //<! Task Priorities.

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
/* Task Handles */
extern TaskHandle_t g_xMscTaskHandle;

extern TaskHandle_t g_xRecordTaskHandle;

extern SemaphoreHandle_t g_xSemRecord;

extern SemaphoreHandle_t g_xSemMassStorage;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*!
 * @brief 	Task Responsible For Mass Storage Functionality in Device Mode.
 *
 * @details	This Task Implements USB Mass Storage Class (MSC) Operations, Allowing
 * 			The System to Act As a Mass Storage Device. It Handles Communication
 * 			With The Host and Manages Read/Write Operations.
 *
 * @param 	handle Pointer to The Device Handle Used For The USB Operations.
 */
void msc_task(void *handle);

void record_task(void *handle);
/*!
 * @brief 	Hook Function to Provide Memory For The Idle Task in FreeRTOS.
 *
 * @details This Hook Function Provides The Memory Needed For The Idle Task, Which Is
 * 			Statically Allocated When configSUPPORT_STATIC_ALLOCATION Is Set to 1.
 * 			The FreeRTOS Scheduler Calls This Function to Get The Task Control Block (TCB)
 * 			and Stack For The Idle Task.
 *
 * @param[out] ppxIdleTaskTCBBuffer Pointer to The TCB Buffer For The Idle Task.
 * @param[out] ppxIdleTaskStackBuffer Pointer to The Stack Buffer For The Idle Task.
 * @param[out] pulIdleTaskStackSize Pointer to a Variable Holding The Stack Size.
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);

/*!
 * @brief Hook Function to Provide Memory For The Timer Task in FreeRTOS.
 *
 * @details This Hook Function Provides The Memory Needed For The Timer Task, Which Is
 * 			Statically Allocated When configSUPPORT_STATIC_ALLOCATION Is Set To 1 And
 * 			configUSE_TIMERS Is Enabled. The FreeRTOS Scheduler Calls This Function to
 * 			Get The Task Control Block (TCB) And Stack For The Timer Task.
 *
 * @param[out] ppxTimerTaskTCBBuffer Pointer to The TCB Buffer For The Timer Task.
 * @param[out] ppxTimerTaskStackBuffer Pointer to The Stack Buffer For The Timer Task.
 * @param[out] pulTimerTaskStackSize Pointer to a Variable Holding The Stack Size.
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize);

#endif /* APP_TASKS_H_ */
