
//==================================================================================================
// Written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is distributed
// without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication along
// with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==================================================================================================

#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "random.h"
#include "sphere.h"

#include <string.h>
#include <float.h>
#include <iostream>
#include <omp.h>
#include <chrono>
#include <ctime>

int specifiedNumThreads = 4;


vec3 color(const ray& r, hittable *world, int depth) {
    hit_record rec;
    if (world->hit(r, 0.001, FLT_MAX, rec)) {
        ray scattered;
        vec3 attenuation;
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
             return attenuation*color(scattered, world, depth+1);
        }
        else {
            return vec3(0,0,0);
        }
    }
    else {
        vec3 unit_direction = unit_vector(r.direction());
        float t = 0.5*(unit_direction.y() + 1.0);
        return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
    }
}


hittable *random_scene() {
    int n = 500;
    hittable **list = new hittable*[n+1];
    list[0] =  new sphere(vec3(0,-1000,0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = random_double();
            vec3 center(a+0.9*random_double(),0.2,b+0.9*random_double());
            if ((center-vec3(4,0.2,0)).length() > 0.9) {
                if (choose_mat < 0.8) {  // diffuse
                    list[i++] = new sphere(
                        center, 0.2,
                        new lambertian(vec3(random_double()*random_double(),
                                            random_double()*random_double(),
                                            random_double()*random_double()))
                    );
                }
                else if (choose_mat < 0.95) { // metal
                    list[i++] = new sphere(
                        center, 0.2,
                        new metal(vec3(0.5*(1 + random_double()),
                                       0.5*(1 + random_double()),
                                       0.5*(1 + random_double())),
                                  0.5*random_double())
                    );
                }
                else {  // glass
                    list[i++] = new sphere(center, 0.2, new dielectric(1.5));
                }
            }
        }
    }

    list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
    list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
    list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

    return new hittable_list(list,i);
}


int main(int argc, char* argv[]) {
    int nx = 640;
    int ny = 480;
    int ns = 10;
    bool output = false;
    FILE* fout;
    int ***outputArray;

    if(argc > 1) {
        for(int i = 1; argv[i]; i++) {
            if(!strcmp(argv[i], "-size")) {
                if(argc > i+1) {
                    nx = atoi(argv[i+1]);
                }
                else {
                    std::cerr << "ERROR: -size not given size value x\nUSAGE: main <-size width height> <-threads number> <-output>\n";
                    exit(-1);
                }
                if(argc > i+2) {
                    ny = atoi(argv[i+2]);
                }
                else {
                    std::cerr << "ERROR: -size not given size value y\nUSAGE: main <-size width height> <-threads number> <-output>\n";
                    exit(-1);
                }
            }
            if(!strcmp(argv[i], "-output")) {
                output = true;
                fout = fopen("image.ppm", "w");
            }
            if(!strcmp(argv[i], "-threads")) {
                if(argc > i+1) {
                    specifiedNumThreads = atoi(argv[i+1]);
                }
                else {
                    std::cerr << "ERROR: -threads not given number\nUSAGE: main <-size width height> <-threads number> <-output>\n";
                    exit(-1);
                }
            }
        }
    }

    if(output) fprintf(fout, "P3\n%d %d\n255\n", nx, ny);
    hittable *world = random_scene();

    vec3 lookfrom(13,2,3);
    vec3 lookat(0,0,0);
    float dist_to_focus = 10.0;
    float aperture = 0.1;

    camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus);

    auto start = std::chrono::system_clock::now();
    
    if(output) {
        outputArray = (int***)malloc(sizeof(int**) * nx);
        for(int k = 0; k < nx; k++) {
            outputArray[k] = (int**)malloc(sizeof(int*) * ny);
            for(int n = 0; n < ny; n++) {
                outputArray[k][n] = (int*)malloc(sizeof(int) * 3);
            }
        }
    }

    omp_set_num_threads(specifiedNumThreads);
    #pragma omp parallel for collapse(2)
    for (int j = ny-1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            vec3 col(0, 0, 0);
            for (int s=0; s < ns; s++) {
                float u = float(i + random_double()) / float(nx);
                float v = float(j + random_double()) / float(ny);
                ray r = cam.get_ray(u, v);
                col += color(r, world,0);
            }
            col /= float(ns);
            col = vec3( sqrt(col[0]), sqrt(col[1]), sqrt(col[2]) );
            int ir = int(255.99*col[0]);
            int ig = int(255.99*col[1]);
            int ib = int(255.99*col[2]);
            if(output) {
                outputArray[i][j][0] = ir;
                outputArray[i][j][1] = ig;
                outputArray[i][j][2] = ib;
            }
        }
    }

    if(output) {
        for(int j = ny-1; j >= 0; j--){
            for(int i = 0; i < nx; i++) {
                fprintf(fout, "%d %d %d\n", outputArray[i][j][0],outputArray[i][j][1],outputArray[i][j][2]);
            }
        }
        for(int i = 0; i < nx; i++){
            for(int j = 0; j < ny; j++){
                free(outputArray[i][j]);
            }
            free(outputArray[i]);
        }
        free(outputArray);
        fclose(fout);
    }

    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> secs = end - start;
    std::cout << "Threads: " << specifiedNumThreads << "\nTime: " << secs.count() << "\n";
}

