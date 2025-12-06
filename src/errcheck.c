/**
 * =============================================================================
 * errcheck.c
 * =============================================================================
 */

#include "errcheck.h"
#include <stdio.h> // For logging in the stub

// Initialize the global context structure
failure_context_t g_error_context = {
    .code = ERR_SUCCESS,
    .inner_code = 0,
    .file = NULL,
    .line = 0,
    .logged_to_nvram = false
};

#ifdef ERRCHECK_ENABLE_RUNTIME_INJECTION
// Global variable for debugger-controlled fault injection
volatile uint8_t g_inject_error_flag = 0;
#endif


/**
 * @brief CRITICAL: Logs the current g_error_context to Non-Volatile Memory (NVRAM).
 * * This function MUST be implemented to store the error context persistently
 * before any system reset or halt.
 */
void errcheck_log_to_nvram(void)
{
    if (g_error_context.logged_to_nvram) {
        // Prevent double-logging during cascading cleanup/resets
        return;
    }
    
    // --- STUB IMPLEMENTATION (Replace with actual Flash/EEPROM write) ---
    printf("--- NVRAM Logging Triggered! ---\n");
    printf("FAILURE LOGGED: Code=%d, Inner=0x%lX, File=%s:%lu\n",
           g_error_context.code, 
           (unsigned long)g_error_context.inner_code, 
           g_error_context.file ? g_error_context.file : "N/A", 
           g_error_context.line);
    // --------------------------------------------------------------------
    
    g_error_context.logged_to_nvram = true;
}

// Optionally define functions to read the last logged error from NVRAM upon boot.
