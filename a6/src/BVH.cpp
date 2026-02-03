#include <algorithm>
#include <cassert>
#include <chrono>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    auto start = std::chrono::high_resolution_clock::now();
    
    if (primitives.empty())
        return;

    root = recursiveBuild(primitives);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    
    printf("\rBVH Generation complete: \nTime Taken: %lld ms\n\n", duration.count());
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                       f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                       f2->getBounds().Centroid().y;
            });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                       f2->getBounds().Centroid().z;
            });
            break;
        }

        // Bonus
        size_t splitIndex = objects.size() / 2; // NAIVE
        if (splitMethod == SplitMethod::SAH) {
            // Cost = C_trav + (S_A / S_N) * N_A * C_isect + (S_B / S_N) * N_B * C_isect
            constexpr float C_trav = 0.125f;
            constexpr float C_isect = 1.0f;

            double S_N = bounds.SurfaceArea();  // Parent surface area
            double minCost = std::numeric_limits<double>::infinity();
            
            // Try each possible split position
            for (size_t i = 1; i < objects.size(); ++i) {
                // Compute bounds for left and right partitions
                Bounds3 boundsA, boundsB;
                for (size_t j = 0; j < i; ++j)
                    boundsA = Union(boundsA, objects[j]->getBounds());
                for (size_t j = i; j < objects.size(); ++j)
                    boundsB = Union(boundsB, objects[j]->getBounds());

                double S_A = boundsA.SurfaceArea();
                double S_B = boundsB.SurfaceArea();
                size_t N_A = i;
                size_t N_B = objects.size() - i;

                double cost = C_trav + (S_A / S_N) * N_A * C_isect + (S_B / S_N) * N_B * C_isect;

                if (cost < minCost) {
                    minCost = cost;
                    splitIndex = i;
                }
            }
        }

        auto beginning = objects.begin();
        auto middling = objects.begin() + splitIndex;
        auto ending = objects.end();

        auto leftshapes = std::vector<Object*>(beginning, middling);
        auto rightshapes = std::vector<Object*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // TODO Traverse the BVH to find intersection
    Intersection inter, hit1, hit2;
    Vector3f invDir = ray.direction_inv;
    std::array<int, 3> dirIsNeg;

    for (int i = 0; i < 3; ++i) {
        dirIsNeg[i] = static_cast<int>(ray.direction[i] > 0);
    }

    // not intersect with bounding box
    if (!node->bounds.IntersectP(ray, invDir, dirIsNeg)) {
        return inter;
    }

    // leaf node
    if (node->left == nullptr && node->right == nullptr) {
        return node->object->getIntersection(ray);
    }

    // traversal
    hit1 = getIntersection(node->left, ray);
    hit2 = getIntersection(node->right, ray);

    if (hit1.happened && hit2.happened) {
        return hit1.distance < hit2.distance ? hit1 : hit2;
    } else if (hit1.happened && !hit2.happened) {
        return hit1;
    } else if (!hit1.happened && hit2.happened) {
        return hit2;
    } else {
        return inter;
    }
}