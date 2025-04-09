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

/* MISRA Deviation Note:
 * Rule: MISRA 2012 Rule 8.4 [Required]
 * Justification: Variable 'error' is declared in 'error.h',
 * usage here is compliant with the intended structure of the project.
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
/* MISRA Deviation Note:
 * Rule: MISRA 2012 Rule 8.4 [Required]
 * Justification: The function 'ERR_Init', usage here is compliant with the intended structure of the project.
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
