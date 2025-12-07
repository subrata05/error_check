/**
 * =============================================================================
 * err_log.c
 * Debug console output and human-readable error conversion utility.
 * =============================================================================
 * NOTE: This file handles all console output (stdio/UART). It should be 
 * excluded from the linker for production builds to minimize code size.
 * * INTERFACE REQUIREMENT: 
 * The application MUST implement the extern function:
 * extern const char* app_error_to_string(err_t code);
 * =============================================================================
 */

#include "errcheck.h"
#include <stdio.h>       
#include <inttypes.h> // Needed for PRIu32 format specifier

/**
 * @brief External function to map a numeric error code (err_t) to a human-readable string.
 * This function MUST be implemented by the application developer in their source code 
 * (e.g., app_errors.c) to define application-specific names for the numeric codes.
 * * The implementation should typically use a switch statement on the 'code' argument, 
 * casting it to the application's main error enum type for readability.
 * * @param code The numeric err_t code stored in g_error_context.
 * @return A static string literal (const char*) representing the error name.
 * * Example of required user implementation structure:
 *
 * const char* app_error_to_string(err_t code) {
 *   switch (code) { 
 *     case NONE_ERROR:             return "NONE_ERROR";
 *     case INIT_ERROR:             return "INIT_ERROR";
 *     case I2C_ERROR:              return "I2C_ERROR";
 *     // ... all other application errors
 *     default: return "UNKNOWN_ERROR";
 *   }
 * }
 **/
extern const char* app_error_to_string(err_t code);

/**
 * @brief Prints the contents of the global g_error_context in a structured, 
 * human-readable format to the console (UART/stdio).
 */
void errcheck_print_last_error(void)
{
    // Check for success (ERR_SUCCESS is defined as 0x00)
    if (g_error_context.code == ERR_SUCCESS) {
        printf("No fatal error recorded yet.\r\n");
        return;
    }

    printf("\r\n=== FATAL ERROR ===\r\n");
    
    // Line 1: Error Code and Human-Readable Name (via the user-supplied function)
    printf("Error Code   : %" PRIu32 " (0x%02X) -> %s\r\n",
           g_error_context.code,
           g_error_context.code,
           app_error_to_string(g_error_context.code));

    // Line 2: Inner Code (Hardware/Driver Specific Value)
    printf("Inner Code   : %" PRIu32 "\r\n", g_error_context.inner_code);

    // Line 3 & 4: Source File and Line Number
    printf("File         : %s\r\n", g_error_context.file ? g_error_context.file : "N/A");
    printf("Line         : %" PRIu32 "\r\n", g_error_context.line);
    
    // Line 5: NVRAM Logging Status (Compliance Check)
    printf("NVRAM Logged : %s\r\n", g_error_context.logged_to_nvram ? "YES" : "NO");
    
    printf("===================\r\n\r\n");
}
