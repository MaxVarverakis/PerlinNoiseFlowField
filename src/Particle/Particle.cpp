#include "Particle.hpp"

std::mt19937 Particle::m_rng(std::random_device{}());
std::uniform_real_distribution<float> Particle::m_x_dist(0.0f, 1.0f);
std::uniform_real_distribution<float> Particle::m_y_dist(0.0f, 1.0f);

Particle::Particle(float width, float height, const float radius)
    : m_width { width }
    , m_height { height }
    , m_radius { radius }
{
    m_x_dist = std::uniform_real_distribution<float>(0.0f, width);
    m_y_dist = std::uniform_real_distribution<float>(0.0f, height);

    randomizeParticle();
}

Particle::Particle(float width, float height, const float radius, const glm::vec2 position, const glm::vec2 velocity)
    : m_width { width }
    , m_height { height }
    , m_radius { radius }
    , m_position { position }
    , m_velocity { velocity }
{
    m_x_dist = std::uniform_real_distribution<float>(0.0f, width);
    m_y_dist = std::uniform_real_distribution<float>(0.0f, height);
}

void Particle::updatePosition(const float dt)
{
    m_position += m_velocity * dt;
}

void Particle::updateVelocity(const std::vector<PerlinNoise>& perlin, const float t)
{
    
    float nudge { 0.0f };
    for (unsigned int i = 0; i < perlin.size(); ++i)
    {
        // map noise value to angle before adding to nudge
        // [0,1] --> [-1,1] --> [0, 2pi]
        // making sure to scale noise value by octave
        nudge += 2 * static_cast<float>(M_PI) * (2 * perlin[i].noise(m_position, t) - 1) / (float)(i + 1);
    }
    m_velocity += glm::vec2(cos(nudge), sin(nudge));
    m_velocity *= 0.99f; // friction to prevent infinite velocities
}

void Particle::applyBoundaryCondition()
{
    // assumes domain is [0, width] x [0, height]
    if ( (m_position.x - m_radius < 0.0f) || (m_position.x + m_radius > m_width) )
    {
        m_velocity.x *= -1.0f;
    }
    
    if ( (m_position.y - m_radius < 0.0f) || (m_position.y + m_radius > m_height) )
    {
        m_velocity.y *= -1.0f;
    }
}

void Particle::periodicBoundaries()
{
    // assumes domain is [0, width] x [0, height]
    if (m_position.x - m_radius < 0.0f)
    {
        m_position.x += m_width;
    }
    else if (m_position.x + m_radius > m_width)
    {
        m_position.x -= m_width;
    }
    
    if (m_position.y - m_radius < 0.0f)
    {
        m_position.y += m_height;
    }
    else if (m_position.y + m_radius > m_height)
    {
        m_position.y -= m_height;
    }
}

void Particle::randomizeParticle()
{
    m_position.x = m_x_dist(m_rng);
    m_position.y = m_y_dist(m_rng);
    m_velocity.x = 0.0f;
    m_velocity.y = 0.0f;
}

void Particle::outOfBounds()
{
    // assumes domain is [0, width] x [0, height]
    if ( (m_position.x - m_radius < 0.0f) || (m_position.x + m_radius > m_width) || (m_position.y - m_radius < 0.0f) || (m_position.y + m_radius > m_height) )
    {
        randomizeParticle();
    }
}

void Particle::evolve(const float dt)
{
    updatePosition(dt);
    outOfBounds();
    // periodicBoundaries();
}
