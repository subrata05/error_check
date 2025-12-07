/**
 * =============================================================================
 * errcheck.c
 * Global definitions and Non-Volatile Logging stub implementation.
 * =============================================================================
 * NOTE: The NVRAM function here is a stub. Replace the printf calls with 
 * actual hardware write access (Flash/EEPROM).
 * =============================================================================
 */

#include "errcheck.h"
#include <stdio.h> // Used only for the stub implementation

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
 * * This function MUST be implemented by the user to store the error context persistently
 * before any system reset or halt.
 */
void errcheck_log_to_nvram(void)
{
    // Prevent double-logging, especially during rollback cleanup sequences
    if (g_error_context.logged_to_nvram) {
        return;
    }
    
    // Only log if an actual failure code is present
    if (g_error_context.code == ERR_SUCCESS) {
        return;
    }

    // --- USER REPLACEMENT REQUIRED HERE (NVRAM Write) ---
    // This stub demonstrates the data being captured and written.
    
    printf("\n--- [HARDWARE STUB] NVRAM Logging Triggered! ---\n");
    printf("FAILURE LOGGED: Code=%d, Inner=0x%lX\n",
           g_error_context.code, 
           (unsigned long)g_error_context.inner_code);
    printf("Source: %s:%lu\n",
           g_error_context.file ? g_error_context.file : "N/A", 
           g_error_context.line);
    // Actual implementation would write the g_error_context struct to hardware.
    // --------------------------------------------------------------------
    
    g_error_context.logged_to_nvram = true;
}

/* * NOTE: errcheck_print_last_error() is placed in a separate file (err_log.c) 
 * for cleaner separation of logging concerns.
 */
