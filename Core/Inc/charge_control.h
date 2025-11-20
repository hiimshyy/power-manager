/*
 * charge_control.h
 *
 *  Created on: Jan 2025
 *      Author: STM32 Charge Control System
 */

#ifndef INC_CHARGE_CONTROL_H_
#define INC_CHARGE_CONTROL_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include "sk60x.h"
#include "main.h"

/* Defines ------------------------------------------------------------------*/
#define SK60X_INPUT_VOLTAGE_THRESHOLD    24.0f   // Threshold input voltage for sk60x_data.v_in
#define SK60X_SET_VOLTAGE_THRESHOLD    16.8f   // Threshold set voltage for sk60x_data.v_set

/* Exported types -----------------------------------------------------------*/
typedef enum {
    CHARGE_STATE_IDLE = 0,          // Not allowed to charge
    CHARGE_STATE_WAITING = 1,       // Waiting for charge conditions
    CHARGE_STATE_READY = 2,         // Charge finished
    CHARGE_STATE_CHARGING = 3       // Charging
} ChargeState_t;

typedef struct {
    bool charge_request;            // Charge request from Modbus register 0x003F
    bool charge_relay_enabled;      // Charge relay status
    ChargeState_t current_state;    // Current charge state
    bool sk60x_conditions_met;      // SK60X conditions met
    uint32_t last_check_time;       // Last check time
} ChargeControl_t;

/* Exported variables -------------------------------------------------------*/
extern ChargeControl_t charge_control;

/* Exported function prototypes ---------------------------------------------*/

/**
 * @brief Initialize charge control system
 * @retval HAL status
 */
HAL_StatusTypeDef ChargeControl_Init(void);

/**
 * @brief Process charge request from Modbus register 0x003F
 * @param request: Charge request (true/false)
 */
void ChargeControl_HandleRequest(bool request);

/**
 * @brief Process logic charge control
 * @retval Current charge state
 */
ChargeState_t ChargeControl_Process(void);

/**
 * @brief Check SK60X conditions
 * @retval true if conditions are met
 */
bool ChargeControl_CheckSK60XConditions(void);

/**
 * @brief Control charge relay
 * @param enable: true to turn on, false to turn off
 */
void ChargeControl_SetChargeRelay(bool enable);

/**
 * @brief Get current charge relay status
 * @retval true if relay is on
 */
bool ChargeControl_GetChargeRelayStatus(void);

/**
 * @brief Get current charge state for Modbus
 * @retval Charge state (0, 1, 2)
 */
uint8_t ChargeControl_GetChargeStateForModbus(void);


#endif /* INC_CHARGE_CONTROL_H_ */
