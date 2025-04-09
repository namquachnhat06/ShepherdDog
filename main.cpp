// morning_game_fullscreen.cpp
#include <SDL.h>
#include <SDL_image.h>
#include <bits/stdc++.h>

// Kích thước cửa sổ mặc định (sẽ được thay đổi khi fullscreen)
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Hàm load texture từ file
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

// --- Entity cơ sở ---
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

// --- Con chó ---
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
        if (rect.x + rect.w > SCREEN_WIDTH) rect.x =SCREEN_WIDTH - rect.w;
        if (rect.y + rect.h > SCREEN_WIDTH) rect.y =SCREEN_HEIGHT - rect.h;
    }
    void Update() override {}
};

// --- Con cừu ---
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
        if (rect.x < 0) rect.x=0;
        if (rect.y < 0) rect.y=0;
        if (rect.x + rect.w > SCREEN_WIDTH) rect.x =SCREEN_WIDTH - rect.w;
        if (rect.y + rect.h > SCREEN_WIDTH) rect.y =SCREEN_HEIGHT - rect.h;
    }
    void Update() override {}
};

// Hàm kiểm tra cừu đã vào chuồng chưa
bool IsInPen(SDL_Rect sheep, SDL_Rect pen) {
    return SDL_HasIntersection(&sheep, &pen);
}

int main(int argc, char* argv[]) {
    // Khởi tạo SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Init error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Khởi tạo SDL_image, hỗ trợ định dạng PNG
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image Init error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Tạo cửa sổ
    SDL_Window* window = SDL_CreateWindow(
    "Buổi sáng - Lùa cừu",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    1000, 600, // width, height = 0 để SDL tự chọn độ phân giải theo fullscreen
    SDL_WINDOW_FULLSCREEN
);
    if (!window) {
        std::cerr << "Không tạo được cửa sổ: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    // Tạo renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Không tạo được renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    // Load texture cho background, chuồng, chó, cừu
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

    // Khởi tạo đối tượng chó
    Dog dog;
    dog.rect = { 100, 100, 64, 64 };
    dog.texture = dogTexture;

    // Tạo danh sách cừu
    std::vector<Sheep> sheeps;
    for (int i = 0; i < 10; ++i) {
        Sheep s;
        s.rect = { rand() % 400 +50, rand() % 400 +100, 48, 48 };
        s.texture = sheepTexture;
        sheeps.push_back(s);
    }

    // Khu vực chuồng (vị trí và kích thước)
    SDL_Rect penRect = { 650, 150, 220, 300 };

    // Biến lưu trạng thái fullscreen (mặc định false)
    bool isFullscreen = false;

    bool running = true;
    SDL_Event e;
    while (running) {
        // Xử lý sự kiện
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_f: // Nhấn "F" để chuyển đổi fullscreen
                    {
                        isFullscreen = !isFullscreen;
                        if (isFullscreen) {
                            if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
                                std::cerr << "Lỗi chuyển sang fullscreen: " << SDL_GetError() << std::endl;
                            }
                        } else {
                            if (SDL_SetWindowFullscreen(window, 0) != 0) {
                                std::cerr << "Lỗi chuyển ra cửa sổ: " << SDL_GetError() << std::endl;
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
            if (e.type == SDL_KEYDOWN) {
                SDL_Keycode key = e.key.keysym.sym;
                if (key == SDLK_ESCAPE) {
                    running = false;
                }
                else if (key == SDLK_f) {
                    isFullscreen = !isFullscreen;
                    if (isFullscreen) {
                        if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
                            std::cerr << "Lỗi chuyển sang fullscreen: " << SDL_GetError() << std::endl;
                        }
                    }
                    else {
                        if (SDL_SetWindowFullscreen(window, 0) != 0) {
                            std::cerr << "Lỗi chuyển ra cửa sổ: " << SDL_GetError() << std::endl;
                        }
                    }
                }
            }
        }

        // Xử lý bàn phím cho con chó (sử dụng trạng thái bàn phím)
        const Uint8* keystates = SDL_GetKeyboardState(NULL);
        dog.HandleInput(keystates);

        // Cập nhật trạng thái của các con cừu (né khi chó đến gần)
        for (auto& sheep : sheeps) {
            sheep.UpdateAvoidDog(dog.rect);
        }

        // Kiểm tra cừu nào đã vào chuồng
        sheeps.erase(std::remove_if(sheeps.begin(), sheeps.end(),
            [&](Sheep& s) { return IsInPen(s.rect, penRect); }), sheeps.end());

        // Render:
        // Vẽ background toàn màn hình
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

        // Vẽ chuồng, nếu có texture chuồng, dùng đó; nếu không, vẽ hình chữ nhật
        if (penTexture) {
            SDL_RenderCopy(renderer, penTexture, NULL, &penRect);
        } else {
            SDL_SetRenderDrawColor(renderer, 200, 100, 50, 255);
            SDL_RenderFillRect(renderer, &penRect);
        }

        // Vẽ đối tượng chó và cừu
        dog.Render(renderer);
        for (auto& s : sheeps) {
            s.Render(renderer);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    // Giải phóng tài nguyên
    SDL_DestroyTexture(bgTexture);
    if (penTexture)
        SDL_DestroyTexture(penTexture);
    SDL_DestroyTexture(dogTexture);
    SDL_DestroyTexture(sheepTexture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
