/**
 * @file window.h
 * @authors quak
 * @brief Window management utilities.
 */

#ifndef BAGL_WINDOW_H
#define BAGL_WINDOW_H

/* Forward */

typedef struct BaglState BaglState;

/**
 * @brief Describes the "mode" of a window (windowed, fullscreen, or borderless)
 */
typedef enum BaglWindowMode {
  BAGL_WINDOW_MODE_WINDOWED,
  BAGL_wINDOW_MODE_FULLSCREEN,
  BAGL_WINDOW_MODE_BORDERLESS,
} BaglWindowMode;

/**
 * @brief Configuration options for a bagl window.
 */
typedef struct BaglWindowConfig {
  /* Title of the window */
  const char* title;
  /* Window width, in pixels */
  int width;
  /* Window height, in pixels */
  int height;
  /* Mode to open window in */
  BaglWindowMode mode;
} BaglWindowConfig;

/**
 * @brief Stores all information about the application's window.
 */
typedef struct BaglWindow BaglWindow;

/**
 * @brief Returns the width of a state's window in screen coordinates.
 * @param state State of the window to test.
 * @return Width of the window's frame, in screen coordinates.
 */
int baglGetWindowWidth(const BaglState* state);
/**
 * @brief Returns the width of a state's window in screen coordinates.
 * @param state State of the window to test.
 * @return Height of the window's frame, in screen coordinates.
 */
int baglGetWindowHeight(const BaglState* state);

/**
 * @brief Returns the width of a window's frame in pixels.
 * @param state State of the window to test.
 * @return Width of the window's frame, in pixels.
 */
int baglGetFrameWidth(const BaglState* state);
/**
 * @brief Returns the width of a window's frame in pixels.
 * @param state State of the window to test.
 * @return Height of the window's frame, in pixels.
 */
int baglGetFrameHeight(const BaglState* state);

/**
 * @brief Returns whether a state's window should close.
 * @param state State of the window to test.
 * @return True if the window should be closed.
 */
int baglShouldClose(const BaglState* state);

/**
 * @brief Sets the window mode of a state's window.
 * @param state State to modify.
 * @param mode Window mode to set.
 */
void baglSetWindowMode(const BaglState* state, BaglWindowMode mode);

/**
 * @brief Returns a pointer to the underlying GLFWwindow wrapped by a BaglWindow
 * object.
 * @param state State of the window to retrieve.
 * @return Pointer to the wrapped window.
 */
void* baglGetWrappedWindow(const BaglState* state);

#endif