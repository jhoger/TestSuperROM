/**
 * @file platform_compat.h
 * @brief Platform Compatibility Layer - Declarations Only
 *
 * This header provides declarations for platform-independent functions.
 * Implementations are provided by platform_compat_linux or platform_compat_windows
 * libraries, linked at build time based on the target platform.
 *
 * Design Philosophy:
 * - No inline function implementations in this header
 * - All platform-specific code is in separate .c files
 * - Functions are named with platform_ prefix to avoid conflicts
 * - Platform library is selected at link time
 */

#ifndef PLATFORM_COMPAT_H
#define PLATFORM_COMPAT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ============================================================================
 * PLATFORM DETECTION
 * ============================================================================ */

#if defined(_WIN32) || defined(_WIN64)
    #undef PLATFORM_WINDOWS
    #undef PLATFORM_LINUX
    #define PLATFORM_WINDOWS 1
    #define PLATFORM_LINUX 0
#else
    #undef PLATFORM_WINDOWS
    #undef PLATFORM_LINUX
    #define PLATFORM_WINDOWS 0
    #define PLATFORM_LINUX 1
#endif

/* ============================================================================
 * SOCKET TYPES
 * ============================================================================ */

#if PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <windows.h>

    typedef SOCKET platform_socket_t;
    #define PLATFORM_INVALID_SOCKET INVALID_SOCKET
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>

    typedef int platform_socket_t;
    #define PLATFORM_INVALID_SOCKET -1
#endif

/* ============================================================================
 * STRING COMPATIBILITY
 * ============================================================================ */

/**
 * @brief Safely copy a string with truncation
 * @param dest Destination buffer
 * @param dest_size Size of destination buffer
 * @param src Source string
 * @param count Maximum number of characters to copy
 * @return 0 on success, -1 on error
 */
int platform_strncpy_s(char *dest, size_t dest_size, const char *src, size_t count);

/**
 * @brief Safely copy a string
 * @param dest Destination buffer
 * @param dest_size Size of destination buffer
 * @param src Source string
 * @return 0 on success, -1 on error
 */
int platform_strcpy_s(char *dest, size_t dest_size, const char *src);

/**
 * @brief Safely concatenate a string with truncation
 * @param dest Destination buffer
 * @param dest_size Size of destination buffer
 * @param src Source string to append
 * @param count Maximum number of characters to append
 * @return 0 on success, -1 on error
 */
int platform_strncat_s(char *dest, size_t dest_size, const char *src, size_t count);

/**
 * @brief Duplicate a string (allocates new memory)
 * @param src Source string
 * @return Newly allocated copy, or NULL on error
 */
char* platform_strdup(const char *src);

/* ============================================================================
 * FILE COMPATIBILITY
 * ============================================================================ */

/**
 * @brief Get current working directory
 * @param buf Output buffer
 * @param size Size of buffer
 * @return 0 on success, -1 on error
 */
int platform_getcwd(char *buf, size_t size);

/**
 * @brief Resolve a path to its absolute form
 * @param path Path to resolve
 * @param resolved_path Output buffer for resolved path
 * @return Resolved path, or NULL on error
 */
char* platform_realpath(const char *path, char *resolved_path);

/* ============================================================================
 * SLEEP FUNCTIONS
 * ============================================================================ */

/**
 * @brief Sleep for a specified number of milliseconds
 * @param ms Number of milliseconds to sleep
 */
void platform_sleep_ms(uint32_t ms);

/**
 * @brief Sleep for a specified number of microseconds
 * @param us Number of microseconds to sleep
 */
void platform_sleep_us(uint32_t us);

/* ============================================================================
 * THREAD SYNCHRONIZATION
 * ============================================================================ */

/**
 * @brief Mutex type for cross-platform thread synchronization
 */
typedef struct {
    #if PLATFORM_WINDOWS
        CRITICAL_SECTION cs;
    #else
        pthread_mutex_t mutex;
    #endif
} platform_mutex_t;

/**
 * @brief Initialize a mutex
 * @param m Pointer to mutex to initialize
 * @return 0 on success, -1 on error
 */
int platform_mutex_init(platform_mutex_t *m);

/**
 * @brief Lock a mutex
 * @param m Pointer to mutex to lock
 */
void platform_mutex_lock(platform_mutex_t *m);

/**
 * @brief Unlock a mutex
 * @param m Pointer to mutex to unlock
 */
void platform_mutex_unlock(platform_mutex_t *m);

/**
 * @brief Destroy a mutex
 * @param m Pointer to mutex to destroy
 */
void platform_mutex_destroy(platform_mutex_t *m);

/* ============================================================================
 * ERROR HANDLING
 * ============================================================================ */

/**
 * @brief Get the last error code
 * @return Error code (0 if no error)
 */
int platform_get_last_error(void);

/**
 * @brief Get a human-readable error message
 * @param err Error code
 * @return Error message string
 */
const char* platform_strerror(int err);

/* ============================================================================
 * STRING COMPARISON
 * ============================================================================ */

/**
 * @brief Case-insensitive string comparison
 * @param s1 First string
 * @param s2 Second string
 * @return 0 if equal, non-zero otherwise
 */
int platform_stricmp(const char *s1, const char *s2);

/* ============================================================================
 * PROCESS COMPATIBILITY
 * ============================================================================ */

/**
 * @brief Get current directory (alternative to getcwd)
 * @param buf Output buffer
 * @param size Size of buffer
 * @return 0 on success, -1 on error
 */
int platform_get_current_dir(char *buf, size_t size);

#endif /* PLATFORM_COMPAT_H */
