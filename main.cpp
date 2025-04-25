#include <SDL.h>
#include <SDL_image.h>
#include <bits/stdc++.h>

const int SCREEN_WIDTH = 980;
const int SCREEN_HEIGHT = 700;
const int LANE_HEIGHT = 200;
const int DAY_DURATION = 120000; // 2 phút

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
        if (texture) SDL_RenderCopy(renderer, texture, NULL, &rect);
        else SDL_RenderFillRect(renderer, &rect);
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
        if (rect.x < 0) rect.x = 0;
        if (rect.y < 180) rect.y = 180;
        if (rect.x > SCREEN_WIDTH - rect.w) rect.x = SCREEN_WIDTH - rect.w;
        if (rect.y > SCREEN_HEIGHT - rect.h) rect.y = SCREEN_HEIGHT - rect.h;
    }
    void Update() override {}
};

class Sheep : public Entity {
public:
    void UpdateAvoidDog(SDL_Rect dogRect) {
        float dx = rect.x - dogRect.x;
        float dy = rect.y - dogRect.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < 100 && dist > 0.1f) {
            float factor = 2.0f;
            rect.x += static_cast<int>((dx / dist) * factor);
            rect.y += static_cast<int>((dy / dist) * factor);
        }
        if (rect.x < 0) rect.x = 0;
        if (rect.y < 180) rect.y = 180;
        if (rect.x > SCREEN_WIDTH - rect.w) rect.x = SCREEN_WIDTH - rect.w;
        if (rect.y > SCREEN_HEIGHT - rect.h) rect.y = SCREEN_HEIGHT - rect.h;
    }
    void Update() override {}
};

bool IsInPen(SDL_Rect sheep, SDL_Rect pen) {
    return SDL_HasIntersection(&sheep, &pen);
}
class Bullet : public Entity {
public:
    bool active = true;
    void Update() override {
        rect.x += 10;
        if (rect.x > SCREEN_WIDTH) active = false;
    }
};

class Monster : public Entity {
public:
    bool alive = true;
    void Update() override {
        rect.x -= 2;
        if (rect.x + rect.w < 0) alive = false;
    }
};

class Player : public Entity {
public:
    int currentLane = 1;
    Uint32 lastMoveTime = 0;
    const Uint32 moveDelay = 200;

    void MoveUp(){
        if (rect.y > 100)
            rect.y-=LANE_HEIGHT;
    }
    void MoveDown(){
        if (rect.y + rect.h < 100 + 2 * LANE_HEIGHT)
            rect.y+=LANE_HEIGHT;
    }
    void HandleInput(const Uint8* keystates, Uint32 currentTime) {
        if (currentTime - lastMoveTime >= moveDelay) {
            if (keystates[SDL_SCANCODE_UP] && currentLane > 0) {
                currentLane--;
                lastMoveTime = currentTime;
            }
            if (keystates[SDL_SCANCODE_DOWN] && currentLane < 2) {
                currentLane++;
                lastMoveTime = currentTime;
            }
        }
        rect.y = 100 + currentLane * LANE_HEIGHT;
    }

    void Update() override {}
};

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned int>(time(0)));

    if (SDL_Init(SDL_INIT_VIDEO) < 0 || !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL Init lỗi: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("ShepherdDog",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* bgDay   = LoadTexture(renderer, "bg.png");
    SDL_Texture* bgNight = LoadTexture(renderer, "night.png");
    SDL_Texture* transImg= LoadTexture(renderer, "transition.png");
    SDL_Texture* penTex  = LoadTexture(renderer, "pen.png");
    SDL_Texture* dogTex  = LoadTexture(renderer, "dog.png");
    SDL_Texture* sheepTex= LoadTexture(renderer, "sheep.png");
    SDL_Texture* playerTex = LoadTexture(renderer, "player.png");
    SDL_Texture* monsterTex= LoadTexture(renderer, "monster.png");
    SDL_Texture* bulletTex = LoadTexture(renderer, "bullet.png");

    Dog dog;
    dog.rect = { 100, 100, 64, 64 };
    dog.texture = dogTex;

    std::vector<Sheep> sheeps;
    std::vector<Uint32> sheepRespawnTime;

    for (int i = 0; i < 8; ++i) {
        Sheep s;
        s.rect = { rand() % 700, rand() % 500, 48, 48 };
        s.texture = sheepTex;
        sheeps.push_back(s);
    }

    SDL_Rect penRect = { 800, 200, 160, 250 };
    Uint32 dayStart = SDL_GetTicks();
    bool inDay = true;
    bool showTransition = false;
    Uint32 transitionStart = 0;

    bool inNight = false;
    Player player;
    player.rect = { 250, 100 + 1 * LANE_HEIGHT, 64, 64 };
    player.texture = playerTex;

    std::vector<Monster> monsters;
    std::vector<Bullet> bullets;
    Uint32 lastMonsterSpawn = 0;
    Uint32 monsterInterval = 2000;
    bool bulletOnScreen = false;

    SDL_Event e;
    bool running = true;
    Uint32 lastArrowPress = 0;
    Uint32 arrowDelay = 150;
    SDL_Rect nightPen = { 100, 100, 160,250 };
    Uint32 startTicks;
    while (running) {
        startTicks = SDL_GetTicks();
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        Uint32 now = SDL_GetTicks();
        const Uint8* keys = SDL_GetKeyboardState(NULL);

        if (inDay) {
            dog.HandleInput(keys);

            for (auto& s : sheeps) {
                s.UpdateAvoidDog(dog.rect);
            }

            std::vector<Sheep> remaining;
            for (auto& s : sheeps) {
                if (IsInPen(s.rect, penRect)) {
                    sheepRespawnTime.push_back(now + 10000);
                } else {
                    remaining.push_back(s);
                }
            }
            sheeps = remaining;

            auto it = sheepRespawnTime.begin();
            while (it != sheepRespawnTime.end()) {
                if (now >= *it) {
                    Sheep s;
                    s.rect = { rand() % 700, rand() % 500, 48, 48 };
                    s.texture = sheepTex;
                    sheeps.push_back(s);
                    it = sheepRespawnTime.erase(it);
                } else ++it;
            }

            if (now - dayStart > DAY_DURATION) {
                inDay = false;
                showTransition = true;
                transitionStart = now;
            }

            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, bgDay, NULL, NULL);
            SDL_RenderCopy(renderer, penTex, NULL, &penRect);
            dog.Render(renderer);
            for (auto& s : sheeps) s.Render(renderer);
            SDL_RenderPresent(renderer);
        }

        else if (showTransition) {
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, transImg, NULL, NULL);
            SDL_RenderPresent(renderer);
            if (now - transitionStart > 3000) {
                showTransition = false;
                inNight = true;
            }
        }
        else if (inNight) {

            if (keys[SDL_SCANCODE_UP] && now - lastArrowPress > arrowDelay) {
                player.MoveUp();
                lastArrowPress = now;
            } else if (keys[SDL_SCANCODE_DOWN] && now - lastArrowPress > arrowDelay) {
                player.MoveDown();
                lastArrowPress = now;
            }
            if (keys[SDL_SCANCODE_SPACE] && !bulletOnScreen) {
                Bullet b;
                b.rect = { player.rect.x + player.rect.w, player.rect.y + 32, 75, 100 };
                b.texture = bulletTex;
                bullets.push_back(b);
                bulletOnScreen = true;
            }

            for (auto& b : bullets) b.Update();
            bullets.erase(
                std::remove_if(bullets.begin(), bullets.end(),
                               [&](Bullet& b) {
                                   return b.rect.x > SCREEN_WIDTH;
                               }),
                bullets.end()
            );
            if (bullets.empty()) bulletOnScreen = false;

            if (now - lastMonsterSpawn > monsterInterval) {
                Monster m;
                int lane = rand() % 3;
                m.rect = { SCREEN_WIDTH, 100 + lane * LANE_HEIGHT, 80, 80 };
                m.texture = monsterTex;
                monsters.push_back(m);
                lastMonsterSpawn = now;
            }

            for (auto& m : monsters) m.Update();

            std::vector<Monster> newMonsters;
            for (auto& m : monsters) {
                bool hit = false;
                for (auto& b : bullets) {
                    if (SDL_HasIntersection(&m.rect, &b.rect)) {
                        hit = true;
                        b.rect.x = SCREEN_WIDTH + 100;
                        break;
                    }
                }
                if (!hit) newMonsters.push_back(m);
            }
            monsters = newMonsters;

            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, bgNight, NULL, NULL);
            SDL_RenderCopy(renderer, penTex, NULL, &nightPen);
            player.Render(renderer);
            for (auto& b : bullets) b.Render(renderer);
            for (auto& m : monsters) m.Render(renderer);
            SDL_RenderPresent(renderer);
        }
        Uint32 frameTime = SDL_GetTicks() - startTicks;
        if (frameTime < 1000 / 60) {
            SDL_Delay((1000 / 60) - frameTime);
        }
    }

    SDL_DestroyTexture(bgDay);
    SDL_DestroyTexture(bgNight);
    SDL_DestroyTexture(dogTex);
    SDL_DestroyTexture(sheepTex);
    SDL_DestroyTexture(penTex);
    SDL_DestroyTexture(playerTex);
    SDL_DestroyTexture(monsterTex);
    SDL_DestroyTexture(bulletTex);
    SDL_DestroyTexture(transImg);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
