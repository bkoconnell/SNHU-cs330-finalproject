/* ADDED CAMERA FUNCTIONALITY BUT LIGHTING IS STRANGE */

//FIXME: Investigate the Lighting Reflection issue



/*
 * FinalProject.cpp
 *
 *  Created on: Aug 13, 2020
 *      Author: 1455100_snhu
 *      Brendan O'Connell
 */


/* To view the object, hold the ALT key and the Left Mouse button until the object appears.
 *
 * 'H' will display a list of mouse and keyboard functionality.
 *
 * 'P' toggles projection matrix between perspective and ortho (initial projection is set to perspective).
 *
 * 'Q' will Quit the program.
 */

// header inclusions
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "SOIL2/SOIL2.h"

// glm math header inclusions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std; // standard namespace

#define WINDOW_TITLE "Final Project" // macro for window title

// shader program macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

//FIXME: review shader programs ,  Vertex Array Objects, etc.
/* variable declarations for shader, window size initialize, buffer and array objects */
GLint pyramidShaderProgram, lampShaderProgram;
GLint WindowWidth = 800, WindowHeight = 600;
GLuint VBO, PyramidVAO, LightVAO, EBO, texture;

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

// Projection Toggle global variable
int toggleProjection = 1; // Perspective view = 1;  Ortho view = 0
/*	Partial credit for code idea on projection toggle goes to Nexusone from Khronos forum:
*	https://community.khronos.org/t/switching-between-ortho-and-perspective-views/31561
*/


// Subject position and scale
glm::vec3 pyramidPosition(0.0f, 0.0f, 0.0f); // place object at center of viewpoint
glm::vec3 pyramidRotation(0.0f, 1.0f, 0.0f); // rotate the object 45 degrees on Y axis
glm::vec3 pyramidScale(2.0f); // increase object size by 2 units

// Pyramid and Light color
glm::vec3 objectColor(1.0f, 1.0f, 1.0f);
glm::vec3 lightColor(0.8f, 1.0f, 0.8f);

// Light position and scale
glm::vec3 lightPosition(0.5f, 0.5f, -3.0f);
glm::vec3 lightScale(0.3f);

// global vector declarations for camera position, rotation, movement, etc.
float cameraRotation = glm::radians(-25.0f);
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, -6.0f); // initial camera position, placed 5 units in Z
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); // temporary Y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -1.0f); // temporary Z unit vector
glm::vec3 front; // temporary z unit vector for mouse


/* Initialize Functions for window, graphics, shaders, buffers, textures, keyboard & mouse*/
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UGenerateTexture(void);
void UKeyboard(unsigned char key, int x, int y);
void UMouseActiveMotion(int x, int y);
void UMouseClick(int button, int state, int x, int y);

/* Pyramid Vertex Shader Source Code */
const GLchar * pyramidVertexShaderSource = GLSL(330,
        layout (location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
        layout (location = 1) in vec3 normal; // VAP position 1 for normals
        layout (location = 2) in vec2 textureCoordinate; // texture data from vertex attrib pointer 2

        out vec3 FragmentPos; // For outgoing color / pixels to fragment shader
        out vec3 Normal; // For outgoing normals to fragment shader
        out vec2 mobileTextureCoordinate; // variable to transfer texture coordinate data to the fragment shader


        // global variables for the transform matrices
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main(){
        	gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates

        	FragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

        	Normal = mat3(transpose(inverse(model))) *  normal; // get normal vectors in world space only and exclude normal translation properties

        	mobileTextureCoordinate = vec2(textureCoordinate.x, 1 - textureCoordinate.y); // flips the texture horizontal
        }
);



/* Pyramid Fragment Shader Source Code */
const GLchar * pyramidFragmentShaderSource = GLSL(330,

        in vec3 FragmentPos; // incoming fragment position
        in vec3 Normal; // incoming normals
        in vec2 mobileTextureCoordinate; // incoming texture coordinate

        out vec4 pyramidColor; // outgoing pyramid color to the GPU

        // Uniform / Global variables for light color, light position, and camera/view position
        uniform vec3 lightColor;
        uniform vec3 lightPos;
        uniform vec3 viewPosition;

        uniform sampler2D uTexture; // for working with multiple textures

        void main(){

            /* Phong lighting model calculations to generate ambient, diffuse, and specular components */

            // Calculate Ambient Lighting
            float ambientStrength = 0.1f; // Set ambient or global lighting strength
            vec3 ambient = ambientStrength * lightColor; // Generate ambient light color


            // Calculate Diffuse Lighting
            vec3 norm = normalize(Normal); // Normalize vectors to 1 unit
            vec3 lightDirection = normalize(lightPos - FragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on
            float impact = max(dot(norm, lightDirection), 0.0); // Calculate diffuse impact by generating dot product of normal and light
            vec3 diffuse = impact * lightColor; // Generate diffuse light color


            // Calculate Specular lighting
            float specularIntensity = 0.8f; // Set specular light strength
            float highlightSize = 128.0f; // Set specular highlight size
            vec3 viewDir = normalize(viewPosition - FragmentPos); // Calculate view direction
            vec3 reflectDir = reflect(-lightDirection, norm); // Calculate reflection vector
            // Calculate specular component
            float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
            vec3 specular = specularIntensity * specularComponent * lightColor;

            // Calculate phong result
            vec3 objectColor = texture(uTexture, mobileTextureCoordinate).xyz;
            vec3 phong = (ambient + diffuse) * objectColor + specular;
            pyramidColor = vec4(phong, 1.0f); // Send lighting results to GPU

        }
);


/* Lamp Shader Source Code */
const GLchar * lampVertexShaderSource = GLSL(330,

        layout (location = 0) in vec3 position; // VAP position 0 for vertex position data

        // Uniform / Global variables for the transform matrices
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main(){

            gl_Position = projection * view *model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
        }
);


/* Fragment Shader Source Code */
const GLchar * lampFragmentShaderSource = GLSL(330,

        out vec4 color; // outgoing lamp color (smaller pyramid) to the GPU

        void main(){

            color = vec4(1.0f); // Set color to white (1.0f, 1.0f, 1.0f) with alpha 1.0

        }
);


/* main program */
int main(int argc, char* argv[]){


	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
	if (glewInit()!= GLEW_OK){
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// Beginning console output
	cout << "To start: \n\n"
			"Hold ALT key and LEFT mouse button \n"
			"until image appears. \n\n"

			"Press the 'H' key for additional instructions \n"
			"once the image appears" << endl;

	UCreateShader();
	UCreateBuffers();
	UGenerateTexture();

	glClearColor(0.0f, 5.0f, 1.0f, 1.0f); // set background color

	glutDisplayFunc(URenderGraphics); // renders graphics on the screen

	// keyboard & mouse functions
	glutKeyboardFunc(UKeyboard); // detects key press
	glutMotionFunc(UMouseActiveMotion); // detects mouse movement & mouse click
	glutMouseFunc(UMouseClick); // detects mouse click

	glutMainLoop(); // enters GLUT event processing loop

	// destroys buffer objects once used
	glDeleteVertexArrays(1, &PyramidVAO);
	glDeleteVertexArrays(1, &LightVAO);
	glDeleteBuffers(1, &VBO);

	return 0; // exits main function
}


// resize the window
void UResizeWindow(int w, int h){
	WindowWidth = w, WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}


/* renders graphics */
void URenderGraphics(void){

    glEnable(GL_DEPTH_TEST); // enable Z depth
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clears the screen

    CameraForwardZ = front; // replaces camera forward vector with Radians normalized as a unit vector

    // initialize variables for matrix uniforms
    GLint modelLoc, viewLoc, projLoc, uTextureLoc, lightColorLoc, lightPositionLoc, viewPositionLoc;

    // initialize transformation matrices
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;


    glUseProgram(pyramidShaderProgram); // use the pyramid shader program
    glBindVertexArray(PyramidVAO); // activate the pyramid vertex array object for rendering/transforming

    // Transform the pyramid
    model = glm::translate(model, pyramidPosition);
    model = glm::rotate(model, degrees, pyramidRotation);
    model = glm::scale(model, pyramidScale);

    // Transform the camera
    view = glm::lookAt(cameraPosition - CameraForwardZ, cameraPosition, CameraUpY);


    /* INITIAL TOGGLE PROJECTION BRANCH */
    // initiates projection toggle between Perspective view and Ortho view..
   	if(toggleProjection == 1){
    	projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);
   	}
   	else{
    	projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
   	}

    // Reference matrix uniforms from the pyramid Shader program
    modelLoc = glGetUniformLocation(pyramidShaderProgram, "model");
    viewLoc = glGetUniformLocation(pyramidShaderProgram, "view");
    projLoc = glGetUniformLocation(pyramidShaderProgram, "projection");

    // Pass matrix data to the pyramid Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the pyramid Shader program for the pyramid color, light color, light position, and camera position
    uTextureLoc = glGetUniformLocation(pyramidShaderProgram, "uTexture");
    lightColorLoc = glGetUniformLocation(pyramidShaderProgram, "lightColor");
    lightPositionLoc = glGetUniformLocation(pyramidShaderProgram, "lightPos");
    viewPositionLoc = glGetUniformLocation(pyramidShaderProgram, "viewPosition");

    // Pass color, light, and camera data to the pyramid Shader programs corresponding uniforms
    glUniform1i(uTextureLoc, 0); // texture unit 0
    glUniform3f(lightColorLoc, lightColor.r, lightColor.g, lightColor.b);
    glUniform3f(lightPositionLoc, lightPosition.x, lightPosition.y, lightPosition.z);
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    glActiveTexture(GL_TEXTURE0); // texture unit 0 is active
    glBindTexture(GL_TEXTURE_2D, texture); // bind texture to texture unit 0
    glDrawArrays(GL_TRIANGLES, 0, 18);	// Draw the primitives / pyramid

    glBindVertexArray(0); // Deactivate the Pyramid Vertex Array Object



    glUseProgram(lampShaderProgram); // use Lamp Shader program
    glBindVertexArray(LightVAO); // activate lamp vertex array object for rendering/transforming

    // Transform the smaller pyramid (lamp) used as a visual cue for the light source
    model = glm::translate(model, lightPosition);
    model = glm::scale(model, lightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(lampShaderProgram, "model");
    viewLoc = glGetUniformLocation(lampShaderProgram, "view");
    projLoc = glGetUniformLocation(lampShaderProgram, "projection");

    // Pass matrix uniforms from the Lamp Shader Program
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindTexture(GL_TEXTURE_2D, texture); // activate texture

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, 18);

    glBindVertexArray(0); // Deactivate the Lamp Vertex Array Object

    glutPostRedisplay();
    glutSwapBuffers(); // Flips the back buffer with the front buffer every frame. Similar to GL Flush

}

/* Create the Shader program */
void UCreateShader()
{

    // Pyramid Vertex shader
    GLint pyramidVertexShader = glCreateShader(GL_VERTEX_SHADER); // Creates the Vertex shader
    glShaderSource(pyramidVertexShader, 1, &pyramidVertexShaderSource, NULL); // Attaches the Vertex shader to the source code
    glCompileShader(pyramidVertexShader); // Compiles the Vertex shader

    // Pyramid Fragment Shader
    GLint pyramidFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment Shader
    glShaderSource(pyramidFragmentShader, 1, &pyramidFragmentShaderSource, NULL); // Attaches the Fragment shader to the source code
    glCompileShader(pyramidFragmentShader); // Compiles the Fragment Shader

    // Pyramid Shader program
    pyramidShaderProgram = glCreateProgram(); // Creates the Shader program and returns an id
    glAttachShader(pyramidShaderProgram, pyramidVertexShader); // Attaches Vertex shader to the Shader program
    glAttachShader(pyramidShaderProgram, pyramidFragmentShader); // Attaches Fragment shader to the Shader program
    glLinkProgram(pyramidShaderProgram); // Link Vertex and Fragment shaders to the Shader program

    // Delete the Vertex and Fragment shaders once linked
    glDeleteShader(pyramidVertexShader);
    glDeleteShader(pyramidFragmentShader);


    // Lamp Vertex shader
    GLint lampVertexShader = glCreateShader(GL_VERTEX_SHADER); // Creates the Vertex shader
    glShaderSource(lampVertexShader, 1, &lampVertexShaderSource, NULL); // Attaches the Vertex shader to the source code
    glCompileShader(lampVertexShader); // Compiles the Vertex shader

    // Lamp Fragment shader
    GLint lampFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment shader
    glShaderSource(lampFragmentShader, 1, &lampFragmentShaderSource, NULL); // Attaches the Fragment shader to the source code
    glCompileShader(lampFragmentShader); // Compiles the Fragment shader

    // Lamp Shader Program
    lampShaderProgram = glCreateProgram(); // Creates the Shader program and returns an id
    glAttachShader(lampShaderProgram, lampVertexShader); // Attach Vertex shader to the Shader program
    glAttachShader(lampShaderProgram, lampFragmentShader); // Attach Fragment shader to the Shader program
    glLinkProgram(lampShaderProgram); // Link Vertex and Fragment shaders to the Shader program

    // Delete the lamp shaders once linked
    glDeleteShader(lampVertexShader);
    glDeleteShader(lampFragmentShader);

}


/* creates the Buffer and Array Objects */
void UCreateBuffers()
{
    // Position and Texture coordinate data for 18 triangles
    GLfloat vertices[] = {

		//Positions             //Normals               //Texture Coordinates

		//Back Face             //Negative Z Normals
		 0.0f,  0.5f,  0.0f,     0.0f,  0.0f, -1.0f,    0.5f, 1.0f,
		 0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    1.0f, 0.0f,

		//Front Face            //Positive Z Normals
		 0.0f,  0.5f,  0.0f,     0.0f,  0.0f,  1.0f,    0.5f, 1.0f,
		-0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    1.0f, 0.0f,

		 //Left Face            //Negative X Normals
		 0.0f,  0.5f,  0.0f,    -1.0f,  0.0f,  0.0f,    0.5f, 1.0f,
		-0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,

		 //Right Face           //Positive X Normals
		 0.0f,  0.5f,  0.0f,     1.0f,  0.0f,  0.0f,    0.5f, 1.0f,
		 0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,    1.0f, 0.0f,

		 //Bottom Face          //Negative Y Normals
		-0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,    0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,    0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,    1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,    1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,    0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,    1.0f, 0.0f,

    };


    // Generate buffer ids
    glGenVertexArrays(1, &PyramidVAO);
    glGenBuffers(1, &VBO);

    // Activate the PyramidVAO before binding and setting VBOs and VAPs
    glBindVertexArray(PyramidVAO);

    // Activate the VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //Copy vertices to VBO

    // Set attribute pointer 0 to hold position data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0); //Enables vertex attribute

    // Set attribute pointer 1 to hold Normal data
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Set attribute pointer 2 to hold Texture coordinate data
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0); // deactivate the pyramid VAO

    // Generate buffer ids for lamp (smaller pyramid)
    glGenVertexArrays(1, &LightVAO); // Vertex Array for pyramid vertex copies to serve as light source

    // Activate the Vertex Array Object before binding and setting any VBOs and Vertex Attribute Pointers
    glBindVertexArray(LightVAO);

    // Referencing the same VBO for its vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Set attribute pointer to 0 to hold Position data (used for the lamp)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

}

/* Generate and load the texture */
void UGenerateTexture(){

    glGenTextures(1, &texture); // generate texture id
    glBindTexture(GL_TEXTURE_2D, texture); // activate (bind) the texture

    int width, height; // initialize dimensions

    /* JPEG image should be in project source folder */
    unsigned char* image = SOIL_load_image("snhu.jpg", &width, &height, 0, SOIL_LOAD_RGB); // Loads texture file


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind/deactivate the texture
}



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









