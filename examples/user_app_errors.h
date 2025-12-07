#ifndef USER_APP_ERRORS_H
#define USER_APP_ERRORS_H

#include "../src/errcheck.h" // Includes err_t definition

// --- 1. User-Defined Error Codes (Used across the entire application) ---
// Note: These must not overlap with internal library codes (0xFF, 0x00)
typedef enum {
    APP_ERR_NONE = ERR_SUCCESS, // 0x00
    
    // Subsystem Initialization Errors
    ERR_POWER = 1,              
    ERR_SENSOR = 2,
    ERR_RADIO = 3,
    ERR_FLASH = 4,

    // Specific Failure Modes
    ERR_TIMEOUT = 10,
    ERR_BUS_COLLISION = 11,
    ERR_CLEANUP_FAILED = 12
    
} app_err_t;

// --- 2. Required External Interface ---
// This function MUST be implemented by the user in app_error_strings.c 
// to provide human-readable names.
extern const char* app_error_to_string(err_t code);

#endif // USER_APP_ERRORS_H
