//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Material.hpp"
#include "Scene.hpp"
#include "Vector.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    Intersection inter = intersect(ray);
    // no intersection -> return black
    if (!inter.happened) return Vector3f();
    
    // hit light source -> return the color of light source
    if (inter.m->hasEmission()) return inter.m->getEmission();
    
    Vector3f p = inter.coords;
    Vector3f N = inter.normal;
    Vector3f wo = ray.direction;
    Material *m = inter.m;
    
    // calculate offset points based on the incident direction to avoid self-intersection
    Vector3f p_offset = dotProduct(wo, N) < 0 ? p + N * 0.0005f : p - N * 0.0005f;
    
    // direct illumination
    float pdf_light;  // will be calculated in sampleLight()
    Intersection sample_point;
    sampleLight(sample_point, pdf_light);

    Vector3f x = sample_point.coords;
    Vector3f ws = normalize(x - p);
    Vector3f NN = sample_point.normal;
    Vector3f emit = sample_point.emit;
    
    Vector3f L_dir(0.0f);
    float dis = (x - p).norm();
    Intersection shadowHit = intersect(Ray(p_offset, ws));
    // Check whether the shadow ray reaches the light source,
    // allowing for a certain margin of error
    if (shadowHit.distance > dis - 0.01f) {
        L_dir = emit * m->eval(wo, ws, N) 
                     * dotProduct(ws, N) 
                     * dotProduct(-ws, NN) 
                     / (dis * dis) 
                     / pdf_light;
    }

    // indirect illumination
    Vector3f L_indir(0.0f);
    if (get_random_float() < RussianRoulette) {
        Vector3f wi = m->sample(wo, N);
        Ray r_indir(p_offset, wi);
        Intersection anotherInter = intersect(r_indir);
        if (anotherInter.happened && !anotherInter.m->hasEmission()) {
                L_indir = castRay(r_indir, depth + 1) 
                            * m->eval(wo, wi, N) 
                            * dotProduct(wi, N) 
                            / m->pdf(wo, wi, N) 
                            / RussianRoulette; 
        }
    }

    return L_dir + L_indir;
}