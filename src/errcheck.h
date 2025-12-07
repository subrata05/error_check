/**
 * =============================================================================
 * errcheck.h 
 * Clean, Testable, Fail-Fast Error Handling for Safety-Critical Embedded C
 * =============================================================================
 * Features: Fail-Fast, Deterministic Fault Injection, Rich Error Context, Rollback Support.
 * * Note: Compile-Time Injection (CTI) is implemented by the user based on 
 * preprocessor macros defined outside this file (e.g., in a build script).
 * =============================================================================
 */

#ifndef ERRCHECK_H
#define ERRCHECK_H

#include <stdint.h>
#include <stdbool.h>

/* --- User-defined types and constants --- */
#ifndef ERR_T
    typedef uint8_t err_t;
    #define ERR_T
#endif

#ifndef ERR_FAILURE
    #define ERR_FAILURE ((err_t)0xFF)
#endif
#ifndef ERR_SUCCESS
    #define ERR_SUCCESS ((err_t)0x00)
#endif

/* --- Rich Error Context Structure --- */
typedef struct {
    err_t code;
    uint32_t inner_code;        // Specific hardware or driver error code (e.g., I2C bus error)
    const char *file;           // Source file (__FILE__)
    uint32_t line;              // Line number (__LINE__)
    bool logged_to_nvram;       // Flag: has this error been written to persistent storage?
} failure_context_t;

extern failure_context_t g_error_context;

/* Function prototypes */
void errcheck_log_to_nvram(void);
void errcheck_print_last_error(void); // For console debugging (implementation in err_log.c)


/* ========================================================================= */
/* Core Macros (Captures Context and Triggers Logging)                       */
/* ========================================================================= */

// Macro to set context and return ERR_FAILURE immediately (Simple Fail-Fast)
// CRITICAL: This helper ensures NVRAM logging and context capture occur on return.
#define RETURN_ERR_AND_CONTEXT(err_flag, inner_val) do {     \
    g_error_context.code = (err_flag);                       \
    g_error_context.inner_code = (inner_val);                \
    g_error_context.file = __FILE__;                         \
    g_error_context.line = __LINE__;                         \
    errcheck_log_to_nvram();                                 \
    return ERR_FAILURE;                                      \
} while (0)

// 1. STANDARD CHECK: Fail-Fast (for functions NOT needing rollback cleanup)
#define CHECK(call, err_flag) do {                           \
    int __result = (call);                                   \
    if (__result == 0) {                                     \
        RETURN_ERR_AND_CONTEXT((err_flag), 0);               \
    }                                                        \
} while (0)

// 2. GOTO CHECK: Fail-Fast with Jump (for functions REQUIRING rollback cleanup)
// On failure, sets context, DOES NOT return, and jumps to a specific cleanup label.
// NOTE: NVRAM logging must be called manually at the 'exit' or 'cleanup' label.
#define GOTO_CHECK(call, err_flag, label) do {               \
    int __result = (call);                                   \
    if (__result == 0) {                                     \
        g_error_context.code = (err_flag);                   \
        g_error_context.inner_code = 0;                      \
        g_error_context.file = __FILE__;                     \
        g_error_context.line = __LINE__;                     \
        goto label;                                          \
    }                                                        \
} while (0)


/* ========================================================================= */
/* Optional: Runtime Fault Injection (Debug builds only)                     */
/* ========================================================================= */
#ifdef ERRCHECK_ENABLE_RUNTIME_INJECTION
    extern volatile uint8_t g_inject_error_flag;
    #undef CHECK
    #undef GOTO_CHECK
    
    // Redefined CHECK for runtime injection
    #define CHECK(call, err_flag) do {                                    \
        int __result = (call);                                            \
        if (__result == 0 || g_inject_error_flag == (err_flag)) {         \
            g_inject_error_flag = 0;                                      \
            RETURN_ERR_AND_CONTEXT((err_flag), (uint32_t)__result);       \
        }                                                                 \
    } while (0)

    // Redefined GOTO_CHECK for runtime injection
    #define GOTO_CHECK(call, err_flag, label) do {                        \
        int __result = (call);                                            \
        if (__result == 0 || g_inject_error_flag == (err_flag)) {         \
            g_error_context.code = (err_flag);                            \
            g_error_context.inner_code = (uint32_t)__result;              \
            g_error_context.file = __FILE__;                              \
            g_error_context.line = __LINE__;                              \
            g_inject_error_flag = 0;                                      \
            goto label;                                                   \
        }                                                                 \
    } while (0)
#endif

#endif /* ERRCHECK_H */
