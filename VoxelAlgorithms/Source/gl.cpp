#include "gl.h"

#ifdef _MSC_VER
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #include <debugapi.h>
#endif

namespace gl {

    static std::string readFile(const std::string& filename)
    {
        std::ifstream fileStream(filename);
        if (!fileStream)
        {
            std::cerr << "Failed to load file " << filename << std::endl;
            assert(0);
        }

        return std::string{
          (std::istreambuf_iterator<char>(fileStream)),
          std::istreambuf_iterator<char>()
        };
    }


    static std::string getFileExtension(const std::string& filename)
    {
        return filename.substr(filename.find_last_of('.') + 1, filename.size());
    }

    static GLenum getShaderTypeFromExtension(const std::string& extension) {
        if (extension == "vert")
            return GL_VERTEX_SHADER;
        else if (extension == "frag")
            return GL_FRAGMENT_SHADER;
        else if (extension == "comp")
            return GL_COMPUTE_SHADER;

        std::cerr << extension << std::endl;
        assert(!"Undeclared shader type");
        return ~0u;
    }

    GLuint createShader(const std::string& filename) {
        std::string extension = getFileExtension(filename);
        GLenum shaderType = getShaderTypeFromExtension(extension);
        std::string shaderCode = readFile(filename);

        const char* code = shaderCode.c_str();
        GLuint handle = glCreateShader(shaderType);
        glShaderSource(handle, 1, &code, nullptr);
        glCompileShader(handle);

        char errBuffer[4096];
        GLsizei errLength = 0;
        glGetShaderInfoLog(handle, 4096, &errLength, errBuffer);

        if (errLength > 0) {
            std::cout << errBuffer << std::endl;
            std::cout << filename << std::endl;
#ifdef _MSC_VER
            OutputDebugStringA(errBuffer);
#endif
            assert(0);
        }
        return handle;
    }

    GLuint createProgram(GLuint* shaders, uint32_t count) {
        GLuint program = glCreateProgram();
        for (uint32_t i = 0; i < count; ++i)
            glAttachShader(program, shaders[i]);

        glLinkProgram(program);

        char errBuffer[4096];
        GLsizei errLength = 0;
        glGetProgramInfoLog(program, sizeof(errBuffer), &errLength, errBuffer);
        if (errLength > 0)
        {
            std::cout << errBuffer << std::endl;
#ifdef _MSC_VER
            OutputDebugStringA(errBuffer);
#endif
            assert(!"Failed to link program");
        }
        return program;
    }

    GLuint createTexture(const TextureParams& params)
    {
        const GLenum target = params.target;

        GLuint textureId = ~0u;
        glGenTextures(1, &textureId);
        glBindTexture(target, textureId);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, params.filter);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, params.filter);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, params.wrapMode);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, params.wrapMode);
        glTexParameteri(target, GL_TEXTURE_WRAP_R, params.wrapMode);

        if (target == GL_TEXTURE_2D)
            glTexImage2D(target, 0, params.internalFormat, params.width, params.height, 0, params.format, params.type, params.data);
        else if (target == GL_TEXTURE_3D)
            glTexImage3D(target, 0, params.internalFormat, params.width, params.height, params.depth, 0, params.format, params.type, params.data);
        glBindTexture(target, 0);
        return textureId;
    }
}