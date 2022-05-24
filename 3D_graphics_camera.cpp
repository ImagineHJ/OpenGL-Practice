//
//  DrawScene.cpp
//
//  Written for CSE4170
//  Department of Computer Science and Engineering
//  Copyright © 2022 Sogang University. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "LoadScene.h"

// Begin of shader setup
#include "Shaders/LoadShaders.h"
#include "ShadingInfo.h"

extern SCENE scene;

// for simple shaders
GLuint h_ShaderProgram_simple, h_ShaderProgram_TXPS; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// for Phong Shading (Textured) shaders
#define NUMBER_OF_LIGHT_SUPPORTED 13
GLint loc_global_ambient_color;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;
GLint loc_texture;
GLint loc_flag_texture_mapping;
GLint loc_flag_fog;

// include glm/*.hpp only if necessary
// #include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
# include <glm/gtc/matrix_inverse.hpp> // affineInverse
// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;
// ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix
glm::mat4 ModelViewProjectionMatrix; // This one is sent to vertex shader when ready.
glm::mat4 ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

/*********************************  START: camera *********************************/
typedef enum {
	CAMERA_0,
	CAMERA_1,
	CAMERA_2,
	CAMERA_3,
	CAMERA_M,
	CAMERA_T,
	CAMERA_G,
	NUM_CAMERAS
} CAMERA_INDEX;

typedef struct _Camera {
	float pos[3];
	float uaxis[3], vaxis[3], naxis[3];
	float fovy, aspect_ratio, near_c, far_c;
	int move, rotation_axis;
} Camera;

int current_camera_index;
Camera camera_info[NUM_CAMERAS];
Camera current_camera;

glm::mat4 ModelMatrix_TO_TIGER_EYE, ModelMatrix_TIGER;
void calculate_model_matrix_to_tiger_eye() {
	ModelMatrix_TO_TIGER_EYE = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -88.0f, 62.0f));
	ModelMatrix_TO_TIGER_EYE = glm::rotate(ModelMatrix_TO_TIGER_EYE, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelMatrix_TO_TIGER_EYE = glm::rotate(ModelMatrix_TO_TIGER_EYE, 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
}

int tiger_node_timer = 0;
void set_ViewMatrix_for_tiger(void) {
	glm::mat4 ModelMatrix_TIGER_CAM, Matrix_CAMERA_Tiger_inverse;

	// tiger nodding
	if (tiger_node_timer< 6) ModelMatrix_TIGER_CAM = glm::rotate(ModelMatrix_TIGER, (3.0f - tiger_node_timer * 6.0f / 6.0f) * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	else if (tiger_node_timer < 12) ModelMatrix_TIGER_CAM = glm::rotate(ModelMatrix_TIGER, ((tiger_node_timer - 6) * 6.0f / 6.0f - 3.0f) * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	
	Matrix_CAMERA_Tiger_inverse = ModelMatrix_TIGER_CAM * ModelMatrix_TO_TIGER_EYE;
	ViewMatrix = glm::affineInverse(Matrix_CAMERA_Tiger_inverse);
}

void set_ViewMatrix_for_tiger_follower(void) {
	glm::mat4 ModelMatrix_TIGER__FOLLOWER_CAM, Matrix_CAMERA_Tiger_inverse;

	ModelMatrix_TIGER__FOLLOWER_CAM = glm::translate(ModelMatrix_TIGER, glm::vec3(0.0f, 200.0f, 0.0f));

	Matrix_CAMERA_Tiger_inverse = ModelMatrix_TIGER__FOLLOWER_CAM * ModelMatrix_TO_TIGER_EYE;
	ViewMatrix = glm::affineInverse(Matrix_CAMERA_Tiger_inverse);
}

using glm::mat4;
void set_ViewMatrix_from_camera_frame(void) {
	if (current_camera_index == CAMERA_T) set_ViewMatrix_for_tiger();
	else if (current_camera_index == CAMERA_G) set_ViewMatrix_for_tiger_follower();
	else {
		ViewMatrix = glm::mat4(current_camera.uaxis[0], current_camera.vaxis[0], current_camera.naxis[0], 0.0f,
			current_camera.uaxis[1], current_camera.vaxis[1], current_camera.naxis[1], 0.0f,
			current_camera.uaxis[2], current_camera.vaxis[2], current_camera.naxis[2], 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		ViewMatrix = glm::translate(ViewMatrix, glm::vec3(-current_camera.pos[0], -current_camera.pos[1], -current_camera.pos[2]));
	}
}

void set_ProjectionMatrix_from_camera_fov(void) {
	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
}

void set_current_camera(int camera_num) {
	Camera* pCamera = &camera_info[camera_num];
	current_camera_index = camera_num;

	memcpy(&current_camera, pCamera, sizeof(Camera));
	set_ViewMatrix_from_camera_frame();
	set_ProjectionMatrix_from_camera_fov();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void save_current_camera_to_camera_info() {
	Camera* pCamera = &camera_info[current_camera_index];

	pCamera->pos[0] = current_camera.pos[0]; pCamera->pos[1] = current_camera.pos[1]; pCamera->pos[2] = current_camera.pos[2];
	pCamera->uaxis[0] = current_camera.uaxis[0]; pCamera->uaxis[1] = current_camera.uaxis[1]; pCamera->uaxis[2] = current_camera.uaxis[2];
	pCamera->vaxis[0] = current_camera.vaxis[0]; pCamera->vaxis[1] = current_camera.vaxis[1]; pCamera->vaxis[2] = current_camera.vaxis[2];
	pCamera->naxis[0] = current_camera.naxis[0]; pCamera->naxis[1] = current_camera.naxis[1]; pCamera->naxis[2] = current_camera.naxis[2];
	pCamera->move = 0;
	pCamera->fovy = current_camera.fovy; pCamera->aspect_ratio = current_camera.aspect_ratio;
	pCamera->near_c = current_camera.near_c; pCamera->far_c = current_camera.far_c;
}

void initialize_camera(void) {

	//CAMERA_0 : circle dome view
	Camera* pCamera = &camera_info[CAMERA_0];
	pCamera->pos[0] = 10.452802f; pCamera->pos[1] = 1229.583862f; pCamera->pos[2] = 1127.422485f;
	pCamera->uaxis[0] = -0.999464f; pCamera->uaxis[1] = -0.000871f; pCamera->uaxis[2] = 0.032395f;
	pCamera->vaxis[0] = 0.025032f; pCamera->vaxis[1] = -0.562199f; pCamera->vaxis[2] = 0.826603f;
	pCamera->naxis[0] = 0.015568f; pCamera->naxis[1] = 0.863789f; pCamera->naxis[2] = 0.503572f;
	pCamera->move = 0;
	pCamera->fovy = 0.868132, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_1 : center statue view
	pCamera = &camera_info[CAMERA_1];
	pCamera->pos[0] = 472.979431f; pCamera->pos[1] = -3620.326904f; pCamera->pos[2] = 880.662415f;
	pCamera->uaxis[0] = 0.937737f; pCamera->uaxis[1] = 0.347334f; pCamera->uaxis[2] = 0.001239f;
	pCamera->vaxis[0] = -0.182926f; pCamera->vaxis[1] = 0.490829f; pCamera->vaxis[2] = 0.851832f;
	pCamera->naxis[0] = 0.295264f; pCamera->naxis[1] = -0.799025f; pCamera->naxis[2] = 0.523806f;
	pCamera->move = 0;
	pCamera->fovy = 1.178131, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_2 : hallway view
	pCamera = &camera_info[CAMERA_2];
	pCamera->pos[0] = -34.731163f; pCamera->pos[1] = -4000.711426f; pCamera->pos[2] = 276.430939f;
	pCamera->uaxis[0] = -0.999008f; pCamera->uaxis[1] = -0.041420f; pCamera->uaxis[2] = -0.016236f;
	pCamera->vaxis[0] = -0.012671f; pCamera->vaxis[1] = -0.084912f; pCamera->vaxis[2] = 0.996306f;
	pCamera->naxis[0] = -0.042646f; pCamera->naxis[1] = 0.995525f; pCamera->naxis[2] = 0.084303f;
	pCamera->move = 0;
	pCamera->fovy = 0.888131, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_3 : top view
	pCamera = &camera_info[CAMERA_3];
	pCamera->pos[0] = 21.528721f; pCamera->pos[1] = -1548.346680f; pCamera->pos[2] = 1191.002808f;
	pCamera->uaxis[0] = -0.998385f; pCamera->uaxis[1] = 0.025137f; pCamera->uaxis[2] = 0.050859f;
	pCamera->vaxis[0] = 0.025160f; pCamera->vaxis[1] = -0.607291f; pCamera->vaxis[2] = 0.794074f;
	pCamera->naxis[0] = 0.050849f; pCamera->naxis[1] = 0.794075f; pCamera->naxis[2] = 0.605679f;
	pCamera->move = 0;
	pCamera->fovy = 0.928131, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_M : moving view
	pCamera = &camera_info[CAMERA_M];
	pCamera->pos[0] = 472.979431f; pCamera->pos[1] = -3620.326904f; pCamera->pos[2] = 880.662415f;
	pCamera->uaxis[0] = 0.937737f; pCamera->uaxis[1] = 0.347334f; pCamera->uaxis[2] = 0.001239f;
	pCamera->vaxis[0] = -0.182926f; pCamera->vaxis[1] = 0.490829f; pCamera->vaxis[2] = 0.851832f;
	pCamera->naxis[0] = 0.295264f; pCamera->naxis[1] = -0.799025f; pCamera->naxis[2] = 0.523806f;
	pCamera->move = 0;
	pCamera->fovy = 1.178131, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_T : tiger view
	pCamera = &camera_info[CAMERA_T];
	for (int k = 0; k < 3; k++)
	{
		pCamera->pos[k] = scene.camera.e[k];
		pCamera->uaxis[k] = scene.camera.u[k];
		pCamera->vaxis[k] = scene.camera.v[k];
		pCamera->naxis[k] = scene.camera.n[k];
	}

	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy + 1.0f, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_G : tiger follower view
	pCamera = &camera_info[CAMERA_G];
	for (int k = 0; k < 3; k++)
	{
		pCamera->pos[k] = scene.camera.e[k];
		pCamera->uaxis[k] = scene.camera.u[k];
		pCamera->vaxis[k] = scene.camera.v[k];
		pCamera->naxis[k] = scene.camera.n[k];
	}

	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy + 1.0f, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	set_current_camera(CAMERA_0);
}

enum axes { X_AXIS, Y_AXIS, Z_AXIS };
#define CAM_TSPEED 3.0f

void renew_cam_position(int del, int flag_translation_axis) {
	switch (flag_translation_axis) {
	case X_AXIS:
		current_camera.pos[0] += CAM_TSPEED * del * (current_camera.uaxis[0]);
		current_camera.pos[1] += CAM_TSPEED * del * (current_camera.uaxis[1]);
		current_camera.pos[2] += CAM_TSPEED * del * (current_camera.uaxis[2]);
		break;
	case Y_AXIS:
		current_camera.pos[0] += CAM_TSPEED * del * (current_camera.vaxis[0]);
		current_camera.pos[1] += CAM_TSPEED * del * (current_camera.vaxis[1]);
		current_camera.pos[2] += CAM_TSPEED * del * (current_camera.vaxis[2]);
		break;
	case Z_AXIS:
		current_camera.pos[0] += CAM_TSPEED * del * (-current_camera.naxis[0]);
		current_camera.pos[1] += CAM_TSPEED * del * (-current_camera.naxis[1]);
		current_camera.pos[2] += CAM_TSPEED * del * (-current_camera.naxis[2]);
		break;
	}
	save_current_camera_to_camera_info();
}

#define CAM_RSPEED 0.1f
void renew_cam_orientation_rotation(int angle, int flag_rotation_axis) {
	// let's get a help from glm
	glm::mat3 RotationMatrix;
	glm::vec3 direction;

	switch (flag_rotation_axis) {
	case X_AXIS:
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle,
			glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2])));

		direction = RotationMatrix * glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);
		current_camera.vaxis[0] = direction.x; current_camera.vaxis[1] = direction.y; current_camera.vaxis[2] = direction.z;
		direction = RotationMatrix * glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
		current_camera.naxis[0] = direction.x; current_camera.naxis[1] = direction.y; current_camera.naxis[2] = direction.z;
		break;
	case Y_AXIS:
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle,
			glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2])));

		direction = RotationMatrix * glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);
		current_camera.uaxis[0] = direction.x; current_camera.uaxis[1] = direction.y; current_camera.uaxis[2] = direction.z;
		direction = RotationMatrix * glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
		current_camera.naxis[0] = direction.x; current_camera.naxis[1] = direction.y; current_camera.naxis[2] = direction.z;
		break;
	case Z_AXIS:
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle,
			glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2])));

		direction = RotationMatrix * glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);
		current_camera.uaxis[0] = direction.x; current_camera.uaxis[1] = direction.y; current_camera.uaxis[2] = direction.z;
		direction = RotationMatrix * glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);
		current_camera.vaxis[0] = direction.x; current_camera.vaxis[1] = direction.y; current_camera.vaxis[2] = direction.z;
		break;
	}
	save_current_camera_to_camera_info();
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

	ShaderInfo shader_info_TXPS[3] = {
	{ GL_VERTEX_SHADER, "Shaders/Phong_Tx.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/Phong_Tx.frag" },
	{ GL_NONE, NULL }
	};

	h_ShaderProgram_simple = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram_simple);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");

	h_ShaderProgram_TXPS = LoadShaders(shader_info_TXPS);
	loc_ModelViewProjectionMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_global_ambient_color");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_TXPS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_exponent");

	loc_texture = glGetUniformLocation(h_ShaderProgram_TXPS, "u_base_texture");
	loc_flag_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_texture_mapping");
	loc_flag_fog = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_fog");
}
/*******************************  END: shder setup ******************************/

/****************************  START: geometry setup ****************************/
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))
#define INDEX_VERTEX_POSITION	0
#define INDEX_NORMAL			1
#define INDEX_TEX_COORD			2

bool b_draw_grid = false;

//axes
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

//grid
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

int flag_fog;
bool* flag_texture_mapping;

void initialize_lights(void) { // follow OpenGL conventions for initialization
	int i;

	glUseProgram(h_ShaderProgram_TXPS);

	glUniform4f(loc_global_ambient_color, 1.0f, 1.0f, 1.0f, 1.0f);

	for (i = 0; i < scene.n_lights; i++) {
		glUniform1i(loc_light[i].light_on, 1);
		glUniform4f(loc_light[i].position,
			scene.light_list[i].pos[0],
			scene.light_list[i].pos[1],
			scene.light_list[i].pos[2],
			1.0f);

		glUniform4f(loc_light[i].ambient_color, 0.13f, 0.13f, 0.13f, 1.0f);
		glUniform4f(loc_light[i].diffuse_color, 0.5f, 0.5f, 0.5f, 1.0f);
		glUniform4f(loc_light[i].specular_color, 0.8f, 0.8f, 0.8f, 1.0f);
		glUniform3f(loc_light[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light[i].spot_exponent, 0.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 20.0f, 0.0f, 0.0f, 1.0f); // .w != 0.0f for no ligth attenuation
	}

	glUseProgram(0);
}

void initialize_flags(void) {
	flag_fog = 0;

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_flag_fog, flag_fog);
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
	if (tx_bits_per_pixel == 32)
		tx_pixmap_32 = tx_pixmap;
	else {
		//fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap_32 = FreeImage_ConvertTo32Bits(tx_pixmap);
	}

	width = FreeImage_GetWidth(tx_pixmap_32);
	height = FreeImage_GetHeight(tx_pixmap_32);
	data = FreeImage_GetBits(tx_pixmap_32);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap_32);
	if (tx_bits_per_pixel != 32)
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

	flag_texture_mapping = (bool*)malloc(sizeof(bool) * scene.n_textures);

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
		glActiveTexture(GL_TEXTURE0 + texId);
		glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[texId]);

		bool bReturn = readTexImage2D_from_file(scene.texture_file_name[texId]);

		if (bReturn) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			flag_texture_mapping[texId] = true;
		}
		else {
			flag_texture_mapping[texId] = false;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	fprintf(stdout, " * Loaded sun temple textures into graphics memory.\n\n");
	
	free(sun_temple_vertices);
}

void draw_sun_temple(void) {
	glUseProgram(h_ShaderProgram_TXPS);
	ModelViewMatrix = ViewMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		// set material
		glUniform4fv(loc_material.ambient_color, 1, scene.material_list[materialIdx].shading.ph.ka);
		glUniform4fv(loc_material.diffuse_color, 1, scene.material_list[materialIdx].shading.ph.kd);
		glUniform4fv(loc_material.specular_color, 1, scene.material_list[materialIdx].shading.ph.ks);
		glUniform1f(loc_material.specular_exponent, scene.material_list[materialIdx].shading.ph.spec_exp);
		glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);

		int texId = scene.material_list[materialIdx].diffuseTexId;
		glUniform1i(loc_texture, texId);
		glUniform1i(loc_flag_texture_mapping, flag_texture_mapping[texId]);

		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0 + texId);
		glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[texId]);

		glBindVertexArray(sun_temple_VAO[materialIdx]);
		glDrawArrays(GL_TRIANGLES, 0, 3 * sun_temple_n_triangles[materialIdx]);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
}

//tiger
#define N_TIGER_FRAMES 12

int cur_frame_tiger = 0, tiger_timer = 0, tiger_moving = 1;
GLuint tiger_VBO, tiger_VAO;
int tiger_n_triangles[N_TIGER_FRAMES];
int tiger_vertex_offset[N_TIGER_FRAMES];
GLfloat* tiger_vertices[N_TIGER_FRAMES];


int read_geometry(GLfloat** object, int bytes_per_primitive, char* filename) {
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

void prepare_tiger(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tiger_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(filename, "Data/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry(&tiger_vertices[i], n_bytes_per_triangle, filename);
		// Assume all geometry files are effective.
		tiger_n_total_triangles += tiger_n_triangles[i];

		if (i == 0)
			tiger_vertex_offset[i] = 0;
		else
			tiger_vertex_offset[i] = tiger_vertex_offset[i - 1] + 3 * tiger_n_triangles[i - 1];
	}

	// Initialize vertex buffer object.
	glGenBuffers(1, &tiger_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glBufferData(GL_ARRAY_BUFFER, tiger_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_TIGER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, tiger_vertex_offset[i] * n_bytes_per_vertex,
			tiger_n_triangles[i] * n_bytes_per_triangle, tiger_vertices[i]);

	// As the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_TIGER_FRAMES; i++)
		free(tiger_vertices[i]);

	// Initialize vertex array object.
	glGenVertexArrays(1, &tiger_VAO);
	glBindVertexArray(tiger_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);

	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded tiger into graphics memory.\n");
	calculate_model_matrix_to_tiger_eye();
}

void draw_tiger(void) {

	glFrontFace(GL_CCW);

	glUniform3f(loc_primitive_color, 0.0f, 1.0f, 1.0f); // Tiger wireframe color = cyan
	
	glBindVertexArray(tiger_VAO);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);
}

// spider
#define N_SPIDER_FRAMES 16
GLuint spider_VBO, spider_VAO;
int spider_n_triangles[N_SPIDER_FRAMES];
int spider_vertex_offset[N_SPIDER_FRAMES];
GLfloat* spider_vertices[N_SPIDER_FRAMES];
int cur_frame_spider = 0, spider_timer = 0;

void prepare_spider(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, spider_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_SPIDER_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/spider/spider_vnt_%d%d.geom", i / 10, i % 10);
		spider_n_triangles[i] = read_geometry(&spider_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		spider_n_total_triangles += spider_n_triangles[i];

		if (i == 0)
			spider_vertex_offset[i] = 0;
		else
			spider_vertex_offset[i] = spider_vertex_offset[i - 1] + 3 * spider_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &spider_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glBufferData(GL_ARRAY_BUFFER, spider_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_SPIDER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, spider_vertex_offset[i] * n_bytes_per_vertex,
			spider_n_triangles[i] * n_bytes_per_triangle, spider_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_SPIDER_FRAMES; i++)
		free(spider_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &spider_VAO);
	glBindVertexArray(spider_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded spider into graphics memory.\n");
}

void draw_spider(void) {

	glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 1.0f, 0.0f, 1.0f);

	glBindVertexArray(spider_VAO);
	glDrawArrays(GL_TRIANGLES, spider_vertex_offset[cur_frame_spider], 3 * spider_n_triangles[cur_frame_spider]);
	glBindVertexArray(0);
}

// optimus
GLuint optimus_VBO, optimus_VAO;
int optimus_n_triangles;
GLfloat* optimus_vertices;

Material_Parameters material_optimus;

void prepare_optimus(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, optimus_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/optimus_vnt.geom");
	optimus_n_triangles = read_geometry(&optimus_vertices, n_bytes_per_triangle, filename);
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
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded optimus into graphics memory.\n");
}

void draw_optimus(void) {
	glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 1.0f, 0.0f, 1.0f);

	glBindVertexArray(optimus_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * optimus_n_triangles);
	glBindVertexArray(0);
}

// ironman
GLuint ironman_VBO, ironman_VAO;
int ironman_n_triangles;
GLfloat* ironman_vertices;

Material_Parameters material_ironman;

void prepare_ironman(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, ironman_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/ironman_vnt.geom");
	ironman_n_triangles = read_geometry(&ironman_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	ironman_n_total_triangles += ironman_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &ironman_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glBufferData(GL_ARRAY_BUFFER, ironman_n_total_triangles * 3 * n_bytes_per_vertex, ironman_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(ironman_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &ironman_VAO);
	glBindVertexArray(ironman_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded ironman into graphics memory.\n");
}

void draw_ironman(void) {
	glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 1.0f, 0.0f, 0.0f);

	glBindVertexArray(ironman_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * ironman_n_triangles);
	glBindVertexArray(0);
}

// tank object
GLuint tank_VBO, tank_VAO;
int tank_n_triangles;
GLfloat* tank_vertices;

Material_Parameters material_tank;

void prepare_tank(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tank_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/tank_vnt.geom");
	tank_n_triangles = read_geometry(&tank_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	tank_n_total_triangles += tank_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &tank_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glBufferData(GL_ARRAY_BUFFER, tank_n_total_triangles * 3 * n_bytes_per_vertex, tank_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(tank_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &tank_VAO);
	glBindVertexArray(tank_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded tank into graphics memory.\n");
}

void draw_tank(void) {
	glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 0.0f, 0.0f, 1.0f);

	glBindVertexArray(tank_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * tank_n_triangles);
	glBindVertexArray(0);
}

// dragon object
GLuint dragon_VBO, dragon_VAO;
int dragon_n_triangles;
GLfloat* dragon_vertices;

Material_Parameters material_dragon;

void prepare_dragon(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, dragon_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/dragon_vnt.geom");
	dragon_n_triangles = read_geometry(&dragon_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	dragon_n_total_triangles += dragon_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &dragon_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glBufferData(GL_ARRAY_BUFFER, dragon_n_total_triangles * 3 * n_bytes_per_vertex, dragon_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(dragon_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &dragon_VAO);
	glBindVertexArray(dragon_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded dragon into graphics memory.\n");
}

void draw_dragon(void) {
	glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 0.0f, 0.0f, 0.0f);

	glBindVertexArray(dragon_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * dragon_n_triangles);
	glBindVertexArray(0);
}

// wolf
#define N_WOLF_FRAMES 17
GLuint wolf_VBO, wolf_VAO;
int wolf_n_triangles[N_WOLF_FRAMES];
int wolf_vertex_offset[N_WOLF_FRAMES];
GLfloat* wolf_vertices[N_WOLF_FRAMES];
int cur_frame_wolf = 0, wolf_timer = 0;

Material_Parameters material_wolf;

void prepare_wolf(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, wolf_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_WOLF_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/wolf/wolf_%02d_vnt.geom", i);
		wolf_n_triangles[i] = read_geometry(&wolf_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		wolf_n_total_triangles += wolf_n_triangles[i];

		if (i == 0)
			wolf_vertex_offset[i] = 0;
		else
			wolf_vertex_offset[i] = wolf_vertex_offset[i - 1] + 3 * wolf_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &wolf_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, wolf_VBO);
	glBufferData(GL_ARRAY_BUFFER, wolf_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_WOLF_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, wolf_vertex_offset[i] * n_bytes_per_vertex,
			wolf_n_triangles[i] * n_bytes_per_triangle, wolf_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_WOLF_FRAMES; i++)
		free(wolf_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &wolf_VAO);
	glBindVertexArray(wolf_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, wolf_VBO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded wolf into graphics memory.\n");
}

void draw_wolf(void) {
	glFrontFace(GL_CW);

	glUniform3f(loc_primitive_color, 0.5f, 0.5f, 0.5f);

	glBindVertexArray(wolf_VAO);
	glDrawArrays(GL_TRIANGLES, wolf_vertex_offset[cur_frame_wolf], 3 * wolf_n_triangles[cur_frame_wolf]);
	glBindVertexArray(0);
}

void prepare_objects(void) {
	prepare_tiger();
	prepare_spider();
	prepare_optimus();
	prepare_ironman();
	prepare_tank();
	prepare_dragon();
	prepare_wolf();
}

void draw_objects(void) {
	glUseProgram(h_ShaderProgram_simple);

	ModelMatrix_TIGER = glm::mat4(1.0f);
	if (tiger_timer < 180) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, -tiger_timer * 2.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	// location of tiger
	if (tiger_timer < 180) ModelMatrix_TIGER = glm::translate(ModelMatrix_TIGER, glm::vec3(0.0f, -600.0f, 250.0f));
	else if (tiger_timer < 270) ModelMatrix_TIGER = glm::translate(ModelMatrix_TIGER, glm::vec3(0.0f, -600.0f - ((tiger_timer - 180) * 1300.0f / 90.0f), 250.0f));
	else if (tiger_timer < 300) ModelMatrix_TIGER = glm::translate(ModelMatrix_TIGER, glm::vec3((tiger_timer - 270) * 600.0f / 30.0f, -1900.0f, 250.0f));
	else if (tiger_timer < 315) ModelMatrix_TIGER = glm::translate(ModelMatrix_TIGER, glm::vec3(600.0f, -1900.0f - ((tiger_timer - 300) * 1500.0f / 90.0f), 250.0f));
	else if (tiger_timer < 340) ModelMatrix_TIGER = glm::translate(ModelMatrix_TIGER, glm::vec3(600.0f, -1900.0f - ((tiger_timer - 300) * 1500.0f / 90.0f), 250.0f - ((tiger_timer - 315) * 200.0f / 25.0f)));
	else if (tiger_timer < 390) ModelMatrix_TIGER = glm::translate(ModelMatrix_TIGER, glm::vec3(600.0f, -1900.0f - ((tiger_timer - 300) * 1500.0f / 90.0f), 50.0f));
	else if (tiger_timer < 480) ModelMatrix_TIGER = glm::translate(ModelMatrix_TIGER, glm::vec3(600.0f - ((tiger_timer - 390) * 1200.0f / 90.f), -3400.0f, 50.0f));
	else if (tiger_timer < 530) ModelMatrix_TIGER = glm::translate(ModelMatrix_TIGER, glm::vec3(-600.0f, -3400.0f + ((tiger_timer - 480) * 1500.0f / 90.0f), 50.0f));
	else if (tiger_timer < 555) ModelMatrix_TIGER = glm::translate(ModelMatrix_TIGER, glm::vec3(-600.0f, -3400.0f + ((tiger_timer - 480) * 1500.0f / 90.0f), 50.0f + ((tiger_timer - 530) * 200.0f / 25.0f)));
	else if (tiger_timer < 570) ModelMatrix_TIGER = glm::translate(ModelMatrix_TIGER, glm::vec3(-600.0f, -3400.0f + ((tiger_timer - 480) * 1500.0f / 90.0f), 250.0f));
	else if (tiger_timer < 600) ModelMatrix_TIGER = glm::translate(ModelMatrix_TIGER, glm::vec3(-600.0f + (tiger_timer - 570) * 600.0f / 30.0f, -1900.0f, 250.0f));
	else if (tiger_timer < 690) ModelMatrix_TIGER = glm::translate(ModelMatrix_TIGER, glm::vec3(0.0f, -1900.0f + ((tiger_timer - 600) * 1300.0f / 90.0f), 250.0f));
	// orientation of tiger
	if (tiger_timer < 170) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (tiger_timer < 180) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, -(90.0f - (tiger_timer - 170) * 9.0f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (tiger_timer < 260);
	else if (tiger_timer < 270) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, (tiger_timer - 260) * 9.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (tiger_timer < 290) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (tiger_timer < 300) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, (90.0f - (tiger_timer - 290) * 9.0f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (tiger_timer < 380);
	else if (tiger_timer < 390) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, -(tiger_timer - 380) * 9.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (tiger_timer < 470) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (tiger_timer < 480) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, (-90.0f - (tiger_timer - 470) * 9.0f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (tiger_timer < 560) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (tiger_timer < 570) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, (-180.0f - (tiger_timer - 560) * 9.0f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (tiger_timer < 590) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, -270.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (tiger_timer < 600) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, (-270.0f + (tiger_timer - 590) * 9.0f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (tiger_timer < 680) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (tiger_timer < 690) ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, (-180.0f + (tiger_timer - 680) * 9.0f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_TIGER = glm::scale(ModelMatrix_TIGER, glm::vec3(3.0f, 3.0f, 3.0f));
	ModelViewMatrix = ViewMatrix * ModelMatrix_TIGER;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_tiger();

	// spider moving up-down
	if ((spider_timer % 20) < 10) ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, 0.0f, (spider_timer % 20) * 8));
	else ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, 0.0f, 160 - (spider_timer % 20) * 8));
	// location of spider
	if (spider_timer < 60) ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(250.0f, -4500.0f - (spider_timer * 2300.0f / 60.0f), 300.0f));
	else if (spider_timer < 90) ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(250.0f - ((spider_timer - 60) * 500.0f / 30.0f), -6800.0f, 300.0f));
	else if (spider_timer < 150) ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(-250.0f, -6800.0f + ((spider_timer - 90) * 2300.0f / 60.0f), 300.0f));
	else if (spider_timer < 210) ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(-250.0f, -4500.0f - ((spider_timer - 150) * 2300.0f / 60.0f), 300.0f));
	else if (spider_timer < 240) ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(-250.0f + ((spider_timer - 210) * 500.0f / 30.0f), -6800.0f, 300.0f));
	else if (spider_timer < 300) ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(250.0f, -6800.0f + ((spider_timer - 240) * 2300.0f / 60.0f), 300.0f));
	// orientation of spider
	if (spider_timer < 50);
	else if (spider_timer < 60) ModelViewMatrix = glm::rotate(ModelViewMatrix, -((spider_timer - 50) * 9.0f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (spider_timer < 80) ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (spider_timer < 90) ModelViewMatrix = glm::rotate(ModelViewMatrix, (-90.0f - (spider_timer - 80) * 9.0f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (spider_timer < 140) ModelViewMatrix = glm::rotate(ModelViewMatrix, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (spider_timer < 150) {
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (spider_timer - 140) * 18.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (spider_timer < 200) {
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (spider_timer < 210) {
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (-180.0f - (spider_timer - 200) * 9.0f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (spider_timer < 230) {
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -270.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (spider_timer < 240) {
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (-270.0f - (spider_timer - 230) * 9.0f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (spider_timer < 290) {
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -360.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (spider_timer < 300) {
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f - ((spider_timer - 290) * 18.0f) * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -360.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(150.0f, 150.0f, 150.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_spider();

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-750.0f, -3000.0f,50.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(0.5f,0.5f, 0.5f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_optimus();

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(750.0f, -3100.0f, 50.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_optimus();

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -2400.0f, 50.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(170.0f, 170.0f, 170.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_ironman();

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -3700.0f, 1300.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(60.0f, 60.0f, 60.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_tank();

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-270.0f, -2780.0f, 600.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(8.0f, 8.0f, 8.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_dragon();

	// wolf moving up-down
	if ((wolf_timer % 10) < 5) ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, 0.0f, (wolf_timer % 10) * 20));
	else ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, 0.0f, 200 - (wolf_timer % 10) * 20));
	// location of wolf
	if (wolf_timer < 60) ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(120.0f, -4500.0f-(wolf_timer*2000.0f/60.0f), 20.0f));
	else if (wolf_timer <70) ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(120.0f-((wolf_timer-60)*24.0f), -6500.0f, 20.0f));
	else if (wolf_timer < 130) ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(-120.0f, -6500.0f + ((wolf_timer - 70) * 2000.0f / 60.0f), 20.0f));
	else if (wolf_timer < 140) ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(-120.0f + ((wolf_timer - 130) * 24.0f), -4500, 20.0f));
	// orientation of wolf
	if (wolf_timer < 60);
	else if (wolf_timer < 70) ModelViewMatrix = glm::rotate(ModelViewMatrix, -((wolf_timer - 60) * 18.0f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (wolf_timer < 130) ModelViewMatrix = glm::rotate(ModelViewMatrix, -180.f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	else if (wolf_timer < 140) ModelViewMatrix = glm::rotate(ModelViewMatrix, (-180.0f - (wolf_timer - 130) * 18.0f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(300.0f, 300.0f, 300.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_wolf();

	glUseProgram(0);
}
/*****************************  END: geometry setup *****************************/

/********************  START: callback function definitions *********************/
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw_grid();
	draw_axes();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	draw_sun_temple();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	draw_objects();

	glutSwapBuffers();
}

int tiger_timestamp_scene = 0;

void timer_scene(int timestamp_scene) {
	if (current_camera_index == CAMERA_T) {
		set_ViewMatrix_for_tiger();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	}
	else if(current_camera_index == CAMERA_G) {
		set_ViewMatrix_for_tiger_follower();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	}
	tiger_node_timer = tiger_timestamp_scene % 12;
	cur_frame_tiger = tiger_timestamp_scene % N_TIGER_FRAMES;
	tiger_timer = tiger_timestamp_scene % 690;
	cur_frame_spider = timestamp_scene % N_SPIDER_FRAMES;
	spider_timer = timestamp_scene % 300;
	cur_frame_wolf = timestamp_scene % N_WOLF_FRAMES;
	wolf_timer = timestamp_scene % 140;
	glutPostRedisplay();
	if (tiger_moving) tiger_timestamp_scene = (tiger_timestamp_scene + 1) % INT_MAX;
	glutTimerFunc(100, timer_scene, (timestamp_scene + 1) % INT_MAX);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'f':
		b_draw_grid = b_draw_grid ? false : true;
		glutPostRedisplay();
		break;
	case '0':
		set_current_camera(CAMERA_0);
		glutPostRedisplay();
		break;
	case '1':
		set_current_camera(CAMERA_1);
		glutPostRedisplay();
		break;
	case '2':
		set_current_camera(CAMERA_2);
		glutPostRedisplay();
		break;
	case '3':
		set_current_camera(CAMERA_3);
		glutPostRedisplay();
		break;
	case 'm':
		set_current_camera(CAMERA_M);
		glutPostRedisplay();
		break;
	case 's':
		tiger_moving = 1 - tiger_moving ;
		break;
	case 't':
		set_current_camera(CAMERA_T);
		glutPostRedisplay();
		break;
	case 'g':
		set_current_camera(CAMERA_G);
		glutPostRedisplay();
		break;
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

void special(int key, int x, int y) {
	if (current_camera_index != CAMERA_M) return; // work only in camera_m mode

	switch (key) {
	case GLUT_KEY_LEFT:
		renew_cam_position(-10.0f, X_AXIS);
		break;
	case GLUT_KEY_RIGHT:
		renew_cam_position(10.0f, X_AXIS);
		break;
	case GLUT_KEY_DOWN:
		if (current_camera.move == 2) renew_cam_position(-10.0f, Y_AXIS);
		else renew_cam_position(-10.0f, Z_AXIS);
		break;
	case GLUT_KEY_UP:
		if (current_camera.move == 2) renew_cam_position(10.0f, Y_AXIS);
		else renew_cam_position(10.0f, Z_AXIS);
		break;
	}

	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	glutPostRedisplay();
}

void reshape(int width, int height) {
	float aspect_ratio;

	glViewport(0, 0, width, height);

	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glutPostRedisplay();
}

int prevx, prevy;

void motion(int x, int y) {
	if (!current_camera.move) return;

	if (current_camera.move == 2) renew_cam_orientation_rotation(prevx - x, Z_AXIS);
	else {
		renew_cam_orientation_rotation(prevx - x, Y_AXIS);
		renew_cam_orientation_rotation(prevy - y, X_AXIS);
	}

	prevx = x; prevy = y;

	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
	if (current_camera_index != CAMERA_M) return; // work only in camera_m mode

	if ((button == GLUT_LEFT_BUTTON)) {
		if (state == GLUT_DOWN) {
			current_camera.move = 1;
			prevx = x; prevy = y;
		}
		else if (state == GLUT_UP) current_camera.move = 0;
	}
	if ((button == GLUT_RIGHT_BUTTON)) {
		if (state == GLUT_DOWN) {
			current_camera.move = 2;
			prevx = x; prevy = y;
		}
		else if (state == GLUT_UP) current_camera.move = 0;
	}
}

void wheel(int wheel, int direction, int x, int y) {
	int modifier = glutGetModifiers();
	if (modifier == GLUT_ACTIVE_SHIFT) {
		if (direction > 0) {
			if (current_camera.fovy < 2.0f) current_camera.fovy += 0.01f;
			else printf("end of zoom-out");
		}
		else {
			if (current_camera.fovy > 0.0f) current_camera.fovy -= 0.01f;
			else printf("end of zoom-in");
		}
		set_ProjectionMatrix_from_camera_fov();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		save_current_camera_to_camera_info();
		glutPostRedisplay();
	}
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &grid_VAO);
	glDeleteBuffers(1, &grid_VBO);

	glDeleteVertexArrays(scene.n_materials, sun_temple_VAO);
	glDeleteBuffers(scene.n_materials, sun_temple_VBO);

	glDeleteTextures(scene.n_textures, sun_temple_texture_names);

	free(sun_temple_n_triangles);
	free(sun_temple_vertex_offset);

	free(sun_temple_VAO);
	free(sun_temple_VBO);

	free(sun_temple_texture_names);
	free(flag_texture_mapping);

	glDeleteVertexArrays(1, &tiger_VAO);
	glDeleteBuffers(1, &tiger_VBO);
}
/*********************  END: callback function definitions **********************/

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);
	glutSpecialFunc(special);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMouseWheelFunc(wheel);
	glutTimerFunc(100, timer_scene, 0);
}

void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::mat4(1.0f);
	ProjectionMatrix = glm::mat4(1.0f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	initialize_lights();
	initialize_flags();
}

void prepare_scene(void) {
	prepare_axes();
	prepare_grid();
	prepare_sun_temple();
	prepare_objects();
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
void drawScene(int argc, char* argv[]) {
	char program_name[64] = "Sogang CSE4170 Sun Temple Scene 20180223 Haejin Lim HW3";
	char messages[N_MESSAGE_LINES][256] = { 
		"    - Keys used:",
		"		'f' : draw x, y, z axes and grid",
		"		'1' : set the camera for original view",
		"		'2' : set the camera for bronze statue view",
		"		'3' : set the camera for bronze statue view",	
		"		'4' : set the camera for top view",
		"		'5' : set the camera for front view",
		"		'6' : set the camera for side view",
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
