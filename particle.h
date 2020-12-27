#include <glm/glm.hpp>
#include <vector> 
#include <iostream>
#include <cmath>
#include <chrono>

#include "opengl_shader.h"

struct Particle {

    glm::vec3 pos;
    glm::vec3 velocity;
    glm::vec3 color;
    float life = 0;
    float alpha;
  
    Particle(glm::vec3 pos, glm::vec3 velocity, glm::vec3 color, float life, float alpha) 
      : pos(pos), velocity(velocity), color(color), life(life), alpha(alpha) { }

};


class ParticleGenerator {

    public: 

    virtual Particle generate_new_particle(float life_coef) = 0;
    virtual glm::vec3 get_color_by_distance(float d) = 0;
    virtual float get_alpha_by_distance(float d) = 0;
    virtual int get_particles_count() = 0;
    virtual int get_particles_count_on_iteration() = 0;
    virtual int find_particle_index(std::vector<Particle>& particles) = 0;
};


class FireParticleGenerator : public ParticleGenerator {

    private:

    const int particles_count = 5000;
    const int particles_count_on_iteration = 100;


    public:

    Particle generate_new_particle(float life_coef) override {
        float random = 1.0 * rand() / RAND_MAX;
        float sign = 1.0 * rand() / RAND_MAX;
        float r = 1.0 * rand() / RAND_MAX;
        float phi = 1.0 * rand() / RAND_MAX * 2 * 3.14f;
        float speed = 1.0 * rand() / RAND_MAX * 5;
        float real_r = 2.0 * r;
        glm::vec3 pos =  glm::vec3(real_r * cos(phi), real_r * sin(phi), 0.0);
        float d = glm::distance(pos, glm::vec3(0, 0, 0));
        float life = sign > 0.5 ? life_coef * (1 - r) + 0.5 * life_coef * random : life_coef * (1 - r) - 0.5 * life_coef * random;
        Particle new_particle(
            pos,
            glm::vec3(0.0, 0.0, speed),
            get_color_by_distance(d),
            life,
            1.0
        );
        return new_particle;
    }



    glm::vec3 get_color_by_distance(float d) override {
        return d < 1 ? glm::vec3(1.0, 1.0, 1.0 - d) : glm::vec3(1.0, 2.0 - d, 0.0);
    }

    int get_particles_count() override {
        return particles_count;

    }

    int get_particles_count_on_iteration() override {
        return particles_count_on_iteration;
    }

    float get_alpha_by_distance(float d) override {
        float a = 1.0 - d / 4 + 0.5;
        return a >= 0.0 && a <=1.0 ? a : 1.0;
    }

    int find_particle_index(std::vector<Particle>& particles) override {
        return floor(1.0 * rand() / RAND_MAX * get_particles_count());
    }
};


class SmokeParticleGenerator : public ParticleGenerator {

    private:

    const int particles_count = 100;
    const int particles_count_on_iteration = 10;


    public:

    Particle generate_new_particle(float life_coef) override {
        float random = 1.0 * rand() / RAND_MAX;
        float sign = 1.0 * rand() / RAND_MAX;
        float r = 1.0 * rand() / RAND_MAX;
        float phi = 1.0 * rand() / RAND_MAX * 2 * 3.14f;
        float speed = 1.0 * rand() / RAND_MAX * 5.0;
        float real_r = 2.0 * r;
        glm::vec3 pos =  glm::vec3(real_r * cos(phi), real_r * sin(phi), 0.0);
        float d = glm::distance(pos, glm::vec3(0, 0, 0));
        float life = sign > 0.5 ? life_coef * (1 - r) + 0.5 * life_coef * random : life_coef * (1 - r) - 0.5 * life_coef * random;
        Particle new_particle(
            pos,
            glm::vec3(0.0, 0.0, speed),
            get_color_by_distance(d),
            life,
            0.02f
        );
        return new_particle;
    }



    glm::vec3 get_color_by_distance(float d) override {
        return glm::vec3(0.3, 0.3, 0.3);
    }

    int get_particles_count() override {
        return particles_count;

    }

    int get_particles_count_on_iteration() override {
        return particles_count_on_iteration;
    }

    float get_alpha_by_distance(float d) override {
        float a  = 0;
        if (d / 3 >= 1.0) {
            a = 0.11f - d / 3 * 0.04;
        } else {
            a = d / 3 / 30;
        }
        return a >= 0.0 ? a : 0.0;
    }

    int find_particle_index(std::vector<Particle>& particles) override {
        float minn = 100000000;
        int index = 0;
        for (int i = 0; i < get_particles_count(); i++) {
            if (particles[i].life < minn) {
                minn = particles[i].life;
                index = i;
            }
        }
        return index;
    }

};


class ParticlesGenerator
{
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> Time;

    private: 

    std::vector<Particle> particles; 
    std::array<GLuint, 1> textures;
    ParticleGenerator* generator;

    Time prev_time;
    Time prev_update_time;
    int time_delta = 16680576;
    int min_time = 20000000;

    GLuint vbo;
    GLuint vao;
    GLuint ebo;

    void init_buffers() {

        float particle_vertexes[] = {
            -1.0, -1.0,
            -1.0, 1.0,
            1.0, 1.0,
            1.0, -1.0
        };

        unsigned int triangles_indices[36] = { 0, 1, 2, 2, 3, 0 };

        GLuint vbo, vao, ebo;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(particle_vertexes), particle_vertexes, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangles_indices), triangles_indices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        this->vbo = vbo;
        this->vao = vao;
        this->ebo = ebo;

    }


    public:

    ParticlesGenerator(ParticleGenerator* generator, std::array<GLuint, 1>& textures)
        : generator(generator)
        , textures(textures)
        , prev_time(std::chrono::high_resolution_clock::now())
        , prev_update_time(std::chrono::high_resolution_clock::now())
    {
        init_buffers();

        for (int i = 0; i <  generator->get_particles_count(); i++) {
            particles.push_back(generator->generate_new_particle(0.7));
        }
    }


    void update(float life_coef) {
        Time current_time = std::chrono::high_resolution_clock::now();
        int d_time = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - prev_time).count();
        int d_update_time = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - prev_update_time).count();
        
        float dt = 0.01;
        int newCount = 0;
        for (auto& particle : particles) {
           float d = glm::distance(particle.pos, glm::vec3(0, 0, 0));
           if (particle.life <= 0) {
               particle = generator->generate_new_particle(life_coef);
               newCount++;
           } else {
               particle.life -= dt / time_delta * d_time;
               particle.pos = particle.pos + (dt / time_delta * d_time) * particle.velocity;
               particle.color = generator->get_color_by_distance(d);
               particle.alpha = generator->get_alpha_by_distance(d);
           }
        }
        if (d_update_time > min_time) {
             if (newCount <  generator->get_particles_count_on_iteration()) { 
                int diff =  generator->get_particles_count_on_iteration() - newCount;
                for (int i = 0; i < diff; i++) {
                    float index = generator->find_particle_index(particles);
                    particles[index] = generator->generate_new_particle(life_coef);
                }
             }
             prev_update_time = current_time;
        }
    
        prev_time = current_time;
    }


    void render_particles(shader_t& shader) {

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);  
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE);


        glActiveTexture(GL_TEXTURE0 + textures[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, textures[0]);

        for (auto& particle : particles) {

            shader.use();
            shader.set_uniform("color", particle.color.x, particle.color.y, particle.color.z);
            shader.set_uniform("position", particle.pos.x, particle.pos.y, particle.pos.z);
            shader.set_uniform("particle_texture", (int) textures[0]);
            shader.set_uniform("alpha", particle.alpha);
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        // glDisable(GL_ALPHA_TEST);
    }
};


