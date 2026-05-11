#include "bagl/obj.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bagl/mesh.h"
#include "bagl/material.h"
#include "bagl/model.h"
#include "internal/bagl_mesh.h"
#include "internal/bagl_model.h"
#include "internal/bagl_state.h"
#include "internal/utils.h"

/* Stores data for an OBJ file */
typedef struct BaglOBJ {
} BaglOBJ;

/* Stores data for an MTL file */
typedef struct BaglMTL {
} BaglMTL;

/* Callback to process a line of a file */
typedef bool (*BaglLineCallback)(BaglState* state, char* line, void* data);

/* Default delimiters used to tokenize a string */
static const char* BAGL_DEFAULT_DELIMITERS = " \r\n";

/* Tokenizes a string */
static char* baglTokenize(char* str, const char* delimiters, char** context) {
  return strtok_s(str, delimiters ? delimiters : BAGL_DEFAULT_DELIMITERS,
                  context);
}
/* Gets the next token of a tokenized string */
static char* baglGetNextToken(const char* delimiters, char** context) {
  return strtok_s(NULL, delimiters ? delimiters : BAGL_DEFAULT_DELIMITERS,
                  context);
}

/* Parses the lines of a file, invoking the given callback for each line */
static bool baglParseLines(BaglState* state,
                           const char* filename,
                           BaglLineCallback callback,
                           void* data) {
  if (!state || !filename || !callback) {
    return NULL;
  }

  /* Load file data */
  FILE* file;
  errno_t err = fopen_s(&file, filename, "rt");
  if (err) {
    baglLog(state, ERROR, "Error opening file (in baglParseLines)");
    return false;
  }

  /* Get file contents */
  char buffer[128];
  while (fgets(buffer, sizeof(buffer), file)) {
    callback(state, &buffer[0], data);
  }
  if (!feof(file)) {
    baglLog(state, ERROR, "Error reading file (in baglParseLines)");
    fclose(file);
    return false;
  }
  fclose(file);

  return true;
}

/* Processes a single line of an OBJ file */
bool baglProcessOBJLine(BaglState* state, char* line, void* data) {
  /* TEMP */
  /* Tokenize line */
  char* context;
  char* token = baglTokenize(line, NULL, &context);
  while (token) {
    /* Process tokens */
    baglLog(state, WARNING, token);
    token = baglGetNextToken(NULL, &context);
  }
  baglLog(state, ERROR, "newline");
  return true;
}

BaglMesh* baglLoadOBJ(BaglState* state, const char* filename) {
  BaglOBJ obj = {};
  /* Process all lines of the OBJ file */
  if (!baglParseLines(state, filename, baglProcessOBJLine, &obj)) {
    baglLog(state, ERROR, "Error parsing obj file (in baglLoadOBJ)");
    return NULL;
  }

  /* Turn OBJ data into a mesh */
  /* Triangulate faces */
  /* Generate vertex buffer */
  /* Generate element buffer */
  /* Create the mesh w/ vertices and elements */
  return NULL;
}

BaglMaterial* baglLoadMTL(BaglState* state, const char* filename) {
  return NULL;
}

BaglModel* baglLoadModel(BaglState* state,
                         const char* objFilename,
                         const char* mtlFilename) {
  if (!state || !objFilename || !mtlFilename) {
    return NULL;
  }

  /* Load model components */
  BaglMesh* mesh = baglLoadOBJ(state, objFilename);
  if (!mesh) {
    baglLog(state, ERROR, "Could not load mesh from obj (in baglLoadModel)");
    return NULL;
  }
  BaglMaterial* material = baglLoadMTL(state, mtlFilename);
  if (!material) {
    baglLog(state, ERROR,
            "Could not load material from mtl (in baglLoadModel)");
    baglDestroyMesh(state, &mesh);
    return NULL;
  }

  /* Create the model */
  BaglModel* model = baglCreateModel(state, mesh, material);
  if (!model) {
    baglLog(state, ERROR, "Could not create model (in baglLoadModel)");
    baglDestroyMesh(state, &mesh);
    baglDestroyMaterial(state, &material);
    return NULL;
  }
  model->ownsMaterial = true;
  model->ownsMesh = true;

  baglLog(state, INFO, "Model loaded");
  return model;
}