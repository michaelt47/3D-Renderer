#include <iostream>
#include <vector>
#include <SDL2/SDL.h>

float rotationSpeed = 0.5f;
float moveSpeed = 2.0f;

int RES_X = 640;
int RES_Y = 480;

float aspectRatio = static_cast<float>(RES_X) / RES_Y;
int FOV = 110;
float fovRadians = 110.0f * M_PI / 180.0f;
float halfHeight = tan(fovRadians / 2.0f);
float halfWidth = halfHeight * aspectRatio;

bool wKeyDown, aKeyDown, sKeyDown, dKeyDown, qKeyDown, eKeyDown;

std::vector<float> cameraPos(3);

std::vector<float> cameraForward = {0, 0, -1};
std::vector<float> cameraUp = {0, 1, 0};


float get3DMag(std::vector<float> &vec) {
        return std::sqrt(vec.at(0) * vec.at(0) + vec.at(1) * vec.at(1) + vec.at(2) * vec.at(2));
}

void normalize3D(std::vector<float> &vec) {
    float mag = get3DMag(vec);
    if (mag == 0.0f) return;
    vec[0] /= mag;
    vec[1] /= mag;
    vec[2] /= mag;
}

void rotateZ(std::vector<float> &vec, float angle) {
    float x = vec[0] * cos(angle) - vec[1] * sin(angle);
    float y = vec[0] * sin(angle) + vec[1] * cos(angle);
    vec[0] = x;
    vec[1] = y;
}

void rotateY(std::vector<float> &vec, float angle) {
    float x = vec[0] * cos(angle) + vec[2] * sin(angle);
    float z = -vec[0] * sin(angle) + vec[2] * cos(angle);
    vec[0] = x;
    vec[2] = z;
}

void rotateX(std::vector<float> &vec, float angle) {
    float y = vec[1] * cos(angle) - vec[2] * sin(angle);
    float z = vec[1] * sin(angle) + vec[2] * cos(angle);
    vec[1] = y;
    vec[2] = z;
}

using std::cout, std::endl, std::cin;

void setPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) return;

    // likely might need to add a surface mutex

    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
    *(Uint32*)p = color;
}

void drawBackground(SDL_Surface *surface) {

    std::vector<float> right = {
        cameraForward[1] * cameraUp[2] - cameraForward[2] * cameraUp[1],
        cameraForward[2] * cameraUp[0] - cameraForward[0] * cameraUp[2],
        cameraForward[0] * cameraUp[1] - cameraForward[1] * cameraUp[0]
    };

    normalize3D(right);

    for (int y = 0; y < RES_Y; y++) {
        for (int x = 0; x < RES_X; x++) {
            float u = (x + 0.5f) / RES_X * 2 - 1; // -1 to 1
            float v = (y + 0.5f) / RES_Y * 2 - 1;
            u *= halfWidth;
            v *= halfHeight;

            std::vector<float> rayDir = {
                cameraForward[0] + right[0] * u + cameraUp[0] * v,
                cameraForward[1] + right[1] * u + cameraUp[1] * v,
                cameraForward[2] + right[2] * u + cameraUp[2] * v
            };
            normalize3D(rayDir);

            float t = 10.0f;
            std::vector<float> point = {
                cameraPos[0] + rayDir[0] * t,
                cameraPos[1] + rayDir[1] * t,
                cameraPos[2] + rayDir[2] * t
            };

            Uint8 r = static_cast<Uint8>(std::max(0.0f, std::min(1.0f, point[0])) * 255.0f);
            Uint8 g = static_cast<Uint8>(std::max(0.0f, std::min(1.0f, point[1])) * 255.0f);
            Uint8 b = static_cast<Uint8>(std::max(0.0f, std::min(1.0f, point[2])) * 255.0f);

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
}

int main() {
    SDL_Surface *winSurface = NULL;
    SDL_Window *window = NULL;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        cout << "Error initializing SDL: " << SDL_GetError() << endl;
        cin.get();
        return 1;
    }


    window = SDL_CreateWindow("Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, RES_X, RES_Y, SDL_WINDOW_SHOWN);
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

    SDL_FillRect(winSurface, NULL, SDL_MapRGB(winSurface->format, 255, 255, 255));

    SDL_UpdateWindowSurface(window);

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        pollKeysPressed();

        int move = 0;
        int rotate = 0;
        int roll = 0;

        if (wKeyDown) {
            move += 1; // move forwards
        }
        if (sKeyDown) {
            move += -1; // move backwards
        }
        if (dKeyDown) {
            rotate += -1; // rotate right
        }
        if (aKeyDown) {
            rotate += 1; // rotate left
        }
        if (qKeyDown) {
            roll += -1; // roll left
        }
        if (eKeyDown) {
            roll += 1; // roll right
        }

        // 1. Apply yaw to forward and up (rotate around Y)
        rotateY(cameraForward, rotate * rotationSpeed);
        rotateY(cameraUp, rotate * rotationSpeed);

        // 2. Apply pitch if you want to look up/down (optional)
        // rotateX(cameraForward, pitchAmount);
        // rotateX(cameraUp, pitchAmount);

        // 3. Apply roll to up and forward vectors (rotate around Z)
        rotateZ(cameraUp, roll * rotationSpeed);
        rotateZ(cameraForward, roll * rotationSpeed);

        normalize3D(cameraForward);
        normalize3D(cameraUp);

        // 6. Move camera along forward vector
        cameraPos[0] += cameraForward[0] * move * moveSpeed;
        cameraPos[1] += cameraForward[1] * move * moveSpeed;
        cameraPos[2] += cameraForward[2] * move * moveSpeed;



        drawBackground(winSurface);
        SDL_UpdateWindowSurface(window);

        SDL_Delay(10); // avoid maxing out CPU
    }

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}