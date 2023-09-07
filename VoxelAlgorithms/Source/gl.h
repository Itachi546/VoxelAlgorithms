#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>
#include <stdint.h>
#include <iostream>
#include <assert.h>
#include <fstream>

namespace gl {

    GLuint createShader(const std::string& filename);

    inline void destroyShader(GLuint shader) {
        glDeleteShader(shader);
    }

    GLuint createProgram(GLuint* shaders, uint32_t count);

    inline void destroyProgram(GLuint program) {
        glDeleteProgram(program);
    }


    inline GLuint createBuffer(void* data, uint32_t size, GLbitfield flags)
    {
        GLuint handle = ~0u;
        glCreateBuffers(1, &handle);
        glNamedBufferStorage(handle, size, data, flags);
        return handle;
    }

    inline void destroyBuffer(GLuint buffer) {
        glDeleteBuffers(1, &buffer);
    }

    inline GLuint createVertexArray() {
        GLuint vao = ~0u;
        glGenVertexArrays(1, &vao);
        return vao;
    }

    inline void destroyVertexArray(GLuint vao) {
        glDeleteVertexArrays(1, &vao);
    }

    struct TextureParams {
        GLenum wrapMode;
        GLenum filter;
        GLenum format;
        GLenum internalFormat;
        GLenum type;
        GLenum target;
        
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t nChannel;

        void* data;
    };

    inline void initializeTextureParams(TextureParams& params) {
        params.wrapMode = GL_REPEAT;
        params.filter = GL_LINEAR;
        params.format = GL_RGBA;
        params.internalFormat = GL_RGBA8;
        params.type = GL_UNSIGNED_BYTE;
        params.target = GL_TEXTURE_2D;

		params.width = 256;
		params.height = 256;
        params.depth = 1;
        params.nChannel = 4;

		params.data = nullptr;
    }

    GLuint createTexture(const TextureParams& param);

	inline void lockBuffer(GLsync& sync)
    {
        if (sync)
            glDeleteSync(sync);
        sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    inline void waitBuffer(GLsync& sync)
    {
        if (sync)
        {
            while (1)
            {
                GLenum waitReturn = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
                if (waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED)
                    return;
            }
        }
    }
};
