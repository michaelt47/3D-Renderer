#include <iostream>
#include <vector>
#include <future>
#include <cmath>
#include <chrono>
#include <SDL2/SDL.h>
#include "include/vec3.hpp"
#include "include/triangle.hpp"
#include "include/renderedObj.hpp"

using std::cout, std::endl, std::cin;

thread_local int lastShadowed = -1;

enum lightType {
    AMBIENT, POINT, DIRECTIONAL
};

struct Vec2 {
    float x, y;
};

struct Light {
    lightType type;
    float intensity;
    Vec3 pos;
};

RenderedObj teapot;

Light lights[3] = {
    {AMBIENT, 0.2f},
    {POINT, 0.6f, Vec3(2, 1, 0)},
    {DIRECTIONAL, 0.2f, Vec3(1, 4, 4)}
};

float rotationSpeed = 0.05f;
float moveSpeed = 0.5f;

int RES_X = 500;
int RES_Y = 500;

#define BACKGROUND_COLOR {50, 50, 50}

int INF = 16777215;
float TMIN = 0.05f;

int reflectRecursion = 3;

float distance = 1.0f;

int FOV = 53;

float aspectRatio = (float)RES_X / RES_Y;
float fovScale = tanf(FOV * 0.5f * M_PI / 180.0f); // if FOV in degrees
float aspectTimesFovScale = aspectRatio * fovScale;

bool wKeyDown, aKeyDown, sKeyDown, dKeyDown, qKeyDown, eKeyDown, spaceKeyDown, shiftKeyDown, backspaceKeyDown;

Vec3 cameraPos = {0, 0, -50};
Vec3 cameraForward = {0, 0, 1};
Vec3 cameraUp = {0, 1, 0};
Vec3 cameraRight = {1, 0, 0};

Vec3 rotateAroundAxis(const Vec3& vec, const Vec3& axis, float angle) {
    Vec3 k = axis.normalized();
    float cosTheta = std::cos(angle);
    float sinTheta = std::sin(angle);
    Vec3 vCrossK = k.cross(vec);
    float kDotV = k.dot(vec);

    return {
        vec.x * cosTheta + vCrossK.x * sinTheta + k.x * kDotV * (1 - cosTheta),
        vec.y * cosTheta + vCrossK.y * sinTheta + k.y * kDotV * (1 - cosTheta),
        vec.z * cosTheta + vCrossK.z * sinTheta + k.z * kDotV * (1 - cosTheta)
    };
}

void setPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
    Uint8 *p = (Uint8*)surface->pixels + y * surface->pitch + x * 4; // 4 being bytes per pixel
    *(Uint32*)p = color;
}

Vec3 canvasToViewport(int x, int y) {
    float u = ((x + 0.5f) / RES_X * 2 - 1) * aspectTimesFovScale;
    float v = (1 - (y + 0.5f) / RES_Y * 2) * fovScale;

    return (cameraForward + cameraRight * u + cameraUp * v).normalized();
}

Vec3 reflectRay(Vec3 ray, Vec3 normal) {
    return normal * normal.dot(ray) * 2 - ray;
}

HitIndexAndDistance closestIntersection(RenderedObj obj, Vec3 origin, Vec3 dir, float tMax) {
    float closestT = tMax;
    Triangle *closestTriangle = nullptr;

    return obj.checkRayIntersect(origin, dir, tMax);
}

bool isShadowed(Vec3 origin, Vec3 dir, float tMax, RenderedObj obj) {
    for (int i = 0; i < obj.triangles.size(); i++) {

        const Triangle &tri = obj.triangles[i];
        const Vec3 &normal = obj.normals[i];

        float t;
        if (obj.intersectRayTriangle(origin, dir, tri, normal, tMax, t)) {
            if (t > TMIN && t < tMax) {
                return true;
            }
        }
    }
    return false;
}


float computeLighting(Vec3 point, Vec3 normal, Vec3 pointToCamera, int specular) {
    float intensity = 0.0f;
    for (int i = 0; i < sizeof(lights) / sizeof(Light); i++) {
        Light &light = lights[i];
        if (light.type == AMBIENT) {
            intensity += light.intensity;
        } else {
            Vec3 L;
            float tMax;
            if (light.type == POINT) {
                L = light.pos - point;
                tMax = 1;
            } else { // type is directional
                L = light.pos;
                tMax = (float)INF;
            }

            // Shadow check
            if (isShadowed(point, L, tMax, teapot)) {
                continue;
            }

            // Diffuse
            float n = normal.dot(L);
            if (n > 0) {
                intensity += light.intensity * n / (normal.mag() * L.mag());
            }

            // Specular
            if (specular != -1) {
                Vec3 R = normal * normal.dot(L) * 2 - L;
                float rv = R.dot(pointToCamera);
                if (rv > 0) {
                    intensity += light.intensity * pow(rv / (R.mag() * pointToCamera.mag()), specular);
                }
            }
        }
    }
    return intensity;
}

Vec3 traceRay(Vec3 origin, Vec3 dir, int recursionDepth) {
    HitIndexAndDistance triangleHit = closestIntersection(teapot, origin, dir, (float)INF);
    
    if (triangleHit.hitIndex == -1) {

        return BACKGROUND_COLOR;
    }  
    
    float closestT = triangleHit.t;
    Triangle closestTriangle = teapot.triangles[triangleHit.hitIndex];
    
    // Compute local color
    Vec3 P = origin + dir * closestT;
    Vec3 normal = teapot.normals[triangleHit.hitIndex];
    Vec3 localColor = teapot.color * computeLighting(P, normal, dir * -1, teapot.specular);

    // if we hit the recursion limit or the object is not reflective, we're done
    float r = teapot.reflectiveness;
    if (recursionDepth <= 0 || r <= 0) {
        return localColor;
    }

    // compute the reflected color
    Vec3 R = reflectRay(dir * -1, normal);
    Vec3 reflectedColor = traceRay(P, R, recursionDepth - 1);

    return localColor * (1 - r) + reflectedColor * r;
}

void renderTile(SDL_Surface *surface, int startX, int startY, int tileSize) {
    for (int y = startY; y < startY + tileSize && y < RES_Y; y++) {
        for (int x = startX; x < startX + tileSize && x < RES_X; x++) {
            Vec3 rayDir = canvasToViewport(x, y);
            Vec3 color = traceRay(cameraPos, rayDir, reflectRecursion);
            color.x = std::min(color.x, 255.0f);
            color.y = std::min(color.y, 255.0f);
            color.z = std::min(color.z, 255.0f);
            Uint32 mappedColor = SDL_MapRGBA(surface->format, color.x, color.y, color.z, 255);
            setPixel(surface, x, y, mappedColor);
        }
    }
}


int TILE = 128;



void drawScene(SDL_Surface *surface) {
    std::vector<std::future<void>> futures;

    for (int yy = 0; yy < RES_Y; yy += TILE) {
        for (int xx = 0; xx < RES_X; xx += TILE) {
            futures.emplace_back(std::async(std::launch::async, renderTile, surface, xx, yy, TILE));
        }
    }

    // wait for threads to finish
    for (auto &f : futures) {
        f.get();
    }
}

void pollKeysPressed() {
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    wKeyDown = state[SDL_SCANCODE_W];
    aKeyDown = state[SDL_SCANCODE_A];
    sKeyDown = state[SDL_SCANCODE_S];
    dKeyDown = state[SDL_SCANCODE_D];
    qKeyDown = state[SDL_SCANCODE_Q];
    eKeyDown = state[SDL_SCANCODE_E];
    spaceKeyDown = state[SDL_SCANCODE_SPACE];
    shiftKeyDown = state[SDL_SCANCODE_LSHIFT];
    backspaceKeyDown = state[SDL_SCANCODE_BACKSPACE];
}

int main() {
    SDL_Surface *winSurface = NULL;
    SDL_Window *window = NULL;
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        cout << "Error initializing SDL: " << SDL_GetError() << endl;
        cin.get();
        return 1;
    }

    window = SDL_CreateWindow("Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, RES_X, RES_Y, SDL_WINDOW_SHOWN);
    if (!window) {
        cout << "Error creating window: " << SDL_GetError() << endl;
        cin.get();
        return 1;
    }

    winSurface = SDL_GetWindowSurface(window);
    if (!winSurface) {
        cout << "Error getting window surface: " << SDL_GetError() << endl;
        cin.get();
        return 1;
    }

    teapot.loadObjFile("../cube.obj");


    teapot.color = {255, 0, 0};
    teapot.reflectiveness = 0.2;
    teapot.specular = 10;

    int frameCount = 0;
    auto startTime = std::chrono::high_resolution_clock::now();

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        pollKeysPressed();

        float yaw = 0.0f;
        float pitch = 0.0f;
        float roll = 0.0f;
        float move = 0.0f;

        if (aKeyDown) yaw -= 1.0f;
        if (dKeyDown) yaw += 1.0f;
        if (sKeyDown) pitch += 1.0f;
        if (wKeyDown) pitch -= 1.0f;
        if (qKeyDown) roll += 1.0f;
        if (eKeyDown) roll -= 1.0f;
        if (spaceKeyDown) move += 1.0f;
        if (shiftKeyDown) move -= 1.0f;

        yaw *= rotationSpeed;
        pitch *= rotationSpeed;
        roll *= rotationSpeed;

        // yaw
        cameraForward = rotateAroundAxis(cameraForward, cameraUp, yaw);
        cameraRight = rotateAroundAxis(cameraRight, cameraUp, yaw);

        // pitch
        cameraForward = rotateAroundAxis(cameraForward, cameraRight, pitch);
        cameraUp = rotateAroundAxis(cameraUp, cameraRight, pitch);

        // roll
        cameraUp = rotateAroundAxis(cameraUp, cameraForward, roll);
        cameraRight = rotateAroundAxis(cameraRight, cameraForward, roll);


        cameraForward = cameraForward.normalized();
        cameraUp = cameraUp.normalized();
        cameraRight = cameraRight.normalized();

        if (backspaceKeyDown) {
            cameraPos = {0, 0, 0};
        } else {
            cameraPos = cameraPos + cameraForward * (move * moveSpeed);
        }
        
        frameCount++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();

        if (elapsed >= 1) {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            startTime = currentTime;
        }
        // drawBackground(winSurface);
        drawScene(winSurface);
        SDL_UpdateWindowSurface(window);
        SDL_Delay(8);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
