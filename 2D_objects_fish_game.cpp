#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

														  // include glm/*.hpp only if necessary
														  //#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, ortho, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0

int win_width = 0, win_height = 0;
float centerx = 0.0f, centery = 0.0f, rotate_angle = 0.0f;

GLfloat axes[4][2];
GLfloat axes_color[3] = { 0.0f, 0.0f, 0.0f };
GLuint VBO_axes, VAO_axes;

void prepare_axes(void) { // Draw axes in their MC.
	axes[0][0] = -win_width / 2.5f; axes[0][1] = 0.0f;
	axes[1][0] = win_width / 2.5f; axes[1][1] = 0.0f;
	axes[2][0] = 0.0f; axes[2][1] = -win_height / 2.5f;
	axes[3][0] = 0.0f; axes[3][1] = win_height / 2.5f;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_axes);
	glBindVertexArray(VAO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void update_axes(void) {
	axes[0][0] = -win_width / 2.25f; axes[1][0] = win_width / 2.25f;
	axes[2][1] = -win_height / 2.25f;
	axes[3][1] = win_height / 2.25f;

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_axes(void) {
	glUniform3fv(loc_primitive_color, 1, axes_color);
	glBindVertexArray(VAO_axes);
	glDrawArrays(GL_LINES, 0, 4);
	glBindVertexArray(0);
}

GLfloat line[2][2];
GLfloat line_color[3] = { 1.0f, 0.0f, 0.0f };
GLuint VBO_line, VAO_line;

void prepare_line(void) { 	// y = x - win_height/4
	line[0][0] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height;
	line[0][1] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height - win_height / 4.0f;
	line[1][0] = win_width / 2.5f;
	line[1][1] = win_width / 2.5f - win_height / 4.0f;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_line);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_line);
	glBindVertexArray(VAO_line);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void update_line(void) { 	// y = x - win_height/4
	line[0][0] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height;
	line[0][1] = (1.0f / 4.0f - 1.0f / 2.5f)*win_height - win_height / 4.0f;
	line[1][0] = win_width / 2.5f;
	line[1][1] = win_width / 2.5f - win_height / 4.0f;

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_line(void) { // Draw line in its MC.
					   // y = x - win_height/4
	glUniform3fv(loc_primitive_color, 1, line_color);
	glBindVertexArray(VAO_line);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}

#define AIRPLANE_BIG_WING 0
#define AIRPLANE_SMALL_WING 1
#define AIRPLANE_BODY 2
#define AIRPLANE_BACK 3
#define AIRPLANE_SIDEWINDER1 4
#define AIRPLANE_SIDEWINDER2 5
#define AIRPLANE_CENTER 6
GLfloat big_wing[6][2] = { { 0.0, 0.0 },{ -20.0, 15.0 },{ -20.0, 20.0 },{ 0.0, 23.0 },{ 20.0, 20.0 },{ 20.0, 15.0 } };
GLfloat small_wing[6][2] = { { 0.0, -18.0 },{ -11.0, -12.0 },{ -12.0, -7.0 },{ 0.0, -10.0 },{ 12.0, -7.0 },{ 11.0, -12.0 } };
GLfloat body[5][2] = { { 0.0, -25.0 },{ -6.0, 0.0 },{ -6.0, 22.0 },{ 6.0, 22.0 },{ 6.0, 0.0 } };
GLfloat back[5][2] = { { 0.0, 25.0 },{ -7.0, 24.0 },{ -7.0, 21.0 },{ 7.0, 21.0 },{ 7.0, 24.0 } };
GLfloat sidewinder1[5][2] = { { -20.0, 10.0 },{ -18.0, 3.0 },{ -16.0, 10.0 },{ -18.0, 20.0 },{ -20.0, 20.0 } };
GLfloat sidewinder2[5][2] = { { 20.0, 10.0 },{ 18.0, 3.0 },{ 16.0, 10.0 },{ 18.0, 20.0 },{ 20.0, 20.0 } };
GLfloat center[1][2] = { { 0.0, 0.0 } };
GLfloat airplane_color[7][3] = {
	{ 255 / 255.0f, 102 / 255.0f, 178 / 255.0f },  // big_wing
	{ 245 / 255.0f, 211 / 255.0f,   0 / 255.0f },  // small_wing
	{ 255 / 255.0f,  51 / 255.0f, 153 / 255.0f },  // body
	{ 255 / 255.0f, 102 / 255.0f, 102 / 255.0f },  // back
	{ 245 / 255.0f, 211 / 255.0f,   0 / 255.0f },  // sidewinder1
	{ 245 / 255.0f, 211 / 255.0f,   0 / 255.0f },  // sidewinder2
	{ 255 / 255.0f,   0 / 255.0f,   0 / 255.0f }   // center
};

GLuint VBO_airplane, VAO_airplane;

int airplane_clock = 0;
float airplane_s_factor = 1.0f;

void prepare_airplane() {
	GLsizeiptr buffer_size = sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back)
		+ sizeof(sidewinder1) + sizeof(sidewinder2) + sizeof(center);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_airplane);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_airplane);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(big_wing), big_wing);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing), sizeof(small_wing), small_wing);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing), sizeof(body), body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body), sizeof(back), back);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back),
		sizeof(sidewinder1), sidewinder1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back)
		+ sizeof(sidewinder1), sizeof(sidewinder2), sidewinder2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back)
		+ sizeof(sidewinder1) + sizeof(sidewinder2), sizeof(center), center);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_airplane);
	glBindVertexArray(VAO_airplane);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_airplane);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_airplane() { // Draw airplane in its MC.
	glBindVertexArray(VAO_airplane);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BIG_WING]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SMALL_WING]);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 6);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BACK]);
	glDrawArrays(GL_TRIANGLE_FAN, 17, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SIDEWINDER1]);
	glDrawArrays(GL_TRIANGLE_FAN, 22, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SIDEWINDER2]);
	glDrawArrays(GL_TRIANGLE_FAN, 27, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_CENTER]);
	glPointSize(5.0);
	glDrawArrays(GL_POINTS, 32, 1);
	glPointSize(1.0);
	glBindVertexArray(0);
}

//house
#define HOUSE_ROOF 0
#define HOUSE_BODY 1
#define HOUSE_CHIMNEY 2
#define HOUSE_DOOR 3
#define HOUSE_WINDOW 4

GLfloat roof[3][2] = { { -12.0, 0.0 },{ 0.0, 12.0 },{ 12.0, 0.0 } };
GLfloat house_body[4][2] = { { -12.0, -14.0 },{ -12.0, 0.0 },{ 12.0, 0.0 },{ 12.0, -14.0 } };
GLfloat chimney[4][2] = { { 6.0, 6.0 },{ 6.0, 14.0 },{ 10.0, 14.0 },{ 10.0, 2.0 } };
GLfloat door[4][2] = { { -8.0, -14.0 },{ -8.0, -8.0 },{ -4.0, -8.0 },{ -4.0, -14.0 } };
GLfloat window[4][2] = { { 4.0, -6.0 },{ 4.0, -2.0 },{ 8.0, -2.0 },{ 8.0, -6.0 } };

GLfloat house_color[5][3] = {
	{ 200 / 255.0f, 39 / 255.0f, 42 / 255.0f },
	{ 235 / 255.0f, 225 / 255.0f, 196 / 255.0f },
	{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 233 / 255.0f, 113 / 255.0f, 23 / 255.0f },
	{ 44 / 255.0f, 180 / 255.0f, 49 / 255.0f }
};

GLuint VBO_house, VAO_house;
void prepare_house() {
	GLsizeiptr buffer_size = sizeof(roof) + sizeof(house_body) + sizeof(chimney) + sizeof(door)
		+ sizeof(window);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_house);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(roof), roof);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof), sizeof(house_body), house_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body), sizeof(chimney), chimney);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body) + sizeof(chimney), sizeof(door), door);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body) + sizeof(chimney) + sizeof(door),
		sizeof(window), window);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_house);
	glBindVertexArray(VAO_house);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_house() {
	glBindVertexArray(VAO_house);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_ROOF]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 3, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_CHIMNEY]);
	glDrawArrays(GL_TRIANGLE_FAN, 7, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_DOOR]);
	glDrawArrays(GL_TRIANGLE_FAN, 11, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 15, 4);

	glBindVertexArray(0);
}

//draw cocktail
#define COCKTAIL_NECK 0
#define COCKTAIL_LIQUID 1
#define COCKTAIL_REMAIN 2
#define COCKTAIL_STRAW 3
#define COCKTAIL_DECO 4

GLfloat neck[6][2] = { { -6.0, -12.0 },{ -6.0, -11.0 },{ -1.0, 0.0 },{ 1.0, 0.0 },{ 6.0, -11.0 },{ 6.0, -12.0 } };
GLfloat liquid[6][2] = { { -1.0, 0.0 },{ -9.0, 4.0 },{ -12.0, 7.0 },{ 12.0, 7.0 },{ 9.0, 4.0 },{ 1.0, 0.0 } };
GLfloat remain[4][2] = { { -12.0, 7.0 },{ -12.0, 10.0 },{ 12.0, 10.0 },{ 12.0, 7.0 } };
GLfloat straw[4][2] = { { 7.0, 7.0 },{ 12.0, 12.0 },{ 14.0, 12.0 },{ 9.0, 7.0 } };
GLfloat deco[8][2] = { { 12.0, 12.0 },{ 10.0, 14.0 },{ 10.0, 16.0 },{ 12.0, 18.0 },{ 14.0, 18.0 },{ 16.0, 16.0 },{ 16.0, 14.0 },{ 14.0, 12.0 } };

GLfloat cocktail_color[5][3] = {
	{ 235 / 255.0f, 225 / 255.0f, 196 / 255.0f },
	{ 0 / 255.0f, 63 / 255.0f, 122 / 255.0f },
	{ 235 / 255.0f, 225 / 255.0f, 196 / 255.0f },
	{ 191 / 255.0f, 255 / 255.0f, 0 / 255.0f },
	{ 218 / 255.0f, 165 / 255.0f, 32 / 255.0f }
};

GLuint VBO_cocktail, VAO_cocktail;
void prepare_cocktail() {
	GLsizeiptr buffer_size = sizeof(neck) + sizeof(liquid) + sizeof(remain) + sizeof(straw)
		+ sizeof(deco);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_cocktail);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cocktail);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(neck), neck);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(neck), sizeof(liquid), liquid);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(neck) + sizeof(liquid), sizeof(remain), remain);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(neck) + sizeof(liquid) + sizeof(remain), sizeof(straw), straw);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(neck) + sizeof(liquid) + sizeof(remain) + sizeof(straw),
		sizeof(deco), deco);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_cocktail);
	glBindVertexArray(VAO_cocktail);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cocktail);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_cocktail() {
	glBindVertexArray(VAO_cocktail);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_NECK]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_LIQUID]);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 6);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_REMAIN]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_STRAW]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glUniform3fv(loc_primitive_color, 1, cocktail_color[COCKTAIL_DECO]);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 8);

	glBindVertexArray(0);
}

//draw car2
#define CAR2_BODY 0
#define CAR2_FRONT_WINDOW 1
#define CAR2_BACK_WINDOW 2
#define CAR2_FRONT_WHEEL 3
#define CAR2_BACK_WHEEL 4
#define CAR2_LIGHT1 5
#define CAR2_LIGHT2 6

GLfloat car2_body[8][2] = { { -18.0, -7.0 },{ -18.0, 0.0 },{ -13.0, 0.0 },{ -10.0, 8.0 },{ 10.0, 8.0 },{ 13.0, 0.0 },{ 18.0, 0.0 },{ 18.0, -7.0 } };
GLfloat car2_front_window[4][2] = { { -10.0, 0.0 },{ -8.0, 6.0 },{ -2.0, 6.0 },{ -2.0, 0.0 } };
GLfloat car2_back_window[4][2] = { { 0.0, 0.0 },{ 0.0, 6.0 },{ 8.0, 6.0 },{ 10.0, 0.0 } };
GLfloat car2_front_wheel[8][2] = { { -11.0, -11.0 },{ -13.0, -8.0 },{ -13.0, -7.0 },{ -11.0, -4.0 },{ -7.0, -4.0 },{ -5.0, -7.0 },{ -5.0, -8.0 },{ -7.0, -11.0 } };
GLfloat car2_back_wheel[8][2] = { { 7.0, -11.0 },{ 5.0, -8.0 },{ 5.0, -7.0 },{ 7.0, -4.0 },{ 11.0, -4.0 },{ 13.0, -7.0 },{ 13.0, -8.0 },{ 11.0, -11.0 } };
GLfloat car2_light1[3][2] = { { -18.0, -1.0 },{ -17.0, -2.0 },{ -18.0, -3.0 } };
GLfloat car2_light2[3][2] = { { -18.0, -4.0 },{ -17.0, -5.0 },{ -18.0, -6.0 } };

GLfloat car2_color[7][3] = {
	{ 100 / 255.0f, 141 / 255.0f, 159 / 255.0f },
	{ 235 / 255.0f, 219 / 255.0f, 208 / 255.0f },
	{ 235 / 255.0f, 219 / 255.0f, 208 / 255.0f },
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 249 / 255.0f, 244 / 255.0f, 0 / 255.0f },
	{ 249 / 255.0f, 244 / 255.0f, 0 / 255.0f }
};

GLuint VBO_car2, VAO_car2;
void prepare_car2() {
	GLsizeiptr buffer_size = sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel)
		+ sizeof(car2_back_wheel) + sizeof(car2_light1) + sizeof(car2_light2);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_car2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car2);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(car2_body), car2_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body), sizeof(car2_front_window), car2_front_window);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window), sizeof(car2_back_window), car2_back_window);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window), sizeof(car2_front_wheel), car2_front_wheel);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel),
		sizeof(car2_back_wheel), car2_back_wheel);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel)
		+ sizeof(car2_back_wheel), sizeof(car2_light1), car2_light1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel)
		+ sizeof(car2_back_wheel) + sizeof(car2_light1), sizeof(car2_light2), car2_light2);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_car2);
	glBindVertexArray(VAO_car2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car2);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_car2() {
	glBindVertexArray(VAO_car2);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 8);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_FRONT_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_BACK_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_FRONT_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 8);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_BACK_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 24, 8);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_LIGHT1]);
	glDrawArrays(GL_TRIANGLE_FAN, 32, 3);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_LIGHT2]);
	glDrawArrays(GL_TRIANGLE_FAN, 35, 3);

	glBindVertexArray(0);
}

// sword

#define SWORD_BODY 0
#define SWORD_BODY2 1
#define SWORD_HEAD 2
#define SWORD_HEAD2 3
#define SWORD_IN 4
#define SWORD_DOWN 5
#define SWORD_BODY_IN 6

GLfloat sword_body[4][2] = { { -6.0, 0.0 },{ -6.0, -4.0 },{ 6.0, -4.0 },{ 6.0, 0.0 } };
GLfloat sword_body2[4][2] = { { -2.0, -4.0 },{ -2.0, -6.0 } ,{ 2.0, -6.0 },{ 2.0, -4.0 } };
GLfloat sword_head[4][2] = { { -2.0, 0.0 },{ -2.0, 16.0 } ,{ 2.0, 16.0 },{ 2.0, 0.0 } };
GLfloat sword_head2[3][2] = { { -2.0, 16.0 },{ 0.0, 19.46 } ,{ 2.0, 16.0 } };
GLfloat sword_in[4][2] = { { -0.3, 0.7 },{ -0.3, 15.3 } ,{ 0.3, 15.3 },{ 0.3, 0.7 } };
GLfloat sword_down[4][2] = { { -2.0, -6.0 } ,{ 2.0, -6.0 },{ 4.0, -8.0 },{ -4.0, -8.0 } };
GLfloat sword_body_in[4][2] = { { 0.0, -1.0 } ,{ 1.0, -2.732 },{ 0.0, -4.464 },{ -1.0, -2.732 } };

GLfloat sword_color[7][3] = {
	{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 155 / 255.0f, 155 / 255.0f, 155 / 255.0f },
{ 155 / 255.0f, 155 / 255.0f, 155 / 255.0f },
{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f }
}; 

GLuint VBO_sword, VAO_sword;

void prepare_sword() {
	GLsizeiptr buffer_size = sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in) + sizeof(sword_down) + sizeof(sword_body_in);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_sword);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sword_body), sword_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body), sizeof(sword_body2), sword_body2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2), sizeof(sword_head), sword_head);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head), sizeof(sword_head2), sword_head2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2), sizeof(sword_in), sword_in);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in), sizeof(sword_down), sword_down);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in) + sizeof(sword_down), sizeof(sword_body_in), sword_body_in);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_sword);
	glBindVertexArray(VAO_sword);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_sword() {
	glBindVertexArray(VAO_sword);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY2]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_HEAD]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_HEAD2]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 3);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_IN]);
	glDrawArrays(GL_TRIANGLE_FAN, 15, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_DOWN]);
	glDrawArrays(GL_TRIANGLE_FAN, 19, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY_IN]);
	glDrawArrays(GL_TRIANGLE_FAN, 23, 4);

	glBindVertexArray(0);
}

// fish(custom)

#define FISH_BODY1 0
#define FISH_BODY2 1
#define FISH_BODY3 2
#define FISH_FIN 3
#define FISH_TAIL1 4
#define FISH_TAIL2 5
#define FISH_EYE 6


GLfloat fish_body1[16][2] = { {-10.0,0.0},{-9.0,2.0},{-7.0,4.0},{-5.0,5.0},{-3.0,6.0},{0.0,7.0},{3.0,6.0},{6.0,4.0},{10.0,0.0},{6.0,-4.0},{3.0,-6.0},{0.0,-7.0},{-3.0,-6.0},{-5.0,-5.0},{-7.0,-4.0},{-9.0,-2.0} };
GLfloat fish_body2[4][2] = { {-5.0,5.0},{-5.0,-5.0},{-3.0,-6.0},{-3.0,6.0} };
GLfloat fish_body3[4][2] = { {3.0,6.0},{3.0,-6.0},{6.0,-4.0},{6.0,4.0} };
GLfloat fish_fin[4][2] = { {-2.0,2.0},{-2.0,-2.0},{1.0,-4.0},{1.0,4.0} };
GLfloat fish_tail1[3][2] = { {10.0,0.0},{12.0,3.0},{12.0,-3.0} };
GLfloat fish_tail2[4][2] = { {12.0,3.0},{12.0,-3.0},{14.0,-7.0},{14.0,7.0} };
GLfloat fish_eye[4][2] = { {-6.0,1.0},{-6.0,2.0},{-7,2.0},{-7.0,1.0} };

GLfloat fish_color[7][3] = {
{ 255 / 255.0f, 128 / 255.0f, 0 / 255.0f },
{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f },
{ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f },
{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
{ 255 / 255.0f, 128 / 255.0f, 0 / 255.0f },
{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
};

GLuint VBO_fish, VAO_fish;

void prepare_fish() {
	GLsizeiptr buffer_size = sizeof(fish_body1) + sizeof(fish_body2) + sizeof(fish_body3) + sizeof(fish_fin) + sizeof(fish_tail1) + sizeof(fish_tail2) + sizeof(fish_eye);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_fish);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fish);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fish_body1), fish_body1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fish_body1), sizeof(fish_body2), fish_body2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fish_body1) + sizeof(fish_body2), sizeof(fish_body3), fish_body3);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fish_body1) + sizeof(fish_body2) + sizeof(fish_body3), sizeof(fish_fin), fish_fin);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fish_body1) + sizeof(fish_body2) + sizeof(fish_body3) + sizeof(fish_fin), sizeof(fish_tail1), fish_tail1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fish_body1) + sizeof(fish_body2) + sizeof(fish_body3) + sizeof(fish_fin) + sizeof(fish_tail1), sizeof(fish_tail2), fish_tail2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fish_body1) + sizeof(fish_body2) + sizeof(fish_body3) + sizeof(fish_fin) + sizeof(fish_tail1) + sizeof(fish_tail2), sizeof(fish_eye), fish_eye);


	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_fish);
	glBindVertexArray(VAO_fish);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_fish);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_fish() {
	glBindVertexArray(VAO_fish);

	glUniform3fv(loc_primitive_color, 1, fish_color[FISH_BODY1]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 16);

	glUniform3fv(loc_primitive_color, 1, fish_color[FISH_BODY2]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glUniform3fv(loc_primitive_color, 1, fish_color[FISH_BODY3]);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	glUniform3fv(loc_primitive_color, 1, fish_color[FISH_FIN]);
	glDrawArrays(GL_TRIANGLE_FAN, 24, 4);

	glUniform3fv(loc_primitive_color, 1, fish_color[FISH_TAIL1]);
	glDrawArrays(GL_TRIANGLE_FAN, 28, 3);

	glUniform3fv(loc_primitive_color, 1, fish_color[FISH_TAIL2]);
	glDrawArrays(GL_TRIANGLE_FAN, 31, 4);

	glUniform3fv(loc_primitive_color, 1, fish_color[FISH_EYE]);
	glDrawArrays(GL_TRIANGLE_FAN, 35, 4);

	glBindVertexArray(0);
}

// hook

#define HOOK_BODY1 0
#define HOOK_BODY2 1
#define HOOK_BODY3 2
#define HOOK_ROPE 3

GLfloat hook_body1[5][2] = { {-5.0,6.0},{-3.0,3.0},{-3.0,-8.0},{-7.0,-5.0},{-7.0,3.0} };
GLfloat hook_body2[4][2] = { {-3.0,-5.0},{-3.0,-8.0},{3.0,-8.0},{3.0,-5.0} };
GLfloat hook_body3[4][2] = { {3.0,-8.0},{7.0,-5.0},{7.0,9.0},{3.0,9.0} };
GLfloat hook_rope[4][2] = { {4.0,9.0},{6.0,9.0},{6.0,300.0},{4.0,300.0} };

GLfloat hook_color[4][3] = {
{ 128 / 255.0f, 128 / 255.0f, 128 / 255.0f },
{ 128 / 255.0f, 128 / 255.0f, 128 / 255.0f },
{ 128 / 255.0f, 128 / 255.0f, 128 / 255.0f },
{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f }
};

GLuint VBO_hook, VAO_hook;

void prepare_hook() {
	GLsizeiptr buffer_size = sizeof(hook_body1) + sizeof(hook_body2) + sizeof(hook_body3) + sizeof(hook_rope);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_hook);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_hook);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(hook_body1), hook_body1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(hook_body1), sizeof(hook_body2), hook_body2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(hook_body1) + sizeof(hook_body2), sizeof(hook_body3), hook_body3);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(hook_body1) + sizeof(hook_body2) + sizeof(hook_body3), sizeof(hook_rope), hook_rope);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_hook);
	glBindVertexArray(VAO_hook);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_hook);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_hook() {
	glBindVertexArray(VAO_hook);

	glUniform3fv(loc_primitive_color, 1, hook_color[HOOK_BODY1]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 5);


	glUniform3fv(loc_primitive_color, 1, hook_color[HOOK_BODY2]);
	glDrawArrays(GL_TRIANGLE_FAN, 5, 4);


	glUniform3fv(loc_primitive_color, 1, hook_color[HOOK_BODY3]);
	glDrawArrays(GL_TRIANGLE_FAN, 9, 4);

	glUniform3fv(loc_primitive_color, 1, hook_color[HOOK_ROPE]);
	glDrawArrays(GL_TRIANGLE_FAN, 13, 4);

	glBindVertexArray(0);
}

#define AIRPLANE_ROTATION_RADIUS 100.0f
#define CAR_ROTATION_RADIUS 200.0f
#define COCKTAIL_ROTATION_RADIUS 70.0f
int cocktail_clock = 0;
float hook_location[4][2] = { {-400.0f, 0.0f}, {-200.0f, 0.0f}, {200.0f, 0.0f}, {400.0f, 0.0f} };
float fish_location[2] = { 0.0f, 0.0f };
int fish_left = 1, fish_caught = -1;
int animation_mode = 1;
unsigned int timestamp = 0;

void timer(int value) {
	timestamp = (timestamp + 1) % UINT_MAX;
	glutPostRedisplay();
	if (animation_mode)
		glutTimerFunc(10, timer, 0);
}

void check_if_fish_is_caught() {
	float fish_mouth_x = fish_location[0] - 50.0f, fish_mouth_y = fish_location[1], hook_x, hook_y;
	if (fish_left == 0) fish_mouth_x = fish_location[0] + 50.0f;

	for (int i = 0; i < 4; i++) {
		hook_x = hook_location[i][0]; hook_y = hook_location[i][1];
		// printf("%f, %f, %f, %f, for %d \n", fish_mouth_x, fish_mouth_y, hook_x, hook_y, i);
		if (fish_mouth_x >= (hook_x - 45.0f) && fish_mouth_x <= (hook_x - 10.0f) && fish_mouth_y <= (hook_y + 35.0f) && fish_mouth_y >= (hook_y)) {
			fish_caught = i;
			break;
		}
	}
}

void display(void) {
	glm::mat4 ModelMatrix;

	glClear(GL_COLOR_BUFFER_BIT);

	ModelMatrix = glm::mat4(1.0f);
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_axes();

	//car2 start

	int car_clock = timestamp % 360; // 0 <= car_clock <= 359
	ModelMatrix = glm::rotate(glm::mat4(1.0f), car_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(CAR_ROTATION_RADIUS, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelMatrix = glm::rotate(ModelMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car2();

	ModelMatrix = glm::rotate(glm::mat4(1.0f), -car_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-CAR_ROTATION_RADIUS, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelMatrix = glm::rotate(ModelMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car2();

	//car2 end

	//airplane start

	int airplane_clock = car_clock; // 0 <= clock <= 359
	float center_x = cos(airplane_clock * TO_RADIAN) * CAR_ROTATION_RADIUS; //calculate car location
	float center_y = sin(airplane_clock * TO_RADIAN) * CAR_ROTATION_RADIUS;
	float airplane_rotation_factor = (airplane_clock %120) * 360.0f / 120.f;

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(center_x, center_y, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -airplane_rotation_factor * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-AIRPLANE_ROTATION_RADIUS, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_airplane();

	if (airplane_clock <= 180) airplane_clock = 180 - airplane_clock;
	else airplane_clock = 540 - airplane_clock;
	center_x = cos(airplane_clock * TO_RADIAN) * CAR_ROTATION_RADIUS; //calculate 2nd car location
	center_y = sin(airplane_clock * TO_RADIAN) * CAR_ROTATION_RADIUS;
	airplane_rotation_factor = (airplane_clock % 120) * 360.0f / 120.f;
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(center_x, center_y, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -airplane_rotation_factor * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-AIRPLANE_ROTATION_RADIUS, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_airplane();

	// airplane end

	//house start

	int house_clock = timestamp % 360; // 0 <= house_clock <= 359
	float house_x = 200.0f + 10.0f * 16.0f * sin(house_clock * TO_RADIAN) * sin(house_clock * TO_RADIAN) * sin(house_clock * TO_RADIAN);
	float house_y = 10.0f * (13.0f * cos(house_clock * TO_RADIAN) - 5.0f * cos(house_clock * TO_RADIAN * 2.0f) - 2.0f * cos(house_clock * TO_RADIAN * 3.0f) - cos(house_clock * TO_RADIAN * 4.0f));
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(house_x, house_y, 0.0f));
	float house_scale_factor;
	if (house_clock <= 180) house_scale_factor = 2.0f + 1.5 * house_clock / 180.0f;
	else house_scale_factor = 2.0f + 1.5f * (360.0-house_clock) / 180.0f;
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(house_scale_factor, house_scale_factor, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_house();

	house_clock = 360 - house_clock;
	house_x = 200.0f + 10.0f * 16.0f * sin(house_clock * TO_RADIAN) * sin(house_clock * TO_RADIAN) * sin(house_clock * TO_RADIAN);
	house_y = 10.0f * (13.0f * cos(house_clock * TO_RADIAN) - 5.0f * cos(house_clock * TO_RADIAN * 2.0f) - 2.0f * cos(house_clock * TO_RADIAN * 3.0f) - cos(house_clock * TO_RADIAN * 4.0f));
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(house_x, house_y, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(house_scale_factor, house_scale_factor, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_house();

	house_x = -200.0f + 10.0f * 16.0f * sin(house_clock * TO_RADIAN) * sin(house_clock * TO_RADIAN) * sin(house_clock * TO_RADIAN);
	house_y = 10.0f * (13.0f * cos(house_clock * TO_RADIAN) - 5.0f * cos(house_clock * TO_RADIAN * 2.0f) - 2.0f * cos(house_clock * TO_RADIAN * 3.0f) - cos(house_clock * TO_RADIAN * 4.0f));
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(house_x, house_y, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(house_scale_factor, house_scale_factor, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_house();

	house_clock = 360 - house_clock;
	house_x = -200.0f + 10.0f * 16.0f * sin(house_clock * TO_RADIAN) * sin(house_clock * TO_RADIAN) * sin(house_clock * TO_RADIAN);
	house_y = 10.0f * (13.0f * cos(house_clock * TO_RADIAN) - 5.0f * cos(house_clock * TO_RADIAN * 2.0f) - 2.0f * cos(house_clock * TO_RADIAN * 3.0f) - cos(house_clock * TO_RADIAN * 4.0f));
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(house_x, house_y, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(house_scale_factor, house_scale_factor, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_house();

	// house end

	// cocktail start

	cocktail_clock++;
	if (cocktail_clock >= 1080) cocktail_clock = 0;
	float cocktail_translate_factor, cocktail_scale_factor, cocktail_rotation_direction = 1.0f;
	if (cocktail_clock <= 180) { cocktail_translate_factor = 70.0f; cocktail_rotation_direction = -1.0; }
	else if(cocktail_clock <= 360) { cocktail_translate_factor = 210.0f; cocktail_rotation_direction = 1.0;}
	else if (cocktail_clock <= 540) { cocktail_translate_factor = 350.0f; cocktail_rotation_direction = -1.0; }
	else if (cocktail_clock <= 720) { cocktail_translate_factor = 350.0f; cocktail_rotation_direction = -1.0; }
	else if (cocktail_clock <= 900) { cocktail_translate_factor = 210.0f; cocktail_rotation_direction = 1.0; }
	else if (cocktail_clock <= 1080) { cocktail_translate_factor = 70.0f; cocktail_rotation_direction = -1.0; }

	if (cocktail_clock <= 540) cocktail_scale_factor = 1.0f + cocktail_clock * 2.0f / 540.0f;
	else cocktail_scale_factor = 1.0f + (1080.0f-cocktail_clock) * 2.0f / 540.0f;

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(cocktail_translate_factor, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, cocktail_rotation_direction * cocktail_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(cocktail_rotation_direction * COCKTAIL_ROTATION_RADIUS, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(cocktail_scale_factor, cocktail_scale_factor, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_cocktail();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-cocktail_translate_factor, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -cocktail_rotation_direction * cocktail_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-cocktail_rotation_direction * COCKTAIL_ROTATION_RADIUS, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(cocktail_scale_factor, cocktail_scale_factor, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_cocktail();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(cocktail_translate_factor, 200.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, cocktail_rotation_direction * cocktail_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(cocktail_rotation_direction * COCKTAIL_ROTATION_RADIUS, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(cocktail_scale_factor, cocktail_scale_factor, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_cocktail();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-cocktail_translate_factor, 200.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -cocktail_rotation_direction * cocktail_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-cocktail_rotation_direction * COCKTAIL_ROTATION_RADIUS, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(cocktail_scale_factor, cocktail_scale_factor, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_cocktail();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(cocktail_translate_factor, -200.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, cocktail_rotation_direction * cocktail_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(cocktail_rotation_direction * COCKTAIL_ROTATION_RADIUS, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(cocktail_scale_factor, cocktail_scale_factor, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_cocktail();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-cocktail_translate_factor, -200.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -cocktail_rotation_direction * cocktail_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-cocktail_rotation_direction * COCKTAIL_ROTATION_RADIUS, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(cocktail_scale_factor, cocktail_scale_factor, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_cocktail();

	//cocktail end

	//sword start

	float sword_translate_factor_x, sword_translate_factor_y, sword_rotation_factor;
	int sword_clock = timestamp % 360; // 0 <= sword_clock <= 359
	if (sword_clock <= 90) {
		sword_translate_factor_x = 300.0f * -sword_clock / 90.0f + 300.0f; 
		sword_translate_factor_y = -sword_translate_factor_x + 300;
	}
	else if (sword_clock <= 180) {
		sword_translate_factor_x = 300.0f * -(sword_clock-90.0f) / 90.0f;
		sword_translate_factor_y = sword_translate_factor_x + 300;
	}
	else if (sword_clock <= 270) {
		sword_translate_factor_x = 300.0f * (sword_clock - 180.0f) / 90.0f - 300.f;
		sword_translate_factor_y = -sword_translate_factor_x - 300;
	}
	else {
		sword_translate_factor_x = 300.0f * (sword_clock - 270.0f) / 90.0f;
		sword_translate_factor_y = sword_translate_factor_x - 300;
	}
	sword_rotation_factor = (sword_clock % 30) * 360.0f / 30.0f;

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(sword_translate_factor_x, sword_translate_factor_y, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, sword_rotation_factor * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.5f, 3.5f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_sword();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-sword_translate_factor_x, -sword_translate_factor_y, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, sword_rotation_factor * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.5f, 3.5f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_sword();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(sword_translate_factor_y, sword_translate_factor_x, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, sword_rotation_factor * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.5f, 3.5f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_sword();

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-sword_translate_factor_y, -sword_translate_factor_x, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, sword_rotation_factor * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 10.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.5f, 3.5f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_sword();

	//sword end

	//hopk start

	int hook_clock = timestamp % 360; // 0 <= hook_clock <= 359
	if (hook_clock <= 90) hook_location[0][1] = 300.0f * -hook_clock / 90.0f;
	else if (hook_clock <= 180) hook_location[0][1] = 300.0f * (hook_clock - 90.0f) / 90.0f - 300.0f;
	else if (hook_clock <= 270) hook_location[0][1] = 300.0f * (hook_clock - 180.0f) / 90.0f;
	else hook_location[0][1] = 300.0f * -(hook_clock - 270.0f) / 90.0f + 300.0f;
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(hook_location[0][0], hook_location[0][1], 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_hook();

	hook_clock = (hook_clock + 50) % 360;
	if (hook_clock <= 90) hook_location[1][1] = 300.0f * hook_clock / 90.0f;
	else if (hook_clock <= 180) hook_location[1][1] = 300.0f * -(hook_clock - 90.0f) / 90.0f + 300.0f;
	else if (hook_clock <= 270) hook_location[1][1] = 300.0f * -(hook_clock - 180.0f) / 90.0f;
	else hook_location[1][1] = 300.0f * (hook_clock - 270.0f) / 90.0f - 300.0f;
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(hook_location[1][0], hook_location[1][1], 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_hook();

	hook_clock = (hook_clock + 110) % 360;
	if (hook_clock <= 90) hook_location[2][1] = 300.0f * -hook_clock / 90.0f;
	else if (hook_clock <= 180) hook_location[2][1] = 300.0f * (hook_clock - 90.0f) / 90.0f - 300.0f;
	else if (hook_clock <= 270) hook_location[2][1] = 300.0f * (hook_clock - 180.0f) / 90.0f;
	else hook_location[2][1] = 300.0f * -(hook_clock - 270.0f) / 90.0f + 300.0f;
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(hook_location[2][0], hook_location[2][1], 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_hook();

	hook_clock = (hook_clock + 250) % 360;
	if (hook_clock <= 90) hook_location[3][1] = 300.0f * hook_clock / 90.0f;
	else if (hook_clock <= 180) hook_location[3][1] = 300.0f * -(hook_clock - 90.0f) / 90.0f + 300.0f;
	else if (hook_clock <= 270) hook_location[3][1] = 300.0f * -(hook_clock - 180.0f) / 90.0f;
	else hook_location[3][1] = 300.0f * (hook_clock - 270.0f) / 90.0f - 300.0f;
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(hook_location[3][0], hook_location[3][1], 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_hook();

	//hook end

	//fish start

	int fish_clock = timestamp % 360;
	float fish_rotation_factor = 0.0f;
	if (fish_caught == -1) check_if_fish_is_caught();
	else {
		fish_location[0] = hook_location[fish_caught][0]; fish_location[1] = hook_location[fish_caught][1];
		fish_rotation_factor = (fish_clock % 60) * 360.0f / 60.0f;
	}
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(fish_location[0], fish_location[1], 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, fish_rotation_factor * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 1.0f));
	if (fish_left ==0) ModelMatrix = glm::scale(ModelMatrix, glm::vec3(-1.0f, 1.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_fish();

	//fish end

	glFlush();
}

void reset_fish() {
	fish_location[0] = 0.0f; fish_location[1] = 0.0f;
	fish_left = 1; fish_caught = -1;
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 'r':
		reset_fish();
		break;
	}
}

void move_fish(float x, float y) {
	if ((fish_location[0] + x) >= (win_width/2) - 50.0f || (fish_location[0] + x) <= -(win_width / 2) + 50.0f || (fish_location[1] + y) <= -(win_height / 2) + 50.0f || (fish_location[1] + y) >= (win_height / 2) - 50.0f) return;
	fish_location[0] += x;
	fish_location[1] += y;
}

void special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		if (fish_caught != -1) break; // do not move if fish is caught
		fish_left = 1;
		move_fish(-10.00f, 0.0f);
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		if (fish_caught != -1) break;
		fish_left = 0;
		move_fish(10.00f, 0.0f);
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		if (fish_caught != -1) break;
		move_fish(0.0f, -10.00f);
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP:
		if (fish_caught != -1) break;
		move_fish(0.0f, 10.00f);
		glutPostRedisplay();
		break;
	}
}

void reshape(int width, int height) {
	win_width = width, win_height = height;

	glViewport(0, 0, win_width, win_height);
	ProjectionMatrix = glm::ortho(-win_width / 2.0, win_width / 2.0,
		-win_height / 2.0, win_height / 2.0, -1000.0, 1000.0);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	update_axes();
	update_line();

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &VAO_axes);
	glDeleteBuffers(1, &VBO_axes);

	glDeleteVertexArrays(1, &VAO_line);
	glDeleteBuffers(1, &VBO_line);

	glDeleteVertexArrays(1, &VAO_airplane);
	glDeleteBuffers(1, &VBO_airplane);

	glDeleteVertexArrays(1, &VAO_car2);
	glDeleteBuffers(1, &VBO_car2);

	glDeleteVertexArrays(1, &VAO_house);
	glDeleteBuffers(1, &VBO_house);

	glDeleteVertexArrays(1, &VAO_cocktail);
	glDeleteBuffers(1, &VBO_cocktail);

	glDeleteVertexArrays(1, &VAO_sword);
	glDeleteBuffers(1, &VBO_sword);

	glDeleteVertexArrays(1, &VAO_fish);
	glDeleteBuffers(1, &VBO_fish);

	glDeleteVertexArrays(1, &VAO_hook);
	glDeleteBuffers(1, &VBO_hook);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);
	glutSpecialFunc(special);
	glutTimerFunc(10, timer, 0); // animation_mode = 1
}

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}

void initialize_OpenGL(void) {
	glEnable(GL_MULTISAMPLE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(64 / 255.0f, 91 / 255.0f, 196 / 255.0f, 1.0f);
	ViewMatrix = glm::mat4(1.0f);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_line();
	prepare_airplane();
	prepare_house();
	prepare_cocktail();
	prepare_car2();
	prepare_sword();
	prepare_fish();
	prepare_hook();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 HW2\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 Homework2 20180223 Haejin Lim";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'ESC' "
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize(1000, 700);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}


