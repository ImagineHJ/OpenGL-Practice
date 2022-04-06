#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>

#define PI 3.141592
#define TIMER_INTERVAL 100

float r = 114.0f / 255.0f, g = 250.0f / 255.0f, b = 114.0f / 255.0f; // Background color
int window_width = 800; int window_height = 600; // initial size of the window
int right_button_pressed = 0; float right_button_pressed_x = 0.0f; float right_button_pressed_y = 0.0f;

float connected_dots[200][2]; // dots
int current_dot_idx = -1;
int is_polygon = 0;

int rotation_mode = 0;
float center_of_gravity_x = 0.0f;
float center_of_gravity_y = 0.0f;

float transform_x_to_vertex(int x) {
	// calculate x coordinate to program size
	float half_width = window_width / 2.0f;
	return (x - half_width) / half_width;
}

float transform_y_to_vertex(int y) {
	// calculate y coordinate to program size
	float half_height = window_height / 2.0f;
	return -(y - half_height) / half_height;
}

void move_polygon(float x_move, float y_move) {
	for (int idx = 0; idx <= current_dot_idx; idx++) {
		connected_dots[idx][0] += x_move;
		connected_dots[idx][1] += y_move;
	}
}

void roatate_polygon_counterclockwise(int degree) {
	float x, y, new_x, new_y;
	for (int idx = 0; idx <= current_dot_idx; idx++) {
		x = connected_dots[idx][0]; y = connected_dots[idx][1];

		x -= center_of_gravity_x; y -= center_of_gravity_y;
		new_x = x * cos(degree * (PI / 180)) - y * sin(degree * (PI / 180));
		new_y = x * sin(degree * (PI / 180)) + y * cos(degree * (PI / 180));
		new_x += center_of_gravity_x; new_y += center_of_gravity_y;

		connected_dots[idx][0] = new_x; connected_dots[idx][1] = new_y;
	}
}

void rotation_timer(int value) {
	roatate_polygon_counterclockwise(10);
	glutPostRedisplay();
	if (rotation_mode)
		glutTimerFunc(TIMER_INTERVAL, rotation_timer, 0);
}

void display(void) {
	glClearColor(r, g, b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glColor3f(0.0f, 0.0f, 0.0f);
	for (int idx = 0; idx <= current_dot_idx; idx++) {
		glVertex2f(connected_dots[idx][0],connected_dots[idx][1]); // draw dots
	}
	glColor3f(0.0f, 0.0f, 1.0f);
	if(rotation_mode==1) glVertex2f(center_of_gravity_x, center_of_gravity_y); // draw center of gravity
	glEnd();

	glColor3f(1.0f, 0.0f, 0.0f);
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	for (int idx = 1; idx <= current_dot_idx; idx++) {
		glVertex2f(connected_dots[idx-1][0], connected_dots[idx-1][1]); // draw lines
		glVertex2f(connected_dots[idx][0], connected_dots[idx][1]);
	}
	if (is_polygon == 1) {
		glVertex2f(connected_dots[0][0], connected_dots[0][1]); // connect first, last dots to make it polygon
		glVertex2f(connected_dots[current_dot_idx][0], connected_dots[current_dot_idx][1]);
	}
	glEnd();

	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'p':
		if (is_polygon) break; // ignore action if polygon is already made
		if (current_dot_idx >= 2) { // only make polygon when 3 or more dots are drawn
			is_polygon = 1;
			fprintf(stdout, "$$$ Created polygon with %d dots/lines \n", current_dot_idx+1);
			glutPostRedisplay();
		}
		else { // if less than 2 dots are drawn, print error msg
			fprintf(stdout, "@@@ ERROR : Choose at least three points! \n");
		}
		break;
	case 'c':
		if (rotation_mode == 1 || current_dot_idx == -1) break; // ignore action if rotation mode is on
		fprintf(stdout, "$$$ Deleted %d dots \n", current_dot_idx + 1);
		current_dot_idx = -1; is_polygon = 0; // remove dots
		glutPostRedisplay();
		break;
	case 'r':
		if (is_polygon == 0) break; // ignore action if polygon is not made
		rotation_mode = 1 - rotation_mode;
		if (rotation_mode==1) {
			// calculate center of gravity
			float x_sum = 0, y_sum = 0;
			for (int idx = 0; idx <= current_dot_idx; idx++) {
				x_sum += connected_dots[idx][0]; 
				y_sum += connected_dots[idx][1];
			}
			center_of_gravity_x = x_sum / (current_dot_idx + 1);
			center_of_gravity_y = y_sum / (current_dot_idx + 1);
			glutTimerFunc(TIMER_INTERVAL, rotation_timer, 0);
		}
		else {
			glutPostRedisplay();
		}
		break;
	case 'f':
		glutLeaveMainLoop();
		break;
	}
}

void special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		if (is_polygon == 0 || rotation_mode) break; // if polygon is not made, or rotation_mode is on ignore action
		move_polygon(-0.05f, 0.0f);
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		if (is_polygon == 0 || rotation_mode) break; // if polygon is not made, or rotation_mode is on ignore action
		move_polygon(0.05f, 0.0f);
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		if (is_polygon == 0 || rotation_mode) break; // if polygon is not made, or rotation_mode is on ignore action
		move_polygon(0.0f, -0.05f);
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP:
		if (is_polygon == 0 || rotation_mode) break; // if polygon is not made, or rotation_mode is on ignore action
		move_polygon(0.0f, 0.05f);
		glutPostRedisplay();
		break;
	}
}

void mousepress(int button, int state, int x, int y) {
	int modifier = glutGetModifiers();
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN) && (modifier == GLUT_ACTIVE_SHIFT) && (is_polygon==0) && (rotation_mode == 0)) {
		current_dot_idx++;
		connected_dots[current_dot_idx][0] = transform_x_to_vertex(x); 
		connected_dots[current_dot_idx][1] = transform_y_to_vertex(y);
		glutPostRedisplay();
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)){
		right_button_pressed = 1; 
		right_button_pressed_x = transform_x_to_vertex(x); 
		right_button_pressed_y = transform_y_to_vertex(y);
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)){
		right_button_pressed = 0; right_button_pressed_x = 0.0f; right_button_pressed_y = 0.0f;
	}
}

void mousemove(int x, int y) {
	if (right_button_pressed && is_polygon && rotation_mode == 0){
		float x_vertex = transform_x_to_vertex(x); float y_vertex = transform_y_to_vertex(y);
		move_polygon(x_vertex - right_button_pressed_x, y_vertex - right_button_pressed_y);
		glutPostRedisplay();
		right_button_pressed_x = x_vertex;
		right_button_pressed_y = y_vertex;
	}
}

void reshape(int width, int height) {
	fprintf(stdout, "### The new window size is %dx%d.\n", width, height);
	window_width = width; window_height = height;
	glViewport(0, 0, width, height);
}

void close(void) {
	fprintf(stdout, "\n^^^ The control is at the close callback function now.\n\n");
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mousepress);
	glutMotionFunc(mousemove);
	glutReshapeFunc(reshape);
	glutCloseFunc(close);
}

void initialize_renderer(void) {
	register_callbacks();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = TRUE;
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

void greetings(char* program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 Homework by 20180223 Haejin Lim \n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 4
void main(int argc, char* argv[]) {
	char program_name[64] = "CSE4170 HW1 20180223 Haejin Lim";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'p', 'c', 'r', 'f'",
		"    - Special keys used: LEFT, RIGHT, UP, DOWN",
		"    - Mouse used: L-click+SHIFT, R-click and move",
		"    - Other operations: window size change"
	};

	glutInit(&argc, argv);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_RGBA);

	glutInitWindowSize(window_width, window_height);
	glutInitWindowPosition(200, 200);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glutMainLoop();
	fprintf(stdout, "^^^ The control is at the end of main function now. Good-Bye. \n\n");
}