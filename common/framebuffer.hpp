#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <glad/glad.h>
#include <textures/texture2d.hpp>

class FrameBuffer {
private:
    GLuint id;

    //Delete copy constructor and assignment operation
    FrameBuffer(FrameBuffer const &) = delete;
    FrameBuffer & operator =(FrameBuffer const &) = delete;
public:
    FrameBuffer(){glGenFramebuffers(1, &id);}
    ~FrameBuffer(){glDeleteFramebuffers(1, &id);}

    //Bind the fbo to the target GL_FRAMEBUFFER
    void bind(){glBindFramebuffer(GL_FRAMEBUFFER, id);}

    void attach(Texture2D* texture, GLenum attachment){
        this->bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->getID(), 0);
    }

    bool isComplete(){
        this->bind();
        return glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE;
    }

	GLuint getID() { return id; }

    static void unbind(){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
   
};

#endif