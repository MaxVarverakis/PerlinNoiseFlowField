#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <random>

#include "PerlinNoise/PerlinNoise.hpp"

class Particle
{
private:
    float m_width;
    float m_height;
    float m_radius;
    glm::vec2 m_position;
    glm::vec2 m_velocity;
    // glm::vec2 m_acceleration;

public:
    static std::mt19937 m_rng;
    static std::uniform_real_distribution<float> m_dist;

    Particle(float width, float height, const float radius);
    Particle(float width, float height, const float radius, const glm::vec2 position, const glm::vec2 velocity);

    float radius() const { return m_radius; }
    glm::vec2 position()  const { return m_position; }
    glm::vec2 velocity() const { return m_velocity; }
    // glm::vec2 acceleration() { return m_acceleration; }

    void setPosition(const glm::vec2 position) { m_position = position; }
    void setVelocity(const glm::vec2 velocity) { m_velocity = velocity; }
    // void setAcceleration(const glm::vec2 acceleration) { m_acceleration = acceleration; }
    
    void randomizeParticle();
    void updatePosition(const float dt);
    void updateVelocity(const std::vector<PerlinNoise>& perlin, const float t);
    void curlVelocity(const std::vector<PerlinNoise>& perlin, const float t);
    void gradVelocity(const std::vector<PerlinNoise>& perlin, const float t);
    void mixVelocity(const std::vector<PerlinNoise>& perlin, const float t);
    void periodicBoundaries();
    void applyBoundaryCondition();
    void outOfBounds();
    void evolve(const float dt);
};
