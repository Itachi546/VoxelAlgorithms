#pragma once

#include "gl.h"

#define GLM_FORCE_XYZW_ONLY
#include <glm/glm.hpp>
#include <vector>

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
};

struct Vertex2D {
    glm::vec2 position;
};

struct Mesh {
    uint32_t vertexBuffer;
    uint32_t indexBuffer;
    uint32_t indexCount;

    uint32_t vao;

    Mesh() : vertexBuffer(~0u), indexBuffer(~0u), indexCount(0), vao(~0u) {}

    void create(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    {
        vertexBuffer = gl::createBuffer(vertices.data(), (uint32_t)vertices.size() * sizeof(Vertex), 0);
        indexBuffer = gl::createBuffer(indices.data(), (uint32_t)indices.size() * sizeof(uint32_t), 0);
        indexCount = (uint32_t)indices.size();

        vao = gl::createVertexArray();
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));
        glBindVertexArray(0);
    }

    void create(std::vector<Vertex2D> vertices, std::vector<uint32_t> indices)
    {
        vertexBuffer = gl::createBuffer(vertices.data(), (uint32_t)vertices.size() * sizeof(Vertex2D), 0);
        indexBuffer = gl::createBuffer(indices.data(), (uint32_t)indices.size() * sizeof(uint32_t), 0);
        indexCount = (uint32_t)indices.size();

        vao = gl::createVertexArray();
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
        glBindVertexArray(0);
    }

    void draw() {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void destroy() {
        gl::destroyVertexArray(vao);
        gl::destroyBuffer(vertexBuffer);
        gl::destroyBuffer(indexBuffer);
    }


};

struct FullScreenQuad {

    static FullScreenQuad* getInstance() {
        static FullScreenQuad* quad = new FullScreenQuad();
        return quad;
    }

    void initialize() {
        if (mInitialized) return;
        // Create Mesh
        std::vector<Vertex2D> vertices = {
          Vertex2D{{-1, -1}},
          Vertex2D{{-1,  1}},
          Vertex2D{{ 1,  1}},
          Vertex2D{{ 1, -1}},
        };

        std::vector<uint32_t> indices = { 0, 2, 1, 0, 3, 2 };
        mQuad.create(vertices, indices);

        mInitialized = true;
    }

    void draw() {
        mQuad.draw();
    }

    void destroy() {
        mQuad.destroy();
    }
private:
    FullScreenQuad() = default;

    Mesh mQuad;
    bool mInitialized = false;
};

