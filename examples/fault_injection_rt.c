/**
 * =============================================================================
 * examples/fault_injection_rt.c
 * * Demonstrates RUNTIME (Debugger-Controlled) Fault Injection.
 * * Compile with: -D ERRCHECK_ENABLE_RUNTIME_INJECTION
 * =============================================================================
 */

#define ERRCHECK_ENABLE_RUNTIME_INJECTION   // REQUIRED to activate injection hooks
#include <stdio.h>
#include "../src/errcheck.h"
#include "user_app_errors.h"

// Global injection trigger is externed from errcheck.c

// --- Mock Driver (would normally pass) ---
int radio_start(void)
{
    printf("Radio hardware start → normally would succeed (Result: 1)\n");
    return 1;
}

/**
 * @brief System initialization using the runtime-injectable check.
 */
err_t init_radio_rt(void)
{
    printf("--- Running RTI Test ---\n");

    // The CHECK macro is redefined to check both the call result AND the injection flag.
    CHECK(radio_start(), ERR_RADIO); // Line 1

    printf("Radio initialized successfully!\n");
    return APP_ERR_NONE;
}


int main(void)
{
    // --- Test Setup ---
    printf("Injecting error %d (ERR_RADIO) manually...\n", ERR_RADIO);
    // In a real scenario, the debugger would set this variable:
    // (gdb) set var g_inject_error_flag = 3
    g_inject_error_flag = ERR_RADIO; 
    
    err_t result = init_radio_rt();

    if (result == ERR_FAILURE) {
        printf("\nTest Result: FAILED (Triggered by Runtime Injection)!\n");
        errcheck_print_last_error();
    } else {
        printf("\nAll good — initialization passed!\n");
    }

    return 0;
}
