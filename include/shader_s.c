#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "shader_s.h"


int ShaderInit(Shader *shader)
{

    FILE *vShaderFile;
    FILE *fShaderFile;
    if (!(vShaderFile = fopen(shader->vertexPath, "r"))){
        fprintf(stderr, "can't open %s\n", shader->vertexPath);
        return 1;
    }
    if (!(fShaderFile = fopen(shader->fragmentPath, "r"))){
        fprintf(stderr, "can't open %s\n", shader->fragmentPath);
        return 1;
    }
    
    fseek(vShaderFile, 0, SEEK_END);
    fseek(fShaderFile, 0, SEEK_END);

    long vfileSize = ftell(vShaderFile);
    long ffileSize = ftell(fShaderFile);

    rewind(vShaderFile);
    rewind(fShaderFile);

    char *vbuffer = (char*) malloc((vfileSize + 1) * sizeof(char));
    char *fbuffer = (char*) malloc((ffileSize + 1) * sizeof(char));

    if (vbuffer == NULL){
        perror("error allocating");
        fclose(vShaderFile);
        return 1;
    }

    if (fbuffer == NULL){
        perror("error allocating");
        fclose(fShaderFile);
        return 1;
    }
    
    size_t vbytesRead = fread(vbuffer, sizeof(char), vfileSize, vShaderFile);
    size_t fbytesRead = fread(fbuffer, sizeof(char), ffileSize, fShaderFile);
    vbuffer[vbytesRead] = '\0';
    fbuffer[fbytesRead] = '\0';
    const char *vShaderCode = vbuffer;
    const char *fShaderCode = fbuffer;
    fclose(vShaderFile);
    fclose(fShaderFile);

    unsigned int vertex;
    unsigned int fragment;
    int success;
    char infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        printf("ERROR VERTEX SHADER compilation failed\n");
        printf("%s\n", infoLog);
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        printf("ERROR FRAGMENT SHADER compilation failed\n");
        printf("%s\n", infoLog);
    }
    
    shader->ID = glCreateProgram();
    glAttachShader((shader->ID), vertex);
    glAttachShader((shader->ID), fragment);
    glLinkProgram((shader->ID));


    glGetShaderiv((shader->ID), GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog((shader->ID), 512, NULL, infoLog);
        printf("ERROR SHADER compilation failed\n");
        printf("%s\n", infoLog);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    free(vbuffer);
    free(fbuffer);
    return 0;
}



void ShaderUse(Shader shader)
{
    glUseProgram((shader.ID));
}


void ShaderSetBool(Shader shader, const char name[], bool value)
{
    glUniform1i(glGetUniformLocation((shader.ID), name), (int) value);
}

void ShaderSetInt(Shader shader, const char name[], int value)
{
    glUniform1i(glGetUniformLocation((shader.ID), name), value);
}


void ShaderSetFloat(Shader shader, const char name[], float value)
{
    glUniform1f(glGetUniformLocation((shader.ID), name), value);
}
