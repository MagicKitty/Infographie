#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <iostream>
#include <stdlib.h>
#include <limits>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green   = TGAColor(0, 255,   0,   255);
Model *model = NULL;
const int width  = 2000;
const int height = 2000;

void triangleZBuffer(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color);

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

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
    if (t0.y==t1.y && t0.y==t2.y) return; //degenerated triangles
    int minx = std::min(t0.x,std::min(t1.x,t2.x));
    int maxx = std::max(t0.x,std::max(t1.x,t2.x));
    int miny = std::min(t0.y,std::min(t1.y,t2.y));
    int maxy = std::max(t0.y,std::max(t1.y,t2.y));
    int dx10=t1.x-t0.x;
    int dx02=t0.x-t2.x;
    int dx21=t2.x-t1.x;
    int dy10=t1.y-t0.y;
    int dy02=t0.y-t2.y;
    int dy21=t2.y-t1.y;
    for(int j=miny;j<maxy;j++){
        int dyj0=j-t0.y;
        int dyj2=j-t2.y;
        int dyj1=j-t1.y;
        for(int i=minx;i<maxx;i++){
            if(((dx10*dyj0-dy10*(i-t0.x)>=0)&&
                (dx02*dyj2-dy02*(i-t2.x)>=0)&&
                (dx21*dyj1-dy21*(i-t1.x)>=0))||
               ((dx10*dyj0-dy10*(i-t0.x)<=0)&&
                (dx02*dyj2-dy02*(i-t2.x)<=0)&&
                (dx21*dyj1-dy21*(i-t1.x)<=0))){
                image.set(i, j, color);
            }
        }
    }
}

void triangleZBuffer(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color) {
    Vec3f t0 = pts[0];
    Vec3f t1 = pts[1];
    Vec3f t2 = pts[2];
    if (t0.y==t1.y && t0.y==t2.y) return; //degenerated triangles
    int minx = std::min(t0.x,std::min(t1.x,t2.x));
    int maxx = std::max(t0.x,std::max(t1.x,t2.x));
    int miny = std::min(t0.y,std::min(t1.y,t2.y));
    int maxy = std::max(t0.y,std::max(t1.y,t2.y));
    int dx10=t1.x-t0.x;
    int dx02=t0.x-t2.x;
    int dx21=t2.x-t1.x;
    int dy10=t1.y-t0.y;
    int dy02=t0.y-t2.y;
    int dy21=t2.y-t1.y;
    for(int j=miny;j<maxy;j++){
        int dyj0=j-t0.y;
        int dyj2=j-t2.y;
        int dyj1=j-t1.y;
        for(int i=minx;i<maxx;i++){
            if(((dx10*dyj0-dy10*(i-t0.x)>=0)&&
                (dx02*dyj2-dy02*(i-t2.x)>=0)&&
                (dx21*dyj1-dy21*(i-t1.x)>=0))||
               ((dx10*dyj0-dy10*(i-t0.x)<=0)&&
                (dx02*dyj2-dy02*(i-t2.x)<=0)&&
                (dx21*dyj1-dy21*(i-t1.x)<=0))){
                image.set(i, j, color);
            }
        }
    }
}

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);

    Vec3f light_dir(0,0,-1);

    float *zbuffer = new float[width*height];
    for (int i=0;i<width*height; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec3f pts_s[3];
        Vec3f pts_w[3];
        Vec2i screen_coords[3];
        for (int j=0; j<3; j++){
            Vec3f v = model->vert(face[j]);
            screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.);
            pts_s[j] = world2screen(model->vert(face[j]));
            pts_w[j] = model->vert(face[j]);
        }
        Vec3f n = (pts_w[2]-pts_w[0])^(pts_w[1]-pts_w[0]);
        n.normalize();
        float intensity = n*light_dir;
        if (intensity>0) {
            triangleZBuffer(pts_s, zbuffer, image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}

