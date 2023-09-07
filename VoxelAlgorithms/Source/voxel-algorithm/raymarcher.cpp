#include "raymarcher.h"

#include "../timer.h"
#include "../gl.h"
#include "../mesh.h"
#include "../orbit-camera.h"

Raymarcher::Raymarcher(VoxelChunk* chunk) : mChunk(chunk), mShader(~0u)
{
    mStepCount = mChunk->getNumVoxel() * 2;
}

void Raymarcher::initialize()
{
    GLuint vs = gl::createShader("Assets/Shaders/raymarch.vert");
    GLuint fs = gl::createShader("Assets/Shaders/raymarch.frag");
    GLuint shaders[] = { vs, fs };
    mShader = gl::createProgram(shaders, 2);
    gl::destroyShader(vs);
    gl::destroyShader(fs);
}

void Raymarcher::render(OrbitCamera* camera)
{
    FullScreenQuad* fsQuad = FullScreenQuad::getInstance();
    glUseProgram(mShader);
    glBindTexture(GL_TEXTURE_3D, mChunk->getDensityTexture());

    glm::mat4 P = camera->getProjectionMatrix();
    glm::mat4 V = camera->getViewMatrix();
    glm::vec3 cameraPosition = camera->getPosition();
    glm::mat4 inverseVP = glm::inverse(P * V);

	glUniformMatrix4fv(glGetUniformLocation(mShader, "P"), 1, GL_FALSE, &P[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(mShader, "V"), 1, GL_FALSE, &V[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(mShader, "uInverseVP"), 1, GL_FALSE, &inverseVP[0][0]);
    glUniform3fv(glGetUniformLocation(mShader, "uCamPos"), 1, &cameraPosition[0]);
    glUniform1i(glGetUniformLocation(mShader, "uTexture"), 0);
    glUniform1f(glGetUniformLocation(mShader, "uAspect"), mWidth / float(mHeight));
    glUniform1i(glGetUniformLocation(mShader, "uStepCount"), mStepCount);
    glUniform1i(glGetUniformLocation(mShader, "uVoxelCount"), mChunk->getNumVoxel());

    float elapsedTime = (float)Timer::getInstance()->getElapsedSeconds();
    glUniform1f(glGetUniformLocation(mShader, "uTime"), elapsedTime);
	fsQuad->draw();
    glUseProgram(0);
}

void Raymarcher::destroy()
{
    gl::destroyProgram(mShader);
}
