/**
 * @file utils.h
 * @authors quak
 * @brief Utilities for working within the bagl library.
 */

#ifndef BAGL_UTILS_H
#define BAGL_UTILS_H

/**
 * @brief Attempts to log a message.
 * @param state State to send message with.
 * @param level Message severity (INFO, STANDARD, WARNING, ERROR).
 * @param message Message to send.
 */
#define baglLog(state, level, message)           \
  do {                                           \
    if ((state)->logFn) {                        \
      (state)->logFn(BAGL_LOG_##level, message); \
    }                                            \
  } while (0)

#endif