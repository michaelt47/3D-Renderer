#include <iostream>
#include <vector>
#include <future>
#include <cmath>
#include <chrono>
#include <SDL2/SDL.h>

using std::cout, std::endl, std::cin;

thread_local int lastShadowed = -1;

enum lightType {
    AMBIENT, POINT, DIRECTIONAL
};

struct Vec2 {
    float x, y;
};

struct Vec3 {
    float x, y, z;

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const {
        return {
            y * o.z - z * o.y,
            z * o.x - x * o.z,
            x * o.y - y * o.x
        };
    }
    Vec3 normalized() const {
        float m = std::sqrt(x * x + y * y + z * z);
        return m > 0.0f ? *this * (1.0f / m) : *this;
    }
    float mag() const { return sqrt(x * x + y * y + z * z); }
};

struct Sphere {
    Vec3 origin;
    Vec3 color;
    float radius;
    float f;
    float reflectiveness;
    int specular;
    float rSquared;
};

struct Light {
    lightType type;
    float intensity;
    Vec3 pos;
};

Sphere spheres[4] = {
    {{0, -1, 3}, {255, 0, 0}, 1, 0, 0.2f, 500, 1},         // red, shiny, bit reflective
    {{2, 0, 4}, {0, 0, 255}, 1, 0, 0.3f, 500, 1},          // blue, shiny, bit more reflective
    {{-2, 0, 4}, {0, 255, 0}, 1, 0, 0.4f, 10, 1},          // green, somewhat shiny, more reflective
    {{0, -5001, 0}, {255, 255, 0}, 5000, 0, 0.5f, 1000, static_cast<float>(pow(5000, 2))} // yellow, very Shiny, half reflective
};

Sphere calculateBoundingSphere(Sphere *spheres, int numSpheres) {
    Vec3 avg = {0, 0, 0};
    for (int i = 0; i < numSpheres; i++) {
        avg.x += spheres[i].origin.x;
        avg.y += spheres[i].origin.y;
        avg.z += spheres[i].origin.z;
    }

    avg.x /= numSpheres;
    avg.y /= numSpheres;
    avg.z /= numSpheres;

    float radius = 0;
    for (int i = 0; i < numSpheres; i++) {
        Vec3 avgToSphere = avg - spheres[i].origin;
        Vec3 avgToSphereEdge = avgToSphere + avgToSphere.normalized() * spheres[i].radius;
        float mag = avgToSphereEdge.mag();
        if (mag > radius) {
            radius = mag;
        }
    }
    return Sphere {avg, {0,0,0}, radius, 0, 0, 0, static_cast<float>(pow(radius, 2))};
}

Sphere smallSphereBoundingSphere;

Light lights[3] = {{AMBIENT, 0.2}, {POINT, 0.6, {2, 1, 0}}, {DIRECTIONAL, 0.2, {1, 4, 4}}};

float rotationSpeed = 0.05f;
float moveSpeed = 0.5f;

int RES_X = 500;
int RES_Y = 500;

#define BACKGROUND_COLOR {0, 0, 0}

int INF = 16777215;
float TMIN = 0.05f;

int reflectRecursion = 3;

float distance = 1.0f;

int FOV = 53;

float aspectRatio = (float)RES_X / RES_Y;
float fovScale = tanf(FOV * 0.5f * M_PI / 180.0f); // if FOV in degrees
float aspectTimesFovScale = aspectRatio * fovScale;

bool wKeyDown, aKeyDown, sKeyDown, dKeyDown, qKeyDown, eKeyDown, spaceKeyDown, shiftKeyDown, backspaceKeyDown;

Vec3 cameraPos = {0, 0, 0};
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

Vec2 intersectRaySphere(Vec3 origin, Vec3 dir, float dotDir, Sphere sphere) {
    float r = sphere.radius;
    Vec3 co = origin - sphere.origin;

    float a = dotDir;
    float b = 2 * co.dot(dir);
    float c = co.dot(co) - sphere.rSquared;

    float discriminant = b*b - 4*a*c;
    if (discriminant < 0) {
        return Vec2 {(float)INF, (float)INF};
    }

    float t1 = (-b + sqrt(discriminant)) / (2*a);
    float t2 = (-b - sqrt(discriminant)) / (2*a);

    return Vec2 {t1, t2};
}

Sphere *closestIntersection(Vec3 origin, Vec3 dir, float tMax) {
    float closestT = tMax;
    Sphere *closestSphere = nullptr;
    float dotDir = dir.dot(dir);

    // custom bounding logic

    Vec2 t = intersectRaySphere(origin, dir, dotDir, smallSphereBoundingSphere);
    if (t.x != (float)INF || t.y != (float)INF) {
        for (int i = 0; i < 3; i++) {
            Vec2 t = intersectRaySphere(origin, dir, dotDir, spheres[i]);
            if (t.x > TMIN && t.x < tMax && t.x < closestT) {
                closestT = t.x;
                closestSphere = &spheres[i];
            }
            if (t.y > TMIN && t.y < tMax && t.y < closestT) {
                closestT = t.y;
                closestSphere = &spheres[i];
            }
        }
    } 
    t = intersectRaySphere(origin, dir, dotDir, spheres[3]);
    if (t.x > TMIN && t.x < tMax && t.x < closestT) {
        closestT = t.x;
        closestSphere = &spheres[3];
    }
    if (t.y > TMIN && t.y < tMax && t.y < closestT) {
        closestT = t.y;
        closestSphere = &spheres[3];
    }


    if (closestSphere != nullptr) closestSphere->f = closestT;
    return closestSphere;
}

bool isShadowed(Vec3 origin, Vec3 dir, float tMax, int lastShadowedSphere) {
    float dotDir = dir.dot(dir);

    if (lastShadowedSphere != -1) {
        Vec2 t = intersectRaySphere(origin, dir, dotDir, spheres[lastShadowedSphere]);
        if (t.x != (float)INF || t.y != (float)INF) {
            return false;
        }
    }

    for (int i = 0; i < sizeof(spheres) / sizeof(Sphere); i++) {
        Vec2 t = intersectRaySphere(origin, dir, dotDir, spheres[i]);
        if (t.x != (float)INF || t.y != (float)INF) {
            lastShadowed = i;
            return false;
        }
    }
    lastShadowed = -1;
    return true;
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
            if (isShadowed(point, L, tMax, lastShadowed)) {
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
    Sphere *closestSphere = closestIntersection(origin, dir, (float)INF);
    
    if (closestSphere == nullptr) {
        return BACKGROUND_COLOR;
    }
    
    float closestT = closestSphere->f;
    
    // Compute local color
    Vec3 P = origin + dir * closestT;
    Vec3 normal = (P - closestSphere->origin).normalized();
    Vec3 localColor = closestSphere->color * computeLighting(P, normal, dir * -1, closestSphere->specular);

    // if we hit the recursion limit or the object is not reflective, we're done
    float r = closestSphere->reflectiveness;
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

    smallSphereBoundingSphere = calculateBoundingSphere(spheres, 3); // just the smaller 3 spheres

    std::cout << smallSphereBoundingSphere.origin.x << " " << smallSphereBoundingSphere.origin.y << " " << smallSphereBoundingSphere.origin.z << ", " << smallSphereBoundingSphere.radius << std::endl;

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
