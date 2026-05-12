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

typedef struct BaglOBJVertex {
  float x, y, z;
} BaglOBJVertex;

typedef struct BaglOBJTexCoord {
  float u, v;
} BaglOBJTexCoord;

typedef struct BaglOBJNormal {
  float x, y, z;
} BaglOBJNormal;

typedef struct BaglOBJIndices {
  size_t vertexIdx;
  size_t texCoordIdx;
  size_t normalIdx;
} BaglOBJIndices;

typedef struct BaglOBJFace {
  size_t numIndices;
  size_t capIndices;
  BaglOBJIndices* indices;
} BaglOBJFace;

/* Stores data for an OBJ file */
typedef struct BaglOBJ {
  size_t numVertices;
  size_t capVertices; /* Capacity of the vertices array */
  BaglOBJVertex* vertices;
  size_t numTexCoords;
  size_t capTexCoords; /* Capacity of the tex coords array */
  BaglOBJTexCoord* texCoords;
  size_t numNormals;
  size_t capNormals; /* Capacity of the normals array */
  BaglOBJNormal* normals;
  size_t numFaces;
  size_t capFaces; /* Capacity of the faces array */
  BaglOBJFace* faces;
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

/* Attempts to grow a dynamically-sized array */
static bool baglGrowArray(BaglState* state,
                          void** array,
                          size_t* capacity,
                          size_t elemSize) {
  if (!state || !capacity) {
    return false;
  }
  size_t numElems = (*capacity == 0) ? 1 : *capacity * 2;
  void* result = state->reallocFn(*array, numElems * elemSize);
  if (!result) {
    baglLog(state, ERROR, "Could not grow array (in baglGrowArray)");
    return false;
  }
  *capacity = numElems;
  *array = result;
  return true;
}

/* Parses the lines of a file, invoking the given callback for each line */
static bool baglParseLines(BaglState* state,
                           const char* filename,
                           BaglLineCallback callback,
                           void* data) {
  if (!state || !filename || !callback) {
    return false;
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
    if (!callback(state, &buffer[0], data)) {
      baglLog(
          state, WARNING,
          "Error occurred when processing file contents (in baglParseLines)");
      fclose(file);
      return false;
    }
  }
  if (!feof(file)) {
    baglLog(state, ERROR, "Error reading file (in baglParseLines)");
    fclose(file);
    return false;
  }
  fclose(file);

  return true;
}

static void baglDestroyOBJ(BaglState* state, BaglOBJ* obj) {
  if (!obj) {
    return;
  }
  state->reallocFn(obj->vertices, 0);
  state->reallocFn(obj->texCoords, 0);
  state->reallocFn(obj->normals, 0);
  for (size_t i = 0; i < obj->numFaces; ++i) {
    state->reallocFn(obj->faces[i].indices, 0);
  }
  state->reallocFn(obj->faces, 0);
}

bool baglProcessOBJVertex(BaglState* state,
                          char* token,
                          char* context,
                          BaglOBJ* obj) {
  if (!state || !token || !context || !obj) {
    return false;
  }

  /* Grow vertex array if necessary */
  if (obj->capVertices <= obj->numVertices &&
      !baglGrowArray(state, (void**)&obj->vertices, &obj->capVertices,
                     sizeof(BaglOBJVertex))) {
    baglLog(state, ERROR,
            "Could not grow vertex array (in baglProcessOBJVertex)");
    return false;
  }

  BaglOBJVertex vertex = {};

  /* Read each position entry */
  if (!(token = baglGetNextToken(NULL, &context))) {
    baglLog(state, ERROR,
            "No x position given to vertex (in baglProcessOBJVertex)");
    return false;
  }
  vertex.x = strtof(token, NULL);
  if (!(token = baglGetNextToken(NULL, &context))) {
    baglLog(state, ERROR,
            "No y position given to vertex (in baglProcessOBJVertex)");
    return false;
  }
  vertex.y = strtof(token, NULL);
  if (!(token = baglGetNextToken(NULL, &context))) {
    baglLog(state, ERROR,
            "No z position given to vertex (in baglProcessOBJVertex)");
    return false;
  }
  vertex.z = strtof(token, NULL);

  /* W is optional (not supported) */
  if (baglGetNextToken(NULL, &context)) {
    baglLog(state, WARNING,
            "Bagl does not support four-dimensional vertices (in "
            "baglProcessOBJVertex)");
  }

  /* Warn if there are any tokens remaining */
  if (baglGetNextToken(NULL, &context)) {
    baglLog(state, WARNING,
            "Unused tokens at end of line (in baglProcessOBJVertex)");
  }

  obj->vertices[obj->numVertices] = vertex;
  ++obj->numVertices;
  return true;
}

bool baglProcessOBJTexCoord(BaglState* state,
                            char* token,
                            char* context,
                            BaglOBJ* obj) {
  if (!state || !token || !context || !obj) {
    return false;
  }

  /* Grow tex coord array if necessary */
  if (obj->capTexCoords <= obj->numTexCoords &&
      !baglGrowArray(state, (void**)&obj->texCoords, &obj->capTexCoords,
                     sizeof(BaglOBJTexCoord))) {
    baglLog(
        state, ERROR,
        "Could not grow texture coordinate array (in baglProcessOBJTexCoord)");
    return false;
  }

  BaglOBJTexCoord texCoord = {};

  /* Read u and v components */
  if (!(token = baglGetNextToken(NULL, &context))) {
    baglLog(state, ERROR,
            "No u given for texture coordinate (in baglProcessOBJTexCoord)");
    return false;
  }
  texCoord.u = strtof(token, NULL);
  if (!(token = baglGetNextToken(NULL, &context))) {
    baglLog(state, ERROR,
            "No v given for texture coordinate (in baglProcessOBJTexCoord)");
    return false;
  }
  texCoord.v = strtof(token, NULL);

  /* W is optional (not supported) */
  if (baglGetNextToken(NULL, &context)) {
    baglLog(state, WARNING,
            "Bagl does not support three-dimensional texture coordinates (in "
            "baglProcessOBJTexCoord)");
  }

  /* Warn if there are any tokens remaining */
  if (baglGetNextToken(NULL, &context)) {
    baglLog(state, WARNING,
            "Unused tokens at end of line (in baglProcessOBJTexCoord)");
  }

  obj->texCoords[obj->numTexCoords] = texCoord;
  ++obj->numTexCoords;
  return true;
}

bool baglProcessOBJNormal(BaglState* state,
                          char* token,
                          char* context,
                          BaglOBJ* obj) {
  if (!state || !token || !context || !obj) {
    return false;
  }

  /* Grow normal array if necessary */
  if (obj->capNormals <= obj->numNormals &&
      !baglGrowArray(state, (void**)&obj->normals, &obj->capNormals,
                     sizeof(BaglOBJVertex))) {
    baglLog(state, ERROR,
            "Could not grow normal array (in baglProcessOBJNormal)");
    return false;
  }

  BaglOBJNormal normal = {};

  /* Read each entry */
  if (!(token = baglGetNextToken(NULL, &context))) {
    baglLog(state, ERROR,
            "No x coordinate given to normal (in baglProcessOBJNormal)");
    return false;
  }
  normal.x = strtof(token, NULL);
  if (!(token = baglGetNextToken(NULL, &context))) {
    baglLog(state, ERROR,
            "No y coordinate given to normal (in baglProcessOBJNormal)");
    return false;
  }
  normal.y = strtof(token, NULL);
  if (!(token = baglGetNextToken(NULL, &context))) {
    baglLog(state, ERROR,
            "No z coordinate given to normal (in baglProcessOBJNormal)");
    return false;
  }
  normal.z = strtof(token, NULL);

  /* Warn if there are any tokens remaining */
  if (baglGetNextToken(NULL, &context)) {
    baglLog(state, WARNING,
            "Unused tokens at end of line (in baglProcessOBJNormal)");
  }

  obj->normals[obj->numNormals] = normal;
  ++obj->numNormals;
  return true;
}

bool baglProcessOBJFace(BaglState* state,
                        char* token,
                        char* context,
                        BaglOBJ* obj) {
  if (!state || !token || !context || !obj) {
    return false;
  }

  /* Grow faces array if necessary */
  if (obj->capFaces <= obj->numFaces &&
      !baglGrowArray(state, (void**)&obj->faces, &obj->capFaces,
                     sizeof(BaglOBJFace))) {
    baglLog(state, ERROR, "Could not grow faces array (in baglProcessOBJFace)");
    return false;
  }

  BaglOBJFace face = {};

  /* Iterate over vertex indices */
  while ((token = baglGetNextToken(NULL, &context))) {
    /* Grow the indices array if necessary */
    if (face.capIndices <= face.numIndices &&
        !baglGrowArray(state, (void**)&face.indices, &face.capIndices,
                       sizeof(BaglOBJIndices))) {
      baglLog(state, ERROR,
              "Could not grow indices array (in baglProcessOBJFace)");
      return false;
    }

    BaglOBJIndices indices = {};

    /* Process the indices */
    size_t entry = 0;
    char* start = token;
    char* end = token;
    while (true) {
      ++end;
      if (*start == '/') {
        start = end + 1;
        ++entry;
        continue;
      }
      if (*end == '/' || *end == '\0') {
        /* Determine entry value */
        size_t value = strtoul(start, &end, 10);
        if (value < 1) {
          baglLog(state, WARNING, "Invalid face index (in baglProcessOBJFace)");
        } else {
          switch (entry) {
            case 0:
              indices.vertexIdx = value;
              break;
            case 1:
              indices.texCoordIdx = value;
              break;
            case 2:
              indices.normalIdx = value;
              break;
            default:
              baglLog(state, WARNING,
                      "Unused face entry (in baglProcessOBJFace)");
              break;
          }
        }
        start = end + 1;
        ++entry;
      }
      if (*end == '\0') {
        break;
      }
    }

    face.indices[face.numIndices] = indices;
    ++face.numIndices;
  }

  obj->faces[obj->numFaces] = face;
  ++obj->numFaces;
  return true;
}

/* Processes a single line of an OBJ file */
bool baglProcessOBJLine(BaglState* state, char* line, void* data) {
  if (!state || !line || !data) {
    return false;
  }
  BaglOBJ* obj = data;

  /* Tokenize the line */
  char* context = NULL;
  char* token = baglTokenize(line, NULL, &context);
  /* Ignore comments */
  if (token[0] == '#') {
    return true;
  }

  /* Vertex entry */
  if (strcmp(token, "v") == 0) {
    if (!baglProcessOBJVertex(state, token, context, obj)) {
      baglLog(state, ERROR, "Could not process vertex (in baglProcessOBJLine)");
      return false;
    }
  }
  /* Texture coordinates */
  else if (strcmp(token, "vt") == 0) {
    if (!baglProcessOBJTexCoord(state, token, context, obj)) {
      baglLog(state, ERROR,
              "Could not process texture coordinate (in baglProcessOBJLine)");
      return false;
    }
  }
  /* Normals */
  else if (strcmp(token, "vn") == 0) {
    if (!baglProcessOBJNormal(state, token, context, obj)) {
      baglLog(state, ERROR, "Could not process normal (in baglProcessOBJLine)");
      return false;
    }
  }
  /* Faces */
  else if (strcmp(token, "f") == 0) {
    if (!baglProcessOBJFace(state, token, context, obj)) {
      baglLog(state, ERROR, "Could not process face (in baglProcessOBJLine)");
      return false;
    }
  }

  return true;
}

BaglMesh* baglLoadOBJ(BaglState* state, const char* filename) {
  BaglOBJ obj = {};
  /* Process all lines of the OBJ file */
  if (!baglParseLines(state, filename, baglProcessOBJLine, &obj)) {
    baglLog(state, ERROR, "Error parsing obj file (in baglLoadOBJ)");
    baglDestroyOBJ(state, &obj);
    return NULL;
  }

  /* Turn OBJ data into a mesh */
  /* Triangulate faces */
  /* Generate vertex buffer */
  /* Generate element buffer */
  /* Create the mesh w/ vertices and elements */
  baglDestroyOBJ(state, &obj);
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