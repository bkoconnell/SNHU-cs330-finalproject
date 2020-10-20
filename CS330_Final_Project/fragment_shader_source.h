/*
 * fragment_shader_source.h
 *
 *  Created on: Oct 20, 2020
 *      Author: bkoco
 */

#include <GL/glew.h>

// shader program macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif


#ifndef FRAGMENT_SHADER_SOURCE_H_
#define FRAGMENT_SHADER_SOURCE_H_

/* triforce Fragment Shader SOURCE CODE */
const GLchar * triforceFragmentShaderSource = GLSL(330,

        in vec3 FragPos; // incoming fragment position
        in vec3 Normal; // incoming normals
        in vec2 mobileTextureCoordinate; // incoming texture coordinate

        out vec4 triforceColor; // outgoing triforce color/lighting to the GPU

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


#endif /* FRAGMENT_SHADER_SOURCE_H_ */
