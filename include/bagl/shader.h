/**
 * @file shader.h
 * @authors quak
 * @brief Information about shaders/shader stages.
 */

#ifndef BAGL_SHADER_H
#define BAGL_SHADER_H

#include <stdbool.h> /* bool */

/* Forward */

typedef struct BaglState BaglState;

/**
 * @brief Describes the type of stage stored by a BaglShaderStage.
 */
typedef enum BaglStageType {
  BAGL_STAGE_VERTEX,
  BAGL_STAGE_GEOMETRY,
  BAGL_STAGE_FRAGMENT
} BaglStageType;

/**
 * @brief Stores information about a single stage of a shader.
 */
typedef struct BaglShaderStage BaglShaderStage;

/**
 * @brief Configuration options for a BaglShader.
 */
typedef struct BaglShaderConfig {
  /*
   * NOTE: If both vertex and vertexFilename are NULL, a fullscreen vertex stage
   * with NO VERTEX INPUT will be used (standard for post-processing shaders).
   */

  /* Vertex stage to use */
  BaglShaderStage* vertex;
  /* Path to the vertex stage to load */
  const char* vertexFilename;

  /* Geometry stage to use */
  BaglShaderStage* geometry;
  /* Path to the geometry stage to load */
  const char* geometryFilename;

  /* Fragment stage to use */
  BaglShaderStage* fragment;
  /* Path to the fragment stage to load */
  const char* fragmentFilename;
} BaglShaderConfig;

typedef struct BaglShader BaglShader;

/**
 * @brief Loads a shader stage.
 * @param state State to load stage with.
 * @param type Type of shader stage to load.
 * @param filename Name of the shader stage source file.
 * @return Pointer to the loaded shader stage, or NULL if the stage could not be
 * created.
 */
BaglShaderStage* baglLoadShaderStage(BaglState* state,
                                     BaglStageType type,
                                     const char* filename);

/**
 * @brief Compiles a shader stage from source.
 * @param state State to compile stage with.
 * @param type Type of shader stage to compile.
 * @param source Source code of the shader stage to compile.
 * @return Pointer to the compiled shader stage, or NULL if the stage could not
 * be created.
 */
BaglShaderStage* baglCompileShaderStage(BaglState* state,
                                        BaglStageType type,
                                        const char* source);

/**
 * @brief Creates a shader.
 * @param state State to create shader with.
 * @param config Configuration to create shader from. Must not be NULL.
 * @return Pointer to the shader, or NULL if shader could not be created.
 */
BaglShader* baglCreateShader(BaglState* state, BaglShaderConfig* config);

/**
 * @brief Sets the value of a shader uniform.
 * @param shader Shader to set uniform for.
 * @param name Name of the uniform to set.
 * @param value Value to assign.
 */
bool baglSetUniformInt(BaglState* state,
                       BaglShader* shader,
                       const char* name,
                       int value);
/**
 * @brief Sets the value of a shader uniform.
 * @param shader Shader to set uniform for.
 * @param name Name of the uniform to set.
 * @param value Value to assign.
 */
bool baglSetUniformFloat(BaglState* state,
                         BaglShader* shader,
                         const char* name,
                         float value);

/**
 * @brief Sets the value of a shader uniform.
 * @param shader Shader to set uniform for.
 * @param name Name of the uniform to set.
 * @param v1 First value to assign.
 * @param v2 Second value to assign.
 */
bool baglSetUniformVec2(BaglState* state,
                        BaglShader* shader,
                        const char* name,
                        float v1,
                        float v2);
/**
 * @brief Sets the value of a shader uniform.
 * @param shader Shader to set uniform for.
 * @param name Name of the uniform to set.
 * @param v1 First value to assign.
 * @param v2 Second value to assign.
 * @param v3 Third value to assign.
 */
bool baglSetUniformVec3(BaglState* state,
                        BaglShader* shader,
                        const char* name,
                        float v1,
                        float v2,
                        float v3);
/**
 * @brief Sets the value of a shader uniform.
 * @param shader Shader to set uniform for.
 * @param name Name of the uniform to set.
 * @param v1 First value to assign.
 * @param v2 Second value to assign.
 * @param v3 Third value to assign.
 * @param v4 Fourth value to assign.
 */
bool baglSetUniformVec4(BaglState* state,
                        BaglShader* shader,
                        const char* name,
                        float v1,
                        float v2,
                        float v3,
                        float v4);

/**
 * @brief Destroys a shader stage.
 * @param state State to destroy stage with.
 * @param stage Stage to destroy.
 */
void baglDestroyShaderStage(BaglState* state, BaglShaderStage** stage);

/**
 * @brief Destroys a shader.
 * @param state State to destroy shader with.
 * @param shader Shader to destroy.
 */
void baglDestroyShader(BaglState* state, BaglShader** shader);

#endif