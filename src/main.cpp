#include <iostream>
#include <math.h>
#include <random>
#include <algorithm>

// OpenGL helpers
#include "VertexBuffer/VertexBuffer.hpp"
#include "IndexBuffer/IndexBuffer.hpp"
#include "VertexArray/VertexArray.hpp"
#include "Shader/Shader.hpp"
#include "VertexBufferLayout/VertexBufferLayout.hpp"
#include "Rectangle/Rectangle.hpp"
#include "Circle/Circle.hpp"

#include "Point/Point.hpp"
#include "Grid/Grid.hpp"
#include "PerlinNoise/PerlinNoise.hpp"
#include "Parallel/Parallel.hpp"

SDL_Window* window;
SDL_GLContext gl_context;

bool is_running;
bool paused { true };
bool reset_particles { false };

unsigned int octaves { 1 };
unsigned int frequency { 5 };
unsigned int num_particles { 50000 };

const float width { 1280.0f };
const float height { 768.0f };
// const unsigned int res { 250 };
const float REFRESH_ALPHA { 0.025f };
unsigned int RESET_FRAME_COUNT { 0 };

float GLOBAL_TIME { 0.0f };
float DT { 0.025f };

void set_sdl_gl_attributes()
{
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
}

void evolveTime(SDL_Event& event)
{
    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.scancode == SDL_SCANCODE_LEFT)
        {
            GLOBAL_TIME < DT ? GLOBAL_TIME = 0.0f : GLOBAL_TIME -= DT;
        }
        else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT)
        {
            GLOBAL_TIME += DT;
        }
        else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE)
        {
            paused = paused ? false : true;
        }
        else if (event.key.keysym.scancode == SDL_SCANCODE_RETURN)
        {
            reset_particles = reset_particles ? false : true;
        }
    }
}

void printKey()
{
    std::cout << "###################################" << '\n';
    std::cout << "KEY" << '\n';
    std::cout << "Spacebar" << '\t' << "Play/pause" << '\n';
    std::cout << "Right arrow" << '\t' << "Time forward" << '\n';
    std::cout << "Left arrow" << '\t' << "Time reverse" << '\n';
    std::cout << "Enter/Return" << '\t' << "Reset particles" << '\n';
    std::cout << "###################################" << '\n';
}

int main()
{
    if(SDL_Init(SDL_INIT_EVERYTHING)==0)
    {
        std::cout<<"SDL2 initialized successfully."<<std::endl;
        set_sdl_gl_attributes();
        
        window = SDL_CreateWindow("Perlin Noise", 0.0f, 0.0f, static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_OPENGL);
        gl_context = SDL_GL_CreateContext(window);
        SDL_GL_SetSwapInterval(1);

        if(glewInit() == GLEW_OK)
        {
            std::cout << "GLEW initialization successful" << std::endl;
        }
        else
        {
            std::cout << "GLEW initialization failed" << std::endl;
            return -1;
        }

        std::cout << "Thread count: " << std::thread::hardware_concurrency() << '\n';

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        printKey();

        std::vector<PerlinNoise> pVec;
        pVec.reserve(octaves);
        for (unsigned int i = 1; i < octaves + 1; ++i)
        {
            pVec.emplace_back(PerlinNoise(width, height, frequency * i));
        }

        // Grid grid(width, height, res, pVec);
        // std::vector<Rectangle> rects;
        // rects.reserve(grid.size());
        // const std::vector<Point>& points { grid.points() };
        // const std::vector<float>& values { grid.values() };
        // for (unsigned int i = 0; i < grid.size(); ++i)
        // {
        //     const Point& point { points[i] };
        //     float val { values[i] };
        //     rects.emplace_back(Rectangle(point.position(), width / (res - 1), height / (res - 1), glm::vec4(glm::vec3(val), 1.0f)));
        // }
        
        Parallel parallel(num_particles);
        std::vector<Particle> particles;
        std::vector<Circle> circs;
        circs.reserve(num_particles);
        particles.reserve(num_particles);
        for (unsigned int i = 0; i < num_particles; ++i)
        {
            const Particle& particle { width, height, 1.0f };
            particles.push_back(particle);
            circs.emplace_back(Circle(particle.position(), particle.radius(), glm::vec4(glm::vec3(1.0f), 0.1f)));
        }

        // Rectangles rectangles(rects);
        Circles circles(circs);

        VertexArray VAO, C_VAO;

        // dummy VAO for full screen triangle
        VertexArray FS_VAO;
        FS_VAO.bind();

        // constructor automatically binds buffer
        // VertexBuffer VBO(rectangles.m_vertices.data(), static_cast<unsigned int>(rectangles.m_vertices.size() * sizeof(float)), GL_DYNAMIC_DRAW);
        VertexBuffer C_VBO(circles.m_vertices.data(), static_cast<unsigned int>(circles.m_vertices.size() * sizeof(float)), GL_DYNAMIC_DRAW);
        
        VertexBufferLayout layout, c_layout;
        // i < 3 for 'Rectangle'
        for (unsigned int i = 0; i < 5; ++i)
        {
            // if (i < 3)
            // {
            //     layout.push<float>(Rectangle::layout_descriptor[i]);
            // }
            c_layout.push<float>(Circle::layout_descriptor[i]);
        }
        // VAO.addBuffer(VBO, layout);
        C_VAO.addBuffer(C_VBO, c_layout);

        // constructor automatically binds buffer
        // IndexBuffer IBO(rectangles.m_indices.data(), static_cast<unsigned int>(rectangles.m_indices.size()));
        IndexBuffer C_IBO(circles.m_indices.data(), static_cast<unsigned int>(circles.m_indices.size()));

        // set up MPV matrix
        glm::mat4 proj { glm::ortho(0.0f, width, 0.0f, height, -1.0f, 1.0f) };
        glm::mat4 view { glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)) };
        glm::mat4 model { glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) };
        glm::mat4 MVP { proj * view * model };

        // Shader shader("/Users/max/OpenGL_Framework/res/shaders", "rectangle_");
        // shader.bind();
        // shader.setUniformMatrix4fv("u_MVP", MVP);

        Shader c_shader("/Users/max/OpenGL_Framework/res/shaders", "circle_");
        c_shader.bind();
        c_shader.setUniformMatrix4fv("u_MVP", MVP);
        
        Shader fullScreenShader("./res/shaders", "FSTriangle_");
        fullScreenShader.bind();
        fullScreenShader.setUniform4f("uColor", 0.0f, 0.0f, 0.0f, REFRESH_ALPHA);

        // VBO.unbind();
        // VAO.unbind();
        // IBO.unbind();
        // shader.unbind();
        C_VBO.unbind();
        C_VAO.unbind();
        C_IBO.unbind();
        c_shader.unbind();
        FS_VAO.unbind();
        fullScreenShader.unbind();

        Renderer renderer;

        // Main loop
        SDL_Event event;
        is_running = true;

        while(is_running)
        {
            while(SDL_PollEvent(&event))
            {
                if( event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) )
                {
                    is_running = false;
                }

                evolveTime(event);
            }

            // evolve time if unpaused
            if (not paused)
            {
                GLOBAL_TIME += DT / 10;
            }

            // update z layer if reached the ceiling
            if (GLOBAL_TIME >= 0.99999f)
            {
                GLOBAL_TIME = 0.0f;
                for (PerlinNoise& p : pVec) { p.nextZGradients(); }
            }

            // update grid values
            // grid.assignValues(pVec, GLOBAL_TIME);

            // update rectangle buffer
            // rectangles.udpateColors(values);
            // VBO.updateBuffer(rectangles.m_vertices.data());

            // update particles/circles in parallel (on CPU)
            parallel.parallelizeParticleUpdates(particles, pVec, circles, GLOBAL_TIME, DT, reset_particles);
            // for (unsigned int i = 0; i < particles.size(); ++i)
            // {
            //     Particle& particle{ particles[i] };
            //     if (reset_particles){ particle.randomizeParticle(); }
            //     particle.curlVelocity(pVec, GLOBAL_TIME);
            //     // particle.updateVelocity(pVec, GLOBAL_TIME);
            //     particle.evolve(DT);
            //     circles.updatePosition(i, particle.position());
            // }
            
            if (reset_particles)
            {
                RESET_FRAME_COUNT = 2;
                reset_particles = false;
            }
            if (RESET_FRAME_COUNT > 0)
            {
                --RESET_FRAME_COUNT;
                fullScreenShader.bind();
                fullScreenShader.setUniform4f("uColor", 0.0f, 0.0f, 0.0f, 1.0f);
                renderer.drawTriangles(FS_VAO, fullScreenShader);
                fullScreenShader.setUniform4f("uColor", 0.0f, 0.0f, 0.0f, REFRESH_ALPHA);
                fullScreenShader.unbind();
            }

            // update buffers
            // VBO.updateBuffer(rectangles.m_vertices.data());
            C_VBO.updateBuffer(circles.m_vertices.data());

            // Render
            // renderer.clear();
            renderer.drawTriangles(FS_VAO, fullScreenShader);

            // renderer.drawRectangles(VAO, IBO, shader); // to show grid point values in color
            renderer.drawCircles(C_VAO, C_IBO, c_shader);

            SDL_GL_SwapWindow(window);
        }

        SDL_Quit();
    }
    else
    {
        std::cout<<"SDL2 initialization failed."<<std::endl;
        return -1;
    }

    return 0;
}
