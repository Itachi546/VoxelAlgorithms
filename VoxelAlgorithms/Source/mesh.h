#pragma once

#include "gl.h"

#include "glm-includes.h"

#include <vector>

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
};

struct Vertex2D {
    glm::vec2 position;
};

struct GpuMesh {
    uint32_t vertexBuffer;
    uint32_t indexBuffer;

    uint32_t vertexCount;
    uint32_t indexCount;

    void create(uint32_t vertexSize, uint32_t indexSize) {
		vertexBuffer = gl::createBuffer(nullptr, vertexSize, 0);
		indexBuffer = gl::createBuffer(nullptr, indexSize, 0);
    }

    void draw() {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

    void destroy() {
		gl::destroyBuffer(vertexBuffer);
        gl::destroyBuffer(indexBuffer);
        indexCount = 0;
    }
};

struct Mesh {
    uint32_t vertexBuffer;
    uint32_t indexBuffer;
    uint32_t indexCount;

    uint32_t vao;

    Mesh() : vertexBuffer(~0u), indexBuffer(~0u), indexCount(0), vao(~0u) {}

    void create(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    {
        vertexBuffer = gl::createBuffer(vertices.data(), (uint32_t)vertices.size() * sizeof(Vertex), GL_DYNAMIC_STORAGE_BIT);
        indexBuffer = gl::createBuffer(indices.data(), (uint32_t)indices.size() * sizeof(uint32_t), GL_DYNAMIC_STORAGE_BIT);
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
        indexCount = 0;
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

class DefaultMesh {

public:
    static DefaultMesh* getInstance() {
        static DefaultMesh* defaultMesh = new DefaultMesh();
        return defaultMesh;
    }

    Mesh* getSphere() {
        return &mSphere;
    }

private:
    Mesh mSphere;

    DefaultMesh() {
        initializeSphere();
    }

    void initializeSphere(int sectorCount = 16, int stackCount = 8) {

        float radius = 1.0f;
        float x, y, z, xy;
        float nx, ny, nz, lengthInv = 1.0f / radius;

        float sectorStep = 2 * glm::pi<float>() / sectorCount;
        float stackStep = glm::pi<float>() / stackCount;
        float sectorAngle, stackAngle;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        for (int i = 0; i <= stackCount; ++i)
        {
            stackAngle = glm::pi<float>() / 2 - i * stackStep;
            xy = radius * cosf(stackAngle);
            z = radius * sinf(stackAngle);

            for (int j = 0; j <= sectorCount; ++j)
            {
                sectorAngle = j * sectorStep;

                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);

                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                vertices.emplace_back(Vertex{ glm::vec3(x, y, z), glm::vec3(nx, ny, nz) });
            }
        }

        int k1, k2;
        for (int i = 0; i < stackCount; ++i)
        {
            k1 = i * (sectorCount + 1);
            k2 = k1 + sectorCount + 1;
            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                if (i != 0)
                {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                if (i != (stackCount - 1))
                {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }

        mSphere.create(vertices, indices);
    }
};

