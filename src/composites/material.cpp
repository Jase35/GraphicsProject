#include "material.hpp"

namespace engine{
    namespace{
        typedef GLuint64 (GLAD_API_PTR *PFN_glGetTextureHandleARB)(GLuint texture);
        typedef void (GLAD_API_PTR *PFN_glMakeTextureHandleResidentARB)(GLuint64 handle);
        typedef void (GLAD_API_PTR *PFN_glMakeTextureHandleNonResidentARB)(GLuint64 handle);

        PFN_glGetTextureHandleARB getTextureHandle = nullptr;
        PFN_glMakeTextureHandleResidentARB makeTextureHandleResident = nullptr;
        PFN_glMakeTextureHandleNonResidentARB makeTextureHandleNonResident = nullptr;
    }

    bool loadBindlessTextureFunctions(){
        if(!glfwExtensionSupported("GL_ARB_bindless_texture")){
            return false;
        }

        getTextureHandle = reinterpret_cast<PFN_glGetTextureHandleARB>(glfwGetProcAddress("glGetTextureHandleARB"));
        makeTextureHandleResident = reinterpret_cast<PFN_glMakeTextureHandleResidentARB>(glfwGetProcAddress("glMakeTextureHandleResidentARB"));
        makeTextureHandleNonResident = reinterpret_cast<PFN_glMakeTextureHandleNonResidentARB>(glfwGetProcAddress("glMakeTextureHandleNonResidentARB"));

        return getTextureHandle && makeTextureHandleResident && makeTextureHandleNonResident;
    }

    Material::Material(
        Handle<Shader> shader,
        const vec4 baseColor, 
        Image textureImage,
        MaterialType type
    ) : shader(shader), baseColor(baseColor), type(type), textureImage(textureImage) {

        if(textureImage.data){
            glGenTextures(1, &textureGLId);
            glBindTexture(GL_TEXTURE_2D, textureGLId);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

            if(getTextureHandle){
                textureHandle = getTextureHandle(textureGLId);
                makeTextureHandleResident(textureHandle);
            }else{
                printf("Bindless textures unavailable; texture will be ignored.\n");
            }
        }
    }

    Material::~Material(){
        release();
    }

    void Material::release(){
        if(textureHandle != 0){
            makeTextureHandleNonResident(textureHandle);
            textureHandle = 0;
        }
        if(textureGLId != 0){
            glDeleteTextures(1, &textureGLId);
            textureGLId = 0;
        }
    }

    Material::Material(Material&& other) noexcept : 
        shader(other.shader),
        baseColor(other.baseColor),
        type(other.type),
        textureImage(other.textureImage),
        textureGLId(other.textureGLId),
        textureHandle(other.textureHandle)
    {
        other.textureGLId = 0;
        other.textureHandle = 0;
        other.textureImage = {};
    }

    Material& Material::operator=(Material&& other) noexcept{
        if(this != &other){
            release();

            shader = other.shader;
            baseColor = other.baseColor;
            type = other.type;
            textureImage = other.textureImage;
            textureGLId = other.textureGLId;
            textureHandle = other.textureHandle;

            other.textureGLId = 0;
            other.textureHandle = 0;
            other.textureImage = {};
        }
        return *this;
    }
}