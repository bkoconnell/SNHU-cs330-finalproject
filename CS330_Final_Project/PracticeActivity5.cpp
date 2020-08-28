/*
 * PracticeActivity5.cpp
 *
 *  Created on: Aug 27, 2020
 *      Author: 1455100_snhu
 */


/*
 * 4-1_Practice_Activity_5.cpp
 *
 *  Created on: Aug 5, 2020
 *      Author: 1455100_snhu
 *      Brendan O'Connell
 */

/* Header Inclusions */
#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std; // standard namespace

#define WINDOW_TITLE "4-1 Practice Activity 5" // Window title macro

/* SHADER program macro */
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

/* variable declarations for shader, window size init, buffer and array objects */
GLint shaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, VAO, EBO, texture;

// global variable declarations for mouse, keyboard, camera/graphics, etc.
GLfloat degrees = glm::radians(-45.0f); // converts float to degrees
GLfloat fov = glm::radians(45.0f); // initialize field of view at 45 degrees
GLfloat lastMouseX = (WindowWidth / 2), lastMouseY = (WindowHeight / 2); // locks mouse cursor at the center of the screen
GLfloat mouseXOffset, mouseYOffset, yaw = -90.0f, pitch = 0.0f; // mouse offset, yaw, and pitch variables
GLfloat sensitivity = 0.005f; // used for mouse / camera rotation sensitivity
GLfloat cameraSpeed = 0.05f; // movement speed per frame
bool mouseDetected = true; // initially true when mouse movement is detected
GLint mouseLeftClick = -1; // used for the IF branch statements, if left mouse is clicked
GLint mouseRightClick = -1; // used for the IF branch statements, if right mouse is clicked
GLfloat deltaY = 0.0; // used to determine Y direction for right-mouse-click IF statement
GLfloat previousY = 0.0; // used to determine Y direction for right-mouse-click IF statement
GLint mod = 0; // initialize mod variable for glutGetModifiers()

// global vector declarations for camera position, rotation, movement, etc.
float cameraRotation = glm::radians(-25.0f);
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, -6.0f); // initial camera position, placed 5 units in Z
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); // temporary Y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -1.0f); // temporary Z unit vector
glm::vec3 front; // temporary z unit vector for mouse


// Projection Toggle global variable
int toggleProjection = 1; // Perspective view = 1;  Ortho view = 0
/*	Partial credit for code idea on projection toggle goes to Nexusone from Khronos forum:
*	https://community.khronos.org/t/switching-between-ortho-and-perspective-views/31561
*/


/* function prototypes */
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);

void UKeyboard(unsigned char key, int x, int y);
void UMouseActiveMotion(int x, int y);
void UMouseClick(int button, int state, int x, int y);


/* vertex shader source code */
const GLchar * vertexShaderSource = GLSL(330,
	layout (location = 0) in vec3 position; // transforms vertex data using matrix
	layout (location = 1) in vec3 color; // references incoming color data

	out vec3 mobileColor; // variable to transfer color data to the fragment shader

	// global variables for the transform matrices
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main() {
		gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
		mobileColor = color; // references incoming color data
	}
);

/* fragment shader source code */
const GLchar * fragmentShaderSource = GLSL(330,
	in vec3 mobileColor; // variable to hold incoming color data from vertex shader

	out vec4 gpuColor; // variable to pass color data to the gpu

	void main() {
		gpuColor = vec4(mobileColor, 1.0); // sends color data to the gpu for rendering
	}
);


/* main program */
int main(int argc, char* argv[]) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	UCreateShader(); // calls function to create shader

	UCreateBuffers(); // calls function to create buffers

	// use the Shader program
	glUseProgram(shaderProgram);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // set background color

	glutDisplayFunc(URenderGraphics); // sets the display callback

	// keyboard & mouse functions
	glutKeyboardFunc(UKeyboard); // detects key press
	glutMotionFunc(UMouseActiveMotion); // detects mouse movement & mouse click
	glutMouseFunc(UMouseClick); // detects mouse click

	glutMainLoop(); // enters loop for GLUT event processing

	// destroys buffer objects once used
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	return 0; // exits main function
}


/* resizes the window */
void UResizeWindow(int w, int h) {

	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}


/* renders graphics */
void URenderGraphics(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clears the screen

    CameraForwardZ = front; // replaces camera forward vector with Radians normalized as a unit vector

    // initialize variables for matrix uniforms
    GLint modelLoc, viewLoc, projLoc;

    // initialize transformation matrices
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

	glBindVertexArray(VAO); // activate the vertex array object before rendering and transforming them

	// transforms the object
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // place the object at the center of the viewport
	model = glm::rotate(model, degrees, glm::vec3(1.0f, 1.0f, 1.0f)); // rotate the object 45 degrees on x, y, and z axis
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f)); // increase the object size by a scale of 1.5

    // transform the camera
    view = glm::lookAt(cameraPosition - CameraForwardZ, cameraPosition, CameraUpY);

	// creates a perspective projection
	projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);


	// retrieves and passes transform matrices to the shader program
	modelLoc = glGetUniformLocation(shaderProgram, "model");
	viewLoc = glGetUniformLocation(shaderProgram, "view");
	projLoc = glGetUniformLocation(shaderProgram, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// draws the triangle
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0); // deactivate the vertex array object

	glutSwapBuffers(); // flips the back & front buffers every frame (similar to glFlush)

}

/* creates the shader program */
void UCreateShader() {

	// vertex shader
	GLint vertexShader = glCreateShader(GL_VERTEX_SHADER); // creates the vertex shader
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // attaches the vertex shader to the source code
	glCompileShader(vertexShader); // compiles the vertex shader

	// fragment shader
	GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // creates the fragment shader
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // attaches the fragment shader to the source code
	glCompileShader(fragmentShader); // compiles the fragment shader

	// shader program
	shaderProgram = glCreateProgram(); // creates the shader program and returns an id
	glAttachShader(shaderProgram, vertexShader); // attach vertex shader to the shader program
	glAttachShader(shaderProgram, fragmentShader); // attach fragment shader to the shader program
	glLinkProgram(shaderProgram); // link vertex and fragment shaders to shader program

	// delete the vertex and fragment shaders once linked
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}


/* creates the buffer and array objects */
void UCreateBuffers() {

	// position and color data
	GLfloat vertices[] = {

		/* Beak */
		//	Vertex                   // Color
			 0.000,	-0.300,	 0.900,   1.0f,  1.0f,  1.0f,  // V0
			-0.325,	-0.150,	 0.200,   1.0f,  1.0f,  1.0f,  // V1
			 0.325,	-0.150,	 0.200,   1.0f,  1.0f,  1.0f,  // V2
			-0.315,	-0.170,	 0.250,   1.0f,  1.0f,  1.0f,  // V3
			-0.290,	-0.190,	 0.300,   1.0f,  1.0f,  1.0f,  // V4
			-0.280,	-0.210,	 0.350,   1.0f,  1.0f,  1.0f,  // V5
			-0.270,	-0.220,	 0.400,   1.0f,  1.0f,  1.0f,  // V6
			-0.265,	-0.230,	 0.450,   1.0f,  1.0f,  1.0f,  // V7
			-0.245,	-0.240,	 0.500,   1.0f,  1.0f,  1.0f,  // V8
			-0.240,	-0.250,	 0.550,   1.0f,  1.0f,  1.0f,  // V9
			-0.245,	-0.270,	 0.600,   1.0f,  1.0f,  1.0f,  // V10
			-0.250,	-0.290,	 0.650,   1.0f,  1.0f,  1.0f,  // V11
			-0.265,	-0.310,	 0.700,   1.0f,  1.0f,  1.0f,  // V12
			-0.270,	-0.320,	 0.750,   1.0f,  1.0f,  1.0f,  // V13
			-0.240,	-0.330,	 0.800,   1.0f,  1.0f,  1.0f,  // V14
			-0.180,	-0.340,	 0.850,   1.0f,  1.0f,  1.0f,  // V15
			 0.180,	-0.340,	 0.850,   1.0f,  1.0f,  1.0f,  // V16
			 0.240,	-0.330,	 0.800,   1.0f,  1.0f,  1.0f,  // V17
			 0.270,	-0.320,	 0.750,   1.0f,  1.0f,  1.0f,  // V18
			 0.265,	-0.310,	 0.700,   1.0f,  1.0f,  1.0f,  // V19
			 0.250,	-0.290,	 0.650,   1.0f,  1.0f,  1.0f,  // V20
			 0.245,	-0.270,	 0.600,   1.0f,  1.0f,  1.0f,  // V21
			 0.240,	-0.250,	 0.550,   1.0f,  1.0f,  1.0f,  // V22
			 0.245,	-0.240,	 0.500,   1.0f,  1.0f,  1.0f,  // V23
			 0.265,	-0.230,	 0.450,   1.0f,  1.0f,  1.0f,  // V24
			 0.270,	-0.220,	 0.400,   1.0f,  1.0f,  1.0f,  // V25
			 0.280,	-0.210,	 0.350,   1.0f,  1.0f,  1.0f,  // V26
			 0.290,	-0.190,	 0.300,   1.0f,  1.0f,  1.0f,  // V27
			 0.315,	-0.170,	 0.250,   1.0f,  1.0f,  1.0f,  // V28
			 0.000,	-0.290,	 0.850,   1.0f,  1.0f,  1.0f,  // V29
			 0.000,	-0.280,	 0.800,   1.0f,  1.0f,  1.0f,  // V30
			 0.000,	-0.250,	 0.750,   1.0f,  1.0f,  1.0f,  // V31
			 0.000,	-0.210,	 0.700,   1.0f,  1.0f,  1.0f,  // V32
			 0.000,	-0.200,	 0.650,   1.0f,  1.0f,  1.0f,  // V33
			 0.000,	-0.180,	 0.600,   1.0f,  1.0f,  1.0f,  // V34
			 0.000,	-0.130,	 0.550,   1.0f,  1.0f,  1.0f,  // V35
			 0.000,	-0.100,	 0.500,   1.0f,  1.0f,  1.0f,  // V36
			 0.000,	-0.070,	 0.450,   1.0f,  1.0f,  1.0f,  // V37
			 0.000,	-0.030,	 0.400,   1.0f,  1.0f,  1.0f,  // V38
			 0.000,	 0.000,	 0.375,   1.0f,  1.0f,  1.0f,  // V39
			-0.300,	 0.050,	 0.250,   1.0f,  1.0f,  1.0f,  // V40
			 0.300,	 0.050,	 0.250,   1.0f,  1.0f,  1.0f,  // V41
			-0.200,	 0.040,	 0.300,   1.0f,  1.0f,  1.0f,  // V42
			-0.100,	 0.020,	 0.350,   1.0f,  1.0f,  1.0f,  // V43
			 0.100,	 0.020,	 0.350,   1.0f,  1.0f,  1.0f,  // V44
			 0.200,	 0.040,	 0.300,   1.0f,  1.0f,  1.0f,  // V45
			-0.330,	 0.000,	 0.260,   1.0f,  1.0f,  1.0f,  // V46
			-0.375,	-0.075,	 0.270,   1.0f,  1.0f,  1.0f,  // V47
			-0.400,	-0.110,	 0.260,   1.0f,  1.0f,  1.0f,  // V48
			-0.350,	-0.130,	 0.230,   1.0f,  1.0f,  1.0f,  // V49
			 0.330,	 0.000,	 0.260,   1.0f,  1.0f,  1.0f,  // V50
			 0.375,	-0.075,	 0.270,   1.0f,  1.0f,  1.0f,  // V51
			 0.400,	-0.110,	 0.260,   1.0f,  1.0f,  1.0f,  // V52
			 0.350,	-0.130,	 0.230,   1.0f,  1.0f,  1.0f,  // V53
			-0.238,	-0.030,	 0.250,   1.0f,  1.0f,  1.0f,  // V54
			-0.163,	-0.050,	 0.300,   1.0f,  1.0f,  1.0f,  // V55
			-0.093,	-0.070,	 0.350,   1.0f,  1.0f,  1.0f,  // V56
			-0.090,	-0.093,	 0.400,   1.0f,  1.0f,  1.0f,  // V57
			-0.088,	-0.123,	 0.450,   1.0f,  1.0f,  1.0f,  // V58
			-0.082,	-0.147,	 0.500,   1.0f,  1.0f,  1.0f,  // V59
			-0.080,	-0.170,	 0.550,   1.0f,  1.0f,  1.0f,  // V60
			-0.082,	-0.210,	 0.600,   1.0f,  1.0f,  1.0f,  // V61
			-0.083,	-0.230,	 0.650,   1.0f,  1.0f,  1.0f,  // V62
			-0.088,	-0.243,	 0.700,   1.0f,  1.0f,  1.0f,  // V63
			-0.090,	-0.273,	 0.750,   1.0f,  1.0f,  1.0f,  // V64
			-0.080,	-0.297,	 0.800,   1.0f,  1.0f,  1.0f,  // V65
			-0.060,	-0.307,	 0.850,   1.0f,  1.0f,  1.0f,  // V66
			 0.060,	-0.307,	 0.850,   1.0f,  1.0f,  1.0f,  // V67
			 0.080,	-0.297,	 0.800,   1.0f,  1.0f,  1.0f,  // V68
			 0.090,	-0.273,	 0.750,   1.0f,  1.0f,  1.0f,  // V69
			 0.088,	-0.243,	 0.700,   1.0f,  1.0f,  1.0f,  // V70
			 0.083,	-0.230,	 0.650,   1.0f,  1.0f,  1.0f,  // V71
			 0.082,	-0.210,	 0.600,   1.0f,  1.0f,  1.0f,  // V72
			 0.080,	-0.170,	 0.550,   1.0f,  1.0f,  1.0f,  // V73
			 0.082,	-0.147,	 0.500,   1.0f,  1.0f,  1.0f,  // V74
			 0.088,	-0.123,	 0.450,   1.0f,  1.0f,  1.0f,  // V75
			 0.090,	-0.093,	 0.400,   1.0f,  1.0f,  1.0f,  // V76
			 0.093,	-0.070,	 0.350,   1.0f,  1.0f,  1.0f,  // V77
			 0.163,	-0.050,	 0.300,   1.0f,  1.0f,  1.0f,  // V78
			 0.238,	-0.030,	 0.250,   1.0f,  1.0f,  1.0f,  // V79
			-0.277,	-0.100,	 0.250,   1.0f,  1.0f,  1.0f,  // V80
			-0.227,	-0.120,	 0.300,   1.0f,  1.0f,  1.0f,  // V81
			-0.187,	-0.140,	 0.350,   1.0f,  1.0f,  1.0f,  // V82
			-0.180,	-0.157,	 0.400,   1.0f,  1.0f,  1.0f,  // V83
			-0.177,	-0.177,	 0.450,   1.0f,  1.0f,  1.0f,  // V84
			-0.163,	-0.193,	 0.500,   1.0f,  1.0f,  1.0f,  // V85
			-0.160,	-0.210,	 0.550,   1.0f,  1.0f,  1.0f,  // V86
			-0.163,	-0.240,	 0.600,   1.0f,  1.0f,  1.0f,  // V87
			-0.167,	-0.260,	 0.650,   1.0f,  1.0f,  1.0f,  // V88
			-0.177,	-0.277,	 0.700,   1.0f,  1.0f,  1.0f,  // V89
			-0.180,	-0.297,	 0.750,   1.0f,  1.0f,  1.0f,  // V90
			-0.160,	-0.313,	 0.800,   1.0f,  1.0f,  1.0f,  // V91
			-0.120,	-0.323,	 0.850,   1.0f,  1.0f,  1.0f,  // V92
			 0.120,	-0.323,	 0.850,   1.0f,  1.0f,  1.0f,  // V93
			 0.160,	-0.313,	 0.800,   1.0f,  1.0f,  1.0f,  // V94
			 0.180,	-0.297,	 0.750,   1.0f,  1.0f,  1.0f,  // V95
			 0.177,	-0.277,	 0.700,   1.0f,  1.0f,  1.0f,  // V96
			 0.167,	-0.260,	 0.650,   1.0f,  1.0f,  1.0f,  // V97
			 0.163,	-0.240,	 0.600,   1.0f,  1.0f,  1.0f,  // V98
			 0.160,	-0.210,	 0.550,   1.0f,  1.0f,  1.0f,  // V99
			 0.163,	-0.193,	 0.500,   1.0f,  1.0f,  1.0f,  // V100
			 0.177,	-0.177,	 0.450,   1.0f,  1.0f,  1.0f,  // V101
			 0.180,	-0.157,	 0.400,   1.0f,  1.0f,  1.0f,  // V102
			 0.187,	-0.140,	 0.350,   1.0f,  1.0f,  1.0f,  // V103
			 0.227,	-0.120,	 0.300,   1.0f,  1.0f,  1.0f,  // V104
			 0.277,	-0.100,	 0.250,   1.0f,  1.0f,  1.0f,  // V105
			 0.000,	-0.390,	 0.850,   1.0f,  1.0f,  1.0f,  // V106
			 0.000,	-0.420,	 0.800,   1.0f,  1.0f,  1.0f,  // V107
			 0.000,	-0.400,	 0.750,   1.0f,  1.0f,  1.0f,  // V108
			 0.000,	-0.380,	 0.700,   1.0f,  1.0f,  1.0f,  // V109
			 0.000,	-0.370,	 0.650,   1.0f,  1.0f,  1.0f,  // V110
			 0.000,	-0.360,	 0.600,   1.0f,  1.0f,  1.0f,  // V111
			 0.000,	-0.340,	 0.550,   1.0f,  1.0f,  1.0f,  // V112
			 0.000,	-0.320,	 0.500,   1.0f,  1.0f,  1.0f,  // V113
			 0.000,	-0.310,	 0.450,   1.0f,  1.0f,  1.0f,  // V114
			 0.000,	-0.300,	 0.400,   1.0f,  1.0f,  1.0f,  // V115
			 0.000,	-0.290,	 0.350,   1.0f,  1.0f,  1.0f,  // V116
			 0.000,	-0.280,	 0.300,   1.0f,  1.0f,  1.0f,  // V117
			 0.000,	-0.260,	 0.250,   1.0f,  1.0f,  1.0f,  // V118
			 0.000,	-0.250,	 0.225,   1.0f,  1.0f,  1.0f,  // V119
			-0.163,	-0.200,	 0.207,   1.0f,  1.0f,  1.0f,  // V120
			 0.163,	-0.200,	 0.207,   1.0f,  1.0f,  1.0f,  // V121
			-0.158,	-0.215,	 0.250,   1.0f,  1.0f,  1.0f,  // V122
			-0.145,	-0.235,	 0.300,   1.0f,  1.0f,  1.0f,  // V123
			-0.140,	-0.250,	 0.350,   1.0f,  1.0f,  1.0f,  // V124
			-0.135,	-0.260,	 0.400,   1.0f,  1.0f,  1.0f,  // V125
			-0.133,	-0.270,	 0.450,   1.0f,  1.0f,  1.0f,  // V126
			-0.123,	-0.280,	 0.500,   1.0f,  1.0f,  1.0f,  // V127
			-0.120,	-0.295,	 0.550,   1.0f,  1.0f,  1.0f,  // V128
			-0.123,	-0.315,	 0.600,   1.0f,  1.0f,  1.0f,  // V129
			-0.125,	-0.330,	 0.650,   1.0f,  1.0f,  1.0f,  // V130
			-0.133,	-0.345,	 0.700,   1.0f,  1.0f,  1.0f,  // V131
			-0.135,	-0.360,	 0.750,   1.0f,  1.0f,  1.0f,  // V132
			-0.120,	-0.375,	 0.800,   1.0f,  1.0f,  1.0f,  // V133
			-0.090,	-0.365,	 0.850,   1.0f,  1.0f,  1.0f,  // V134
			 0.090,	-0.365,	 0.850,   1.0f,  1.0f,  1.0f,  // V135
			 0.120,	-0.375,	 0.800,   1.0f,  1.0f,  1.0f,  // V136
			 0.135,	-0.360,	 0.750,   1.0f,  1.0f,  1.0f,  // V137
			 0.133,	-0.345,	 0.700,   1.0f,  1.0f,  1.0f,  // V138
			 0.125,	-0.330,	 0.650,   1.0f,  1.0f,  1.0f,  // V139
			 0.123,	-0.315,	 0.600,   1.0f,  1.0f,  1.0f,  // V140
			 0.120,	-0.295,	 0.550,   1.0f,  1.0f,  1.0f,  // V141
			 0.123,	-0.280,	 0.500,   1.0f,  1.0f,  1.0f,  // V142
			 0.133,	-0.270,	 0.450,   1.0f,  1.0f,  1.0f,  // V143
			 0.135,	-0.260,	 0.400,   1.0f,  1.0f,  1.0f,  // V144
			 0.140,	-0.250,	 0.350,   1.0f,  1.0f,  1.0f,  // V145
			 0.145,	-0.235,	 0.300,   1.0f,  1.0f,  1.0f,  // V146
			 0.158,	-0.215,	 0.250,   1.0f,  1.0f,  1.0f,  // V147
			-0.140,	-0.300,	 0.875,   1.0f,  1.0f,  1.0f,  // V148
			 0.140,	-0.200,	 0.875,   1.0f,  1.0f,  1.0f   // V149

	};

	// index data to share position data
	GLuint indices[] = {

			// triangles
			  0, 148,  92, // triangle 1
			  1,  92,  66, // triangle 2
			  0,  66,  29, // triangle 3
			  0,  29,  67, // triangle 4
			  0,  67,  93, // triangle 5
			  0,  93, 149, // triangle 6
			  4,  15,  92, // triangle 7
			  4,  93,  16, // triangle 8


			 14,  15,  91, // t
			 15,  91,  92, // t
			 91,  92,  65, // t
			 92,  65,  66, // t
			 29,  65,  66, // t
			 29,  30,  65  // t

	};

	// generate buffer ids
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	//activate the vertex array object before binding and setting any VBOs & vertex attribute pointers
	glBindVertexArray(VAO);

	// activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Copy vertices to VBO

	// Activate the Element Buffer Object / Indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // copy indices to EBO

	// set attribute pointer 0 to hold position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); // enables vertex attribute

	// set attribute pointer 1 to hold color data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1); // enables color attribute

	glBindVertexArray(0); // deactivates the VAO which is good practice

};


/* implements the UKeyboard function */
void UKeyboard(unsigned char key, GLint x, GLint y){

	switch(key){

		case 'h':
		case 'H':
			cout << "\n"
					"ALT + Left Mouse Button + Mouse Movement \n"
					"will Orbit the camera around the cube. \n\n"
					"ALT + Right Mouse Button + Mouse Upward Movement \n"
					"will Zoom Out. \n\n"
					"ALT + Right Mouse Button + Mouse Downward Movement \n"
					"will Zoom In. \n\n"
					"Press 'P' to toggle camera Projections.\n\n"
					"Press 'Q' to Quit the program. \n" << endl;
			break;

		case 'p':
		case 'P':
			toggleProjection = abs(toggleProjection - 1);
			if(toggleProjection == 1){
				cout << "Projection matrix is set to Perspective" << endl;
			}
			else{
				cout << "Projection matrix is set to Ortho" << endl;
			}
			break;

		case 'q':
		case 'Q':
			cout << "\nGoodbye!!" << endl;
			exit(0);
			break;
		default:
			cout << "Press 'H' to display a list of mouse & keyboard functionality.\n"
					"Press 'P' to toggle camera projection between Perspective and Ortho.\n"
					"Press 'Q' to Quit the program.\n" << endl;
			break;
	}
}

/* implements the UMouseActiveMotion function */
void UMouseActiveMotion(int x, int y){

	/* ALT key --> confirm it is being pressed */
	mod = glutGetModifiers(); // assign mod variable to glutGetModifiers()

	if(mod == GLUT_ACTIVE_ALT){ // if ALT button is being pressed

		/* LEFT CLICK  camera logic*/
		// left mouse button pressed down (see UMouseClick function)
		if(mouseLeftClick >= 0){

			// immediately replaces center locked coordinates with new mouse coordinates
			if(mouseDetected){
				lastMouseX = x;
				lastMouseY = y;
				mouseDetected = false;
			}

			// gets the direction the mouse was moved in x and y
			mouseXOffset = x - lastMouseX;
			mouseYOffset = lastMouseY - y; // inverted Y

			// updates with new mouse coordinates
			lastMouseX = x;
			lastMouseY = y;

			// applies sensitivity to mouse direction
			mouseXOffset *= sensitivity;
			mouseYOffset *= sensitivity;

			// accumulates the yaw and pitch variables
			yaw += mouseXOffset;
			pitch += mouseYOffset;

			// maintains a 90 degree pitch for gimbal lock
			if(pitch > 89.0f)
				pitch = 89.0f;

			if(pitch < -89.0f)
				pitch = -89.0f;

			// orbits around the center
			front.x = 10.0f * cos(yaw);
			front.y = 10.0f * sin(pitch);
			front.z = sin(yaw) * cos(pitch) * 10.0f;
		}


		/* RIGHT CLICK camera logic */
		// right mouse button pressed down (see UMouseClick function)
		if(mouseRightClick >= 0){

			// 1st pass through on initial right click
			if(mouseDetected){
				previousY = y; // set previousY variable equal to Y coordinate
				deltaY = y - previousY; // sets deltaY to 0 (initial click has no vertical change)

				// sets to FALSE so the "else" branch will execute on future pass throughs until right mouse button is released
				mouseDetected = false;

			} // end 1st pass through

			// for 2nd+ pass throughs, this branch will execute until right-mouse button is released
			else{
				deltaY = y - previousY; // set deltaY equal to the difference between Y coordinate and previous Y coordinate
				previousY = y; // set previousY to Y coordinate to prep for next pass through calculation
			}


			/* Zoom In & Zoom Out camera logic */

			if(deltaY > 0){ // mouse is moving down
				cameraPosition += cameraSpeed * CameraForwardZ; // "zoom in"
			}
			if(deltaY < 0){ // mouse is moving up
				cameraPosition -= cameraSpeed * CameraForwardZ; // "zoom out"
			}
		}
	}
	else{ // if ALT key is not pressed
		cout << "Press and Hold the ALT key while you click the mouse \n:)" << endl;
	}
}


/* implements the UMouseClick function */
void UMouseClick(int button, int state, int x, int y){

	/* LEFT MOUSE CLICK*/
	if(button == GLUT_LEFT_BUTTON){

		if(state == GLUT_UP){
			mouseLeftClick = -1;
			cout << "Left mouse button released" << endl;
		}
		else{
			//(state == GLUT_DOWN
			mouseDetected = true;
			mouseLeftClick = x; // value is >= 0
			cout << "Left mouse button clicked" << endl;
		}
	}

	/* RIGHT MOUSE CLICK*/
	if(button == GLUT_RIGHT_BUTTON){

		if(state == GLUT_UP){
			mouseRightClick = -1;
			cout << "Right mouse button released" << endl;
		}
		else{
			//(state == GLUT_DOWN
			mouseDetected = true;
			mouseRightClick = y; // value is >= 0
			cout << "Right mouse button clicked" << endl;
		}
	}
}






