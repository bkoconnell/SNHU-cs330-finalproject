/*
 * 3D TRIFORCE
 *
 * glDrawArrays
 *
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

#define WINDOW_TITLE "3D Triforce -- glDrawArrays" // Window title macro

/* SHADER program macro */
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

/* variable declarations for shader, window size init, buffer and array objects */
GLint shaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, VAO, texture;

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
	glEnable(GL_DEPTH_TEST); // enable Z depth
	glDepthFunc(GL_LESS); // accept fragment if closer to the camera than the former one

	glBindVertexArray(VAO); // activate the vertex array object before rendering and transforming them

	// enable cull facing
//	glEnable(GL_CULL_FACE);

    CameraForwardZ = front; // replaces camera forward vector with Radians normalized as a unit vector

    // initialize variables for matrix uniforms
    GLint modelLoc, viewLoc, projLoc;

    // initialize transformation matrices
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

	// transforms the object
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // place the object at the center of the viewport
	model = glm::rotate(model, degrees, glm::vec3(1.0f, 1.0f, 1.0f)); // rotate the object 45 degrees on x, y, and z axis
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f)); // increase the object size by a scale of 1.5

    // transform the camera
    view = glm::lookAt(cameraPosition - CameraForwardZ, cameraPosition, CameraUpY);

    //FIXME:  Need IF branch statement for Toggling Perspective
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
	glDrawArrays(GL_TRIANGLES, 0, 72);

	glBindVertexArray(0); // deactivate the vertex array object

	glutSwapBuffers(); // flips the back & front buffers every frame (similar to glFlush)

}

/* creates the shader program */
void UCreateShader() {

	// vertex shader
	GLint vertexShader = glCreateShader(GL_VERTEX_SHADER); // creates the vertex shader
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // attaches the vertex shader to the source code
	glCompileShader(vertexShader); // compiles the vertex shader

	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success){
	    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
	    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// fragment shader
	GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // creates the fragment shader
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // attaches the fragment shader to the source code
	glCompileShader(fragmentShader); // compiles the fragment shader

    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

	// shader program
	shaderProgram = glCreateProgram(); // creates the shader program and returns an id
	glAttachShader(shaderProgram, vertexShader); // attach vertex shader to the shader program
	glAttachShader(shaderProgram, fragmentShader); // attach fragment shader to the shader program
	glLinkProgram(shaderProgram); // link vertex and fragment shaders to shader program

    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

	// delete the vertex and fragment shaders once linked
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}


/* creates the buffer and array objects */
void UCreateBuffers() {

	// position and color data
	GLfloat vertices[] = {


		//	Vertex data         // Color data

/* FRONT TRIFORCE */
		// FRONT: Bottom Left triangle
		 -0.5f, -0.5f,  0.0f,   1.0f, 0.73f, 0.0f,	// v0
		  0.0f, -0.5f,  0.0f,	1.0f, 0.73f, 0.0f,	// v1
		-0.25f,  0.0f,  0.0f,	1.0f, 0.73f, 0.0f,	// v2

		// FRONT: Bottom Right triangle
		  0.0f, -0.5f,  0.0f,	1.0f, 0.73f, 0.0f,	// v3
		  0.5f, -0.5f,  0.0f,	1.0f, 0.73f, 0.0f,	// v4
		 0.25f,  0.0f,  0.0f,	1.0f, 0.73f, 0.0f,	// v5

		// FRONT: Top triangle
		 0.25f,  0.0f,  0.0f,	1.0f, 0.73f, 0.0f,	// v6
		  0.0f,  0.5f,  0.0f,	1.0f, 0.73f, 0.0f,	// v7
		-0.25f,  0.0f,  0.0f,	1.0f, 0.73f, 0.0f,	// v8

/* BACK TRIFORCE */
		// BACK: Bottom Left Triangle
		 -0.5f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v9
		  0.0f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v10
	    -0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v11

		// BACK: Bottom Right Triangle
		  0.0f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v12
		  0.5f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v13
	     0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v14


		// BACK: Top Triangle
		 0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v15
		  0.0f,  0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v16
	    -0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v17


/* TRIFORCE SIDES: ... LEFT ... BOTTOM ... RIGHT ...  */

	/* BOTTOM LEFT TRIANGLE */
		// LEFT SIDE
		 -0.5f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v18
		 -0.5f, -0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v19
	    -0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v20

		 -0.5f, -0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v21
		-0.25f,  0.0f,  0.0f,   1.0f, 0.73f, 0.0f,  // v22
	    -0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v23

		// BOTTOM SIDE
		 -0.5f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v24
		 -0.5f, -0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v25
	      0.0f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v26

		 -0.5f, -0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v27
		  0.0f, -0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v28
	      0.0f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v29

		// RIGHT SIDE
		  0.0f, -0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v30
		  0.0f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v31
	    -0.25f,  0.0f,  0.0f,   1.0f, 0.73f, 0.0f,  // v32

		  0.0f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v33
	    -0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v34
	    -0.25f,  0.0f,  0.0f,   1.0f, 0.73f, 0.0f,  // v35

	/* BOTTOM RIGHT TRIANGLE */
		// LEFT SIDE
		  0.0f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v36
		  0.0f, -0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v37
	     0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v38

		  0.0f, -0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v39
		 0.25f,  0.0f,  0.0f,   1.0f, 0.73f, 0.0f,  // v40
	     0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v41

		// BOTTOM SIDE
		  0.0f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v42
		  0.0f, -0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v43
	      0.5f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v44

		  0.0f, -0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v45
		  0.5f, -0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v46
	      0.5f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v47

		// RIGHT SIDE
		  0.5f, -0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v48
		  0.5f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v49
	     0.25f,  0.0f,  0.0f,   1.0f, 0.73f, 0.0f,  // v50

		  0.5f, -0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v51
		 0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v52
	     0.25f,  0.0f,  0.0f,   1.0f, 0.73f, 0.0f,  // v53

	/* TOP TRIANGLE */
		// LEFT SIDE
		-0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v54
		-0.25f,  0.0f,  0.0f,   1.0f, 0.73f, 0.0f,  // v55
	      0.0f,  0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v56

		-0.25f,  0.0f,  0.0f,   1.0f, 0.73f, 0.0f,  // v57
		  0.0f,  0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v58
	      0.0f,  0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v59

		// BOTTOM SIDE
		-0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v60
		-0.25f,  0.0f,  0.0f,   1.0f, 0.73f, 0.0f,  // v61
	     0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v62

		-0.25f,  0.0f,  0.0f,   1.0f, 0.73f, 0.0f,  // v63
		 0.25f,  0.0f,  0.0f,   1.0f, 0.73f, 0.0f,  // v64
	     0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v65

		// RIGHT SIDE
		 0.25f,  0.0f,  0.0f,   1.0f, 0.73f, 0.0f,  // v66
		 0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v67
	      0.0f,  0.5f,  0.0f,   1.0f, 0.73f, 0.0f,  // v68

		 0.25f,  0.0f, -0.1f,   1.0f, 0.73f, 0.0f,  // v69
		  0.0f,  0.5f, -0.1f,   1.0f, 0.73f, 0.0f,  // v70
		  0.0f,  0.5f,  0.0f,   1.0f, 0.73f, 0.0f   // v71

	};


	// generate buffer ids
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	//activate the vertex array object before binding and setting any VBOs & vertex attribute pointers
	glBindVertexArray(VAO);

	// activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Copy vertices to VBO


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






