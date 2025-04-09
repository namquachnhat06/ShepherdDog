#include <SDL.h>
#include <SDL_image.h>
#include <bits/stdc++.h>

const int SCREEN_WIDTH = 980;
const int SCREEN_HEIGHT = 700;

SDL_Texture* LoadTexture(SDL_Renderer* renderer, const char* filePath) {
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) {
        std::cerr << "Không tải được ảnh: " << filePath
                  << " | Lỗi: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

struct Entity {
    SDL_Rect rect;
    SDL_Texture* texture;
    virtual void Update() = 0;
    virtual void Render(SDL_Renderer* renderer) {
        if (texture) {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
        } else {
            SDL_RenderFillRect(renderer, &rect);
        }
    }
};

class Dog : public Entity {
public:
    void HandleInput(const Uint8* keystates) {
        int speed = 3;
        if (keystates[SDL_SCANCODE_UP])    rect.y -= speed;
        if (keystates[SDL_SCANCODE_DOWN])  rect.y += speed;
        if (keystates[SDL_SCANCODE_LEFT])  rect.x -= speed;
        if (keystates[SDL_SCANCODE_RIGHT]) rect.x += speed;
        if (rect.x < 0) rect.x=0;
        if (rect.y < 0) rect.y=0;
        if (rect.x > SCREEN_WIDTH) rect.x =SCREEN_WIDTH;
        if (rect.y > SCREEN_HEIGHT) rect.y =SCREEN_HEIGHT;
    }
    void Update() override {}
};

class Sheep : public Entity {
public:
    void UpdateAvoidDog(SDL_Rect dogRect) {
        float dx = rect.x - dogRect.x;
        float dy = rect.y - dogRect.y;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist < 100 && dist > 0.1f) {
            float factor = 2.0f;
            rect.x += static_cast<int>((dx / dist) * factor);
            rect.y += static_cast<int>((dy / dist) * factor);
        }
        // Kiểm tra chạm viền, nếu có thì dịch chuyển ngẫu nhiên nhẹ để tránh kẹt
        bool hitWall = false;
        if (rect.x < 0) { rect.x = 0; hitWall = true; }
        if (rect.y < 0) { rect.y = 0; hitWall = true; }
        if (rect.x > SCREEN_WIDTH) { rect.x = SCREEN_WIDTH ; hitWall = true; }
        if (rect.y > SCREEN_HEIGHT) { rect.y = SCREEN_HEIGHT ; hitWall = true; }
        if (hitWall) {
            rect.x += (rand() % 11 - 5);
            rect.y += (rand() % 11 - 5);
        }
    }
    void Update() override {}
};

bool IsInPen(SDL_Rect sheep, SDL_Rect pen) {
    return SDL_HasIntersection(&sheep, &pen);
}

int main(int argc, char* argv[]) {
    struct PendingSheep {
        Uint32 spawnTime;
    };
std::vector<PendingSheep> sheepToSpawn;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Init error: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image Init error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "ShepherDog",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        980, 700,
        SDL_WINDOW_FULLSCREEN
    );
    if (!window) {
        std::cerr << "Không tạo được cửa sổ: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Không tạo được renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Texture* bgTexture    = LoadTexture(renderer, "bg.png");
    SDL_Texture* penTexture   = LoadTexture(renderer, "pen.png");
    SDL_Texture* dogTexture   = LoadTexture(renderer, "dog.png");
    SDL_Texture* sheepTexture = LoadTexture(renderer, "sheep.png");

    if (!bgTexture || !dogTexture || !sheepTexture) {
        std::cerr << "Không load được các texture cần thiết." << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    Dog dog;
    dog.rect = { 100, 100, 64, 64 };
    dog.texture = dogTexture;

    std::vector<Sheep> sheeps;
    for (int i = 0; i < 10; ++i) {
        Sheep s;
        s.rect = { rand() % 600, rand() % 400, 48, 48 };
        s.texture = sheepTexture;
        sheeps.push_back(s);
    }

    SDL_Rect penRect = { 650, 150, 220, 300 };
    bool isFullscreen = false;
    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
                else if (e.key.keysym.sym == SDLK_f) {
                    isFullscreen = !isFullscreen;
                    if (SDL_SetWindowFullscreen(window, isFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0) {
                        std::cerr << "Lỗi chuyển đổi fullscreen: " << SDL_GetError() << std::endl;
                    }
                }
            }
        }

        const Uint8* keystates = SDL_GetKeyboardState(NULL);
        dog.HandleInput(keystates);

        Uint32 now = SDL_GetTicks();
        auto it = sheepToSpawn.begin();
        while (it != sheepToSpawn.end()) {
            if (now >= it->spawnTime) {
                Sheep s;
                s.rect = { rand() % (SCREEN_WIDTH - 48), rand() % (SCREEN_HEIGHT - 48), 48, 48 };
                s.texture = sheepTexture;
                sheeps.push_back(s);
                it = sheepToSpawn.erase(it);
            } else {
                ++it;
            }
        }

        for (auto& sheep : sheeps) {
            sheep.UpdateAvoidDog(dog.rect);
        }

        std::vector<Sheep> remainingSheep;
        for (auto& s : sheeps) {
            if (IsInPen(s.rect, penRect)) {
                Uint32 spawnAt = SDL_GetTicks() + 10000; // 10 giây sau
                sheepToSpawn.push_back({ spawnAt });
            }
            else {
                remainingSheep.push_back(s);
            }
        }
sheeps = remainingSheep;

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
        if (penTexture) SDL_RenderCopy(renderer, penTexture, NULL, &penRect);
        dog.Render(renderer);
        for (auto& s : sheeps) s.Render(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(bgTexture);
    if (penTexture) SDL_DestroyTexture(penTexture);
    SDL_DestroyTexture(dogTexture);
    SDL_DestroyTexture(sheepTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
