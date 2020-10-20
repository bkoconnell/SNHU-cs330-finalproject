/*
 * vertex_shader_source.h
 *
 *  Created on: Oct 20, 2020
 *      Author: bkoco
 */

#include <GL/glew.h>

// shader program macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif


#ifndef VERTEX_SHADER_SOURCE_H_
#define VERTEX_SHADER_SOURCE_H_

/* triforce Vertex Shader SOURCE CODE */
const GLchar * triforceVertexShaderSource = GLSL(330,
        layout (location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
        layout (location = 1) in vec3 normal; // VAP position 1 for normals (lighting)
        layout (location = 2) in vec2 textureCoordinate; // texture data from vertex attrib pointer 2

        out vec3 FragPos; // For outgoing fragment / pixels to fragment shader
        out vec3 Normal; // For outgoing normals to fragment shader
        out vec2 mobileTextureCoordinate; // variable to transfer texture coordinate data to the fragment shader

        // Uniform variables for the transform matrices
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main(){
        	gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
            FragPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)
            Normal = mat3(transpose(inverse(model))) *  normal; // get normal vectors in world space only and exclude normal translation properties
            mobileTextureCoordinate = vec2(textureCoordinate.x, 1 - textureCoordinate.y); // flips the texture horizontal
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


#endif /* VERTEX_SHADER_SOURCE_H_ */
