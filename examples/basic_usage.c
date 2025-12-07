/**
 * =============================================================================
 * examples/basic_usage.c
 * * Demonstrates the standard Fail-Fast flow using the CHECK macro.
 * =============================================================================
 */

#include <stdio.h>
#include "../src/errcheck.h"
#include "../app/user_app_errors.h" // Contains error definitions

// --- Mock Drivers (Return 1 for Success, 0 for Failure) ---
int init_power(void)   { printf("Power regulator: OK\n"); return 1; }
int init_sensor(void)  { printf("Sensor: OK\n"); return 1; }
int init_radio(void)   { printf("Radio: FAILED\n"); return 0; } // Intentional failure

/**
 * @brief Initializes devices using the simple CHECK macro.
 * Execution stops immediately at the first failure.
 */
err_t device_init_simple(void)
{
    printf("--- Running Simple Device Init ---\n");
    CHECK(init_power(),  ERR_POWER);    // Success
    CHECK(init_sensor(), ERR_SENSOR);   // Success
    CHECK(init_radio(),  ERR_RADIO);    // Failure -> Returns ERR_FAILURE here

    // This line is never reached on failure
    return APP_ERR_NONE; 
}

int main(void)
{
    err_t result = device_init_simple();

    if (result == ERR_FAILURE) {
        printf("\nInitialization FAILED!\n");
        // Use the print utility to display the captured context
        errcheck_print_last_error();
    } else {
        printf("\nInitialization successful!\n");
    }
    return 0;
}
