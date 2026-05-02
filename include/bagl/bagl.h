/**
 * @file bagl.h
 * @authors quak
 * @brief Declares all types and functions related to a bagl state.
 */

#ifndef BAGL_FOO_H
#define BAGL_FOO_H

/**
 * TYPES
 */

/**
 * @brief Describes the severity of a log message sent by bagl.
 */
typedef enum BaglLogLevel {
  BAGL_LOG_INFO,
  BAGL_LOG_STANDARD,
  BAGL_LOG_WARNING,
  BAGL_LOG_ERROR,
  BAGL_LOG_FATAL,
} BaglLogLevel;

/**
 * @brief Callback used by a bagl state to allocate, reallocate, and free
 * memory.
 * @param data Data to reallocate/free. May be NULL, in which case a new block
 * of memory should be allocated and returned.
 * @param newSize New size to grow/shrink data to. If `data` is NULL, this
 * describes how many bytes to allocate. If zero, `data` should be freed.
 * @return Pointer to the new block of data. May return NULL if memory could not
 * be allocated, or `data` was freed.
 */
typedef void* (*BaglReallocFn)(void* data, size_t newSize);
/**
 * @brief Callback used by bagl to log a message.
 * @param level Severity of the log message.
 * @param message Message that was sent.
 */
typedef void (*BaglLogFn)(BaglLogLevel level, const char* message);

/**
 * @brief Configuration options for a bagl state.
 */
typedef struct BaglConfig {
  /*
   * Function used for memory allocation/reallocation/cleanup.
   * In the default configuration, `realloc` is used for all memory management.
   */
  BaglReallocFn reallocFn;
  /*
   * Function used to log messages.
   * In the default configuration, messages are sent to stdout/stderr.
   */
  BaglLogFn logFn;
} BaglConfig;

/**
 * @brief Stores all information about a rendering state.
 */
typedef struct BaglState BaglState;

/**
 * FUNCTIONS
 */

/**
 * @brief Initializes bagl. This should be called once at the beginning of your
 * application, before using any bagl functions.
 */
void baglInit(void);

/**
 * @brief Creates a new bagl state.
 * @param config Configuration to create state with. If NULL, a default
 * configuration is used.
 * @return Pointer to the new bagl state, or NULL if a state could not be
 * created.
 */
BaglState* baglCreateState(const BaglConfig* config);

/**
 * @brief Destroys a bagl state, invalidating it and releasing all associated
 * resources.
 * @param state State to destroy.
 */
void baglDestroyState(BaglState** state);

/**
 * @brief Terminates bagl. This should be called once at the end of your
 * application, after destroying states.
 */
void baglTerminate(void);

#endif