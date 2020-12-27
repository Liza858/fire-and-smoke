#include <iostream>
#include <vector>
#include <chrono>
#include <utility>
	

#include <fmt/format.h>

#include <GL/glew.h>

// Imgui + bindings
#include "imgui.h"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// Math constant and routines for OpenGL interop
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl_shader.h"
#include "environment.h"
#include "textures.h"
#include "plane.h"
#include "particle.h"

float mouse_offset_x = 0.0;
float mouse_offset_y = 0.0;

float angle_x = 0.0;
float angle_y = 0.0;

int zoom_sensitivity = 10;
float zoom = 0.1;

double mouse_prev_x = 0.0;
double mouse_prev_y = 0.0;

bool button_is_pressed = false;

static void glfw_error_callback(int error, const char *description)
{
   std::cerr << fmt::format("Glfw Error {}: {}\n", error, description);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
   zoom = zoom + yoffset * zoom_sensitivity / 10000.0;
   if (zoom < 0) {
      zoom = 0;
   }
}


void mouse_button_callback(GLFWwindow* window, int action)
{
   int display_w, display_h;
   glfwGetWindowSize(window, &display_w, &display_h);
    if (action == GLFW_PRESS) {
         double x, y;
         glfwGetCursorPos(window, &x, &y);
         if (button_is_pressed) {
            mouse_offset_x = x - mouse_prev_x;
            mouse_offset_y = y - mouse_prev_y;  
            mouse_prev_x = x;
            mouse_prev_y = y;
            angle_x -= mouse_offset_x * 0.25;
            if (angle_y - mouse_offset_y * 0.25 > 0.0) {
               angle_y = 0.0;
            } else if (angle_y - mouse_offset_y * 0.25 < -89.9) {
               angle_y = -89.9;
            } else {
               angle_y -= mouse_offset_y * 0.25;
            }
         } else {
            button_is_pressed = true;
            mouse_prev_x = x - mouse_offset_x;
            mouse_prev_y = y - mouse_offset_y;
         }
      } else if (action == GLFW_RELEASE) {
          button_is_pressed = false;
      }
}


int main(int, char **)
{
   // Use GLFW to create a simple window
   glfwSetErrorCallback(glfw_error_callback);
   if (!glfwInit())
      return 1;


   // GL 3.3 + GLSL 330
   const char *glsl_version = "#version 330";
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

   // Create window with graphics context
   GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui - Conan", NULL, NULL);
   if (window == NULL)
      return 1;
   glfwMakeContextCurrent(window);
   glfwSwapInterval(1); // Enable vsync

   // Initialize GLEW, i.e. fill all possible function pointers for current OpenGL context
   if (glewInit() != GLEW_OK)
   {
      std::cerr << "Failed to initialize OpenGL loader!\n";
      return 1;
   }

   glfwSetScrollCallback(window, scroll_callback);

   shader_t env_shader("environment.vs", "environment.fs");
   shader_t plane_shader("plane.vs", "plane.fs");
   shader_t particle_shader("particle.vs", "particle.fs");


   std::array<std::string, 6> textures_src = {
      "../environment/space.jpg",
      "../environment/space.jpg",
      "../environment/space.jpg",
      "../environment/space.jpg",
      "../environment/space.jpg",
      "../environment/space.jpg"
   };

   GLuint cubemap_texture = CubemapTextureLoader::load(textures_src);
   GLuint plane_texture = TextureLoader::load("../plane_textures/tex1.jpg");


   std::array<GLuint, 1> smoke_textures = {
       TextureLoader::load("../smoke_textures/smoke.png")
   };

   std::array<GLuint, 1> fire_textures = {
       TextureLoader::load("../fire_textures/fire.png")
   };

   ParticlesGenerator smoke_generator(new SmokeParticleGenerator(), smoke_textures);
   ParticlesGenerator fire_generator(new FireParticleGenerator(), fire_textures);

   Environment env;
   Plane plane;


   // Setup GUI context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO &io = ImGui::GetIO();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init(glsl_version);
   ImGui::StyleColorsDark();

   static float fire_life = 0.7;
   static float smoke_life = 10;
   static float fire_size = 0.25;
   static float smoke_size = 2.0;

   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);

   while (!glfwWindowShouldClose(window))
   {
      glfwPollEvents();

      // Get windows size
      int display_w, display_h;
      glfwGetFramebufferSize(window, &display_w, &display_h);

      // Set viewport to fill the whole window area
      glViewport(0, 0, display_w, display_h);

      // Fill background with solid color
      glClearColor(0.30f, 0.55f, 0.60f, 1.00f);
      glClear(GL_COLOR_BUFFER_BIT);

      // Gui start new frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      // GUI
      ImGui::Begin("Settings");
      ImGui::SliderInt("zoom sensitivity, %", &zoom_sensitivity, 0, 100);
      ImGui::InputFloat("fire particles life coef", &fire_life);
      ImGui::InputFloat("smoke particles life coef", &smoke_life);
      ImGui::InputFloat("fire particles size", &fire_size);
      ImGui::InputFloat("smoke particles size", &smoke_size);
      ImGui::End();

        
      // Mouse button press action
      if (!(ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive())) {
          auto mouse_action = glfwGetMouseButton(window, 0);
          mouse_button_callback(window, mouse_action);
      }

      glm::vec4 camera(0, 0, 1, 1);
      auto x_rot = glm::rotate(glm::mat4(1), glm::radians(angle_x), glm::vec3(0, 1, 0));
      camera = x_rot * camera;
      camera = glm::rotate(glm::mat4(1), glm::radians(angle_y), glm::vec3(x_rot * glm::vec4(1, 0, 0, 1))) * camera;
      auto model = glm::scale(glm::vec3(zoom, zoom, zoom));
      model = glm::rotate(model, 3.14f / 2.0f, glm::vec3(-1.0f, 0.0f, 0.0f));
      auto view = glm::lookAt<float>(glm::vec3(camera.x, camera.y, camera.z), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
      auto view_cubemap = glm::lookAt<float>(glm::vec3(0, 0, 0), glm::vec3(-camera.x, -camera.y, -camera.z), glm::vec3(0, 1, 0));
      auto projection = glm::perspective<float>(90, float(display_w) / display_h, 0.1, 100);


      glClear(unsigned(GL_COLOR_BUFFER_BIT) | unsigned(GL_DEPTH_BUFFER_BIT));


      // отключаем тест глубины, чтобы все рисовалось поверх environment
      glDepthMask(GL_FALSE);
      env_shader.use();
      env_shader.set_uniform("projection", glm::value_ptr(projection));
      env_shader.set_uniform("view", glm::value_ptr(view_cubemap));
      env_shader.set_uniform("camera_pos", camera.x, camera.y, camera.z);
      env_shader.set_uniform("environment", 1);
      env.render(env_shader, cubemap_texture);
      glDepthMask(GL_TRUE);


      plane_shader.use();
      plane_shader.set_uniform("model", glm::value_ptr(model));
      plane_shader.set_uniform("view", glm::value_ptr(view));
      plane_shader.set_uniform("projection", glm::value_ptr(projection));
      plane_shader.set_uniform("plane_texture", (int) plane_texture);
      plane.render(plane_shader, plane_texture);
   
              
      particle_shader.use();
      particle_shader.set_uniform("model", glm::value_ptr(model));
      particle_shader.set_uniform("view", glm::value_ptr(view));
      particle_shader.set_uniform("projection", glm::value_ptr(projection));
      particle_shader.set_uniform("particle_scale", zoom);
      particle_shader.set_uniform("particle_size", fire_size);
      fire_generator.render_particles(particle_shader);
      fire_generator.update(fire_life);


      particle_shader.use();
      particle_shader.set_uniform("model", glm::value_ptr(model));
      particle_shader.set_uniform("view", glm::value_ptr(view));
      particle_shader.set_uniform("projection", glm::value_ptr(projection));
      particle_shader.set_uniform("particle_scale", zoom);
      particle_shader.set_uniform("particle_size", smoke_size);
      smoke_generator.render_particles(particle_shader);
      smoke_generator.update(smoke_life);


   
      glBindVertexArray(0);

      // Generate gui render commands
      ImGui::Render();

      // Execute gui render commands using OpenGL backend
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      // Swap the backbuffer with the frontbuffer that is used for screen display
      glfwSwapBuffers(window);
   }

   // Cleanup
   ImGui_ImplOpenGL3_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();

   glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}
