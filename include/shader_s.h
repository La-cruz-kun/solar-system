#pragma once
#include <stdbool.h>

typedef struct {
    const char *vertexPath;
    const char *fragmentPath;
    unsigned int ID;
} Shader;

int ShaderInit(Shader *shader);
void ShaderUse(Shader shader);
void ShaderSetBool(Shader shader, const char name[], bool value);
void ShaderSetInt(Shader shader, const char name[], int value);
void ShaderSetFloat(Shader shader, const char name[], float value);

