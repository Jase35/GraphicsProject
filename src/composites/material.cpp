#include "material.hpp"

namespace engine{
    Material::Material(
        Handle<Shader> shader,
        const vec4 baseColor, 
        Image textureImage
    ) : shader(shader), baseColor(baseColor), textureImage(textureImage) {

        if(textureImage.data){
            glGenTextures(1, &textureGLId);
            glBindTexture(GL_TEXTURE_2D, textureGLId);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            if(textureImage.data){
                glTexImage2D(
                    GL_TEXTURE_2D, 
                    0, 
                    GL_RGBA, 
                    textureImage.width, 
                    textureImage.height, 
                    0, 
                    GL_RGBA, 
                    GL_UNSIGNED_BYTE, 
                    textureImage.data
                );
                glGenerateMipmap(GL_TEXTURE_2D);
            }else{
                printf("Failed to create texture.\n");
            }
        }
    }

    Material::~Material(){
        if(textureGLId != 0){
            glDeleteTextures(1, &textureGLId);
        }
    }

    Material::Material(Material&& other) noexcept : 
        shader(other.shader),
        baseColor(other.baseColor),
        textureImage(other.textureImage),
        materialGLId(other.materialGLId),
        textureGLId(other.textureGLId)
    {
        other.materialGLId = 0;
        other.textureGLId = 0;
        other.textureImage = {};
    }

    Material& Material::operator=(Material&& other) noexcept{
        if(this != &other){
            if(textureGLId != 0){
                glDeleteTextures(1, &textureGLId);
            }

            shader = other.shader;
            baseColor = other.baseColor;
            textureImage = other.textureImage;
            materialGLId = other.materialGLId;
            textureGLId = other.textureGLId;

            other.materialGLId = 0;
            other.textureGLId = 0;
            other.textureImage = {};
        }
        return *this;
    }

    void Material::bind() const{
        glUniform4fv(1, 1, &baseColor[0]);
        glUniform1i(2, textureGLId != 0);
        if(textureGLId != 0){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureGLId);
        }
    }
}