/**
 * =============================================================================
 * app_error_strings.c
 * Implementation of the required interface for human-readable error logging.
 * =============================================================================
 */

#include "user_app_errors.h"
#include <inttypes.h>

/**
 * @brief Maps a numeric error code (err_t) to a human-readable string.
 * This function is required by the errcheck library for diagnostics.
 */
const char* app_error_to_string(err_t code)
{
    // Casting the generic err_t to the application's specific enum for clarity.
    switch ((app_err_t)code) { 
        case APP_ERR_NONE:          return "APP_ERR_NONE (Success)";
        case ERR_POWER:             return "ERR_POWER (Regulator init failed)";
        case ERR_SENSOR:            return "ERR_SENSOR (IMU/Accel failure)";
        case ERR_RADIO:             return "ERR_RADIO (RF chip startup failure)";
        case ERR_FLASH:             return "ERR_FLASH (Verification failure)";
        case ERR_TIMEOUT:           return "ERR_TIMEOUT (Bus operation timed out)";
        case ERR_BUS_COLLISION:     return "ERR_BUS_COLLISION (I2C/SPI bus fault)";
        case ERR_CLEANUP_FAILED:    return "ERR_CLEANUP_FAILED (Resource de-init)";
        default:                    return "UNKNOWN_APPLICATION_ERROR";
    }
}
