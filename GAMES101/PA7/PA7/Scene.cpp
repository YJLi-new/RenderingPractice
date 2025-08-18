//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


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


    if (depth > this->maxDepth) {
        return Vector3f();
    }
    //if (depth==100) {
        //std::cout << depth;
    //}

    Vector3f wo = ray.direction;  // 入射光线的方向矢量，指向表面点

    //std::cout << "inter with obj\n";
    Intersection inter_object = this->intersect(ray);  // 光线与场景中对象相交的结果
    //std::cout << "comp\n";

    if (!inter_object.happened) {
        //std::cout << " no inter with obj\n";
        return Vector3f();  // 如果没有交点，返回空向量
    }


    Material* mater;
    mater = inter_object.m;  // 指向相交对象材质的指针
    // norml = (inter_object.normal).normalized();  // 交点处的法向量  // error 未声明

    if (mater->hasEmission()) {
        if (depth == 0) {
            return mater->getEmission();
            //std::cout << "isEmission\n";
        }
        return Vector3f();
    }

    Vector3f norml;
    norml = inter_object.normal.normalized();
    Vector3f p = inter_object.coords;  // current surface point 光线与物体相交的当前表面点  // error 未声明

    Vector3f L_dir = Vector3f();
    Vector3f L_indir = Vector3f();
    
    MaterialType m_type = (*mater).getType();

    Vector3f wi = mater->sample(wo, norml).normalized();
    // float pdf_light = mater->pdf(wo, wi, norml);
    // float cos_theta = dotProduct(wi, norml);

    float pdf_light = 0.0f;
    Intersection inter_light;
    //std::cout << "sampling\n";
    sampleLight(inter_light, pdf_light);

    Vector3f x = inter_light.coords;  // sampled point on the light source.
    Vector3f ws = (p - x).normalized();  // direction vector from p to the light sample point x
    Vector3f NN = inter_light.normal.normalized();  // normal vector at the light sample point x
    Vector3f emit = inter_light.emit;

    switch (m_type) {

    }
    // Intersection inter_block = intersect(Ray(p, x));
    Intersection inter_block = this->intersect(Ray(x, ws));
    float x_p_distance = (p - x).norm();
    if (inter_block.happened && ((inter_block.distance - x_p_distance) > -EPSILON)) {
        L_dir = emit * mater->eval(wo, -ws, norml) * dotProduct(-ws, norml) * dotProduct(ws, NN) / dotProduct(x_p_distance, x_p_distance) / pdf_light;
    }
    else {
        //std::cout << "L_dir = 0\n";
        return Vector3f();
    }

    float P_RR = get_random_float();
    float pdfpdf = inter_block.m->pdf(wo, wi, norml);
    if (P_RR <= RussianRoulette && pdfpdf > EPSILON) {
        Vector3f wi = mater->sample(wo, norml).normalized();
        Ray r_new(p, wi);
        Intersection inter_q = this->intersect(r_new);
        if (inter_q.happened && !inter_q.m->hasEmission()) {
            //L_indir = castRay(r_new, depth + 1) * inter_object.m->eval(wo, wi, norml) * dotProduct(wi, norml) / inter_object.m->pdf(wo, wi, norml) / RussianRoulette;
            L_indir = castRay(r_new, depth + 1) * inter_block.m->eval(wo, wi, norml) * dotProduct(wi, norml) / pdfpdf / RussianRoulette;
        }
    }

    return (L_dir + L_indir);
}