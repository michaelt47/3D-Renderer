#include <iostream>
#include <vector>
#include <cmath>
#include <SDL2/SDL.h>

using std::cout, std::endl, std::cin;

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
};

float rotationSpeed = 0.05f;
float moveSpeed = 2.0f;

int RES_X = 960;
int RES_Y = 540;

float aspectRatio = static_cast<float>(RES_X) / RES_Y;
int FOV = 110;
float fovRadians = 110.0f * M_PI / 180.0f;
float halfHeight = tan(fovRadians / 2.0f);
float halfWidth = halfHeight * aspectRatio;

bool wKeyDown, aKeyDown, sKeyDown, dKeyDown, qKeyDown, eKeyDown, spaceKeyDown, shiftKeyDown, backspaceKeyDown;

Vec3 cameraPos = {0, 0, 0};
Vec3 cameraForward = {0, 0, -1};
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

void drawBackground(SDL_Surface *surface) {
    Vec3 right = cameraForward.cross(cameraUp).normalized();

    for (int y = 0; y < RES_Y; y++) {
        for (int x = 0; x < RES_X; x++) {
            float u = (x + 0.5f) / RES_X * 2 - 1;
            float v = (y + 0.5f) / RES_Y * 2 - 1;
            u *= halfWidth;
            v *= halfHeight;

            Vec3 rayDir = (cameraForward + cameraRight * u + cameraUp * v).normalized();

            float distance = 20.0f;
            Vec3 point = {
                cameraPos.x + rayDir.x * distance,
                cameraPos.y + rayDir.y * distance,
                cameraPos.z + rayDir.z * distance
            };

            Uint8 r = static_cast<Uint8>(std::max(0.0f, std::min(1.0f, point.x)) * 255.0f);
            Uint8 g = static_cast<Uint8>(std::max(0.0f, std::min(1.0f, point.y)) * 255.0f);
            Uint8 b = static_cast<Uint8>(std::max(0.0f, std::min(1.0f, point.z)) * 255.0f);

            Uint32 color = SDL_MapRGBA(surface->format, r, g, b, 255);
            setPixel(surface, x, y, color);
        }
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

        if (aKeyDown) yaw += 1.0f;
        if (dKeyDown) yaw -= 1.0f;
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
        

        drawBackground(winSurface);
        SDL_UpdateWindowSurface(window);
        SDL_Delay(8);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
