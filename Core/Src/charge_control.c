/*
 * charge_control.c
 *
 *  Created on: Jan 2025
 *      Author: tiensy
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
    // Conditions: sk60x_data.v_in >= 240 (24.0V) && sk60x_data.v_set >= 168 (16.8V)
    // Note: SK60X values are in 0.1V units (e.g., 240 = 24.0V)
    bool v_in_ok = (sk60x_data.v_in >= (uint16_t)(SK60X_INPUT_VOLTAGE_THRESHOLD * 10));
    bool v_set_ok = (sk60x_data.v_set >= (uint16_t)(SK60X_SET_VOLTAGE_THRESHOLD * 10));
    
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
    static bool previous_charge_request = false;
    
    // Only process every 100ms to avoid overloading
    if (current_time - charge_control.last_check_time < 100) {
        return charge_control.current_state;
    }
    charge_control.last_check_time = current_time;
    
    // State machine for charge control logic
    if (charge_control.charge_request) {
        // Read data from SK60X for ALL states when charge is requested
        // Only update conditions if read is successful
        if (SK60X_Read_Data()) {
            charge_control.sk60x_conditions_met = ChargeControl_CheckSK60XConditions();
        }
        // If read fails, keep previous conditions_met value

        switch (charge_control.current_state) {
            case CHARGE_STATE_IDLE:
                charge_control.current_state = CHARGE_STATE_WAITING;
                break;
                
            case CHARGE_STATE_WAITING:
                // Check if conditions are now met
                if (charge_control.sk60x_conditions_met) {
                    charge_control.current_state = CHARGE_STATE_READY;
                }
                break;
                
            case CHARGE_STATE_READY:
                // Try to turn on SK60X
                if(SK60X_Set_On_Off(SK60X_ON)) {
                    // Verify SK60X is actually ON by checking the status
                    if (sk60x_data.on_off == SK60X_ON) {
                        // SK60X confirmed ON, safe to enable charge relay
                        ChargeControl_SetChargeRelay(true);
                        charge_control.current_state = CHARGE_STATE_CHARGING;
                    }
                    // If SK60X not confirmed ON yet, stay in READY state
                }
                else {
                    // Failed to send command, go back to WAITING
                    ChargeControl_SetChargeRelay(false);
                    charge_control.current_state = CHARGE_STATE_WAITING;
                }
                break;
                
            case CHARGE_STATE_CHARGING:
                // Check if conditions are still met
                if (!charge_control.sk60x_conditions_met) {
                    charge_control.current_state = CHARGE_STATE_WAITING;
                    ChargeControl_SetChargeRelay(false);
                }
                break;
        }
        previous_charge_request = true;
    }
    else
    {
        // Only send OFF command once when transitioning from request=1 to request=0
        if (previous_charge_request) {
            if(SK60X_Set_On_Off(SK60X_OFF)){
                previous_charge_request = false;
                charge_control.sk60x_conditions_met = false;
                // Clear SK60X data only once when turning off, not every cycle
                memset(&sk60x_data, 0, sizeof(sk60x_data));
                HAL_GPIO_WritePin(LED_SK_GPIO_Port, LED_SK_Pin, GPIO_PIN_RESET);
            }
        }
        
        charge_control.current_state = CHARGE_STATE_IDLE;
        ChargeControl_SetChargeRelay(false);
    }
    return charge_control.current_state;
}