#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <omp.h>

#define WIDTH 900
#define HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_GRAY 0xefefefef
#define COLOR_YELLOW 0x00ffff00
#define COLOR_RED 0xffff0000
#define COLOR_GREEN 0xff00ff00
#define true 1
#define false 0
#define RAYS_NUMBER 360
#define OBSTACLES_NUMBER 3
#define RAY_WIDTH 8
#define TIMEOUT 1000
struct Circle {
    double x;
    double y;
    double r;
};

struct Ray {
    double x_start;
    double y_start;
    double theta;

};

void FillCircle(SDL_Surface *screen, struct Circle circle, Uint32 color) {
    double radius_squared = pow(circle.r, 2);
    for (double x = circle.x-circle.r; x<=circle.x+circle.r; x++) {
        for (double y=circle.y-circle.r; y<=circle.y+circle.r; y++) {
            double distance_squared = pow(x - circle.x, 2) + pow(y - circle.y, 2);
            if (distance_squared < radius_squared) {
                SDL_Rect pixel = (SDL_Rect){x, y, 1, 1};
                SDL_FillRect(screen, &pixel, color);
            }
        }
    }
}

void* generate_rays(struct Circle circle, struct Ray rays[RAYS_NUMBER], struct Circle obstacles[OBSTACLES_NUMBER]){
    #pragma omp parallel for
    for (int i=0; i<RAYS_NUMBER; i++) {
        double angle = M_PI * 2 * ((double)i / RAYS_NUMBER);
        rays[i] = (struct Ray){(circle.x + sin(angle)*(circle.r+0.1)), (circle.y + cos(angle)*(circle.r+0.1)), angle};

    }
    fflush(stdout);

}

void FillRays(SDL_Surface *screen, struct Ray rays[RAYS_NUMBER], Uint32 color, struct Circle obstacles[]) {
    #pragma omp parallel for
    for (int i=0; i<RAYS_NUMBER; i++) {
        double x = rays[i].x_start;
        double y = rays[i].y_start;
        int isGoing = 1;
        while (x<=WIDTH && y<=HEIGHT && x>0 && y>0 && isGoing) {
        #pragma omp parallel for
            for (int j=0; j<OBSTACLES_NUMBER; j++) {
                if (pow(x-obstacles[j].x, 2) + pow(y-obstacles[j].y, 2) <= pow(obstacles[j].r-1, 2)) {
                    isGoing = false;
                }
            }
            if (isGoing) {
                SDL_Rect pixel = (SDL_Rect){x, y, RAY_WIDTH, RAY_WIDTH};
                SDL_FillRect(screen, &pixel, color);
                x+=sin(rays[i].theta)*2;
                y+=cos(rays[i].theta)*2;
            }
        }

    }
}
int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Raytracing ohio edition", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Surface* screen = SDL_GetWindowSurface(window);
    SDL_Rect erase_rect = (SDL_Rect){0, 0, WIDTH, HEIGHT};


    //SDL_FillRect(screen, &rect, COLOR_WHITE);
    SDL_UpdateWindowSurface(window);
    struct Circle circle = {200, 200, 80};
    struct Circle shadow_circle = {WIDTH / 2 + 150, HEIGHT / 2, 140};
    struct Circle shadow_circle2 = {WIDTH / 2, HEIGHT / 2+150, 40};

    struct Circle obstacles[OBSTACLES_NUMBER];
    obstacles[0] = circle;
    obstacles[1] = shadow_circle;
    obstacles[2] = shadow_circle2;

    struct Ray rays[RAYS_NUMBER];
    generate_rays(circle, rays, obstacles);
    struct Ray other_rays[RAYS_NUMBER];
    generate_rays(shadow_circle, other_rays, obstacles);
    struct Ray other_rays2[RAYS_NUMBER];
    generate_rays(shadow_circle2, other_rays2, obstacles);



    int simulation_running = 1;
    int selectedCircle = -1;
    SDL_Event event;

    while (simulation_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                simulation_running = 0;
            }
            if (event.type == SDL_MOUSEMOTION & event.motion.state != 0) {
                if (selectedCircle == 0) {
                    circle.x = event.motion.x;
                    circle.y = event.motion.y;
                    generate_rays(circle, rays, obstacles);
                    generate_rays(shadow_circle, other_rays, obstacles);
                    generate_rays(shadow_circle2, other_rays2, obstacles);
                    obstacles[0] = circle;
                }else if (selectedCircle == 1) {
                    shadow_circle.x = event.motion.x;
                    shadow_circle.y = event.motion.y;
                    obstacles[1] = shadow_circle;
                    generate_rays(circle, rays, obstacles);
                    generate_rays(shadow_circle, other_rays, obstacles);
                    generate_rays(shadow_circle2, other_rays2, obstacles);
                }else if (selectedCircle == 2) {
                    shadow_circle2.x = event.motion.x;
                    shadow_circle2.y = event.motion.y;
                    obstacles[2] = shadow_circle2;
                    generate_rays(circle, rays, obstacles);
                    generate_rays(shadow_circle, other_rays, obstacles);
                    generate_rays(shadow_circle2, other_rays2, obstacles);
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT & selectedCircle == -1) {
                    if (event.button.x > circle.x - circle.r && event.button.x < circle.x + circle.r &
                    event.button.y > circle.y - circle.r && event.button.y < circle.y + circle.r) {
                        selectedCircle = 0;

                    }else if (event.button.x > shadow_circle.x - shadow_circle.r && event.button.x < shadow_circle.x + shadow_circle.r &
                            event.button.y > shadow_circle.y - shadow_circle.r && event.button.y < shadow_circle.y + shadow_circle.r) {

                        selectedCircle = 1;

                    }else if (event.button.x > shadow_circle2.x - shadow_circle2.r && event.button.x < shadow_circle2.x + shadow_circle2.r &
                            event.button.y > shadow_circle2.y - shadow_circle2.r && event.button.y < shadow_circle2.y + shadow_circle2.r) {

                        selectedCircle = 2;

                    }
                }
            }
            if (event.type == SDL_MOUSEBUTTONUP) {
                selectedCircle = -1;
            }
        }
        SDL_FillRect(screen, &erase_rect, COLOR_BLACK);
        FillCircle(screen, circle, COLOR_WHITE);
        FillCircle(screen, shadow_circle, COLOR_WHITE);
        FillCircle(screen, shadow_circle2, COLOR_WHITE);
        FillRays(screen, rays, COLOR_YELLOW, obstacles);
        FillRays(screen, other_rays, COLOR_RED, obstacles);
        FillRays(screen, other_rays2, COLOR_GREEN, obstacles);
        SDL_UpdateWindowSurface(window);
        SDL_Delay(5); //200 fps
    }


}