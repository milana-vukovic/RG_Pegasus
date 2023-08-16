//
// Created by matf-rg on 30.10.20..
//

#ifndef PROJECT_BASE_TEXTURE2D_H
#define PROJECT_BASE_TEXTURE2D_H
#include <glad/glad.h>
#include <stb_image.h>
#include <rg/Error.h>

class Texture2D {
    unsigned int m_Id;
public:
    Texture2D(std::string pathToImg, GLenum filtering, GLenum sampling){

        glGenTextures(1, &m_Id);
        glBindTexture(GL_TEXTURE_2D, m_Id);

        //wrap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,  sampling);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  sampling);

        //filter
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);

        //load img 1
        int width, height, nChannel;
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load(pathToImg.c_str(), &width, &height, &nChannel, 0);
        if(data) {

            GLenum format;
            if(nChannel == 1){
                format = GL_RED;
            }
            else if(nChannel == 3){
                format = GL_RGB;
            }
            else if(nChannel == 4){
                format = GL_RGBA;
            }


            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }else{
            ASSERT(false, "Nije ucitana tekstura\n");
        }
        stbi_image_free(data);
    }
    void activeTexture(GLenum e){
        glActiveTexture(e);
    }
    void bindTexture(){
        glBindTexture(GL_TEXTURE_2D, m_Id);
    }
};

#endif //PROJECT_BASE_TEXTURE2D_H
