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

#define WINDOW_TITLE "3D Triforce" // macro for window title

// shader program macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif


/* variable declarations for shader, window size initialize, buffer and array objects */
GLint triforceShaderProgram, lampShaderProgram;
GLint WindowWidth = 800, WindowHeight = 600;
GLuint VBO, TriforceVAO, LightVAO, EBO, texture;

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

// global variable for Projection matrix toggle
int toggleProjection = 1; // Perspective view = 1;  Ortho view = 0

/*	Partial credit for code idea on Toggle goes to Nexusone from Khronos forum (although I'm sure there are others to credit as well):
*	https://community.khronos.org/t/switching-between-ortho-and-perspective-views/31561
*/

// global variables for gl_polygonMode Toggle
int toggleWireframe = 0; // 0 = Wireframe OFF (a.k.a. Fill = ON);  Wireframe ON = 1  (use Wireframe ON to view lines drawn)
int toggleVertices = 0; // 0 = Vertices OFF (a.k.a. Fill = ON);  Vertices ON = 1  (use Vertices ON to view vertices drawn)


// Triforce position and scale
glm::vec3 triforcePosition(0.0f, 0.0f, 0.0f); // place object at center of viewpoint
glm::vec3 triforceRotation(0.0f, 1.0f, 0.0f); // rotate the object on Y axis
glm::vec3 triforceScale(2.0f); // increase object size by 2 units

// Triforce and Light color
glm::vec3 objectColor(1.0f, 1.0f, 1.0f);
glm::vec3 lightColor(0.8f, 0.9f, 1.0f);

// Light position and scale
glm::vec3 lightPosition(0.5f, 0.5f, -3.0f);
glm::vec3 lightScale(0.2f);

// global vector declarations for camera position, rotation, movement, etc.
float cameraRotation = glm::radians(-25.0f);
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, -6.0f); // initial camera position, placed 5 units in Z
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); // temporary Y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -1.0f); // temporary Z unit vector
glm::vec3 front; // temporary z unit vector for mouse


/* Initialize Functions for window, graphics, shaders, buffers, textures, keyboard & mouse */
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UGenerateTexture(void);
void UKeyboard(unsigned char key, int x, int y);
void UMouseActiveMotion(int x, int y);
void UMouseClick(int button, int state, int x, int y);



/* triforce Vertex Shader SOURCE CODE */
const GLchar * triforceVertexShaderSource = GLSL(330,
        layout (location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
        layout (location = 1) in vec3 normal; // VAP position 1 for normals (lighting)
        layout (location = 2) in vec2 textureCoordinate; // texture data from vertex attrib pointer 2

      // FIXME: DELETE AFTER TROUBLESHOOTING VERTICES
    	layout (location = 3) in vec3 color;

        out vec3 FragPos; // For outgoing fragment / pixels to fragment shader
        out vec3 Normal; // For outgoing normals to fragment shader
        out vec2 mobileTextureCoordinate; // variable to transfer texture coordinate data to the fragment shader

      // FIXME: DELETE AFTER TROUBLESHOOTING VERTICES
        out vec3 fragColor;

        // Uniform variables for the transform matrices
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main(){
        	gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
            FragPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)
            Normal = mat3(transpose(inverse(model))) *  normal; // get normal vectors in world space only and exclude normal translation properties
            mobileTextureCoordinate = vec2(textureCoordinate.x, 1 - textureCoordinate.y); // flips the texture horizontal

          // FIXME: DELETE AFTER TROUBLESHOOTING VERTICES
            fragColor = color;

        }
);


/* triforce Fragment Shader SOURCE CODE */
const GLchar * triforceFragmentShaderSource = GLSL(330,

        in vec3 FragPos; // incoming fragment position
        in vec3 Normal; // incoming normals
        in vec2 mobileTextureCoordinate; // incoming texture coordinate

        out vec4 triforceColor; // outgoing triforce color/lighting to the GPU

      // FIXME: DELETE AFTER TROUBLESHOOTING VERTICES
        out vec4 gpuColor;

        // Uniform variables for light color, light position, and camera/view position
        uniform vec3 lightColor;
        uniform vec3 lightPos;
        uniform vec3 viewPosition;
        uniform sampler2D uTexture; // for working with multiple textures

        void main(){

          /* Phong lighting model calculations to generate ambient, diffuse, and specular components */

            // Calculate Ambient Lighting
            float ambientStrength = 0.2f; // Set ambient lighting strength
            vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

            // Calculate Diffuse Lighting
            vec3 norm = normalize(Normal); // Normalize vectors to 1 unit
            vec3 lightDir = normalize(lightPos - FragPos); // Calculate light's direction vector between light source & fragment position
            float impact = max(dot(norm, lightDir), 0.0); // Calculate diffuse impact (generate dot product of normal & light direction)
            vec3 diffuse = impact * lightColor; // Generate diffuse light color

            // Calculate Specular lighting
            float specularIntensity = 0.5f; // Set specular light strength
            float highlightSize = 32.0f; // Set specular highlight size
            vec3 viewDir = normalize(viewPosition - FragPos); // Calculate view direction
            vec3 reflectDir = reflect(-lightDir, norm); // Calculate reflection vector
            // Calculate specular component
            float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
            vec3 specular = specularIntensity * specularComponent * lightColor;

            // Calculate phong result
            vec3 objectColor = texture(uTexture, mobileTextureCoordinate).xyz;
            vec3 phong = (ambient + diffuse) * objectColor + specular;

            // Send lighting results to GPU
            triforceColor = vec4(phong, 1.0f);


          // FIXME: DELETE AFTER TROUBLESHOOTING VERTICES
      		gpuColor = vec4(mobileColor, 1.0);
        }
);


/* lamp Vertex Shader SOURCE CODE */
const GLchar * lampVertexShaderSource = GLSL(330,

        layout (location = 0) in vec3 position; // VAP position 0 for vertex position data

        // Uniform variables for the transform matrices
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main(){
            gl_Position = projection * view *model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
        }
);


/* lamp Fragment Shader SOURCE CODE */
const GLchar * lampFragmentShaderSource = GLSL(330,

        out vec4 color; // variable for outgoing lamp color to the GPU

        void main(){
        	// send lamp color to GPU
            color = vec4(0.8f, 0.9f, 1.0f, 1.0f); // Set lamp color
        }
);




/* MAIN PROGRAM */
int main(int argc, char* argv[]){

	glutInit(&argc, argv); // initializes freeGLUT library
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA); // memory buffer setup for display
	glutInitWindowSize(WindowWidth, WindowHeight); // specifies display window's width & height
	glutCreateWindow(WINDOW_TITLE); // Creates Display Window

	glutReshapeFunc(UResizeWindow); // function to resize display window as needed

	// confirms GLEW initializes
	glewExperimental = GL_TRUE;
	if (glewInit()!= GLEW_OK){
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// Program Start: console text output for user
	cout << "To start: \n\n"
			"Hold ALT key and LEFT mouse button \n"
			"until image appears. \n\n"

			"Press the 'H' key for additional instructions \n"
			"once the image appears" << endl;


	UCreateShader(); // create Shaders
	UCreateBuffers(); // create Buffers
	UGenerateTexture(); // generate Textures

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // set background color (black)

	glutDisplayFunc(URenderGraphics); // renders graphics on the screen

	// keyboard & mouse functions
	glutKeyboardFunc(UKeyboard); // detects key press from keyboard
	glutMotionFunc(UMouseActiveMotion); // detects mouse movement & mouse click
	glutMouseFunc(UMouseClick); // detects mouse click

	glutMainLoop(); // enters GLUT event processing loop

	// destroys buffer objects once used
	glDeleteVertexArrays(1, &TriforceVAO);
	glDeleteVertexArrays(1, &LightVAO);
	glDeleteBuffers(1, &VBO);

	return 0; // exits main function
}


/* resize the window */
void UResizeWindow(int w, int h){
	WindowWidth = w, WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}


/* renders graphics */
void URenderGraphics(void){

    glEnable(GL_DEPTH_TEST); // enable Z depth
	glDepthFunc(GL_LESS); // accept fragment if closer to the camera than the former one
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clears the screen
	glEnable(GL_CULL_FACE); // enables cull facing so only "front-facing" graphics are rendered (a.k.a. graphics within visible viewing)

	// replaces camera forward vector with Radians normalized as a unit vector
	CameraForwardZ = front;

    // initialize variables for matrix uniforms
    GLint modelLoc, viewLoc, projLoc, uTextureLoc, lightColorLoc, lightPositionLoc, viewPositionLoc;

    // initialize transformation matrices
    glm::mat4 model, view, projection;



  /* TRIFORCE SHADER PROGRAM */
    glUseProgram(triforceShaderProgram);
    glBindVertexArray(TriforceVAO); // activate the triforce vertex array object for rendering/transforming

    // Transform the triforce
    model = glm::translate(model, triforcePosition);
    model = glm::rotate(model, degrees, triforceRotation);
    model = glm::scale(model, triforceScale);

    // Transform the camera
    view = glm::lookAt(cameraPosition - CameraForwardZ, cameraPosition, CameraUpY);


     /* Projection Matrix Toggle */
    // toggles projection matrix between Perspective and Ortho
   	if(toggleProjection == 1){
    	projection = glm::perspective(fov, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);
   	}
   	else{
    	projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
   	}

    // Reference matrix uniforms from the triforce's Vertex Shader ...  Assign location to variable
    modelLoc = glGetUniformLocation(triforceShaderProgram, "model");
    viewLoc = glGetUniformLocation(triforceShaderProgram, "view");
    projLoc = glGetUniformLocation(triforceShaderProgram, "projection");
    // Reference matrix uniforms from the triforce's Fragment Shader ...  Assign location to variable
    uTextureLoc = glGetUniformLocation(triforceShaderProgram, "uTexture");
    lightColorLoc = glGetUniformLocation(triforceShaderProgram, "lightColor");
    lightPositionLoc = glGetUniformLocation(triforceShaderProgram, "lightPos");
    viewPositionLoc = glGetUniformLocation(triforceShaderProgram, "viewPosition");

    // Pass matrix data to the triforce's Vertex Shader matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    // Pass matrix data to the triforce's Fragment Shader matrix uniforms
    glUniform1i(uTextureLoc, 0); // (texture unit 0)
    glUniform3f(lightColorLoc, lightColor.r, lightColor.g, lightColor.b);
    glUniform3f(lightPositionLoc, lightPosition.x, lightPosition.y, lightPosition.z);
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    // Activate texture unit ... Bind texture to texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);


     /* TOGGLE POLYGON MODE for WIREFRAME or VERTICES or DEFAULT (default = FILLED/TEXTURED) */
    // toggles Wireframe ('w') to see lines drawn
    if(toggleWireframe == 0){
    	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Wireframe OFF

    	// Vertices can be toggled when Wireframe = OFF
    	if(toggleVertices == 0){
        	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Vertices OFF
        }
        else{
        	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // Vertices ON
        }
    }
    else{
    	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe ON
    }


    // DRAW PRIMITIVE (TRIFORCE)
    glDrawArrays(GL_TRIANGLES, 0, 72);
    glBindVertexArray(0); // Deactivate the Triforce Vertex Array Object




  /* LAMP SHADER PROGRAM */
    glUseProgram(lampShaderProgram);
    glBindVertexArray(LightVAO); // activate lamp vertex array object for rendering/transforming

    // Transform the lamp used as a visual cue for the light source
    model = glm::translate(model, lightPosition);
    model = glm::scale(model, lightScale);

    // Reference matrix uniforms from the lamp's Vertex Shader ... Assign location to variable
    modelLoc = glGetUniformLocation(lampShaderProgram, "model");
    viewLoc = glGetUniformLocation(lampShaderProgram, "view");
    projLoc = glGetUniformLocation(lampShaderProgram, "projection");

    // Pass matrix data to the lamp's Vertex Shader matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


    // DRAW PRIMITIVE (LAMP)
    glDrawArrays(GL_TRIANGLES, 0, 72);

    glBindVertexArray(0); // Deactivate the Lamp Vertex Array Object

    glutPostRedisplay(); // signals main loop to draw the next frame
    glutSwapBuffers(); // Flips the back buffer with the front buffer every frame. Similar to GL Flush

}


/* Create the Shader program */
void UCreateShader()
{
    /* Source code credit for 'compile log checks' (and link check) goes to Joey DeVries (learnopengl.com)
     * https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/2.5.hello_triangle_exercise3/hello_triangle_exercise3.cpp
     */

	// initialize variables for shader compile log checks
	int success;
	char infoLog[512];

  /* TRIFORCE */
    // Vertex Shader
    GLint triforceVertexShader = glCreateShader(GL_VERTEX_SHADER); // Creates the Vertex shader
    glShaderSource(triforceVertexShader, 1, &triforceVertexShaderSource, NULL); // Attaches the Vertex shader to the source code
    glCompileShader(triforceVertexShader); // Compiles the Vertex shader
    // vertex shader compile log check:
	glGetShaderiv(triforceVertexShader, GL_COMPILE_STATUS, &success);
	if (!success){
	    glGetShaderInfoLog(triforceVertexShader, 512, NULL, infoLog);
	    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

    // Fragment Shader
    GLint triforceFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment Shader
    glShaderSource(triforceFragmentShader, 1, &triforceFragmentShaderSource, NULL); // Attaches the Fragment shader to the source code
    glCompileShader(triforceFragmentShader); // Compiles the Fragment Shader
    // fragment shader compile log check:
    glGetShaderiv(triforceFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(triforceFragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Shader program
    triforceShaderProgram = glCreateProgram(); // Creates the Shader program and returns an id
    glAttachShader(triforceShaderProgram, triforceVertexShader); // Attaches Vertex shader to the Shader program
    glAttachShader(triforceShaderProgram, triforceFragmentShader); // Attaches Fragment shader to the Shader program
    glLinkProgram(triforceShaderProgram); // Link Vertex and Fragment shaders to the Shader program
    // shader program link check:
    glGetProgramiv(triforceShaderProgram, GL_LINK_STATUS, &success);
    if (!success){
        glGetProgramInfoLog(triforceShaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Delete the triforce Vertex and Fragment shaders once linked
    glDeleteShader(triforceVertexShader);
    glDeleteShader(triforceFragmentShader);


  /* LAMP */
    // Vertex Shader
    GLint lampVertexShader = glCreateShader(GL_VERTEX_SHADER); // Creates the Vertex shader
    glShaderSource(lampVertexShader, 1, &lampVertexShaderSource, NULL); // Attaches the Vertex shader to the source code
    glCompileShader(lampVertexShader); // Compiles the Vertex shader

    // Fragment Shader
    GLint lampFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment shader
    glShaderSource(lampFragmentShader, 1, &lampFragmentShaderSource, NULL); // Attaches the Fragment shader to the source code
    glCompileShader(lampFragmentShader); // Compiles the Fragment shader

    // Shader Program
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

//		//Positions             //Normals               //Texture Coordinates
//
//		//Back Face             //Negative Z Normals
//		 0.0f,  0.5f,  0.0f,     0.0f,  0.0f, -1.0f,    0.5f, 1.0f,
//		 0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f,
//		-0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,    1.0f, 0.0f,
//
//		//Front Face            //Positive Z Normals
//		 0.0f,  0.5f,  0.0f,     0.0f,  0.0f,  1.0f,    0.5f, 1.0f,
//		-0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f,
//		 0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,    1.0f, 0.0f,
//
//		 //Left Face            //Positive X Normals
//		 0.0f,  0.5f,  0.0f,     1.0f,  0.0f,  0.0f,    0.5f, 1.0f,
//		-0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
//		-0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
//
//		 //Right Face           //Negative X Normals
//		 0.0f,  0.5f,  0.0f,    -1.0f,  0.0f,  0.0f,    0.5f, 1.0f,
//		 0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
//		 0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
//
//		 //Bottom Face          //Negative Y Normals
//		-0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,    0.0f, 1.0f,
//		 0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,    0.0f, 0.0f,
//		-0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,    1.0f, 1.0f,
//		-0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,    1.0f, 1.0f,
//		 0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,    0.0f, 0.0f,
//		 0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,    1.0f, 0.0f,
//
//    };

		//	Vertex data           // Normals             // Texture Coordinates

    /* FRONT TRIFORCE */
		// FRONT: Bottom Left triangle
		 -0.5f, -0.5f,  0.0f,     0.0f,  0.0f,  0.0f,    0.0f, 0.0f,
		  0.0f, -0.5f,  0.0f,	  0.0f,  0.0f,  0.0f,    0.0f, 0.0f,
		-0.25f,  0.0f,  0.0f,	  0.0f,  0.0f,  0.0f,    0.0f, 0.0f,

		// FRONT: Bottom Right triangle
		  0.0f, -0.5f,  0.0f,	  0.0f,  0.0f,  0.0f,    0.0f, 0.0f,
		  0.5f, -0.5f,  0.0f,	  0.0f,  0.0f,  0.0f,    0.0f, 0.0f,
		 0.25f,  0.0f,  0.0f,	  0.0f,  0.0f,  0.0f,    0.0f, 0.0f,

		// FRONT: Top triangle
		 0.25f,  0.0f,  0.0f,	  0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    1.0f, 0.73f, 0.0f,  // v6
		  0.0f,  0.5f,  0.0f,	  0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    1.0f, 0.73f, 0.0f,  // v7
		-0.25f,  0.0f,  0.0f,	  0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    1.0f, 0.73f, 0.0f,  // v8

    /* BACK TRIFORCE */
		// BACK: Bottom Left Triangle
		 -0.5f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v9
		  0.0f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v10
	    -0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v11

		// BACK: Bottom Right Triangle
		  0.0f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v12
		  0.5f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v13
	     0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v14


		// BACK: Top Triangle
		 0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v15
		  0.0f,  0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v16
	    -0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v17


  /* TRIFORCE SIDES: ... LEFT ... BOTTOM ... RIGHT ...  */

    /* BOTTOM LEFT TRIANGLE */
		// LEFT SIDE
		 -0.5f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v18
		 -0.5f, -0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v19
	    -0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v20

		 -0.5f, -0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v21
		-0.25f,  0.0f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v22
	    -0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v23

		// BOTTOM SIDE
		 -0.5f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v24
		 -0.5f, -0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v25
	      0.0f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v26

		 -0.5f, -0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v27
		  0.0f, -0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v28
	      0.0f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v29

		// RIGHT SIDE
		  0.0f, -0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v30
		  0.0f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v31
	    -0.25f,  0.0f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v32

		  0.0f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v33
	    -0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v34
	    -0.25f,  0.0f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v35

    /* BOTTOM RIGHT TRIANGLE */
		// LEFT SIDE
		  0.0f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v36
		  0.0f, -0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v37
	     0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v38

		  0.0f, -0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v39
		 0.25f,  0.0f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v40
	     0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v41

		// BOTTOM SIDE
		  0.0f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v42
		  0.0f, -0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v43
	      0.5f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v44

		  0.0f, -0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v45
		  0.5f, -0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v46
	      0.5f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v47

		// RIGHT SIDE
		  0.5f, -0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v48
		  0.5f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v49
	     0.25f,  0.0f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v50

		  0.5f, -0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v51
		 0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v52
	     0.25f,  0.0f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v53

    /* TOP TRIANGLE */
		// LEFT SIDE
		-0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v54
		-0.25f,  0.0f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v55
	      0.0f,  0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v56

		-0.25f,  0.0f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v57
		  0.0f,  0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v58
	      0.0f,  0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v59

		// BOTTOM SIDE
		-0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v60
		-0.25f,  0.0f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v61
	     0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v62

		-0.25f,  0.0f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v63
		 0.25f,  0.0f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v64
	     0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v65

		// RIGHT SIDE
		 0.25f,  0.0f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v66
		 0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v67
	      0.0f,  0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v68

		 0.25f,  0.0f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v69
		  0.0f,  0.5f, -0.125f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f,    //1.0f, 0.73f, 0.0f,  // v70
		  0.0f,  0.5f,    0.0f,   0.0f,  0.0f,  0.0f,    0.0f, 0.0f     //1.0f, 0.73f, 0.0f   // v71

	};


    // Generate buffer ids
    glGenVertexArrays(1, &TriforceVAO);
    glGenBuffers(1, &VBO);

    // Activate the TriforceVAO before binding and setting VBOs and VAPs
    glBindVertexArray(TriforceVAO);

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

    glBindVertexArray(0); // deactivate the triforce VAO

    // Generate buffer ids for lamp
    glGenVertexArrays(1, &LightVAO); // Vertex Array for triforce vertex copies to serve as light source

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

    // set texture filter options for currently bound texture object
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // initialize image dimensions
    int width, height;

    // load JPEG image and assign to 'image' variable
    unsigned char* image = SOIL_load_image("texture_71_gold_1940pixels.jpg", &width, &height, 0, SOIL_LOAD_RGB); // Loads texture file

    // use image to generate texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D); // creates mipmaps of varying resolutions to account for further viewing distances
    SOIL_free_image_data(image); // frees the image memory (called from SOIL2.h)
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind/deactivate the texture
}



/* implements the UKeyboard function */
void UKeyboard(unsigned char key, GLint x, GLint y){

	// Keyboard Input options (standard keys)
	switch(key){

		// Help menu
		case 'h':
		case 'H':
			cout << "\n\nHELP MENU:\n\n"
					"ALT + Left Mouse Button + Mouse Movement \n"
					"will Orbit the camera around the object. \n\n"
					"ALT + Right Mouse Button + Mouse Upward Movement \n"
					"will Zoom Out. \n\n"
					"ALT + Right Mouse Button + Mouse Downward Movement \n"
					"will Zoom In. \n\n"
					"Press 'P' to toggle camera Projections (Perspective vs. Ortho)\n"
					"Default Projection = Perspective\n\n"
			        "Press 'V' to toggle Vertices ON and OFF (Note: Wireframe must be OFF).\n\n"
			        "Press 'W' to toggle Wireframe ON and OFF (Note: Vertices must be OFF).\n\n"
					"Press 'Q' to Quit the program. \n" << endl;
			break;

		// Projection Matrix toggle
		case 'p':
		case 'P':
			// toggles between 1 and 0 for 'toggleProjection' value, depending on its assigned value when key is pressed (program default = 1)
			toggleProjection = abs(toggleProjection - 1);

			// the value of toggleProjection (0 or 1) directly impacts the "Projection Matrix Toggle" branch statement in the URenderGraphics function
			if(toggleProjection == 1){
				cout << "Projection matrix is set to Perspective" << endl; // value of 1 triggers Projection matrix in URenderGraphics function
			}
			else{
				cout << "Projection matrix is set to Ortho" << endl; // value of anything other than 1 (such as 0) triggers Ortho matrix in URenderGraphics function
			}
			break;

		// Vertex Polygon Mode toggle
		case 'v':
		case 'V':
			// ensures that toggleWireframe is not ON, otherwise toggleVertices cannot switch on
			if(toggleWireframe == 1){
				cout << "\nUnable to toggle Vertices while Wireframe is on.\n"
						"Press 'w' to toggle wireframe off, then try again.\n" << endl;
			}
			else{
				// toggles between 1 and 0 for 'toggleVertices' value, depending on its assigned value when key is pressed (program default = 0)
			    toggleVertices = abs(toggleVertices - 1);

				// the value of toggleVertices (0 or 1) directly impacts the "TOGGLE POLYGON MODE" vertices branch statement in the URenderGraphics function
				if(toggleVertices == 1){
					cout << "Vertices turned ON" << endl; // value of 1 triggers GL_POINT (draws only vertices) in the URenderGraphics function
				}
	            else{
				    cout << "Vertices turned OFF" << endl; // value of anything other than 1 (such as 0) triggers GL_FILL (colors complete object) in the URenderGraphics function
		        }
			}
			break;

		// Wireframe Polygon Mode toggle
		case 'w':
		case 'W':
			// ensures that toggleVertices is not ON, otherwise toggleWireframe cannot switch on
			if(toggleVertices == 1){
				cout << "\nUnable to toggle Wireframe while Vertices is on.\n"
						"Press 'v' to toggle vertices off, then try again.\n" << endl;
			}
			else{
				// toggles between 1 and 0 for 'toggleWireframe' value, depending on its assigned value when key is pressed	(program default = 0)
			    toggleWireframe = abs(toggleWireframe - 1);

				// the value of toggleWireframe (0 or 1) directly impacts the "TOGGLE POLYGON MODE" wireframe branch statement in the URenderGraphics function
				if(toggleWireframe == 1){
					cout << "Wireframe turned ON" << endl; // value of 1 triggers GL_LINE (draw only lines) in the URenderGraphics function
				}
	            else{
				    cout << "Wireframe turned OFF" << endl; // value of anything other than 1 (such as 0) triggers GL_FILL (colors complete object) in the URenderGraphics function
		        }
			}
			break;

		// End Program
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
	mod = glutGetModifiers(); // assign glutGetModifiers() to variable "mod"

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

			// maintains a 90 degree pitch for GIMBAL LOCK
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









