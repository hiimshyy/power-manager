/*
 * charge_control.c
 *
 *  Created on: Jan 2025
 *      Author: STM32 Charge Control System
 */

#include "charge_control.h"

/* Private variables --------------------------------------------------------*/
ChargeControl_t charge_control = {0};

/* External variables -------------------------------------------------------*/
extern SK60X_Data sk60x_data;

/* Private function prototypes ----------------------------------------------*/

/* Exported functions -------------------------------------------------------*/

/**
 * @brief Initialize charge control system
 * @retval HAL status
 */
HAL_StatusTypeDef ChargeControl_Init(void)
{
    // Initialize charge control structure
    charge_control.charge_request = false;
    charge_control.charge_relay_enabled = false;
    charge_control.current_state = CHARGE_STATE_IDLE;
    charge_control.sk60x_conditions_met = false;
    charge_control.last_check_time = HAL_GetTick();
    
    // Ensure charge relay is off initially
    ChargeControl_SetChargeRelay(false);
    
    return HAL_OK;
}

/**
 * @brief Process charge request from Modbus register 0x003F
 * @param request: Charge request (true/false)
 */
void ChargeControl_HandleRequest(bool request)
{
//	bool current_request = (request != 0); // Convert from uint16_t to bool
    if (request != charge_control.charge_request) {
        charge_control.charge_request = request;
        
        if (!request) {
            // If request is off, immediately return to IDLE and turn off relay
            charge_control.current_state = CHARGE_STATE_IDLE;
            ChargeControl_SetChargeRelay(false);
        }
    }
}

/**
 * @brief Check SK60X conditions
 * @retval true if conditions are met
 */
bool ChargeControl_CheckSK60XConditions(void)
{
    // Conditions: sk60x_data.v_in >= 24V && sk60x_data.v_out == sk60x_data.v_set
    bool v_in_ok = (sk60x_data.v_in >= SK60X_INPUT_VOLTAGE_THRESHOLD);
    bool v_set_ok = (sk60x_data.v_set >= SK60X_SET_VOLTAGE_THRESHOLD);
    
    bool conditions_met = v_in_ok && v_set_ok;
    
    return conditions_met;
}

/**
 * @brief Control charge relay
 * @param enable: true to turn on, false to turn off
 */
void ChargeControl_SetChargeRelay(bool enable)
{
    if (enable != charge_control.charge_relay_enabled) {
        charge_control.charge_relay_enabled = enable;
        
        // Control GPIO relay
        HAL_GPIO_WritePin(RL_CHG_GPIO_Port, RL_CHG_Pin, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}
 
/**
 * @brief Get current charge relay status
 * @retval true if relay is on
 */
bool ChargeControl_GetChargeRelayStatus(void)
{
    return charge_control.charge_relay_enabled;
}

/**
 * @brief Get current charge state for Modbus
 * @retval Charge state (0, 1, 2)
 */
uint8_t ChargeControl_GetChargeStateForModbus(void)
{
    // Return charge state based on current state
    if (charge_control.charge_request) {
        return (uint8_t)charge_control.current_state;
    } else {
        return 0; // If no request, always return 0
    }
}

/**
 * @brief Process logic charge control
 * @retval Current charge state
 */
ChargeState_t ChargeControl_Process(void)
{
    uint32_t current_time = HAL_GetTick();
    
    // Only process every 100ms to avoid overloading
    if (current_time - charge_control.last_check_time < 100) {
        return charge_control.current_state;
    }
    charge_control.last_check_time = current_time;
    
    // State machine for charge control logic
    if (charge_control.charge_request) {
        if (charge_control.current_state != CHARGE_STATE_IDLE) {
            // Read data from SK60X
            SK60X_Read_Data();
        }

        switch (charge_control.current_state) {
            case CHARGE_STATE_IDLE:
                charge_control.current_state = CHARGE_STATE_WAITING;
                break;
                
            case CHARGE_STATE_WAITING:
                // Check SK60X conditions
                charge_control.sk60x_conditions_met = ChargeControl_CheckSK60XConditions();
                
                if (charge_control.sk60x_conditions_met) {
                    charge_control.current_state = CHARGE_STATE_READY;
                }
                break;
                
            case CHARGE_STATE_READY:
                SK60X_Set_On_Off(SK60X_ON);
                ChargeControl_SetChargeRelay(true);
                charge_control.current_state = CHARGE_STATE_CHARGING;
                break;
                
            case CHARGE_STATE_CHARGING:
                if (!charge_control.sk60x_conditions_met) {
                    charge_control.current_state = CHARGE_STATE_WAITING;
                    ChargeControl_SetChargeRelay(false);
                }
                break;
        }
    }
    else
    {
        charge_control.current_state = CHARGE_STATE_IDLE;
        SK60X_Set_On_Off(SK60X_OFF);
        // Clear the struct instead of assigning integer 0
		memset(&sk60x_data, 0, sizeof(sk60x_data));
        ChargeControl_SetChargeRelay(false);
    }
    return charge_control.current_state;
}