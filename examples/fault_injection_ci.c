/**
 * =============================================================================
 * examples/fault_injection_ci.c
 * * Demonstrates Compile-Time Injection (CTI) for 100% structural coverage.
 * * To run this test, the user would compile with: -D INJECT_ERR_SENSOR
 * =============================================================================
 */

#include <stdio.h>
#include "../src/errcheck.h"
#include "../app/user_app_errors.h"

// --- 1. COMPILE-TIME FAULT INJECTION CONTROL ---
// This flag is typically passed via the compiler (e.g., -D INJECT_ERR_SENSOR)
#define INJECT_ERR_SENSOR      // <-- Simulating the compiler flag for this test

/* Redefine the target macro (CHECK_SENSOR) based on the injection flag */
#ifdef INJECT_ERR_SENSOR
    // Force failure: the call is replaced with CHECK(0, ERR_SENSOR)
    #define CHECK_SENSOR(call)  CHECK(0, ERR_SENSOR)
#else
    // Normal behavior: actually execute the function
    #define CHECK_SENSOR(call)  CHECK(call, ERR_SENSOR)
#endif

// --- Mock Driver (would normally pass) ---
int calibrate_imu(void)
{
    printf("IMU calibration: would normally pass (Result: 1)\n");
    return 1;
}

/**
 * @brief Initializes IMU, using the injectable macro.
 */
err_t init_imu_safe(void)
{
    printf("--- Running CTI Test (Forcing ERR_SENSOR) ---\n");

    // This line uses the injected macro, forcing failure regardless of the driver's return value.
    CHECK_SENSOR(calibrate_imu()); // Line 1

    printf("IMU calibration successful!\n"); // Never reached in this test build
    return APP_ERR_NONE;
}

int main(void)
{
    if (init_imu_safe() == ERR_FAILURE) {
        printf("\nTest Result: FAILED (As Expected via Injection)!\n");
        errcheck_print_last_error();
    }
    return 0;
}
