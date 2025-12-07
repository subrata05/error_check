# errcheck — Clean, Testable, Fail‑Fast Error Handling for Safety‑Critical Embedded C

**A compact, deterministic error-handling library for embedded systems — fail-fast behavior, detailed error context, rollback-safe cleanup, and built-in fault‑injection hooks for exhaustive testing.**

---

## Why errcheck?

Embedded safety-critical systems require error handling that is:

* **Deterministic** — behavior must be predictable and testable.
* **Fail‑fast** — detect and capture the first fatal fault immediately.
* **Debuggable** — capture rich context (file, line, inner driver code) for post‑mortem analysis.
* **Testable** — provide compile‑time and runtime fault injection to exercise error handling paths.
* **Rollback friendly** — support structured cleanup when partial initialization fails.

`errcheck` provides a small, auditable set of macros and helpers that let you implement the above without increasing system complexity or memory footprint unnecessarily.

---

## Key features

* **Fail‑Fast CHECK macro** — instantly capture context and return on the first failure.
* **GOTO_CHECK for rollback** — combine deterministic failure capture with structured cleanup via `goto` labels.
* **Rich failure context** — `failure_context_t` records `code`, `inner_code`, `file`, `line`, and `logged_to_nvram`.
* **Non‑volatile logging hook** — `errcheck_log_to_nvram()` stub that must be implemented to persist failure context across resets.
* **Human‑readable diagnostics** — `errcheck_print_last_error()` uses an application-provided `app_error_to_string()` to print readable error information (useful in debug builds / console outputs).
* **Compile‑time fault injection (CTI)** — use preprocessor flags to force failures for structural coverage tests.
* **Runtime fault injection (RTI)** — debugger-controlled injection via `g_inject_error_flag` when `ERRCHECK_ENABLE_RUNTIME_INJECTION` is defined.
* **Minimal public ABI** — single global context (`g_error_context`) and a few macros that are easy to reason about and audit.

---

## Repository layout (summary)

```
/src/
  errcheck.h        // Public header: macros, types, prototypes
  errcheck.c        // Global context + NVRAM logging stub
  err_log.c         // Console printing helper
/examples/
  basic_usage.c     // CHECK() simple fail-fast example
  rollback_cleanup.c// GOTO_CHECK() example with cleanup labels
  fault_injection_ci.c // Compile-time injection example
  fault_injection_rt.c // Runtime (debugger) injection example
/app/
  user_app_errors.h // Example app error enum and required externs
  app_error_strings.c // Example mapping from error code -> string

```

---

## Quickstart

1. Add `src/errcheck.h`, `src/errcheck.c`, and `src/err_log.c` to your project.
2. Provide an application-specific `user_app_errors.h` with an `app_err_t` enum and include `errcheck.h` for the base `err_t` type.
3. Implement the required mapping function in your codebase:

```c
// app_error_strings.c (application code)
#include "user_app_errors.h"

const char* app_error_to_string(err_t code) {
    switch ((app_err_t)code) {
        case APP_ERR_NONE: return "APP_ERR_NONE";
        case ERR_POWER: return "ERR_POWER";
        // ... add your application's codes
        default: return "UNKNOWN_APPLICATION_ERROR";
    }
}
```

4. Implement `errcheck_log_to_nvram()` to persist `g_error_context` to your device's non‑volatile storage (Flash/EEPROM/RAM backed by battery) to guarantee post‑mortem retrieval after resets.

5. Use `CHECK()` where a failing call should abort early and return; use `GOTO_CHECK()` where you need to unwind resources in a deterministic order.

---

## API and usage patterns

### `failure_context_t`

A global structure exposed as `g_error_context`:

```c
typedef struct {
    err_t code;
    uint32_t inner_code;
    const char *file;
    uint32_t line;
    bool logged_to_nvram;
} failure_context_t;

extern failure_context_t g_error_context;
```

Use `inner_code` for driver-specific error codes (I2C return codes, HAL errors, etc.).

### Core macros

* `CHECK(call, ERR_CODE)` — execute `call`; if it returns `0` (failure), capture context, log to NVRAM and immediately return `ERR_FAILURE`.

* `GOTO_CHECK(call, ERR_CODE, label)` — same detection but instead of returning, populate `g_error_context` and `goto label`. Use this for deterministic rollback sequences. **Note**: `errcheck_log_to_nvram()` must be called at your cleanup/exit label.

* `RETURN_ERR_AND_CONTEXT(err_flag, inner_val)` — internal helper that captures context and triggers `errcheck_log_to_nvram()` before returning.

### Fault injection

* **Compile‑time injection (CTI)**: define flags at compile time (e.g. `-DINJECT_ERR_SENSOR`) and adapt your macros to substitute failing calls (see examples/fault_injection_ci.c).

* **Runtime injection (RTI)**: compile with `-DERRCHECK_ENABLE_RUNTIME_INJECTION`. The library exposes `volatile uint8_t g_inject_error_flag`; when this is set (e.g., from the debugger), `CHECK()`/`GOTO_CHECK()` can be forced to fail so you exercise cleanup and error paths.

---

## Examples (conceptual)

### Simple fail‑fast initialization

```c
err_t device_init_simple(void) {
    CHECK(init_power(), ERR_POWER);
    CHECK(init_sensor(), ERR_SENSOR);
    CHECK(init_radio(), ERR_RADIO); // Will return ERR_FAILURE on first failed call
    return APP_ERR_NONE;
}
```

### Rollback with `GOTO_CHECK`

```c
GOTO_CHECK(power_on(), ERR_POWER, cleanup_power);
GOTO_CHECK(sensor_init(), ERR_SENSOR, cleanup_sensor);
// ... success path
final_result = APP_ERR_NONE;
goto exit;

cleanup_sensor:
    sensor_deinit();
cleanup_power:
    power_off();
exit:
    if (final_result == ERR_FAILURE) errcheck_log_to_nvram();
    return final_result;
```

This guarantees resources are released in reverse order and the failure context is logged exactly once at the unified exit point.

---

## Implementation guidance & best practices

* **Implement NVRAM logging securely**: replace the `printf` stub in `errcheck.c` with a robust write sequence that handles power‑loss (CRC, wear‑leveling, atomic write) and confirm the write before continuing. Store a versioned structure so future firmware can parse older failure records.

* **Avoid multiple log attempts**: `g_error_context.logged_to_nvram` prevents double writes during cleanup sequences — keep this semantics intact.

* **Map error codes**: keep `app_err_t` centralized and stable (do not reorder values once released) — numeric codes might be used in telemetry or ground‑station logs.

* **Keep `inner_code` meaningful**: write driver return values or hardware status registers into `inner_code` to make fault triage easier.

* **Limit debug-only code in release**: Exclude `err_log.c` and any `printf`/UART helpers from production linkers to reduce code size and avoid revealing internal state if not desired.

* **Testing**: combine CTI and RTI with unit tests and system integration tests to exercise every cleanup and fail path. Use a hardware‑in‑the‑loop (HIL) or host‑side simulator to verify NVRAM logs survive resets.

---

## Security & Safety considerations

* Persisted error contexts may reveal internal state. Treat logs as sensitive telemetry and protect access (encryption, authenticated bootloader mobile access) if needed by your product security policy.

* When writing to flash, ensure the write is atomic (double‑buffer / sequence with commit flag) to avoid corrupt or half‑written records after resets.

---

## Extending the library

* Add structured event counters or timestamps to `failure_context_t` for timing analysis.
* Integrate with your vehicle/flight telemetry to upload error records during next comms window.
* Add a compact binary on‑device format and an off‑device parser tool (Python/host) to decode logs.

---

## Tests and examples

The repository ships with several small examples demonstrating:

* Basic fail‑fast (`examples/basic_usage.c`) — shows CHECK usage and immediate return.
* Rollback cleanup (`examples/rollback_cleanup.c`) — demonstrates GOTO_CHECK and structured cleanup.
* Compile‑time fault injection (`examples/fault_injection_ci.c`) — shows how to force failures with `#define` flags.
* Runtime injection (`examples/fault_injection_rt.c`) — demonstrates debugger‑driven injection via `g_inject_error_flag`.

Run these examples on host (or in a simulated environment) by compiling with `gcc` or your cross‑toolchain and toggling the injection macros as shown in the examples.

---
