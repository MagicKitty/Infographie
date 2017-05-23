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


void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, Vec2f *uv, float intensity) {
    Vec3f A = pts[0];
    Vec3f B = pts[1];
    Vec3f C = pts[2];
    int minx = std::min(A.x,std::min(B.x,C.x));
    int maxx = std::max(A.x,std::max(B.x,C.x));
    int miny = std::min(A.y,std::min(B.y,C.y));
    int maxy = std::max(A.y,std::max(B.y,C.y));
    double dx10=B.x-A.x;
    double dx02=A.x-C.x;
    double dx21=C.x-B.x;
    double dy10=B.y-A.y;
    double dy02=A.y-C.y;
    double dy21=C.y-B.y;
    Vec3f P;
    double alpha=0.;
    double beta=0.;
    float u=0.;
    float v=0.;
    Vec2f UV;
    for(int j=miny;j<maxy;j++){
        double dyj0=j-A.y;
        double dyj2=j-C.y;
        double dyj1=j-B.y;
        for(int i=minx;i<maxx;i++){
            if(((dx10*dyj0-dy10*(i-A.x)>=0)&&
            (dx02*dyj2-dy02*(i-C.x)>=0)&&
            (dx21*dyj1-dy21*(i-B.x)>=0))||
            ((dx10*dyj0-dy10*(i-A.x)<=0)&&
            (dx02*dyj2-dy02*(i-C.x)<=0)&&
            (dx21*dyj1-dy21*(i-B.x)<=0))){
                P.x=i;P.y=j;
                alpha = ((B.y-C.y)*(P.x-C.x)+(C.x-B.x)*(P.y-C.y))/((B.y-C.y)*(A.x-C.x)+(C.x-B.x)*(A.y-C.y));
                beta = ((C.y-A.y)*(P.x-C.x)+(A.x-C.x)*(P.y-C.y))/((B.y-C.y)*(A.x-C.x)+(C.x-B.x)*(A.y-C.y));
                P.z = alpha*A.z+beta*B.z+C.z*(1-alpha-beta);
                u = alpha*uv[0][0]+beta*uv[1][0]+uv[2][0]*(1-alpha-beta);
                v = alpha*uv[0][1]+beta*uv[1][1]+uv[2][1]*(1-alpha-beta);
                UV[0]=u;UV[1]=v;
                if (zbuffer[int(i+j*width)]<P.z) {
                    zbuffer[int(i+j*width)]=P.z;
		            TGAColor color = model->diffuse(UV);
                    image.set(i,j,TGAColor(color[2]*intensity,color[1]*intensity,color[0]*intensity));
                }
            }
        }
    }
}

void drawZBuffer(float *zbuffer,TGAImage &zbufferImage) {
  float zmin=1e10, zmax=-1e10;
  for (int i=0; i<width*height; i++) {
    float z = zbuffer[i];
    if (z<-1e10) continue;
    zmin = std::min(zmin, z);
    zmax = std::max(zmax, z);
  }

  for(int j=0;j<width;j++) {
    for(int i=0;i<height;i++) {
      float z = zbuffer[i+j*width];
      if (z<-1e10) continue;
      zbufferImage.set(i,j,255.*(z-zmin)/(zmax-zmin));
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
    TGAImage zbufferImage(width, height, TGAImage::GRAYSCALE);
    Vec3f light_dir(0,0,-1);
    float *zbuffer = new float[width*height];
    for (int i=0;i<width*height; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec3f pts_s[3];
        Vec3f pts_w[3];
        Vec2f uv[3];
        for (int j=0; j<3; j++){
            pts_s[j] = world2screen(model->vert(face[j]));
            uv[j]=model->uv(i,j);
            pts_w[j] = model->vert(face[j]);
        }
        Vec3f n = cross((pts_w[2]-pts_w[0]),(pts_w[1]-pts_w[0]));
        n.normalize();
        float intensity = n*light_dir;
        if (intensity>0) {
	        triangle(pts_s, zbuffer, image, uv, intensity);
        }
    }

    drawZBuffer(zbuffer,zbufferImage);

    image.flip_vertically();
    image.write_tga_file("output.tga");
    zbufferImage.flip_vertically();
    zbufferImage.write_tga_file("zbuffer.tga");
    delete model;
    return 0;
}
