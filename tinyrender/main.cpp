#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <iostream>
#include <stdlib.h>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green   = TGAColor(0, 255,   0,   255);
Model *model = NULL;
const int width  = 2000;
const int height = 2000;

void rasterize(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color);

void line(Vec2i t0, Vec2i t1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(t0.x-t1.x)<std::abs(t0.y-t1.y)) {
        std::swap(t0.x, t0.y);
        std::swap(t1.x, t1.y);
        steep = true;
    }
    if (t0.x>t1.x) {
        std::swap(t0.x, t1.x);
        std::swap(t0.y, t1.y);
    }

    for (int x=t0.x; x<=t1.x; x++) {
        float t = (x-t0.x)/(float)(t1.x-t0.x);
        int y = t0.y*(1.-t) + t1.y*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

void triangle2(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
    rasterize(t0,t1,t2,image,color);
}

void rasterize(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    int minx = (int)std::min(t0.x,std::min(t1.x,t2.x));
    int maxx = (int)std::max(t0.x,std::max(t1.x,t2.x));
    int miny = (int)std::min(t0.y,std::min(t1.y,t2.y));
    int maxy = (int)std::max(t0.y,std::max(t1.y,t2.y));
    for(int j=miny;j<maxy;j++){
        for(int i=minx;i<maxx;i++){
            if( ((t1.x-t0.x)*(j-t0.y)-(t1.y-t0.y)*(i-t0.x)<=0)&&
                ((t0.x-t2.x)*(j-t2.y)-(t0.y-t2.y)*(i-t2.x)<=0)&&
                ((t2.x-t1.x)*(j-t1.y)-(t2.y-t1.y)*(i-t1.x)<=0)){
                image.set(i, j, color);
        }
    }
}
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    if (t0.y==t1.y && t0.y==t2.y) return; // i dont care about degenerate triangles
    if (t0.y>t1.y) std::swap(t0, t1);
    if (t0.y>t2.y) std::swap(t0, t2);
    if (t1.y>t2.y) std::swap(t1, t2);
    int total_height = t2.y-t0.y;
    for (int i=0; i<total_height; i++) {
        bool second_half = i>t1.y-t0.y || t1.y==t0.y;
        int segment_height = second_half ? t2.y-t1.y : t1.y-t0.y;
        float alpha = (float)i/total_height;
        float beta  = (float)(i-(second_half ? t1.y-t0.y : 0))/segment_height; // be careful: with above conditions no division by zero here
        Vec2i A =               t0 + (t2-t0)*alpha;
        Vec2i B = second_half ? t1 + (t2-t1)*beta : t0 + (t1-t0)*beta;
        if (A.x>B.x) std::swap(A, B);
        for (int j=A.x; j<=B.x; j++) {
            image.set(j, t0.y+i, color); // attention, due to int casts t0.y+i != A.y
        }
    }
}

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    Vec3f light_dir(0,0,-1);
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for (int j=0; j<3; j++) {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.);
            world_coords[j]  = v;
        }
        Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
        n.normalize();
        float intensity = n*light_dir;
        if (intensity>0) {
            triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}

