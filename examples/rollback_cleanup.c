/**
 * =============================================================================
 * examples/rollback_cleanup.c
 * * Demonstrates GOTO_CHECK for guaranteed resource rollback.
 * =============================================================================
 */

#include <stdio.h>
#include "../src/errcheck.h"
#include "user_app_errors.h"  

// --- Mock Drivers with Cleanup Functions ---
int power_on(void)  { printf("1. Power On: OK\n"); return 1; }
int power_off(void) { printf("Cleanup: Power Off.\n"); return 1; }

int sensor_init(void) { printf("2. Sensor Init: FAILED\n"); return 0; } // Intentional Failure
int sensor_deinit(void) { printf("Cleanup: Sensor Deinit.\n"); return 1; }

int radio_begin(void) { printf("3. Radio Begin: N/A\n"); return 1; } // Never reached
int radio_deinit(void) { printf("Cleanup: Radio Deinit.\n"); return 1; }

/**
 * @brief Initializes devices with guaranteed rollback on failure.
 */
err_t device_init_rollback(void)
{
    err_t final_result = ERR_FAILURE; // Default assumption

    printf("--- Running Rollback Init ---\n");
    
    // 1. Power On (Success)
    GOTO_CHECK(power_on(), ERR_POWER, cleanup_power); 

    // 2. Sensor Init (Failure, jumps to cleanup_sensor)
    GOTO_CHECK(sensor_init(), ERR_SENSOR, cleanup_sensor); 

    // 3. Radio Begin (Skipped)
    GOTO_CHECK(radio_begin(), ERR_RADIO, cleanup_radio); 
    
    // --- SUCCESS PATH ---
    final_result = APP_ERR_NONE;
    goto exit;

// =============================================================================
// --- CLEANUP SECTION (Guaranteed Reverse Order) ---
// Execution starts at the label corresponding to the failure point and falls through.
// =============================================================================
cleanup_radio:
    radio_deinit(); 

cleanup_sensor: // Runs because sensor_init() failed
    sensor_deinit();
    
cleanup_power: // Runs because cleanup_sensor runs
    power_off();

exit:
    // This is the single failure exit point. NVRAM logging is handled here
    if (final_result == ERR_FAILURE) {
        errcheck_log_to_nvram(); // Log the context
    }
    return final_result; 
}

int main(void)
{
    if (device_init_rollback() == ERR_FAILURE) {
        printf("\nInitialization FAILED (Rollback Verified)!\n");
        errcheck_print_last_error();
    }
    return 0;
}
