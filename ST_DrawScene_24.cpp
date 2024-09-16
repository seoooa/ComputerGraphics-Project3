//
//  DrawScene.cpp
//
//  Written for CSE4170
//  Department of Computer Science and Engineering
//  Copyright © 2023 Sogang University. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "LoadScene.h"

// Begin of shader setup
#include "Shaders/LoadShaders.h"
#include "ShadingInfo.h"

#include <math.h>

extern SCENE scene;

// for simple shaders
GLuint h_ShaderProgram_simple; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// for PBR
GLuint h_ShaderProgram_TXPBR;
#define NUMBER_OF_LIGHT_SUPPORTED 13
GLint loc_global_ambient_color;
GLint loc_lightCount;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
GLint loc_ModelViewProjectionMatrix_TXPBR, loc_ModelViewMatrix_TXPBR, loc_ModelViewMatrixInvTrans_TXPBR;
GLint loc_cameraPos;

#define TEXTURE_INDEX_DIFFUSE	(0)
#define TEXTURE_INDEX_NORMAL	(1)
#define TEXTURE_INDEX_SPECULAR	(2)
#define TEXTURE_INDEX_EMISSIVE	(3)
#define TEXTURE_INDEX_SKYMAP	(4)

// for skybox shaders
GLuint h_ShaderProgram_skybox;
GLint loc_cubemap_skybox;
GLint loc_ModelViewProjectionMatrix_SKY;

// include glm/*.hpp only if necessary
// #include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;
// ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix
glm::mat4 ModelViewProjectionMatrix; // This one is sent to vertex shader when ready.
glm::mat4 ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2

/*********************************  START: camera *********************************/
typedef enum {
	CAMERA_1,
	CAMERA_2,
	CAMERA_3,
	CAMERA_4,
	CAMERA_5,
	CAMERA_6,
	CAMERA_A,
	CAMERA_N,
	NUM_CAMERAS
} CAMERA_INDEX;

typedef struct _Camera {
	float pos[3];
	float uaxis[3], vaxis[3], naxis[3];
	float fovy, aspect_ratio, near_c, far_c;
	int move, rotation_axis;
	
	float angle1 = 0.0f, angle2 = 0.0f;
} Camera;

Camera camera_info[NUM_CAMERAS];
Camera current_camera;

using glm::mat4;
void set_ViewMatrix_from_camera_frame(void) {
	// Do not modify this function.
	ViewMatrix = glm::mat4(current_camera.uaxis[0], current_camera.vaxis[0], current_camera.naxis[0], 0.0f,
		current_camera.uaxis[1], current_camera.vaxis[1], current_camera.naxis[1], 0.0f,
		current_camera.uaxis[2], current_camera.vaxis[2], current_camera.naxis[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::translate(ViewMatrix, glm::vec3(-current_camera.pos[0], -current_camera.pos[1], -current_camera.pos[2]));
}

void reposition_camera(int camera_num) {
	// Do not modify this function.
	Camera* pCamera = &camera_info[camera_num];

	memcpy(&current_camera, pCamera, sizeof(Camera));
	set_ViewMatrix_from_camera_frame();

	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void initialize_camera(void) {
	// Do not modify this function.

	glm::mat3 cameraRotation;
	glm::vec3 afterRotate;

	// CAMERA_1 : original view
	Camera* pCamera = &camera_info[CAMERA_1];
	for (int k = 0; k < 3; k++) {
		pCamera->pos[k] = scene.camera.e[k];
		pCamera->uaxis[k] = scene.camera.u[k];
		pCamera->vaxis[k] = scene.camera.v[k];
		pCamera->naxis[k] = scene.camera.n[k];
	}

	// CCTV position
	pCamera->pos[2] = 700.0f;
	cameraRotation = glm::mat3(glm::rotate(glm::mat4(1.0f), -30.0f * TO_RADIAN, glm::vec3(pCamera->uaxis[0], pCamera->uaxis[1], pCamera->uaxis[2])));

	afterRotate = cameraRotation * glm::vec3(pCamera->vaxis[0], pCamera->vaxis[1], pCamera->vaxis[2]);
	pCamera->vaxis[0] = afterRotate.x;
	pCamera->vaxis[1] = afterRotate.y;
	pCamera->vaxis[2] = afterRotate.z;

	afterRotate = cameraRotation * glm::vec3(pCamera->naxis[0], pCamera->naxis[1], pCamera->naxis[2]);
	pCamera->naxis[0] = afterRotate.x;
	pCamera->naxis[1] = afterRotate.y;
	pCamera->naxis[2] = afterRotate.z;

	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	// CAMERA_2 : bronze statue view
	pCamera = &camera_info[CAMERA_2];
	pCamera->pos[0] = -1.463161f; pCamera->pos[1] = 1720.545166f; pCamera->pos[2] = 1300.0f;
	pCamera->uaxis[0] = -0.999413f; pCamera->uaxis[1] = -0.032568f; pCamera->uaxis[2] = -0.010066f;
	pCamera->vaxis[0] = -0.011190f; pCamera->vaxis[1] = -0.034529f; pCamera->vaxis[2] = 0.999328f;
	pCamera->naxis[0] = -0.032200f; pCamera->naxis[1] = 0.998855f; pCamera->naxis[2] = -0.034872f;

	// CCTV position
	cameraRotation = glm::mat3(glm::rotate(glm::mat4(1.0f), -30.0f * TO_RADIAN, glm::vec3(pCamera->uaxis[0], pCamera->uaxis[1], pCamera->uaxis[2])));

	afterRotate = cameraRotation * glm::vec3(pCamera->vaxis[0], pCamera->vaxis[1], pCamera->vaxis[2]);
	pCamera->vaxis[0] = afterRotate.x;
	pCamera->vaxis[1] = afterRotate.y;
	pCamera->vaxis[2] = afterRotate.z;

	afterRotate = cameraRotation * glm::vec3(pCamera->naxis[0], pCamera->naxis[1], pCamera->naxis[2]);
	pCamera->naxis[0] = afterRotate.x;
	pCamera->naxis[1] = afterRotate.y;
	pCamera->naxis[2] = afterRotate.z;

	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	// CAMERA_3 : bronze statue view
	pCamera = &camera_info[CAMERA_3];
	pCamera->pos[0] = -593.047974f; pCamera->pos[1] = -3758.460938f; pCamera->pos[2] = 700.0f;
	pCamera->uaxis[0] = 0.864306f; pCamera->uaxis[1] = -0.502877f; pCamera->uaxis[2] = 0.009328f;
	pCamera->vaxis[0] = 0.036087f; pCamera->vaxis[1] = 0.080500f; pCamera->vaxis[2] = 0.996094f;
	pCamera->naxis[0] = -0.501662f; pCamera->naxis[1] = -0.860599f; pCamera->naxis[2] = 0.087724f;
	
	// CCTV position
	cameraRotation = glm::mat3(glm::rotate(glm::mat4(1.0f), -20.0f * TO_RADIAN, glm::vec3(pCamera->uaxis[0], pCamera->uaxis[1], pCamera->uaxis[2])));

	afterRotate = cameraRotation * glm::vec3(pCamera->vaxis[0], pCamera->vaxis[1], pCamera->vaxis[2]);
	pCamera->vaxis[0] = afterRotate.x;
	pCamera->vaxis[1] = afterRotate.y;
	pCamera->vaxis[2] = afterRotate.z;

	afterRotate = cameraRotation * glm::vec3(pCamera->naxis[0], pCamera->naxis[1], pCamera->naxis[2]);
	pCamera->naxis[0] = afterRotate.x;
	pCamera->naxis[1] = afterRotate.y;
	pCamera->naxis[2] = afterRotate.z;
	
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	// CAMERA_4 : top view
	pCamera = &camera_info[CAMERA_4];
	pCamera->pos[0] = 0.0f; pCamera->pos[1] = 0.0f; pCamera->pos[2] = 18300.0f;
	pCamera->uaxis[0] = 1.0f; pCamera->uaxis[1] = 0.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 1.0f; pCamera->vaxis[2] = 0.0f;
	pCamera->naxis[0] = 0.0f; pCamera->naxis[1] = 0.0f; pCamera->naxis[2] = 1.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	// CAMERA_5 : front view
	pCamera = &camera_info[CAMERA_5];
	pCamera->pos[0] = 0.0f; pCamera->pos[1] = 11700.0f; pCamera->pos[2] = 0.0f;
	pCamera->uaxis[0] = 1.0f; pCamera->uaxis[1] = 0.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = 0.0f; pCamera->naxis[1] = 1.0f; pCamera->naxis[2] = 0.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	// CAMERA_6 : side view
	pCamera = &camera_info[CAMERA_6];
	pCamera->pos[0] = 14600.0f; pCamera->pos[1] = 0.0f; pCamera->pos[2] = 0.0f;
	pCamera->uaxis[0] = 0.0f; pCamera->uaxis[1] = 1.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = 1.0f; pCamera->naxis[1] = 0.0f; pCamera->naxis[2] = 0.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	// CAMERA_A : moving cam
	pCamera = &camera_info[CAMERA_A];
	pCamera->pos[0] = -1.463161f; pCamera->pos[1] = 1720.545166f; pCamera->pos[2] = 683.703491f;
	pCamera->uaxis[0] = -0.999413f; pCamera->uaxis[1] = -0.032568f; pCamera->uaxis[2] = -0.010066f;
	pCamera->vaxis[0] = -0.011190f; pCamera->vaxis[1] = -0.034529f; pCamera->vaxis[2] = 0.999328f;
	pCamera->naxis[0] = -0.032200f; pCamera->naxis[1] = 0.998855f; pCamera->naxis[2] = -0.034872f;
	pCamera->move = 1;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	// CAMERA_N : ben cam
	pCamera = &camera_info[CAMERA_N];
	for (int k = 0; k < 3; k++) {
		pCamera->pos[k] = scene.camera.e[k];
		pCamera->uaxis[k] = scene.camera.u[k];
		pCamera->vaxis[k] = scene.camera.v[k];
		pCamera->naxis[k] = scene.camera.n[k];
	}
	pCamera->pos[0] = 0.0f; pCamera->pos[1] = 0.0f; pCamera->pos[2] = 300.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	reposition_camera(CAMERA_1);
}
/*********************************  END: camera *********************************/

/******************************  START: shader setup ****************************/
// Begin of Callback function definitions
void prepare_shader_program(void) {
	char string[256];

	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_simple = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram_simple);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");

	ShaderInfo shader_info_TXPBR[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Background/PBR_Tx.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Background/PBR_Tx.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_TXPBR = LoadShaders(shader_info_TXPBR);
	glUseProgram(h_ShaderProgram_TXPBR);

	loc_ModelViewProjectionMatrix_TXPBR = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPBR = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPBR = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_ModelViewMatrixInvTrans");

	//loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_global_ambient_color");

	loc_lightCount = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_light_count");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXPBR, string);
		sprintf(string, "u_light[%d].color", i);
		loc_light[i].color = glGetUniformLocation(h_ShaderProgram_TXPBR, string);
	}

	loc_cameraPos = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_camPos");

	//Textures
	loc_material.diffuseTex = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_albedoMap");
	loc_material.normalTex = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_normalMap");
	loc_material.specularTex = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_metallicRoughnessMap");
	loc_material.emissiveTex = glGetUniformLocation(h_ShaderProgram_TXPBR, "u_emissiveMap");

	ShaderInfo shader_info_skybox[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Background/skybox.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Background/skybox.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_skybox = LoadShaders(shader_info_skybox);
	loc_cubemap_skybox = glGetUniformLocation(h_ShaderProgram_skybox, "u_skymap");

	loc_ModelViewProjectionMatrix_SKY = glGetUniformLocation(h_ShaderProgram_skybox, "u_ModelViewProjectionMatrix");
}
/*******************************  END: shder setup ******************************/

/****************************  START: geometry setup ****************************/
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))
#define INDEX_VERTEX_POSITION	0
#define INDEX_NORMAL			1
#define INDEX_TEX_COORD			2

bool b_draw_grid = false;

// axes
GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

void prepare_axes(void) {
	// Initialize vertex buffer object.
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), &axes_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded axes into graphics memory.\n");
}

void draw_axes(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(8000.0f, 8000.0f, 8000.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	glBindVertexArray(axes_VAO);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

// grid
#define GRID_LENGTH			(100)
#define NUM_GRID_VETICES	((2 * GRID_LENGTH + 1) * 4)
GLuint grid_VBO, grid_VAO;
GLfloat grid_vertices[NUM_GRID_VETICES][3];
GLfloat grid_color[3] = { 0.5f, 0.5f, 0.5f };

void prepare_grid(void) {

	//set grid vertices
	int vertex_idx = 0;
	for (int x_idx = -GRID_LENGTH; x_idx <= GRID_LENGTH; x_idx++)
	{
		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = -GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	for (int y_idx = -GRID_LENGTH; y_idx <= GRID_LENGTH; y_idx++)
	{
		grid_vertices[vertex_idx][0] = -GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	// Initialize vertex buffer object.
	glGenBuffers(1, &grid_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices), &grid_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &grid_VAO);
	glBindVertexArray(grid_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VAO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	fprintf(stdout, " * Loaded grid into graphics memory.\n");
}

void draw_grid(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(1.0f);
	glBindVertexArray(grid_VAO);
	glUniform3fv(loc_primitive_color, 1, grid_color);
	glDrawArrays(GL_LINES, 0, NUM_GRID_VETICES);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

//sun_temple
GLuint* sun_temple_VBO;
GLuint* sun_temple_VAO;
int* sun_temple_n_triangles;
int* sun_temple_vertex_offset;
GLfloat** sun_temple_vertices;
GLuint* sun_temple_texture_names;

void initialize_lights(void) { // follow OpenGL conventions for initialization
	glUseProgram(h_ShaderProgram_TXPBR);

	glUniform1i(loc_lightCount, scene.n_lights);

	for (int i = 0; i < scene.n_lights; i++) {
		glUniform4f(loc_light[i].position,
			scene.light_list[i].pos[0],
			scene.light_list[i].pos[1],
			scene.light_list[i].pos[2],
			0.0f);

		glUniform3f(loc_light[i].color,
			scene.light_list[i].color[0],
			scene.light_list[i].color[1],
			scene.light_list[i].color[2]);
	}

	glUseProgram(0);
}

bool readTexImage2D_from_file(char* filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap, * tx_pixmap_32;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	if (tx_pixmap == NULL)
		return false;
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	//fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);
	GLenum format, internalFormat;
	if (tx_bits_per_pixel == 32) {
		format = GL_BGRA;
		internalFormat = GL_RGBA;
	}
	else if (tx_bits_per_pixel == 24) {
		format = GL_BGR;
		internalFormat = GL_RGB;
	}
	else {
		fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap = FreeImage_ConvertTo32Bits(tx_pixmap);
		format = GL_BGRA;
		internalFormat = GL_RGBA;
	}

	width = FreeImage_GetWidth(tx_pixmap);
	height = FreeImage_GetHeight(tx_pixmap);
	data = FreeImage_GetBits(tx_pixmap);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap);

	return true;
}

void prepare_sun_temple(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	// VBO, VAO malloc
	sun_temple_VBO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);
	sun_temple_VAO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);

	sun_temple_n_triangles = (int*)malloc(sizeof(int) * scene.n_materials);
	sun_temple_vertex_offset = (int*)malloc(sizeof(int) * scene.n_materials);

	// vertices
	sun_temple_vertices = (GLfloat**)malloc(sizeof(GLfloat*) * scene.n_materials);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		MATERIAL* pMaterial = &(scene.material_list[materialIdx]);
		GEOMETRY_TRIANGULAR_MESH* tm = &(pMaterial->geometry.tm);

		// vertex
		sun_temple_vertices[materialIdx] = (GLfloat*)malloc(sizeof(GLfloat) * 8 * tm->n_triangle * 3);

		int vertexIdx = 0;
		for (int triIdx = 0; triIdx < tm->n_triangle; triIdx++) {
			TRIANGLE tri = tm->triangle_list[triIdx];
			for (int triVertex = 0; triVertex < 3; triVertex++) {
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].x;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].y;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].z;

				sun_temple_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].x;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].y;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].z;

				sun_temple_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].u;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].v;
			}
		}

		// # of triangles
		sun_temple_n_triangles[materialIdx] = tm->n_triangle;

		if (materialIdx == 0)
			sun_temple_vertex_offset[materialIdx] = 0;
		else
			sun_temple_vertex_offset[materialIdx] = sun_temple_vertex_offset[materialIdx - 1] + 3 * sun_temple_n_triangles[materialIdx - 1];

		glGenBuffers(1, &sun_temple_VBO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, sun_temple_VBO[materialIdx]);
		glBufferData(GL_ARRAY_BUFFER, sun_temple_n_triangles[materialIdx] * 3 * n_bytes_per_vertex,
			sun_temple_vertices[materialIdx], GL_STATIC_DRAW);

		// As the geometry data exists now in graphics memory, ...
		free(sun_temple_vertices[materialIdx]);

		// Initialize vertex array object.
		glGenVertexArrays(1, &sun_temple_VAO[materialIdx]);
		glBindVertexArray(sun_temple_VAO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, sun_temple_VBO[materialIdx]);
		glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
		glEnableVertexAttribArray(INDEX_VERTEX_POSITION);
		glVertexAttribPointer(INDEX_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_NORMAL);
		glVertexAttribPointer(INDEX_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_TEX_COORD);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		if ((materialIdx > 0) && (materialIdx % 100 == 0))
			fprintf(stdout, " * Loaded %d sun temple materials into graphics memory.\n", materialIdx / 100 * 100);
	}
	fprintf(stdout, " * Loaded %d sun temple materials into graphics memory.\n", scene.n_materials);

	// textures
	sun_temple_texture_names = (GLuint*)malloc(sizeof(GLuint) * scene.n_textures);
	glGenTextures(scene.n_textures, sun_temple_texture_names);

	for (int texId = 0; texId < scene.n_textures; texId++) {
		glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[texId]);

		bool bReturn = readTexImage2D_from_file(scene.texture_file_name[texId]);

		if (bReturn) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	fprintf(stdout, " * Loaded sun temple textures into graphics memory.\n");

	free(sun_temple_vertices);
}

void bindTexture(GLint tex, int glTextureId, int texId) {
	if (INVALID_TEX_ID != texId) {
		glActiveTexture(GL_TEXTURE0 + glTextureId);
		glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[texId]);
		glUniform1i(tex, glTextureId);
	}
}

void draw_sun_temple_24(void) {
	glUseProgram(h_ShaderProgram_TXPBR);
	ModelViewMatrix = ViewMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPBR, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPBR, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPBR, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		// set material
		int diffuseTexId = scene.material_list[materialIdx].diffuseTexId;
		int normalMapTexId = scene.material_list[materialIdx].normalMapTexId;
		int specularTexId = scene.material_list[materialIdx].specularTexId;;
		int emissiveTexId = scene.material_list[materialIdx].emissiveTexId;

		bindTexture(loc_material.diffuseTex, TEXTURE_INDEX_DIFFUSE, diffuseTexId);
		bindTexture(loc_material.normalTex, TEXTURE_INDEX_NORMAL, normalMapTexId);
		bindTexture(loc_material.specularTex, TEXTURE_INDEX_SPECULAR, specularTexId);
		bindTexture(loc_material.emissiveTex, TEXTURE_INDEX_EMISSIVE, emissiveTexId);

		glEnable(GL_TEXTURE_2D);

		glBindVertexArray(sun_temple_VAO[materialIdx]);
		glDrawArrays(GL_TRIANGLES, 0, 3 * sun_temple_n_triangles[materialIdx]);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
}

// skybox
GLuint skybox_VBO, skybox_VAO;
GLuint skybox_texture_name;

GLfloat cube_vertices[72][3] = {
	// vertices enumerated clockwise
	// 6*2*3 * 2 (POS & NORM)

	// position
	-1.0f,  1.0f, -1.0f,    1.0f,  1.0f, -1.0f,    1.0f,  1.0f,  1.0f, //right
	 1.0f,  1.0f,  1.0f,   -1.0f,  1.0f,  1.0f,   -1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,   -1.0f, -1.0f,  1.0f,    1.0f, -1.0f, -1.0f, //left
	 1.0f, -1.0f, -1.0f,   -1.0f, -1.0f,  1.0f,    1.0f, -1.0f,  1.0f,

	-1.0f, -1.0f,  1.0f,   -1.0f,  1.0f,  1.0f,    1.0f,  1.0f,  1.0f, //top
	 1.0f,  1.0f,  1.0f,    1.0f, -1.0f,  1.0f,   -1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,   -1.0f, -1.0f, -1.0f,    1.0f, -1.0f, -1.0f, //bottom
	 1.0f, -1.0f, -1.0f,    1.0f,  1.0f, -1.0f,   -1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,   -1.0f, -1.0f, -1.0f,   -1.0f,  1.0f, -1.0f, //back
	-1.0f,  1.0f, -1.0f,   -1.0f,  1.0f,  1.0f,   -1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,    1.0f, -1.0f,  1.0f,    1.0f,  1.0f,  1.0f, //front
	 1.0f,  1.0f,  1.0f,    1.0f,  1.0f, -1.0f,    1.0f, -1.0f, -1.0f,

	 // normal
	 0.0f, 0.0f, -1.0f,      0.0f, 0.0f, -1.0f,     0.0f, 0.0f, -1.0f,
	 0.0f, 0.0f, -1.0f,      0.0f, 0.0f, -1.0f,     0.0f, 0.0f, -1.0f,

	-1.0f, 0.0f,  0.0f,     -1.0f, 0.0f,  0.0f,    -1.0f, 0.0f,  0.0f,
	-1.0f, 0.0f,  0.0f,     -1.0f, 0.0f,  0.0f,    -1.0f, 0.0f,  0.0f,

	 1.0f, 0.0f,  0.0f,      1.0f, 0.0f,  0.0f,     1.0f, 0.0f,  0.0f,
	 1.0f, 0.0f,  0.0f,      1.0f, 0.0f,  0.0f,     1.0f, 0.0f,  0.0f,

	 0.0f, 0.0f, 1.0f,      0.0f, 0.0f, 1.0f,     0.0f, 0.0f, 1.0f,
	 0.0f, 0.0f, 1.0f,      0.0f, 0.0f, 1.0f,     0.0f, 0.0f, 1.0f,

	 0.0f, 1.0f, 0.0f,      0.0f, 1.0f, 0.0f,     0.0f, 1.0f, 0.0f,
	 0.0f, 1.0f, 0.0f,      0.0f, 1.0f, 0.0f,     0.0f, 1.0f, 0.0f,

	 0.0f, -1.0f, 0.0f,      0.0f, -1.0f, 0.0f,     0.0f, -1.0f, 0.0f,
	 0.0f, -1.0f, 0.0f,      0.0f, -1.0f, 0.0f,     0.0f, -1.0f, 0.0f
};

void readTexImage2DForCubeMap(const char* filename, GLenum texture_target) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	//fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);

	width = FreeImage_GetWidth(tx_pixmap);
	height = FreeImage_GetHeight(tx_pixmap);
	FreeImage_FlipVertical(tx_pixmap);
	data = FreeImage_GetBits(tx_pixmap);

	glTexImage2D(texture_target, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap);
}

void prepare_skybox(void) { // Draw skybox.
	glGenVertexArrays(1, &skybox_VAO);
	glGenBuffers(1, &skybox_VBO);

	glBindVertexArray(skybox_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_VBO);
	glBufferData(GL_ARRAY_BUFFER, 36 * 3 * sizeof(GLfloat), &cube_vertices[0][0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenTextures(1, &skybox_texture_name);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture_name);

	readTexImage2DForCubeMap("Scene/Cubemap/px.png", GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	readTexImage2DForCubeMap("Scene/Cubemap/nx.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	readTexImage2DForCubeMap("Scene/Cubemap/py.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	readTexImage2DForCubeMap("Scene/Cubemap/ny.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	readTexImage2DForCubeMap("Scene/Cubemap/pz.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	readTexImage2DForCubeMap("Scene/Cubemap/nz.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
	fprintf(stdout, " * Loaded cube map textures into graphics memory.\n\n");

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void draw_skybox(void) {
	// Do not modifiy this function.
	glUseProgram(h_ShaderProgram_skybox);

	glUniform1i(loc_cubemap_skybox, TEXTURE_INDEX_SKYMAP);

	ModelViewMatrix = ViewMatrix * glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
												0.0f, 0.0f, 1.0f, 0.0f,
												0.0f, 1.0f, 0.0f, 0.0f,
												0.0f, 0.0f, 0.0f, 1.0f);
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(20000.0f, 20000.0f, 20000.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_SKY, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(skybox_VAO);
	glActiveTexture(GL_TEXTURE0 + TEXTURE_INDEX_SKYMAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture_name);

	glFrontFace(GL_CW);
	glDrawArrays(GL_TRIANGLES, 0, 6 * 2 * 3);
	glBindVertexArray(0);
	glDisable(GL_CULL_FACE);
	glUseProgram(0);
}

int read_geometry_vnt(GLfloat** object, int bytes_per_primitive, char* filename) {
	int n_triangles;
	FILE* fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}
	fread(&n_triangles, sizeof(int), 1, fp);

	*object = (float*)malloc(n_triangles * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_triangles, fp);
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

// for multiple materials
int read_geometry_vntm(GLfloat** object, int bytes_per_primitive,
	int* n_matrial_indicies, int** material_indicies,
	int* n_materials, char*** diffuse_texture_names,
	Material_Parameters** material_parameters,
	bool* bOnce,
	char* filename) {
	FILE* fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}

	int n_faces;
	fread(&n_faces, sizeof(int), 1, fp);

	*object = (float*)malloc(n_faces * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...\n", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_faces, fp);
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);

	fread(n_matrial_indicies, sizeof(int), 1, fp);

	int bytes_per_indices = sizeof(int) * 2;
	*material_indicies = (int*)malloc(bytes_per_indices * (*n_matrial_indicies)); // material id, offset
	if (*material_indicies == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...\n", filename);
		return -1;
	}

	fread(*material_indicies, bytes_per_indices, (*n_matrial_indicies), fp);

	/* texture
	if (*bOnce == false) {
		fread(n_materials, sizeof(int), 1, fp);

		*material_parameters = (Material_Parameters*)malloc(sizeof(Material_Parameters) * (*n_materials));
		*diffuse_texture_names = (char**)malloc(sizeof(char*) * (*n_materials));
		for (int i = 0; i < (*n_materials); i++) {
			fread((*material_parameters)[i].ambient_color, sizeof(float), 3, fp); //Ka
			fread((*material_parameters)[i].diffuse_color, sizeof(float), 3, fp); //Kd
			fread((*material_parameters)[i].specular_color, sizeof(float), 3, fp); //Ks
			fread(&(*material_parameters)[i].specular_exponent, sizeof(float), 1, fp); //Ns
			fread((*material_parameters)[i].emissive_color, sizeof(float), 3, fp); //Ke

			(*material_parameters)[i].ambient_color[3] = 1.0f;
			(*material_parameters)[i].diffuse_color[3] = 1.0f;
			(*material_parameters)[i].specular_color[3] = 1.0f;
			(*material_parameters)[i].emissive_color[3] = 1.0f;

			(*diffuse_texture_names)[i] = (char*)malloc(sizeof(char) * 256);
			fread((*diffuse_texture_names)[i], sizeof(char), 256, fp);
		}
		*bOnce = true;
	}
	*/

	fclose(fp);

	return n_faces;
}
/*****************************  END: geometry setup *****************************/


/*****************************  START: dynamic objects *****************************/

// for animation
unsigned int timestamp_scene = 0; // the global clock in the scene
unsigned int timestamp_tiger = 0, timestamp_ben = 0;
int flag_tiger_animation = 1, flag_ben_animation = 1;
int flag_polygon_fill;
int cur_frame_tiger = 0, cur_frame_ben = 0;
float rotation_angle_tiger = 0.0f, rotation_angle_ben = 0.0f;
int tiger_speed = 10, ben_speed = 10;

int tiger_move_start = 0, tiger_move_dir = 0;
float rotation_angle_tiger2 = 0.0f;
float after_tiger_move = 0.0f;
float tiger_x = 0.0f, tiger_y = 0.0f; 
float tiger_z = 180.0f;
int tiger_jump = 0;
float tiger_jump_v = 30.0f, tiger_jump_g = -1.0f;

float ben_x = 100.0f, ben_y = -7000.0f;
float ben_dx = 3.0f, ben_dy = 5.0f, ben_dz = 3.0f;
int ben_dir_flag = 1;
int ben_cam = 0;
float ben_walk = 0.0f, ben_walk_dw = 0.1f;

/*
	tiger
*/
#define N_TIGER_FRAMES 12
GLuint tiger_VBO, tiger_VAO;
int tiger_n_triangles[N_TIGER_FRAMES];
int tiger_vertex_offset[N_TIGER_FRAMES];
GLfloat* tiger_vertices[N_TIGER_FRAMES];

Material_Parameters material_tiger;

void prepare_tiger_20211511(void) { // vertices enumerated clockwise
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tiger_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/tiger/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry_vnt(&tiger_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		tiger_n_total_triangles += tiger_n_triangles[i];

		if (i == 0)
			tiger_vertex_offset[i] = 0;
		else
			tiger_vertex_offset[i] = tiger_vertex_offset[i - 1] + 3 * tiger_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &tiger_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glBufferData(GL_ARRAY_BUFFER, tiger_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_TIGER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, tiger_vertex_offset[i] * n_bytes_per_vertex,
			tiger_n_triangles[i] * n_bytes_per_triangle, tiger_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_TIGER_FRAMES; i++)
		free(tiger_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &tiger_VAO);
	glBindVertexArray(tiger_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/* texture
	material_tiger.ambient_color[0] = 0.24725f;
	material_tiger.ambient_color[1] = 0.1995f;
	material_tiger.ambient_color[2] = 0.0745f;
	material_tiger.ambient_color[3] = 1.0f;

	material_tiger.diffuse_color[0] = 0.75164f;
	material_tiger.diffuse_color[1] = 0.60648f;
	material_tiger.diffuse_color[2] = 0.22648f;
	material_tiger.diffuse_color[3] = 1.0f;

	material_tiger.specular_color[0] = 0.728281f;
	material_tiger.specular_color[1] = 0.655802f;
	material_tiger.specular_color[2] = 0.466065f;
	material_tiger.specular_color[3] = 1.0f;

	material_tiger.specular_exponent = 51.2f;

	material_tiger.emissive_color[0] = 0.1f;
	material_tiger.emissive_color[1] = 0.1f;
	material_tiger.emissive_color[2] = 0.0f;
	material_tiger.emissive_color[3] = 1.0f;

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_TIGER]);

	My_glTexImage2D_from_file("Data/dynamic_objects/tiger/tiger_tex.jpg");

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);
	*/
}

void draw_tiger_20211511(void) {
	/*
	set_material(&material_tiger);
	bind_texture(loc_diffuse_texture, TEXTURE_ID_DIFFUSE, texture_names[TEXTURE_TIGER]);

	glFrontFace(GL_CW);

	glBindVertexArray(tiger_VAO);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	*/

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glFrontFace(GL_CW);
	glUseProgram(h_ShaderProgram_simple);
	glUniform3f(loc_primitive_color, 255.0f, 255.0f, 0.0f);

	//jumping tiger
	if (tiger_jump && flag_tiger_animation) {
		tiger_jump_v += tiger_jump_g;
		tiger_z += tiger_jump_v;

		if (tiger_z < 180.0f) {
			tiger_z = 180.0f;
			tiger_jump_v = 0.0f;
			tiger_jump = 0;
		}
	}

	if (flag_tiger_animation) {
		switch (tiger_move_dir) {
		case 0:
			if (tiger_move_start) tiger_move_dir = 1;
			break;
		case 1:
			if ((rotation_angle_tiger + after_tiger_move) == 270 * TO_RADIAN) {
				tiger_move_dir = 2;
			}
			break;
		case 2:
			rotation_angle_tiger2 += 1 * TO_RADIAN;
			if (rotation_angle_tiger2 >= 90 * TO_RADIAN) {
				tiger_move_dir = 3;
			}
			break;
		case 3:
			tiger_y -= 5;
			if (tiger_y < -1300) {
				tiger_move_dir = 4;
			}
			break;
		case 4:
			rotation_angle_tiger2 += 1 * TO_RADIAN;
			if (rotation_angle_tiger2 >= 180 * TO_RADIAN) {
				tiger_move_dir = 5;
			}
			break;
		case 5:
			tiger_x -= 5;
			if (tiger_x < -500) {
				tiger_move_dir = 6;
			}
			break;
		case 6:
			rotation_angle_tiger2 += 2 * TO_RADIAN;
			if (rotation_angle_tiger2 >= 360 * TO_RADIAN) {
				tiger_move_dir = 7;
			}
			break;
		case 7:
			tiger_x += 5;
			if (tiger_x > 500) {
				tiger_move_dir = 8;
			}
			break;
		case 8:
			rotation_angle_tiger2 += 2 * TO_RADIAN;
			if (rotation_angle_tiger2 >= 540 * TO_RADIAN) {
				tiger_move_dir = 9;
			}
			break;
		case 9:
			tiger_x -= 5;
			if (tiger_x <= 0) {
				tiger_move_dir = 10;
			}
			break;
		case 10:
			rotation_angle_tiger2 += 2 * TO_RADIAN;
			if (rotation_angle_tiger2 >= 630 * TO_RADIAN) {
				tiger_move_dir = 11;
			}
			break;
		case 11:
			tiger_y += 5;
			if (tiger_y >= 0) {
				tiger_move_dir = 0;

				tiger_move_start = 0;
				after_tiger_move = 180.0f * TO_RADIAN;
				rotation_angle_tiger2 = 0.0f;
			}
			break;
		}
	}

	//set_material_tiger();
	//glUniform1i(loc_texture, TEXTURE_ID_TIGER);
	if (tiger_move_dir == 0 || tiger_move_dir == 1) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, 0.0f, tiger_z));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, rotation_angle_tiger + after_tiger_move, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(500.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (tiger_move_dir == 2) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, 0.0f, tiger_z));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 270 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(500.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN - rotation_angle_tiger2, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(tiger_x, tiger_y, tiger_z));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 270 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(500.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN - rotation_angle_tiger2, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	//ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	//glUniformMatrix4fv(loc_ModelViewMatrix, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	//glUniformMatrix3fv(loc_ModelViewMatrixInvTrans, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	glBindVertexArray(tiger_VAO);
	glDrawArrays(GL_LINES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUseProgram(0);
}

/*
	ben
*/
#define N_BEN_FRAMES 30
GLuint ben_VBO, ben_VAO;
int ben_n_triangles[N_BEN_FRAMES];
int ben_vertex_offset[N_BEN_FRAMES];
GLfloat* ben_vertices[N_BEN_FRAMES];

int ben_n_material_indices;
int* ben_material_indices[N_BEN_FRAMES];

int ben_n_materials;
char** ben_diffuse_texture;
GLuint* ben_texture_names;

Material_Parameters* material_ben;

void prepare_ben_20211511(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, ben_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	bool bOnce = false;
	for (i = 0; i < N_BEN_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/ben/ben_vntm_%d%d.geom", i / 10, i % 10);
		ben_n_triangles[i] = read_geometry_vntm(&ben_vertices[i], n_bytes_per_triangle,
			&ben_n_material_indices,
			&ben_material_indices[i],
			&ben_n_materials,
			&ben_diffuse_texture,
			&material_ben,
			&bOnce,
			filename);

		// assume all geometry files are effective
		ben_n_total_triangles += ben_n_triangles[i];

		if (i == 0)
			ben_vertex_offset[i] = 0;
		else
			ben_vertex_offset[i] = ben_vertex_offset[i - 1] + 3 * ben_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &ben_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ben_VBO);
	glBufferData(GL_ARRAY_BUFFER, ben_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_BEN_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, ben_vertex_offset[i] * n_bytes_per_vertex,
			ben_n_triangles[i] * n_bytes_per_triangle, ben_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_BEN_FRAMES; i++)
		free(ben_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &ben_VAO);
	glBindVertexArray(ben_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ben_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	/* texture
	ben_texture_names = (GLuint*)malloc(sizeof(GLuint) * ben_n_materials);
	glGenTextures(ben_n_materials, ben_texture_names);

	for (int i = 0; i < ben_n_materials; i++) {
		if (strcmp("", ben_diffuse_texture[i])) {
			glBindTexture(GL_TEXTURE_2D, ben_texture_names[i]);

			sprintf(filename, "Data/dynamic_objects/ben/%s", ben_diffuse_texture[i]);
			My_glTexImage2D_from_file(filename);

			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	*/
}

void draw_ben_20211511(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glFrontFace(GL_CW);
	glUseProgram(h_ShaderProgram_simple);
	glUniform3f(loc_primitive_color, 255.0f, 255.0f, 255.0f);

	if (flag_ben_animation) {
		double rotationSpeed1 = 5;
		double rotationSpeed2 = 5;

		ben_x += 2.0 * cos(rotation_angle_ben * TO_RADIAN);
		ben_y += 5.0 * sin(rotation_angle_ben * TO_RADIAN);

		if (ben_y <= -7000 && rotation_angle_ben < 360) {
			rotation_angle_ben += rotationSpeed1;
		}
		else if (ben_x >= 100 && rotation_angle_ben < 90) {
			rotation_angle_ben += rotationSpeed2;
		}
		else if (ben_y >= -5000 && rotation_angle_ben < 180) {
			rotation_angle_ben += rotationSpeed1;
		}
		else if (ben_x < -100 && rotation_angle_ben < 270) {
			rotation_angle_ben += rotationSpeed2;
		}

		if (rotation_angle_ben >= 360) {
			rotation_angle_ben -= 360;
		}
	}

	if (!ben_cam) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(ben_x, ben_y, 30.0f));
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(400.0f, 400.0f, 400.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (rotation_angle_ben + 90) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		//ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		//glUniformMatrix4fv(loc_ModelViewMatrix, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		//glUniformMatrix3fv(loc_ModelViewMatrixInvTrans, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

		glBindVertexArray(ben_VAO);

		for (int i = 0; i < ben_n_material_indices - 1; i++) {
			//glDrawArrays(GL_LINES, ben_vertex_offset[cur_frame_ben] + ben_material_indices[cur_frame_ben][2 * i + 1], 3 * ben_n_triangles[cur_frame_ben]);
			int cur_vertex_offset = ben_vertex_offset[cur_frame_ben] + ben_material_indices[cur_frame_ben][2 * i + 1];
			int n_vertices = ben_material_indices[cur_frame_ben][2 * (i + 1) + 1] - ben_material_indices[cur_frame_ben][2 * i + 1];
			glDrawArrays(GL_TRIANGLES, cur_vertex_offset, n_vertices);
		}
		glBindVertexArray(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glUseProgram(0);
	}
}

/*****************************  END: dynamic objects *****************************/


/*****************************  START: static objects *****************************/

/*
	optimus
*/
GLuint optimus_VBO, optimus_VAO;
int optimus_n_triangles;
GLfloat* optimus_vertices;

Material_Parameters material_optimus;

void prepare_optimus_20211511(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, optimus_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/optimus_vnt.geom");
	optimus_n_triangles = read_geometry_vnt(&optimus_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	optimus_n_total_triangles += optimus_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &optimus_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, optimus_VBO);
	glBufferData(GL_ARRAY_BUFFER, optimus_n_total_triangles * 3 * n_bytes_per_vertex, optimus_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(optimus_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &optimus_VAO);
	glBindVertexArray(optimus_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, optimus_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/* texture
	material_optimus.ambient_color[0] = 0.5f;
	material_optimus.ambient_color[1] = 0.5f;
	material_optimus.ambient_color[2] = 0.5f;
	material_optimus.ambient_color[3] = 1.0f;

	material_optimus.diffuse_color[0] = 0.2f;
	material_optimus.diffuse_color[1] = 0.2f;
	material_optimus.diffuse_color[2] = 0.9f;
	material_optimus.diffuse_color[3] = 1.0f;

	material_optimus.specular_color[0] = 1.0f;
	material_optimus.specular_color[1] = 1.0f;
	material_optimus.specular_color[2] = 1.0f;
	material_optimus.specular_color[3] = 1.0f;

	material_optimus.specular_exponent = 52.334717f;

	material_optimus.emissive_color[0] = 0.000000f;
	material_optimus.emissive_color[1] = 0.000000f;
	material_optimus.emissive_color[2] = 0.000000f;
	material_optimus.emissive_color[3] = 1.0f;

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_OPTIMUS]);

	My_glTexImage2D_from_file("Data/static_objects/Optimus.png");

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);
	*/
}

void draw_optimus_20211511(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glFrontFace(GL_CW);
	glUseProgram(h_ShaderProgram_simple);
	glUniform3f(loc_primitive_color, 0.0f, 0.0f, 255.0f);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, 0.0f, 2300.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(1.0f, 1.0f, 1.0f));

	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	
	glBindVertexArray(optimus_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * optimus_n_triangles);
	glBindVertexArray(0);
	
	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

/*
	cat
*/
GLuint cat_VBO, cat_VAO;
int cat_n_triangles;
GLfloat* cat_vertices;

Material_Parameters material_cat;

void prepare_cat_20211511(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, cat_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/cat_vnt.geom");
	cat_n_triangles = read_geometry_vnt(&cat_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	cat_n_total_triangles += cat_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &cat_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, cat_VBO);
	glBufferData(GL_ARRAY_BUFFER, cat_n_total_triangles * 3 * n_bytes_per_vertex, cat_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(cat_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &cat_VAO);
	glBindVertexArray(cat_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, cat_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/* texture
	material_cat.ambient_color[0] = 0.5f;
	material_cat.ambient_color[1] = 0.5f;
	material_cat.ambient_color[2] = 0.5f;
	material_cat.ambient_color[3] = 1.0f;

	material_cat.diffuse_color[0] = 0.7f;
	material_cat.diffuse_color[1] = 0.5f;
	material_cat.diffuse_color[2] = 0.2f;
	material_cat.diffuse_color[3] = 1.0f;

	material_cat.specular_color[0] = 0.4f;
	material_cat.specular_color[1] = 0.4f;
	material_cat.specular_color[2] = 0.2f;
	material_cat.specular_color[3] = 1.0f;

	material_cat.specular_exponent = 5.334717f;

	material_cat.emissive_color[0] = 0.000000f;
	material_cat.emissive_color[1] = 0.000000f;
	material_cat.emissive_color[2] = 0.000000f;
	material_cat.emissive_color[3] = 1.0f;

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_CAT]);

	My_glTexImage2D_from_file("Data/static_objects/cat_diff.tga");

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);
	*/
}

void draw_cat1_20211511(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glFrontFace(GL_CW);
	glUseProgram(h_ShaderProgram_simple);
	glUniform3f(loc_primitive_color, 255.0f, 255.0f, 0.0f);
	
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-450.0f, -2480.0f, 233.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(40.0f, 40.0f, 40.0f));

	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(cat_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * cat_n_triangles);
	glBindVertexArray(0);

	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void draw_cat2_20211511(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glFrontFace(GL_CW);
	glUseProgram(h_ShaderProgram_simple);
	glUniform3f(loc_primitive_color, 0.0f, 255.0f, 0.0f);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(500.0f, -3000.0f, 50.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(40.0f, 40.0f, 40.0f));

	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(cat_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * cat_n_triangles);
	glBindVertexArray(0);

	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

/* 
	bike
*/
GLuint bike_VBO, bike_VAO;
int bike_n_triangles;
GLfloat* bike_vertices;

Material_Parameters material_bike;

void prepare_bike_20211511(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, bike_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/bike_vnt.geom");
	bike_n_triangles = read_geometry_vnt(&bike_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	bike_n_total_triangles += bike_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &bike_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, bike_VBO);
	glBufferData(GL_ARRAY_BUFFER, bike_n_total_triangles * 3 * n_bytes_per_vertex, bike_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(bike_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &bike_VAO);
	glBindVertexArray(bike_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, bike_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/* texture
	material_bike.ambient_color[0] = 0.5f;
	material_bike.ambient_color[1] = 0.5f;
	material_bike.ambient_color[2] = 0.5f;
	material_bike.ambient_color[3] = 1.0f;

	material_bike.diffuse_color[0] = 0.800000f;
	material_bike.diffuse_color[1] = 0.800000f;
	material_bike.diffuse_color[2] = 0.900000f;
	material_bike.diffuse_color[3] = 1.0f;

	material_bike.specular_color[0] = 1.0f;
	material_bike.specular_color[1] = 1.0f;
	material_bike.specular_color[2] = 1.0f;
	material_bike.specular_color[3] = 1.0f;

	material_bike.specular_exponent = 522.334717f;

	material_bike.emissive_color[0] = 0.000000f;
	material_bike.emissive_color[1] = 0.000000f;
	material_bike.emissive_color[2] = 0.000000f;
	material_bike.emissive_color[3] = 1.0f;

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_BIKE]);

	My_glTexImage2D_from_file("Data/static_objects/Bike.png");

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);
	*/
}

void draw_bike_20211511(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glFrontFace(GL_CW);
	glUseProgram(h_ShaderProgram_simple);
	glUniform3f(loc_primitive_color, 255.0f, 0.0f, 0.0f);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(170.0f, -6400.0f, 30.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 15.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(80.0f, 80.0f, 80.0f));

	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(bike_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * bike_n_triangles);
	glBindVertexArray(0);

	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

/*
	ant
*/
GLuint ant_VBO, ant_VAO;
int ant_n_triangles;
GLfloat* ant_vertices;

Material_Parameters material_ant;

void prepare_ant_20211511(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, ant_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/ant_vnt.geom");
	ant_n_triangles = read_geometry_vnt(&ant_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	ant_n_total_triangles += ant_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &ant_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ant_VBO);
	glBufferData(GL_ARRAY_BUFFER, ant_n_total_triangles * 3 * n_bytes_per_vertex, ant_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(ant_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &ant_VAO);
	glBindVertexArray(ant_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ant_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/* texture
	// need to change values
	material_ant.ambient_color[0] = 0.5f;
	material_ant.ambient_color[1] = 0.5f;
	material_ant.ambient_color[2] = 0.5f;
	material_ant.ambient_color[3] = 1.0f;

	material_ant.diffuse_color[0] = 0.7f;
	material_ant.diffuse_color[1] = 0.5f;
	material_ant.diffuse_color[2] = 0.2f;
	material_ant.diffuse_color[3] = 1.0f;

	material_ant.specular_color[0] = 0.4f;
	material_ant.specular_color[1] = 0.4f;
	material_ant.specular_color[2] = 0.2f;
	material_ant.specular_color[3] = 1.0f;

	material_ant.specular_exponent = 5.334717f;

	material_ant.emissive_color[0] = 0.000000f;
	material_ant.emissive_color[1] = 0.000000f;
	material_ant.emissive_color[2] = 0.000000f;
	material_ant.emissive_color[3] = 1.0f;

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_ANT]);

	My_glTexImage2D_from_file("Data/static_objects/antTexture.jpg");

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);
	*/
}

void draw_ant_20211511(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glFrontFace(GL_CW);
	glUseProgram(h_ShaderProgram_simple);
	glUniform3f(loc_primitive_color, 0.0f, 0.0f, 0.0f);

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-250.0f, 1050.0f, 800.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 10.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 30.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(50.0f, 50.0f, 50.0f));

	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(ant_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * ant_n_triangles);
	glBindVertexArray(0);

	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
/*****************************  END: static objects *****************************/


/********************  START: callback function definitions *********************/
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw_grid();
	draw_axes();
	draw_sun_temple_24();
	draw_skybox();

	glLineWidth(1.0f);

	//dynamic objects
	draw_tiger_20211511();
	draw_ben_20211511();

	//static objects
	draw_optimus_20211511();
	draw_cat1_20211511();
	draw_cat2_20211511();
	draw_bike_20211511();
	draw_ant_20211511();

	glutSwapBuffers();
}

void timer_tiger_20211511(int value) {
	timestamp_tiger = (timestamp_tiger + 1) % UINT_MAX;
	cur_frame_tiger = timestamp_tiger % N_TIGER_FRAMES;
	rotation_angle_tiger = (timestamp_tiger % 360) * TO_RADIAN;

	glutPostRedisplay();
	if (flag_tiger_animation)
		glutTimerFunc(tiger_speed, timer_tiger_20211511, 0);
}

void timer_ben_20211511(int value) {
	timestamp_ben = (timestamp_ben + 1) % UINT_MAX;
	cur_frame_ben = timestamp_ben % N_TIGER_FRAMES;
	//rotation_angle_ben = (timestamp_scene % 360) * TO_RADIAN;

	glm::mat3 cameraRotation;
	glm::vec3 afterRotate;
	
	if (ben_cam) {
		reposition_camera(CAMERA_N);
		current_camera.pos[0] = ben_x; current_camera.pos[1] = ben_y;

		ben_walk += ben_walk_dw;
		if (ben_walk > 1 || ben_walk < -1) ben_walk_dw *= -1.0f;

		// v axis rotate
		cameraRotation = glm::mat3(glm::rotate(glm::mat4(1.0f), (rotation_angle_ben - 90) * TO_RADIAN, glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2])));

		// u axis move
		afterRotate = cameraRotation * glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);
		current_camera.uaxis[0] = afterRotate.x;
		current_camera.uaxis[1] = afterRotate.y;
		current_camera.uaxis[2] = afterRotate.z;

		//n axis move
		afterRotate = cameraRotation * glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
		current_camera.naxis[0] = afterRotate.x;
		current_camera.naxis[1] = afterRotate.y;
		current_camera.naxis[2] = afterRotate.z;

		set_ViewMatrix_from_camera_frame();

		ViewMatrix = glm::translate(ViewMatrix, glm::vec3(ben_x, ben_y, 300));
		ViewMatrix = glm::rotate(ViewMatrix, ben_walk * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ViewMatrix = glm::translate(ViewMatrix, glm::vec3((-1.0f) * ben_x, (-1.0f) * ben_y, (-1.0f) * 300));
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	}

	glutPostRedisplay();
	if (flag_ben_animation)
		glutTimerFunc(ben_speed, timer_ben_20211511, 0);
}

void keyboard(unsigned char key, int x, int y) {
	// Camera Moving
	float cameraMove = 50.0f;

	switch (key) {
	case 'f':
		b_draw_grid = b_draw_grid ? false : true;
		glutPostRedisplay();
		break;
	case '1':
		ben_cam = 0;
		reposition_camera(CAMERA_1);
		glutPostRedisplay();
		break;
	case '2':
		ben_cam = 0;
		reposition_camera(CAMERA_2);
		glutPostRedisplay();
		break;
	case '3':
		ben_cam = 0;
		reposition_camera(CAMERA_3);
		glutPostRedisplay();
		break;
	case '4':
		ben_cam = 0;
		reposition_camera(CAMERA_4);
		glutPostRedisplay();
		break;
	case '5':
		ben_cam = 0;
		reposition_camera(CAMERA_5);
		glutPostRedisplay();
		break;
	case '6':
		ben_cam = 0;
		reposition_camera(CAMERA_6);
		glutPostRedisplay();
		break;
	case 'a':
		ben_cam = 0;
		reposition_camera(CAMERA_A);
		glutPostRedisplay();
		break;
	case 'n':
		ben_cam = 1;
		break;
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 's':
		if (current_camera.move) {
			current_camera.pos[1] -= cameraMove;
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 'x':
		if (current_camera.move) {
			current_camera.pos[1] += cameraMove;
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 'z':
		if (current_camera.move) {
			current_camera.pos[0] += cameraMove;
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 'c':
		if (current_camera.move) {
			current_camera.pos[0] -= cameraMove;
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 'u':
		if (current_camera.move) {
			current_camera.pos[2] += cameraMove;
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 'd':
		if (current_camera.move) {
			current_camera.pos[2] -= cameraMove;
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 't':
		flag_tiger_animation = 1 - flag_tiger_animation;

		if (flag_tiger_animation) {
			glutTimerFunc(tiger_speed, timer_tiger_20211511, 0);
		}
		break;
	case 'b':
		flag_ben_animation = 1 - flag_ben_animation;

		if (flag_ben_animation) {
			glutTimerFunc(ben_speed, timer_ben_20211511, 0);
		}
		break;
	case 'j':
		if (flag_tiger_animation) {
			tiger_jump = 1;
			tiger_jump_v = 30.0f;
		}
		break;
	case 'm':
		tiger_move_start = 1;
		break;
	}
}

int prevx, prevy;
int leftbuttonpressed = 0, rightbuttonpressed = 0, middlebuttonpressed = 0;
void mouse(int button, int state, int x, int y) {
	//left mouse
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		leftbuttonpressed = 1;
		prevx = x; 
		prevy = y;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		leftbuttonpressed = 0;

	//right mouse
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		rightbuttonpressed = 1;
		prevx = x;
		prevy = y;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		rightbuttonpressed = 0;
	}

	//middle mouse
	if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
		middlebuttonpressed = 1;
		prevx = x;
		prevy = y;
	}
	else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP) {
		middlebuttonpressed = 0;
	}
}

void mousemotion(int x, int y) {
	
	glm::mat3 cameraRotation1;
	glm::mat3 cameraRotation2;
	glm::vec3 afterRotate1;
	glm::vec3 afterRotate2;
	float cameraRotate = 0.3f;

	if (leftbuttonpressed) {
		//world move camera rotate
		if (current_camera.move) {
			// v axis rotate
			cameraRotation1 = glm::mat3(glm::rotate(glm::mat4(1.0f), (prevx - x) * cameraRotate * TO_RADIAN, glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2])));

			// u axis move
			afterRotate1 = cameraRotation1 * glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);
			current_camera.uaxis[0] = afterRotate1.x;
			current_camera.uaxis[1] = afterRotate1.y;
			current_camera.uaxis[2] = afterRotate1.z;

			//n axis move
			afterRotate1 = cameraRotation1 * glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
			current_camera.naxis[0] = afterRotate1.x;
			current_camera.naxis[1] = afterRotate1.y;
			current_camera.naxis[2] = afterRotate1.z;

			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
	}

	if (rightbuttonpressed) {
		//world move camera rotate
		if (current_camera.move) {
			// u axis rotate
			cameraRotation2 = glm::mat3(glm::rotate(glm::mat4(1.0f), (prevy - y) * cameraRotate * TO_RADIAN, glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2])));

			// v axis move
			afterRotate2 = cameraRotation2 * glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);
			current_camera.vaxis[0] = afterRotate2.x;
			current_camera.vaxis[1] = afterRotate2.y;
			current_camera.vaxis[2] = afterRotate2.z;

			//n axis move
			afterRotate2 = cameraRotation2 * glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
			current_camera.naxis[0] = afterRotate2.x;
			current_camera.naxis[1] = afterRotate2.y;
			current_camera.naxis[2] = afterRotate2.z;

			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
	}

	prevx = x;
	prevy = y;
}

int mod;
void mousewheel(int wheel, int dir, int x, int y) {
	mod = glutGetModifiers();

	glm::mat3 cameraRotation;
	glm::vec3 afterRotate;
	float cameraRotate = 2.0f;

	// world fix camera move
	if (mod == GLUT_ACTIVE_CTRL) {
		// v axis rotate
		if (current_camera.move == 0) {
			if ((current_camera.angle1 > 35.0f && dir > 0) || (current_camera.angle1 < -35.0f && dir < 0)) dir = 0;
			current_camera.angle1 += (dir * cameraRotate);

			cameraRotation = glm::mat3(glm::rotate(glm::mat4(1.0f), dir * cameraRotate * TO_RADIAN, glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2])));

			// u axis move
			afterRotate = cameraRotation * glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);
			current_camera.uaxis[0] = afterRotate.x;
			current_camera.uaxis[1] = afterRotate.y;
			current_camera.uaxis[2] = afterRotate.z;

			//n axis move
			afterRotate = cameraRotation * glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
			current_camera.naxis[0] = afterRotate.x;
			current_camera.naxis[1] = afterRotate.y;
			current_camera.naxis[2] = afterRotate.z;

			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
	}
	if (mod == GLUT_ACTIVE_ALT) {
		if (current_camera.move == 0) {
			// u axis rotate
			if ((current_camera.angle2 > 35.0f && dir > 0) || (current_camera.angle2 < -35.0f && dir < 0)) dir = 0;
			current_camera.angle2 += (dir * cameraRotate);

			cameraRotation = glm::mat3(glm::rotate(glm::mat4(1.0f), dir * cameraRotate * TO_RADIAN, glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2])));

			// v axis move
			afterRotate = cameraRotation * glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);
			current_camera.vaxis[0] = afterRotate.x;
			current_camera.vaxis[1] = afterRotate.y;
			current_camera.vaxis[2] = afterRotate.z;

			//n axis move
			afterRotate = cameraRotation * glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
			current_camera.naxis[0] = afterRotate.x;
			current_camera.naxis[1] = afterRotate.y;
			current_camera.naxis[2] = afterRotate.z;

			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
	}

	// world move camera zoom
	if (mod == GLUT_ACTIVE_SHIFT) {
		if ((current_camera.fovy <= 0.0f && dir > 0) || (current_camera.fovy >= 3.0f && dir < 0)) dir = 0;
		if (dir > 0) {		//zoom in
			current_camera.fovy -= 0.1f;
		}
		else if (dir < 0) {		// zoom out
			current_camera.fovy += 0.1f;
		}

		set_ViewMatrix_from_camera_frame();
		ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
	}
}

void reshape(int width, int height) {
	float aspect_ratio;

	glViewport(0, 0, width, height);

	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &grid_VAO);
	glDeleteBuffers(1, &grid_VBO);

	glDeleteVertexArrays(scene.n_materials, sun_temple_VAO);
	glDeleteBuffers(scene.n_materials, sun_temple_VBO);
	glDeleteTextures(scene.n_textures, sun_temple_texture_names);

	glDeleteVertexArrays(1, &skybox_VAO);
	glDeleteBuffers(1, &skybox_VBO);

	free(sun_temple_n_triangles);
	free(sun_temple_vertex_offset);

	free(sun_temple_VAO);
	free(sun_temple_VBO);

	free(sun_temple_texture_names);
}
/*********************  END: callback function definitions **********************/

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);

	glutMouseFunc(mouse);
	glutMotionFunc(mousemotion);
	glutMouseWheelFunc(mousewheel);
	glutTimerFunc(100, timer_tiger_20211511, 0);
	glutTimerFunc(100, timer_ben_20211511, 0);
}

void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::mat4(1.0f);
	ProjectionMatrix = glm::mat4(1.0f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	initialize_lights();
}

void prepare_scene(void) {
	prepare_axes();
	prepare_grid();
	prepare_sun_temple();
	prepare_skybox();

	//dynamic objects
	prepare_tiger_20211511();
	prepare_ben_20211511();

	//static objects
	prepare_optimus_20211511();
	prepare_cat_20211511();
	prepare_bike_20211511();
	prepare_ant_20211511();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
	initialize_camera();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "********************************************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "********************************************************************************\n\n");
}

void print_message(const char* m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char* program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "********************************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n********************************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 9
void ST_drawScene_24(int argc, char* argv[]) {
	char program_name[64] = "Sogang CSE4170 Sun Temple Scene 24";
	char messages[N_MESSAGE_LINES][256] = { 
		"    - Keys used:",
		"		'1' : set the camera for bronze statue view",
		"		'2' : set the camera for bronze statue view",
		"		'3' : set the camera for tree view",	
		"		'4' : set the camera for top view",
		"		'5' : set the camera for front view",
		"		'6' : set the camera for side view",
		"		'f' : draw x, y, z axes and grid",
		"		'ESC' : program close",
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(900, 600);
	glutInitWindowPosition(20, 20);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
