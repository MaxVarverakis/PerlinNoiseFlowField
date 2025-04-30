#pragma once

#include <thread>
#include "Circle/Circle.hpp"
#include "Particle/Particle.hpp"

class Parallel
{
private:
    unsigned int m_num_particles;
    unsigned int m_num_threads;
    unsigned int m_particles_per_thread;
    std::vector<std::thread> m_workers;
    // const uint8_t m_num_threads { static_cast<uint8_t>(std::thread::hardware_concurrency()) };
public:
    Parallel(const unsigned int num_particles);

    void parallelizeParticleUpdates(std::vector<Particle>& particles, std::vector<PerlinNoise>& perlin, Circles& circles, const float time, const float dt, const bool reset_particle);
    void updateParticles(unsigned int thread_ID, std::vector<Particle>& particles, std::vector<PerlinNoise>& perlin, Circles& circles, const float time, const float dt, const bool reset_particle);
};
