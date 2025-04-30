#include "Parallel.hpp"

Parallel::Parallel(const unsigned int num_particles)
    : m_num_particles { num_particles }
    , m_num_threads { std::thread::hardware_concurrency() }
    , m_particles_per_thread { static_cast<unsigned int>(num_particles / m_num_threads) }
{
    m_workers.reserve(m_num_threads);
}

void Parallel::parallelizeParticleUpdates(std::vector<Particle>& particles, std::vector<PerlinNoise>& perlin, Circles& circles, const float time, const float dt, const bool reset_particle)
{
    for (unsigned int thread_ID = 0; thread_ID < m_num_threads; ++thread_ID)
    {
        // m_workers.emplace_back(&Parallel::updateParticles, this, thread_ID);
        m_workers.emplace_back([this, thread_ID, &particles, &perlin, &circles, time, dt, reset_particle]
        {
            this->updateParticles(thread_ID, particles, perlin, circles, time, dt, reset_particle);
        });
    }

    for (auto& thread : m_workers)
    {
        thread.join();  // Join all threads to ensure they complete before moving on
    }

    m_workers.clear();
}

void Parallel::updateParticles(unsigned int thread_ID, std::vector<Particle>& particles, std::vector<PerlinNoise>& perlin, Circles& circles, const float time, const float dt, const bool reset_particle)
{
    // for (unsigned int i = 0; i < m_num_particles; ++i)
    // this ensures the last thread handles any stragglers
    for (
        unsigned int particle_idx = (thread_ID * m_particles_per_thread);
        particle_idx < ( (thread_ID == m_num_threads - 1) ? m_num_particles : (thread_ID + 1) * m_particles_per_thread );
        ++particle_idx
    )
    {
        Particle& particle{ particles[particle_idx] };
        if (reset_particle){ particle.randomizeParticle(); }
        particle.updateVelocity(perlin, time);
        // particle.mixVelocity(perlin, time);
        // particle.curlVelocity(perlin, time);
        // particle.gradVelocity(perlin, time); // boring!
        particle.evolve(dt);
        circles.updatePosition(particle_idx, particle.position());
    }
}
