/**
 * =============================================================================
 * examples/basic_usage.c
 * * Demonstrates the core Fail-Fast use case with the CHECK macro, 
 * utilizing the new Rich Error Context structure.
 * =============================================================================
 */

#include <stdio.h>      
#include "../src/errcheck.h" 

/* -------------------------------------------------------------------------
 * User-defined error type and constants
 * ------------------------------------------------------------------------- */
typedef enum {
    ERR_NONE = ERR_SUCCESS, // Success
    ERR_POWER,              // Power regulator failed
    ERR_SENSOR,             // Sensor initialization failed
    ERR_RADIO               // Radio module failed
} err_t;

// Global variables are defined in errcheck.c and externed in errcheck.h

/* -------------------------------------------------------------------------
 * Hardware initialization functions
 * ------------------------------------------------------------------------- */
int init_power(void)
{
    printf("Power regulator: OK\n");
    return 1;              // 1 = success, 0 = failure
}

int init_sensor(void)
{
    printf("Sensor: OK\n");
    return 1;
}

int init_radio(void)
{
    printf("Radio: FAILED\n");
    return 0;              // ← This one intentionally fails
}

/* -------------------------------------------------------------------------
 * Device initialization using errcheck.h
 * If any step fails → immediately return with correct error code and context
 * ------------------------------------------------------------------------- */
err_t device_init(void)
{
    /* CHECK(call, error_flag) sets g_error_context on failure and returns ERR_FAILURE */
    CHECK(init_power(),  ERR_POWER);    // Line 1: will succeed
    CHECK(init_sensor(), ERR_SENSOR);   // Line 2: will succeed
    CHECK(init_radio(),  ERR_RADIO);    // Line 3: will fail → function returns here!

    // This line is only reached if ALL checks passed
    return ERR_SUCCESS;
}

int main(void)
{
    printf("Starting device initialization...\n");

    err_t result = device_init();

    if (result == ERR_FAILURE) {
        printf("\nInitialization FAILED!\n");
        printf("→ Error code = %d (ERR_RADIO = %d)\n", g_error_context.code, ERR_RADIO);
        printf("→ Failure occurred in file: %s at line: %lu\n", 
               g_error_context.file, g_error_context.line);
        // The NVRAM logging stub was called automatically by CHECK
    } else {
        printf("\nInitialization successful!\n");
    }

    return 0;
}
