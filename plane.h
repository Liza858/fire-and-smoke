#include "opengl_shader.h"


class Plane {

    private:

    float triangles_vertices[3*4 + 2*4] = {
        -10.0f, -10.0f, 0.0,
        0.0f, 0.0f,
        -10.0f, 10.0f, 0.0,
        0.0f, 1.0f,
         10.0f, 10.0f, 0.0,
         1.0f, 1.0f,
         10.0f, -10.0f, 0.0,
         1.0f, 0.0f,
    };

    const unsigned int triangles_indices[36] = { 0, 1, 2, 2, 3, 0 };

    GLuint vbo;
    GLuint vao;
    GLuint ebo;


    public:

    Plane() {

        GLuint vbo, vao, ebo;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangles_vertices), triangles_vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangles_indices), triangles_indices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        this->vbo = vbo;
        this->vao = vao;
        this->ebo = ebo;
    }



    void render(shader_t& shader, GLuint texture) {

        glActiveTexture(GL_TEXTURE0 + (int) texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, texture);


        shader.use();

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
};