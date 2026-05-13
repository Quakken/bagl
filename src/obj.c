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

  struct BaglOBJIndices* next;
  struct BaglOBJIndices* prev;
} BaglOBJIndices;

typedef struct BaglOBJFace {
  size_t numIndices;
  BaglOBJIndices* front;
  BaglOBJIndices* back;
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

/* Pushes a block of indices to the back of a face's indices list */
static void baglPushOBJFaceIndices(BaglState* state,
                                   BaglOBJFace* face,
                                   size_t vertex,
                                   size_t texCoord,
                                   size_t normal) {
  if (!face || !state) {
    return;
  }

  /* Allocate the indices */
  BaglOBJIndices* indices = state->reallocFn(NULL, sizeof(BaglOBJIndices));
  if (!indices) {
    baglLog(state, ERROR, "Could not allocate indices (in baglInsertIndices)");
  }
  indices->vertexIdx = vertex;
  indices->texCoordIdx = texCoord;
  indices->normalIdx = normal;

  /* List is unpopulated */
  if (!face->front) {
    face->front = indices;
    face->back = indices;
    indices->prev = indices;
    indices->next = indices;
  }
  /* Insert indices at back of list */
  else {
    indices->prev = face->back;
    indices->next = face->front;
    face->back->next = indices;
    face->front->prev = indices;
    face->back = indices;
  }
  ++face->numIndices;
}

static void baglEraseIndices(BaglState* state, BaglOBJIndices* indices) {
  if (!state || !indices) {
    return;
  }
  /* List is looped, so links can never be NULL */
  indices->prev->next = indices->next;
  indices->next->prev = indices->prev;
  state->reallocFn(indices, 0);
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

/* Destroys a face */
static void baglDestroyOBJFace(BaglState* state, BaglOBJFace* face) {
  if (!state || !face) {
    return;
  }

  /* Delete indices */
  BaglOBJIndices* elem = face->front;
  for (size_t j = 0; j < face->numIndices; ++j) {
    BaglOBJIndices* prev = elem;
    elem = elem->next;
    state->reallocFn(prev, 0);
  }

  face->front = NULL;
  face->back = NULL;
  face->numIndices = 0;
}

static void baglDestroyOBJ(BaglState* state, BaglOBJ* obj) {
  if (!obj) {
    return;
  }
  state->reallocFn(obj->vertices, 0);
  state->reallocFn(obj->texCoords, 0);
  state->reallocFn(obj->normals, 0);
  for (size_t i = 0; i < obj->numFaces; ++i) {
    baglDestroyOBJFace(state, obj->faces + i);
  }
  state->reallocFn(obj->faces, 0);
}

static bool baglProcessOBJVertex(BaglState* state,
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

static bool baglProcessOBJTexCoord(BaglState* state,
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

static bool baglProcessOBJNormal(BaglState* state,
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

static bool baglProcessOBJFace(BaglState* state,
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
    size_t indices[3] = {0};

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
          if (entry < 3) {
            indices[entry] = value - 1;
          } else {
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

    baglPushOBJFaceIndices(state, &face, indices[0], indices[1], indices[2]);
  }

  obj->faces[obj->numFaces] = face;
  ++obj->numFaces;
  return true;
}

/* Processes a single line of an OBJ file */
static bool baglProcessOBJLine(BaglState* state, char* line, void* data) {
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

  /* Unsupported features */
  else if (strcmp(token, "vp") == 0) {
    baglLog(
        state, WARNING,
        "Bagl does not support parametric vertices (in baglProcessOBJLine)");
  } else if (strcmp(token, "l") == 0) {
    baglLog(state, WARNING,
            "Bagl does not support polylines (in baglProcessOBJLine)");
  }

  return true;
}

/*
 * Triangulates a face using ear clipping. Returns whether triangulation was
 * successful.
 * The output face will store vertices in a triangle strip format.
 * NOTE: This algorithm destroys the source face!
 */
static bool baglTriangulateFace(BaglState* state,
                                BaglOBJ* obj,
                                BaglOBJFace* src,
                                BaglOBJFace* out) {
  if (!src || !out) {
    return false;
  }
  if (src->numIndices < 3) {
    baglLog(state, WARNING,
            "Face has less than 3 vertices (in baglTriangulateFace)");
    return false;
  }
  out->front = NULL;
  out->back = NULL;
  out->numIndices = 0;

  /* Iterate over all vertices until only one triangle remains */
  BaglOBJIndices* curr = src->front;
  while (src->numIndices > 3) {
    /* Get adjacent vertices */
    BaglOBJIndices* prev = curr->prev;
    BaglOBJIndices* next = curr->next;

    /* Determine if the interior angle of the triangle is convex */
    BaglOBJVertex* prevVtx = obj->vertices + prev->vertexIdx;
    BaglOBJVertex* currVtx = obj->vertices + curr->vertexIdx;
    BaglOBJVertex* nextVtx = obj->vertices + next->vertexIdx;
    float dPrevX = prevVtx->x - currVtx->x;
    float dPrevY = prevVtx->y - currVtx->y;
    float dPrevZ = prevVtx->z - currVtx->z;
    float dNextX = nextVtx->x - currVtx->x;
    float dNextY = nextVtx->y - currVtx->y;
    float dNextZ = nextVtx->x - currVtx->x;

    /* Calculate the cross product between the two directions */
    float crossX = dPrevY * dNextZ - dPrevZ * dNextY;
    float crossY = dPrevZ * dNextX - dPrevX * dNextZ;
    float crossZ = dPrevX * dNextY - dPrevY * dNextX;

    /* Compare to normal of the current vertex */
    BaglOBJNormal* normal = obj->normals + curr->normalIdx;
    float dot = crossX * normal->x + crossY * normal->y + crossZ * normal->z;
    if (dot < 0) {
      /* Push triangle to the output face */
      baglPushOBJFaceIndices(state, out, prev->vertexIdx, prev->texCoordIdx,
                             prev->normalIdx);
      baglPushOBJFaceIndices(state, out, curr->vertexIdx, curr->texCoordIdx,
                             curr->normalIdx);
      baglPushOBJFaceIndices(state, out, next->vertexIdx, next->texCoordIdx,
                             next->normalIdx);

      /* Erase triangle from source face */
      if (curr == src->front) {
        src->front = next;
      }
      if (curr == src->back) {
        src->back = prev;
      }
      baglEraseIndices(state, curr);
      --src->numIndices;
    }
    /* Advance to the next vertex */
    curr = next;
  }
  /* Insert the final triangle */
  BaglOBJIndices* begin = src->front;
  BaglOBJIndices* mid = begin->next;
  BaglOBJIndices* end = mid->next;
  baglPushOBJFaceIndices(state, out, begin->vertexIdx, begin->texCoordIdx,
                         begin->normalIdx);
  baglPushOBJFaceIndices(state, out, mid->vertexIdx, mid->texCoordIdx,
                         mid->normalIdx);
  baglPushOBJFaceIndices(state, out, end->vertexIdx, end->texCoordIdx,
                         end->normalIdx);

  /* Destroy the source face */
  baglDestroyOBJFace(state, src);

  return true;
}

/* Triangulates the faces of an object */
static size_t baglTriangulateFaces(BaglState* state, BaglOBJ* obj) {
  if (!state || !obj) {
    return 0;
  }

  BaglOBJFace* newFace =
      state->reallocFn(NULL, obj->numFaces * sizeof(BaglOBJFace));
  size_t numElements = 0;
  for (size_t i = 0; i < obj->numFaces; ++i) {
    if (!baglTriangulateFace(state, obj, obj->faces + i, newFace + i)) {
      baglLog(state, WARNING, "Triangulation failed (in baglLoadOBJ)");
    }
    numElements += newFace[i].numIndices;
  }
  state->reallocFn(obj->faces, 0);
  obj->faces = newFace;

  return numElements;
}

/* Generates vertex and element buffers from OBJ data */
static void baglGenerateOBJBuffers(BaglState* state,
                                   BaglOBJ* obj,
                                   size_t numElements,
                                   BaglVertex** vertices,
                                   uint32_t** elements) {
  if (!state || !obj || !vertices || !elements) {
    return;
  }

  *vertices = state->reallocFn(NULL, sizeof(BaglVertex) * numElements);
  *elements = state->reallocFn(NULL, sizeof(uint32_t) * numElements);

  BaglVertex* vs = *vertices;
  uint32_t* es = *elements;

  /* Iterate over each face */
  size_t currentElem = 0;
  for (size_t i = 0; i < obj->numFaces; ++i) {
    BaglOBJFace* face = obj->faces + i;
    /* Iterate over each index */
    BaglOBJIndices* indices = face->front;
    do {
      /* Add the vertex data to the vertex buffer */
      vs[currentElem].position.x = obj->vertices[indices->vertexIdx].x;
      vs[currentElem].position.y = obj->vertices[indices->vertexIdx].y;
      vs[currentElem].position.z = obj->vertices[indices->vertexIdx].z;
      vs[currentElem].normal.x = obj->normals[indices->normalIdx].x;
      vs[currentElem].normal.y = obj->normals[indices->normalIdx].y;
      vs[currentElem].normal.z = obj->normals[indices->normalIdx].z;
      vs[currentElem].texCoords.u = obj->texCoords[indices->texCoordIdx].u;
      vs[currentElem].texCoords.v = obj->texCoords[indices->texCoordIdx].v;

      /* Add element to the element buffer */
      es[currentElem] = currentElem;

      ++currentElem;
      indices = indices->next;
    } while (indices != face->back->next);
  }
}

BaglMesh* baglLoadOBJ(BaglState* state, const char* filename) {
  BaglOBJ obj = {};
  /* Process all lines of the OBJ file */
  if (!baglParseLines(state, filename, baglProcessOBJLine, &obj)) {
    baglLog(state, ERROR, "Error parsing obj file (in baglLoadOBJ)");
    baglDestroyOBJ(state, &obj);
    return NULL;
  }

  /* Triangulate faces */
  size_t numElements = baglTriangulateFaces(state, &obj);

  /* Generate buffers */
  BaglVertex* vertices;
  uint32_t* elements;
  baglGenerateOBJBuffers(state, &obj, numElements, &vertices, &elements);
  baglDestroyOBJ(state, &obj);

  /* Create the mesh w/ vertices and elements */
  BaglMeshConfig config = {
      .numVertices = numElements,
      .vertices = vertices,
      .numIndices = numElements,
      .indices = elements,
  };
  BaglMesh* mesh = baglCreateMesh(state, &config);
  state->reallocFn(vertices, 0);
  state->reallocFn(elements, 0);

  if (!mesh) {
    baglLog(state, ERROR, "Could not load mesh (in baglLoadOBJ)");
    return NULL;
  }

  return mesh;
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