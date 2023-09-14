#include "gl.h"

#ifdef _MSC_VER
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #include <debugapi.h>
#endif

namespace gl {

    static void readFile(const std::string& filename, std::vector<unsigned char>& code)
    {
        std::ifstream inFile(filename, std::ios::binary);

        inFile.unsetf(std::ios::skipws);

        // get its size:
        std::streampos fileSize;
        inFile.seekg(0, std::ios::end);
        fileSize = inFile.tellg();
        inFile.seekg(0, std::ios::beg);

        code.reserve(fileSize);

        // read the data:
        code.insert(code.begin(),
            std::istream_iterator<unsigned char>(inFile),
            std::istream_iterator<unsigned char>());
    }


    static std::string getShaderType(const std::string& filename)
    {
        std::string trimmedSpvExt = filename.substr(0, filename.find_last_of('.'));
        return trimmedSpvExt.substr(trimmedSpvExt.find_last_of('.') + 1, trimmedSpvExt.length());
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
        std::string extension = getShaderType(filename);
        GLenum shaderType = getShaderTypeFromExtension(extension);

        std::vector<unsigned char> shaderCode;
		readFile(filename, shaderCode);

        if (shaderCode.size() == 0) {
            std::cerr << "Failed to load file: " << filename << std::endl;
            return 0;
        }

		GLuint handle = glCreateShader(shaderType);
        //glShaderSource(handle, 1, &code, nullptr);
		//glCompileShader(handle);
        glShaderBinary(1, &handle, GL_SHADER_BINARY_FORMAT_SPIR_V, shaderCode.data(), (uint32_t)shaderCode.size());
        glSpecializeShader(handle, "main", 0, 0, 0);

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