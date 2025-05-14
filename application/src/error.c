/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      error.c
 *  Author:         Tomas Dolak
 *  Date:           22.01.2025
 *  Description:    Implements Error Handling Logic.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           ctimer.c
 *  @author         Tomas Dolak
 *  @date           22.01.2025
 *  @brief          Implements Error Handling Logic.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "error.h"
#include "stdbool.h"
/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/**
 * MISRA Deviation Note:
 * Rule: MISRA 2012 Rule 8.4 [Required]
 * Justification: Variable 'error' is Declared in 'error.h',
 * Usage Here is Compliant With The Intended Structure of The Project.
 */
/*lint -e9075 */
error_t error;
/*lint +e9075 */
/*******************************************************************************
 * Code
 ******************************************************************************/
void ERR_HandleError()
{
	while(true)
	{
		; /* Error State */
	}
}
/**
 * MISRA Deviation Note:
 * Rule: MISRA 2012 Rule 8.4 [Required]
 * Justification: The Function 'ERR_Init', Usage Here is Compliant With The Intended Structure of The Project.
 */

/*lint -e957 -e9075 */
void ERR_Init()
{
	error = ERROR_NONE;
}
/*lint +e957 +e9075 */

void ERR_SetState(error_t err)
{
	error = err;
}
