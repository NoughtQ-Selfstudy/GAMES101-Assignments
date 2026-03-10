#include <iostream>
#include <vector>

#include "CGL/include/CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
        const float x_unit = (end.x - start.x) / (num_nodes - 1);
        const float y_unit = (end.y - start.y) / (num_nodes - 1);
        for (int i = 0; i < num_nodes; ++i) {
            float x = start.x + i * x_unit;
            float y = start.y + i * y_unit;
            Vector2D pos(x, y);
            masses.emplace_back(new Mass(pos, node_mass, false));
            if (masses.size() > 1) {
                springs.emplace_back(new Spring(masses[i - 1], masses[i], k));
            }
        }

        // Comment-in this part when you implement the constructor
        for (auto &i : pinned_nodes) {
            masses[i]->pinned = true;
        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            Vector2D a = s->m1->position, b = s->m2->position;
            float l = s->rest_length;
            s->m1->forces += s->k * (b - a).unit() * ((b - a).norm() - l);
            s->m2->forces += s->k * (a - b).unit() * ((a - b).norm() - l);
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                m->forces += gravity * m->mass;
                // TODO (Part 2): Add global damping
                m->forces -= kd_Euler * m->velocity;
                Vector2D accl = m->forces / m->mass;
                Vector2D old_v = m->velocity;
                m->velocity += accl * delta_t;
                // explicit method
                // m->position += old_v * delta_t;
                // semi-implicit method
                m->position += m->velocity * delta_t;
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        constexpr float epsilon = 1e-3;
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet (solving constraints)
            Vector2D a = s->m1->position, b = s->m2->position;
            float l = s->rest_length, d = (b - a).norm();
            float delta_d = std::fabs(d - l);
            if (delta_d > epsilon) {
                if (d > l) {
                    if (s->m1->pinned) {
                        s->m2->position += (a - b) * (delta_d / d);
                    } else if (s->m2->pinned) {
                        s->m1->position += (b - a) * (delta_d / d);
                    } else {
                        s->m1->position += (b - a) * (delta_d / (2 * d));
                        s->m2->position += (a - b) * (delta_d / (2 * d));
                    }
                } else {
                    if (s->m1->pinned) {
                        s->m2->position += (b - a) * (delta_d / d);
                    } else if (s->m2->pinned) {
                        s->m1->position += (a - b) * (delta_d / d);
                    } else {
                        s->m1->position += (a - b) * (delta_d / (2 * d));
                        s->m2->position += (b - a) * (delta_d / (2 * d));
                    }
                }
            }
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                m->forces += gravity * m->mass;
                Vector2D accl = m->forces / m->mass;
                // m->position = temp_position + (temp_position - m->last_position) + accl * delta_t * delta_t;
                // TODO (Part 4): Add global Verlet damping
                m->position = temp_position + (1 - damping_factor) * (temp_position - m->last_position) + accl * delta_t * delta_t;
                m->last_position = temp_position;
                m->forces = Vector2D(0, 0);
            }
        }
    }
}
