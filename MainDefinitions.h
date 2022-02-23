#pragma once
#include <vector>
#include <memory>
#define VEC_LEN 256
#define DEFAULT_MIN_START_LEVEL .45
#define DEFAULT_STEP .05
#define RT_OUTPUT_DOT_DATA

namespace reid_tree{
typedef float Parameter;
typedef std::vector<Parameter> VecParameter;
typedef std::vector<std::vector<Parameter>> VecVecParameter;
typedef unsigned long Id;
typedef float Similarity;
}