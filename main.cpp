#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <bits/stdc++.h>

const int SCREEN_WIDTH = 980;
const int SCREEN_HEIGHT = 700;
const int LANE_HEIGHT = 200;
const int DAY_DURATION = 74000;
const int NIGHT_DURATION = 74000;

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

bool IsOverLapping(const SDL_Rect& a, const SDL_Rect& b) {
    return SDL_HasIntersection(&a, &b);
}


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
    void UpdateAvoidDog(SDL_Rect dogRect, std::vector<Sheep>& allSheeps) {
        float dx = rect.x - dogRect.x;
        float dy = rect.y - dogRect.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < 100 && dist > 0.1f) {
            float speed = 2.0f;
            rect.x += static_cast<int>((dx / dist) * speed);
            rect.y += static_cast<int>((dy / dist) * speed);
        }

        // Tránh các con cừu khác
        for (auto& otherSheep : allSheeps) {
            if (&otherSheep == this) continue; // Bỏ qua chính nó
            if (IsOverLapping(rect, otherSheep.rect)) {
                // Đẩy ra xa dựa trên hướng
                int push = 1.5; // Khoảng cách đẩy mỗi lần
                if (rect.x < otherSheep.rect.x) {
                    rect.x -= push;
                    otherSheep.rect.x += push;
                } else {
                    rect.x += push;
                    otherSheep.rect.x -= push;
                }
                if (rect.y < otherSheep.rect.y) {
                    rect.y -= push;
                    otherSheep.rect.y += push;
                } else {
                    rect.y += push;
                    otherSheep.rect.y -= push;
                }
            }
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
                MoveUp();
                lastMoveTime = currentTime;
            }
            if (keystates[SDL_SCANCODE_DOWN] && currentLane < 2) {
                MoveDown();
                lastMoveTime = currentTime;
            }
        }
    }

    void Update() override {}
};

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned int>(time(0)));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 || !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL Init lỗi: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "Lỗi Mix_OpenAudio: " << Mix_GetError() << std::endl;
        return 1;
    }

    Mix_Music* dayMusic = Mix_LoadMUS("day_music.mp3");
    Mix_Music* nightMusic = Mix_LoadMUS("night_music.mp3");
    Mix_Chunk* sheepSound = Mix_LoadWAV("sheep_sound.wav");
    Mix_Chunk* bulletSound = Mix_LoadWAV("bullet_sound.wav");
    Mix_Chunk* monsterSound = Mix_LoadWAV("monster_sound.wav");

    if (!dayMusic || !nightMusic || !sheepSound || !bulletSound || !monsterSound) {
        std::cout << "Lỗi tải âm thanh: " << Mix_GetError() << std::endl;
        return 1;
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

    SDL_Texture* winTex  = LoadTexture(renderer, "win.png");
    SDL_Texture* loseTex = LoadTexture(renderer, "lose.png");
    Mix_Chunk* winSound = Mix_LoadWAV("win.wav");
    Mix_Chunk* loseSound = Mix_LoadWAV("lose.wav");

    if (!winTex || !loseTex || !winSound || !loseSound) {
        std::cerr << "Lỗi tải tài nguyên kết quả: " << IMG_GetError() << " hoặc " << Mix_GetError() << std::endl;
        return 1;
    }

    int monstersPassed = 0;
    int monstersKilled = 0;
    bool inResult = false;
    bool isWin = false;

    Dog dog;
    dog.rect = { 100, 100, 64, 64 };
    dog.texture = dogTex;

    std::vector<Sheep> sheeps;
    std::vector<Uint32> sheepRespawnTime;

    for (int i = 0; i < 10; ++i) {
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

    Uint32 nightStart = 0;
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

        if (inDay ) {
            static bool dayMusicPlaying = false;
            if (!dayMusicPlaying) {
                Mix_PlayMusic(dayMusic, -1);
                dayMusicPlaying = true;
            }
            dog.HandleInput(keys);

            for (auto& s : sheeps) {
                s.UpdateAvoidDog(dog.rect, sheeps);
            }

            std::vector<Sheep> remaining;
            for (auto& s : sheeps) {
                if (IsInPen(s.rect, penRect)) {
                    Mix_PlayChannel(-1, sheepSound, 0);
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
                Mix_HaltMusic();
                dayMusicPlaying = false;
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
                nightStart = SDL_GetTicks();
                lastMonsterSpawn = SDL_GetTicks();
                Mix_PlayMusic(nightMusic, -1);
            }
        }
        else if (inNight) {
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            player.HandleInput(keys, now);

            if (keys[SDL_SCANCODE_SPACE] && !bulletOnScreen) {
                Bullet b;
                b.rect = { player.rect.x + player.rect.w, player.rect.y - 20, 75, 100 };
                b.texture = bulletTex;
                bullets.push_back(b);
                bulletOnScreen = true;
                Mix_PlayChannel(-1, bulletSound, 0);
            }

            for (auto& b : bullets) b.Update();
            bullets.erase(
                std::remove_if(bullets.begin(), bullets.end(),
                               [&](Bullet& b) { return b.rect.x > SCREEN_WIDTH; }),
                bullets.end()
            );
            if (bullets.empty()) bulletOnScreen = false;

            if (now - lastMonsterSpawn > monsterInterval) {
                Monster m;
                int lane = rand() % 3;
                m.rect = { SCREEN_WIDTH, 100 + lane * LANE_HEIGHT, 64, 64 };
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
                        monstersKilled++;
                        Mix_PlayChannel(-1, monsterSound, 0);
                        b.rect.x = SCREEN_WIDTH + 100;
                        break;
                    }
                }
                if (!hit && m.alive) newMonsters.push_back(m);
            }
            monsters = newMonsters;

            for (auto& m : monsters) {
                if (m.rect.x < player.rect.x && m.alive) {
                    monstersPassed++;
                    m.alive = false;
                }
            }

            if (monstersPassed >= 3) {
                inNight = false;
                inResult = true;
                isWin = false;
                Mix_HaltMusic();
                Mix_PlayChannel(-1, loseSound, 0);
            }

            if (now - nightStart > NIGHT_DURATION) {
                inNight = false;
                inResult = true;
                isWin = (monstersPassed <= 3);
                Mix_HaltMusic();
                Mix_PlayChannel(-1, isWin ? winSound : loseSound, 0);
            }

            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, bgNight, NULL, NULL);
            SDL_RenderCopy(renderer, penTex, NULL, &nightPen);
            player.Render(renderer);
            for (auto& b : bullets) b.Render(renderer);
            for (auto& m : monsters) m.Render(renderer);
            SDL_RenderPresent(renderer);
}
        else if (inResult) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                    running = false;
                }
            }

            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, bgDay, NULL, NULL);

            SDL_Rect resultRect = { (SCREEN_WIDTH - 300) / 2, (SCREEN_HEIGHT - 200) / 2, 300, 200 };
            SDL_RenderCopy(renderer, isWin ? winTex : loseTex, NULL, &resultRect);

            SDL_RenderPresent(renderer);
        }
        Uint32 frameTime = SDL_GetTicks() - startTicks;
        if (frameTime < 1000 / 60) {
            SDL_Delay((1000 / 60) - frameTime);
        }
    }

    Mix_FreeMusic(dayMusic);
    Mix_FreeMusic(nightMusic);
    Mix_FreeChunk(sheepSound);
    Mix_FreeChunk(bulletSound);
    Mix_FreeChunk(monsterSound);
    Mix_CloseAudio();

    SDL_DestroyTexture(winTex);
    SDL_DestroyTexture(loseTex);
    Mix_FreeChunk(winSound);
    Mix_FreeChunk(loseSound);

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
