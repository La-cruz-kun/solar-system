#include "include/glad/glad.h"

#include <GLFW/glfw3.h>
#include <math.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/shader_s.h"
#include "include/stb_image.h"
#include "include/camera.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


typedef struct {
    float position[3];
    float normal[3];
    float texCoord[2];
} Vertex;

typedef struct {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} Material;

typedef struct {
	unsigned char diffuse_data;
	unsigned char specular_data;
} TextureData;

struct PlanetData {
    unsigned char diffuse;
    unsigned char specular;
    float shininess;
    vec3 orbit_position;
    float rotation_speed;
	float orbital_speed;
	float size;
} planets[9];


// used to handle certain states in the app for now:
//	
// bit at 0 position represents planet revolution state
// bit at 1 position represents planet rotation state
//
unsigned char state;

void framebuffer_callback(GLFWwindow *window, int width, int height);
float max(float a, float b);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
unsigned int loadTexture(char const * path);
void planets_setup();
void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLuint create_sphere_vao_ebo(float radius, int slices, int stacks, int *index_count_out);
void generate_sphere_indexed(
float radius, int slices, int stacks,
Vertex **vertices_out, int *vertex_count_out,
GLuint **indices_out, int *index_count_out);
unsigned int create_circle(float r, int s, int *index);


int WINDOWWIDTH = 1280 * 2;
int WINDOWHEIGHT = 720 * 2;

// time related variables
float deltaTime = 0.0;
float lastFrame = 0.0;
float animation_time = 0.0;
float rotation_time = 0.0;

Camera camera;
float yaw = -117.0;
float pitch = -14.0;

bool firstMouse = true;
float lastX = 640;
float lastY = 360;

vec3 result;
vec3 temp;

int main() {
    Camera_init(&camera, (vec3) {12.146158, 7.960372, 28.563208}, (vec3) {0, 1, 0}, yaw, pitch);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow *window = glfwCreateWindow(WINDOWWIDTH, WINDOWHEIGHT, "Test Engine", NULL, NULL);
    if (window == NULL) {
        printf("unable to create window\n");
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("unable to initialize glad\n");
        glfwTerminate();
    }

    glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    vec3 lightPosition = {0.f, 0.0f, 0.0f};

    Shader PlanetShader = {"shaders/planet_vertex.glsl", "shaders/planet_fragment.glsl", 0};
    Shader SunShader = {"shaders/sun_vertex.glsl", "shaders/sun_fragment.glsl", 0};
    Shader OrbitShader = {"shaders/line_vert.glsl", "shaders/line_frag.glsl", 0};
    Shader BackgroundShader = {"shaders/background_vert.glsl", "shaders/background_frag.glsl", 0};
    ShaderInit(&PlanetShader);
    ShaderInit(&SunShader);
    ShaderInit(&OrbitShader);
    ShaderInit(&BackgroundShader);

    TextureData SunData = {loadTexture("resources/2k_sun.jpg"), 0};
    unsigned int BackgroundTexture = loadTexture("resources/8k_stars_milky_way.jpg");

    mat4 model = GLM_MAT4_IDENTITY_INIT;
    mat4 view;
    mat4 projection = GLM_MAT4_IDENTITY_INIT;
    glm_perspective(glm_rad(45.0), (float)WINDOWWIDTH / (float)WINDOWHEIGHT, 0.1f,
                  1000.0f, projection);


    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, keyboard_callback);


    vec3 lightColor = {1, 1, 1};


    int segment = 32;
    int sun_index_count = 0;
    unsigned int SunVAO = create_sphere_vao_ebo(1, segment, segment, &sun_index_count);
    int circle_index = 0;
    int sphere_index_count = 0;
    unsigned int SphereVAO = create_sphere_vao_ebo(1, segment, segment, &sphere_index_count);
    int background_index_count = 0;
    unsigned int BackgroundVAO = create_sphere_vao_ebo(1, 8, 8, &background_index_count);
    planets_setup();
    state = 1;
    float previous_orbital_position[9][3];
		
		
    ShaderUse(PlanetShader);
    glUniform1i(glGetUniformLocation(PlanetShader.ID, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(PlanetShader.ID, "material.specular"), 1);
    ShaderUse(SunShader);
    glUniform1i(glGetUniformLocation(SunShader.ID, "diffuse"), 0);
    ShaderUse(BackgroundShader);
    glUniform1i(glGetUniformLocation(BackgroundShader.ID, "equirectangularMap"), 0);


    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        if (state & (1 << 0)) {
            animation_time += deltaTime;
        }
        if (state & (1 << 1)) {
            rotation_time += deltaTime;
        }
        lastFrame = currentFrame;
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1, 0,0,1);

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        ShaderUse(BackgroundShader);
        GetViewMatrix(&camera, view);
        view[3][0] = view[3][1] = view[3][2] = 0.0f;
        glUniformMatrix4fv(glGetUniformLocation(BackgroundShader.ID, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(BackgroundShader.ID, "projection"), 1, GL_FALSE, &projection[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, BackgroundTexture);
        glBindVertexArray(BackgroundVAO);
        glDrawElements(GL_TRIANGLES, background_index_count, GL_UNSIGNED_INT, (void *)0);

        glDepthMask(GL_TRUE); // Re-enable depth writing
        glEnable(GL_DEPTH_TEST);
                                                    
        ShaderUse(PlanetShader);
        GetViewMatrix(&camera, view);
        glUniform3f(glGetUniformLocation(PlanetShader.ID, "viewPos"), camera.Position[0], camera.Position[1], camera.Position[2]);
        glUniform3f(glGetUniformLocation(PlanetShader.ID, "light.position"), lightPosition[0], lightPosition[1], lightPosition[2]);

        glUniform3f(glGetUniformLocation(PlanetShader.ID, "light.ambient"), 0.2, 0.2, 0.2);
        glUniform3f(glGetUniformLocation(PlanetShader.ID, "light.diffuse"), 0.5, 0.5, 0.5);
        glUniform3f(glGetUniformLocation(PlanetShader.ID, "light.specular"), 1.0, 1.0, 1.0);
        glUniform3f(glGetUniformLocation(PlanetShader.ID, "material.specular"), 0.5, 0.5, 0.5);
        glUniform1f(glGetUniformLocation(PlanetShader.ID, "material.shininess"), 64);
        glUniformMatrix4fv(glGetUniformLocation(PlanetShader.ID, "view"), 1, GL_FALSE,
                                             &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(PlanetShader.ID, "projection"), 1,
                                             GL_FALSE, &projection[0][0]);


        for (int j = 0; j < 8; j++) {	
            ShaderUse(PlanetShader);
            // Render Planets
            glm_mat4_identity(model);
            previous_orbital_position[j][0] = planets[j].orbit_position[0] * sin(animation_time * planets[j].orbital_speed);
            previous_orbital_position[j][1] = 0;
            previous_orbital_position[j][2] = planets[j].orbit_position[0] * cos(animation_time * planets[j].orbital_speed);
            glm_translate(model, (vec3) {
                    previous_orbital_position[j][0], 
                    1, 
                    previous_orbital_position[j][2]
                        });
            glm_scale(model, (vec3) {planets[j].size, planets[j].size, planets[j].size} );
            glm_rotate(model, planets[j].rotation_speed * rotation_time, (vec3) {0, 1, 0}); 
            glUniformMatrix4fv(glGetUniformLocation(PlanetShader.ID, "model"), 1, GL_FALSE, &model[0][0]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, planets[j].diffuse);
            glBindVertexArray(SphereVAO);
            glDrawElements(GL_TRIANGLES, sphere_index_count, GL_UNSIGNED_INT, (void *)0);


            ShaderUse(OrbitShader);
            unsigned int CircleVAO = create_circle(max(planets[j].orbit_position[0], planets[j].orbit_position[2]), 128, &circle_index);
            glm_mat4_identity(model);
            glUniformMatrix4fv(glGetUniformLocation(SunShader.ID, "view"), 1, GL_FALSE,
                                                 &view[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(SunShader.ID, "projection"), 1,
                                                 GL_FALSE, &projection[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(SunShader.ID, "model"), 1, GL_FALSE,
                             &model[0][0]);
            glBindVertexArray(CircleVAO);
            glDrawElements(GL_LINE_LOOP, circle_index, GL_UNSIGNED_INT, (void *)0);
        }

        // render Sun
        ShaderUse(SunShader);
        glm_mat4_identity(model);
        glm_translate(model, lightPosition);
        glm_scale(model, (vec3) {10, 10, 10});
        glUniformMatrix4fv(glGetUniformLocation(SunShader.ID, "view"), 1, GL_FALSE,
                           &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(SunShader.ID, "projection"), 1,
                           GL_FALSE, &projection[0][0]);
        glUniform3f(glGetUniformLocation(SunShader.ID, "Color"), lightColor[0], lightColor[1], lightColor[2]);

        glUniformMatrix4fv(glGetUniformLocation(SunShader.ID, "model"), 1, GL_FALSE,
                 &model[0][0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, SunData.diffuse_data);
        glBindVertexArray(SunVAO);
        glDrawElements(GL_TRIANGLES, sun_index_count, GL_UNSIGNED_INT, (void *)0);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
				state = state ^ (1 << 0);
				printf("%i\n", state);
	}
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
				state = state ^ (1 << 1);
				printf("%i\n", state);
	}
	if (key == GLFW_KEY_L && action == GLFW_PRESS) {
				printf("%f, %f, %f\n", camera.Position[0], camera.Position[1], camera.Position[2]);
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if ((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)) {
        ProcessKeyboard(&camera, UP, deltaTime);
    }
    if ((glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)) {
        ProcessKeyboard(&camera, DOWN, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        ProcessKeyboard(&camera, FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        ProcessKeyboard(&camera, BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        ProcessKeyboard(&camera, RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        ProcessKeyboard(&camera, LEFT, deltaTime);
    }
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    float xpos = xposIn;
    float ypos = yposIn;
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    ProcessMouseMovement(&camera, xoffset, yoffset, true);
}

void framebuffer_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = 0;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else
    {
        printf("Texture failed to load at path: %s\n", path);
        stbi_image_free(data);
    }

    return textureID;
}

// Generates unique vertices and indices for a sphere suitable for EBO rendering.
// Allocates memory for vertices_out and indices_out. Caller must free this memory.
void generate_sphere_indexed(
    float radius, int slices, int stacks,
    Vertex **vertices_out, int *vertex_count_out,
    GLuint **indices_out, int *index_count_out)
{
    // --- Input Validation ---
    if (!vertices_out || !vertex_count_out || !indices_out || !index_count_out || slices < 3 || stacks < 2 || radius <= 0.0f) {
        if (vertices_out) *vertices_out = NULL;
        if (vertex_count_out) *vertex_count_out = 0;
        if (indices_out) *indices_out = NULL;
        if (index_count_out) *index_count_out = 0;
        // Optionally print an error message here
        return;
    }

    // --- Calculate buffer sizes ---
    // Vertices: (slices + 1) vertices per stack ring * (stacks + 1) rings (incl. poles)
    int numVertices = (slices + 1) * (stacks + 1);
    // Indices: 2 triangles per quad * 3 indices per triangle = 6 indices per quad
    //          slices quads per stack * stacks stacks
    int numIndices = slices * stacks * 6;

    // --- Allocate memory ---
    Vertex *vertices = (Vertex*)malloc(numVertices * sizeof(Vertex));
    GLuint *indices = (GLuint*)malloc(numIndices * sizeof(GLuint));

    if (!vertices || !indices) {
        // Allocation failed
        free(vertices); // free(NULL) is safe
        free(indices);
        *vertices_out = NULL;
        *vertex_count_out = 0;
        *indices_out = NULL;
        *index_count_out = 0;
        return;
    }

    // --- Generate Unique Vertices ---
    int vertexIndex = 0;
    for (int i = 0; i <= stacks; ++i) { // Iterate through stacks (latitude) including poles
        float stackAngle = i * M_PI / stacks; // theta (from 0 to PI)
        float y = radius * cosf(stackAngle);
        float xyRadius = radius * sinf(stackAngle); // Radius of the stack ring in the xy-plane

        for (int j = 0; j <= slices; ++j) { // Iterate through slices (longitude) including seam vertex
            float sliceAngle = j * 2.0f * M_PI / slices; // phi (from 0 to 2*PI)

            // Vertex position
            float x = xyRadius * cosf(sliceAngle);
            float z = xyRadius * sinf(sliceAngle);
            vertices[vertexIndex].position[0] = x;
            vertices[vertexIndex].position[1] = y;
            vertices[vertexIndex].position[2] = z;

            // Normal (for a sphere centered at origin, it's the normalized position)
            float invLen = 1.0f / radius; // Assuming radius > 0
            vertices[vertexIndex].normal[0] = x * invLen;
            vertices[vertexIndex].normal[1] = y * invLen;
            vertices[vertexIndex].normal[2] = z * invLen;

            // Texture coordinates (Spherical mapping)
            vertices[vertexIndex].texCoord[0] = (float)j / slices; // U: 0 to 1
            vertices[vertexIndex].texCoord[1] = (float)i / stacks; // V: 0 to 1

            vertexIndex++;
        }
    }

    // --- Generate Indices ---
    int indexIndex = 0;
    for (int i = 0; i < stacks; ++i) { // Iterate through stack bands
        // Calculate the starting vertex index for the current and next stack rings
        GLuint k1 = i * (slices + 1); // Start index of current stack
        GLuint k2 = k1 + (slices + 1); // Start index of next stack

        for (int j = 0; j < slices; ++j, ++k1, ++k2) {
            // For each slice, form a quad using vertices from the current (k1) and next (k2) stack rings
            // Vertices forming the quad: k1, k2, k1+1, k2+1
            // Need to handle the wrap-around for the last slice? No, because we generated j <= slices vertices.
            // k1+1 will correctly point to the seam vertex when j = slices-1.

            // Triangle 1: (k1, k2, k1+1)
            indices[indexIndex++] = k1;
            indices[indexIndex++] = k2;
            indices[indexIndex++] = k1 + 1;

            // Triangle 2: (k1+1, k2, k2+1)
            indices[indexIndex++] = k1 + 1;
            indices[indexIndex++] = k2;
            indices[indexIndex++] = k2 + 1;
        }
    }

    // --- Set output parameters ---
    *vertices_out = vertices;
    *vertex_count_out = numVertices;
    *indices_out = indices;
    *index_count_out = numIndices;

    // Caller is responsible for freeing vertices and indices!
}

GLuint create_sphere_vao_ebo(float radius, int slices, int stacks, int *index_count_out)
{
    if (!index_count_out) {
        // Need a place to store the index count
        return 0;
    }
    *index_count_out = 0; // Initialize output parameter

    Vertex *vertices = NULL;
    GLuint *indices = NULL;
    int vertex_count = 0;
    int local_index_count = 0;

    // 1. Generate vertex and index data
    generate_sphere_indexed(radius, slices, stacks, &vertices, &vertex_count, &indices, &local_index_count);

    // Check if data generation was successful
    if (!vertices || !indices || vertex_count == 0 || local_index_count == 0) {
        free(vertices); // Free any potentially allocated memory
        free(indices);
        return 0; // Indicate failure
    }

    GLuint VAO, VBO, EBO; // Add EBO handle

    // 2. Generate VAO, VBO, and EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO); // Generate the Element Buffer Object

    // 3. Bind the VAO
    // All subsequent VBO, EBO, and attribute pointer settings will be associated with this VAO
    glBindVertexArray(VAO);

    // 4. Bind and buffer VBO data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    // 5. Bind and buffer EBO data
    // IMPORTANT: Bind EBO *while* the VAO is still bound.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, local_index_count * sizeof(GLuint), indices, GL_STATIC_DRAW);

    // 6. Set up vertex attribute pointers (this part is the same)
    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // Normal attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // Texture coordinate attribute (location 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    // 7. Unbind the VAO (!!! IMPORTANT !!!)
    // Unbinding the VAO prevents accidental modification.
    // It's crucial to unbind the VAO *before* unbinding the EBO if you were to unbind it explicitly,
    // because the EBO binding is stored as part of the VAO's state.
    glBindVertexArray(0);

    // Optional: Unbind VBO and EBO after unbinding VAO (good practice, though not strictly required here
    // as the VAO state captured the necessary bindings).
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Note: EBO binding is part of VAO state

    // 8. Free CPU memory (data is now on the GPU)
    free(vertices);
    free(indices);

    // 9. Return the index count and VAO handle
    *index_count_out = local_index_count;
    return VAO;
}



void planets_setup()
{
	float scale_factor = 10.0; // 1 AU = 100 units
	float planet_distances[] = {
	10 +	0.39 * scale_factor,  // Mercury
	10 +	0.72 * scale_factor,  // Venus
	10 +	1.00 * scale_factor,  // Earth
	10 +	1.52 * scale_factor,  // Mars
	10 +	5.20 * scale_factor,  // Jupiter
	10 +	9.58 * scale_factor,  // Saturn
	10 +	19.22 * scale_factor, // Uranus
	10 +	30.05 * scale_factor, // Neptune
	10 +	39.48 * scale_factor  // Pluto
	};
	float planet_size[] = {0.5, 1, 1, 0.6, 6, 5, 3, 3};
	float planet_rotation_speed[] = {
		10.5,
		6.5, 
		1674, 
		868, 
		453, 
		34800, 
		9000, 
		9700, 
		48
	};
	float planet_orbital_speed[] = {
		47.4,
		35.0,
		29.8,
		24.1,
		13.1,
		9.7,
		6.8,
		5.4,
		4.7
	};
	planets[0].diffuse = loadTexture("resources/2k_mercury.jpg"); 
	planets[1].diffuse = loadTexture("resources/2k_venus_surface.jpg"); 
	planets[2].diffuse = loadTexture("resources/2k_earth_daymap.jpg"); 
	planets[3].diffuse = loadTexture("resources/2k_mars.jpg"); 
	planets[4].diffuse = loadTexture("resources/2k_jupiter.jpg"); 
	planets[5].diffuse = loadTexture("resources/2k_saturn.jpg"); 
	planets[6].diffuse = loadTexture("resources/2k_uranus.jpg"); 
	planets[7].diffuse = loadTexture("resources/2k_neptune.jpg"); 
	for (int j = 0; j < 8; j++) {
			planets[j].orbit_position[0] = planet_distances[j];
			planets[j].orbit_position[1] = 0;
			planets[j].orbit_position[2] = planet_distances[j];
	}

	for (int i = 0; i < 8; i++) {
		planets[i].size = planet_size[i];
	}
	for (int i = 0; i < 8; i++) {
		planets[i].rotation_speed = planet_rotation_speed[i]/30;
	}
	for (int i = 0; i < 8; i++) {
		planets[i].orbital_speed = planet_orbital_speed[i]/30;
	}
}

unsigned int create_circle(float r, int s, int *index)
{
		// Define circle parameters
		float radius = r;
		int segments = s; // More segments = smoother circle
		float angle_step = 2 * M_PI / segments;

    // Indices: 2 triangles per quad * 3 indices per triangle = 6 indices per quad
    //          slices quads per stack * stacks stacks
    int numIndices = 2 * segments;
		// Allocate memory for vertices (2D positions)
		float *circleVertices = malloc(2 * (segments + 1) * sizeof(float));
    GLuint *indices = (GLuint*)malloc(numIndices * sizeof(GLuint));

		for (int i = 0; i <= segments; i++) {
				float angle = i * angle_step;
				circleVertices[2*i] = radius * cos(angle); // X
				circleVertices[2*i + 1] = radius * sin(angle); // Y
		}


		int indexIndex = 0;
		for (int i = 0; i < segments; i++) {
			unsigned int k1 = i;
			unsigned int k2 = i + 1;

			indices[indexIndex++] = k1;
			indices[indexIndex++] = k2;
		}
		*index = indexIndex;

		GLuint VAO, VBO, EBO;

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO); // Generate the Element Buffer Object

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, 2 * (segments + 1) * sizeof(float), circleVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexIndex * sizeof(GLuint), indices, GL_STATIC_DRAW);
		// Position attribute (location = 0)
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		free(circleVertices); // Free CPU data after uploading to GPU
		free(indices);
		return VAO;
}

float max(float a, float b)
{
	return a > b ? a : b;
}
