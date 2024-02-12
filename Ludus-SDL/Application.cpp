#include <iostream>
#include <SDL.h>
#include <stdio.h>
#include <box2d/box2d.h>
#include "Entity.cpp"
#include <vector>
#undef main

App app;
Player player_entity;
SDL_Texture* background;
#define SCREEN_WIDTH   640
#define SCREEN_HEIGHT  480
int prevTime = 0;
int currentTime = 0;
float deltaTime = 0;
b2Vec2 gravity(0.0f, 9.8f);
b2World world(gravity);
int velocityIterations = 8;
int positionIterations = 3;
std::list<Rusher> all_my_rushers;
std::list<Loner> all_my_loners;
std::list<MetalDebris> all_my_metaldebris;
std::list<BigStoneDebris> all_my_bigstonedebris;
std::list<MediumStoneDebris> all_my_mediumstonedebris;
std::list<SmallStoneDebris> all_my_smallstonedebris;
std::list<Drone> all_my_drones;
std::list<Explosion> my_explosions;
std::list<WeaponUpgrade> all_my_weaponupgrades;
std::list<HealthPickup> all_my_healthpickups;
std::list<CompanionPickup> all_my_companionspickup;
std::list<Companion> all_my_companions;
SDL_Texture* explosions_texture;
float elapsedTime = 0.0f;
float spawnInterval = 0.3f;
int dronesSpawned = 0;


void cleanup(void) {
    player_entity.clean();
    for (auto& rushers : all_my_rushers)
    {
        rushers.clean();
    }
    for (auto& loners : all_my_loners)
    {
        loners.clean();
    }
    for (auto& drone : all_my_drones)
    {
        drone.clean();
    }
    for (auto& explosions : my_explosions)
    {
        explosions.clean();
    }
    for (auto& debris : all_my_metaldebris)
    {
        debris.clean();
    }
    for (auto& debris : all_my_bigstonedebris)
    {
        debris.clean();
    }
    SDL_DestroyRenderer(app.renderer);
    app.renderer = NULL;
    SDL_DestroyWindow(app.window);
    app.window = NULL;
}

SDL_Texture* LoadTexture(const char* filePath, SDL_Renderer* renderTarget) {
    SDL_Texture* texture = nullptr;
    SDL_Surface* surface = SDL_LoadBMP(filePath);

    if (surface == NULL)
        std::cout << "Error loading image" << filePath << std::endl;
    else
    {
        SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 0, 255));
        texture = SDL_CreateTextureFromSurface(renderTarget, surface);
        if (texture == NULL)
            std::cout << "Error creating texture" << std::endl;
    }

    SDL_FreeSurface(surface);

    return texture;
}

void spawnDrones(int maxDrones, float deltaTime, int positionX, int positionY)
{
    if (dronesSpawned < maxDrones)
    {
        elapsedTime += deltaTime;

        if (elapsedTime >= spawnInterval)
        {
            std::cout << "spawned drone" << std::endl;
            all_my_drones.push_back(Drone(positionX, positionY, LoadTexture("Resources/drone.bmp", app.renderer), 8, 0, 2, 0, 2, 1, SDL_GetTicks(), &world));
            elapsedTime = 0.0f;
            dronesSpawned++;
        }
    }

}

void spawnObjects(float deltaTime)
{
    SDL_Texture* EnemyWeapon_Texture = LoadTexture("Resources/EnWeap6.bmp", app.renderer);
    SDL_Texture* Metal_Debris_Texture = LoadTexture("Resources/MAster96.bmp", app.renderer);
    SDL_Texture* Stone_Debris_Texture = LoadTexture("Resources/SAster96.bmp", app.renderer);
    SDL_Texture* RusherTexture = LoadTexture("Resources/rusher.bmp", app.renderer);
    SDL_Texture* LonerTexture = LoadTexture("Resources/LonerA.bmp", app.renderer);
    SDL_Texture* WeaponPickupTexture = LoadTexture("Resources/PUWeapon.bmp", app.renderer);

    struct SpawnDelay {
        float delay;
    };

    std::vector<SpawnDelay> spawnSequence = {
        {1.0f},   
        {3.0f},    
        {5.0f},
        {5.0f},

    };

    static float elapsedTime = 0.0f;
    static int currentSpawnIndex = 0;
    elapsedTime += deltaTime;

    if (currentSpawnIndex < spawnSequence.size()) {
        if (elapsedTime >= spawnSequence[currentSpawnIndex].delay) {
                switch (currentSpawnIndex) {
                case 0:
                    all_my_loners.push_back(Loner(EnemyWeapon_Texture, 650, 100, LonerTexture, 4, 0, 4, 0, 3, 3, &world, 30, 40));
                    all_my_loners.push_back(Loner(EnemyWeapon_Texture, -20, 100, LonerTexture, 4, 0, 4, 0, 3, 3, &world, -30, 40));

                    break;
                case 1:
                    all_my_rushers.push_back(Rusher(120, -10, RusherTexture, 4, 0, 6, 0, 5, 2, &world));
                    all_my_rushers.push_back(Rusher(220, -10, RusherTexture, 4, 0, 6, 0, 5, 2, &world));
                    all_my_rushers.push_back(Rusher(320, -10, RusherTexture, 4, 0, 6, 0, 5, 2, &world));
                    all_my_rushers.push_back(Rusher(420, -10, RusherTexture, 4, 0, 6, 0, 5, 2, &world));
                    all_my_rushers.push_back(Rusher(520, -10, RusherTexture, 4, 0, 6, 0, 5, 2, &world));
                    break;
                case 2:
                    all_my_weaponupgrades.push_back(WeaponUpgrade(320, 5, WeaponPickupTexture, 4, 0, 2, 0, 0, &world));
                    break;
                case 3:
                        spawnDrones(5, deltaTime, 320, -10);

                    break;
                case 4:
                    all_my_metaldebris.push_back(MetalDebris(320, -10, Metal_Debris_Texture, 5, 0, 5, 0, -1, 3, &world));
                    all_my_metaldebris.push_back(MetalDebris(120, -10, Metal_Debris_Texture, 5, 0, 5, 0, -1, 3, &world));
                    all_my_metaldebris.push_back(MetalDebris(520, -10, Metal_Debris_Texture, 5, 0, 5, 0, -1, 3, &world));
                    all_my_bigstonedebris.push_back(BigStoneDebris(220, -50, Stone_Debris_Texture, 5, 0, 5, 0, 5, 1, &world));
                    all_my_bigstonedebris.push_back(BigStoneDebris(420, -50, Stone_Debris_Texture, 5, 0, 5, 0, 5, 1, &world));
                    break;
                }
            
            elapsedTime = 0.0f;
            ++currentSpawnIndex;
            if (currentSpawnIndex >= spawnSequence.size())
                currentSpawnIndex = 0;
        }
    }
}

void init(void)
{
    int rendererFlags, windowFlags;

    rendererFlags = SDL_RENDERER_ACCELERATED;

    windowFlags = SDL_WINDOW_OPENGL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    app.window = SDL_CreateWindow("Xenon 2000 clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);

    if (!app.window)
    {
        printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);

    if (!app.renderer)
    {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Texture* EnemyWeapon_Texture = LoadTexture("Resources/EnWeap6.bmp", app.renderer);
    SDL_Texture* Debris_Texture = LoadTexture("Resources/MAster96.bmp", app.renderer);
    explosions_texture = LoadTexture("Resources/explode64.bmp", app.renderer);

    background = LoadTexture("Resources/galaxy2.bmp", app.renderer);
    player_entity = Player(LoadTexture("Resources/missile.bmp", app.renderer), 320, 400, LoadTexture("Resources/ship1.bmp", app.renderer), 7, 3, 
        1, 0, 5, LoadTexture("Resources/clone.bmp", app.renderer), &world);

}

void doInput(float deltaTime)
{
    SDL_Event event;
    SDL_PumpEvents();
    bool movedHorizontally = false;

    const Uint8* keyState = SDL_GetKeyboardState(NULL);

    while (SDL_PollEvent(&event) != 0)
    {

        if (event.type == SDL_QUIT || keyState[SDL_SCANCODE_ESCAPE])
            exit(0);
    }

    const float playerSpeed = 200.0f;

    if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D]) {
        player_entity.move(playerSpeed, 0.0f, deltaTime, 1);
        movedHorizontally = true;
    }
    else if (keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A]) {
        player_entity.move(-playerSpeed, 0.0f, deltaTime, 0);
        movedHorizontally = true;
    }
    if (keyState[SDL_SCANCODE_DOWN] || keyState[SDL_SCANCODE_S])
        player_entity.move(0.0f, playerSpeed, deltaTime, -1);
    else if (keyState[SDL_SCANCODE_UP] || keyState[SDL_SCANCODE_W])
        player_entity.move(0.0f, -playerSpeed, deltaTime, -1);

    if (!movedHorizontally)
        player_entity.stop(deltaTime);

    if (keyState[SDL_SCANCODE_SPACE])
        player_entity.shoot();


}

void prepareScene(void)
{
    SDL_RenderClear(app.renderer);
    SDL_RenderCopy(app.renderer, background, NULL, NULL);

    player_entity.renderToApplication(app, &world);

    for (auto& metaldebris : all_my_metaldebris)
    {
        metaldebris.renderToApplication(app);
    }
    for (auto& bigdebris : all_my_bigstonedebris)
    {
        bigdebris.renderToApplication(app);
    }
    for (auto& mediumdebris : all_my_mediumstonedebris)
    {
        mediumdebris.renderToApplication(app);
    }
    for (auto& smalldebris : all_my_smallstonedebris)
    {
        smalldebris.renderToApplication(app);
    }
    for (auto& weaponupgrade : all_my_weaponupgrades)
    {
        weaponupgrade.renderToApplication(app);
    }
    for (auto& healthpickup : all_my_healthpickups)
    {
        healthpickup.renderToApplication(app);
    }
    for (auto& companions : all_my_companions)
    {
        companions.renderToApplication(app);
    }
    for (auto& companionpickups : all_my_companionspickup)
    {
        companionpickups.renderToApplication(app);
    }
    for (auto& enemies : all_my_rushers)
    {
        enemies.renderToApplication(app);
    }
    for (auto& drone : all_my_drones)
    {
        drone.renderToApplication(app);
    }
    for (auto& loners : all_my_loners)
    {
        loners.renderToApplication(app);
    }
    for (auto& explosions : my_explosions)
    {
        explosions.renderToApplication(app);
    }

}

void gameLogic(void)
{
    spawnObjects(deltaTime);
    world.Step(deltaTime, 1, 1);

    player_entity.update(deltaTime);

    all_my_rushers.remove_if([&](Rusher& rusher) {
        if (player_entity.doCollision(rusher)) {
            rusher.onHit(player_entity.getDamage());
            if (rusher.isDead())
            {
                my_explosions.push_back(Explosion(rusher.getPosition().x, rusher.getPosition().y,
                    LoadTexture("Resources/explode64.bmp", app.renderer), 5, 0, 2, 0, 0));
                return true;
            }
        }
        if (rusher.OOB())
        {
            return true;
        }
        
        return false;
        });

    all_my_loners.remove_if([&](Loner& loner) {
        if (player_entity.doCollision(loner)) {
            loner.onHit(player_entity.getDamage());
            if (loner.isDead())
            {
                my_explosions.push_back(Explosion(loner.getPosition().x, loner.getPosition().y,
                    LoadTexture("Resources/explode64.bmp", app.renderer), 5, 0, 2, 0, 0));
                return true;
            }
        }
        if (loner.OOB())
        {
            return true;
        }
        return false;
        });

    all_my_drones.remove_if([&](Drone& drone) {
        if (player_entity.doCollision(drone)) {
            drone.onHit(player_entity.getDamage());
            if (drone.isDead())
            {
                my_explosions.push_back(Explosion(drone.getPosition().x, drone.getPosition().y,
                    LoadTexture("Resources/explode64.bmp", app.renderer), 5, 0, 2, 0, 0));
                return true;
            }
        }
        if (drone.OOB())
        {
            return true;
        }
        return false;
        });

    all_my_bigstonedebris.remove_if([&](BigStoneDebris& bigdebris) {
        if (player_entity.doCollision(bigdebris)) {
            my_explosions.push_back(Explosion(bigdebris.getPosition().x, bigdebris.getPosition().y,
                LoadTexture("Resources/explode64.bmp", app.renderer), 5, 0, 2, 0, 0));
            all_my_mediumstonedebris.push_back(MediumStoneDebris(false, bigdebris.getPosition().x + 15, bigdebris.getPosition().y,
                LoadTexture("Resources/SAster64.bmp", app.renderer), 8, 0, 3, 0, 1, 1, &world));
            all_my_mediumstonedebris.push_back(MediumStoneDebris(true, bigdebris.getPosition().x - 15, bigdebris.getPosition().y,
                LoadTexture("Resources/SAster64.bmp", app.renderer), 8, 0, 3, 0, 1, 1, &world));

            return true;
        }
        if (bigdebris.OOB())
        {
            return true;
        }
        return false;
        });

    all_my_mediumstonedebris.remove_if([&](MediumStoneDebris& mediumdebris) {
        if (player_entity.doCollision(mediumdebris)) {
            my_explosions.push_back(Explosion(mediumdebris.getPosition().x, mediumdebris.getPosition().y,
                LoadTexture("Resources/explode64.bmp", app.renderer), 5, 0, 2, 0, 1));
            all_my_smallstonedebris.push_back(SmallStoneDebris(false, mediumdebris.getPosition().x + 15, mediumdebris.getPosition().y,
                LoadTexture("Resources/SAster32.bmp", app.renderer), 8, 0, 2, 0, 1, 1, &world));
            all_my_smallstonedebris.push_back(SmallStoneDebris(true, mediumdebris.getPosition().x - 15, mediumdebris.getPosition().y,
                LoadTexture("Resources/SAster32.bmp", app.renderer), 8, 0, 2, 0, 1, 1, &world));

            return true;
        }
        if (mediumdebris.OOB())
        {
            return true;
        }
        return false;
        });

    all_my_smallstonedebris.remove_if([&](SmallStoneDebris& smalldebris) {
        if (player_entity.doCollision(smalldebris)) {
            my_explosions.push_back(Explosion(smalldebris.getPosition().x, smalldebris.getPosition().y,
                LoadTexture("Resources/explode64.bmp", app.renderer), 5, 0, 2, 0, 0));
            return true;
        }
        if (smalldebris.OOB())
        {
            return true;
        }
        return false;
        });

    all_my_weaponupgrades.remove_if([&](WeaponUpgrade& weaponupgrade) {
        if (weaponupgrade.getCollision()) {
            return true;
        }
        if (weaponupgrade.OOB())
        {
            return true;
        }
        return false;
        });

    all_my_healthpickups.remove_if([&](HealthPickup& healthpickup) {
        if (healthpickup.getCollision()) {
            return true;
        }
        if (healthpickup.OOB())
        {
            return true;
        }
        return false;
        });
    all_my_companionspickup.remove_if([&](CompanionPickup& companionpickup) {
        if (companionpickup.getCollision()) {
            return true;
        }
        if (companionpickup.OOB())
        {
            return true;
        }
        return false;
        });

    all_my_metaldebris.remove_if([&](MetalDebris& metaldebris) {
        if (metaldebris.OOB())
        {
            return true;
        }
        if (player_entity.doCollision(metaldebris)) {
            my_explosions.push_back(Explosion(metaldebris.getPosition().x, metaldebris.getPosition().y,
                LoadTexture("Resources/explode64.bmp", app.renderer), 5, 0, 2, 0, 0));
            return false;
        }
        return false;
        });

    my_explosions.remove_if([&](Explosion& explosion) {
        return explosion.isAnimationFinished();
        });

    for (Explosion& explosion : my_explosions) {
        explosion.update(deltaTime);
    }

    for (Rusher& rusher : all_my_rushers) {
        rusher.update(deltaTime, &player_entity, &my_explosions, explosions_texture);

    }

    for (Loner& loner : all_my_loners) {
        loner.update(deltaTime, &player_entity, app, &my_explosions, explosions_texture);

    }
    for (MetalDebris& metaldebris : all_my_metaldebris)
    {
        metaldebris.update(deltaTime, &player_entity, &my_explosions, explosions_texture);
    }
    for (BigStoneDebris& bigdebris : all_my_bigstonedebris)
    {
        bigdebris.update(deltaTime, &player_entity, &my_explosions, explosions_texture);
    }
    for (MediumStoneDebris& mediumdebris : all_my_mediumstonedebris)
    {
        mediumdebris.update(deltaTime, &player_entity, &my_explosions, explosions_texture);
    }
    for (SmallStoneDebris& smalldebris : all_my_smallstonedebris)
    {
        smalldebris.update(deltaTime, &player_entity, &my_explosions, explosions_texture);
    }
    for (WeaponUpgrade& weaponupgrade : all_my_weaponupgrades)
    {
        weaponupgrade.update(deltaTime, &player_entity, &my_explosions, explosions_texture);
    }
    for (HealthPickup& healthpickup : all_my_healthpickups)
    {
        healthpickup.update(deltaTime, &player_entity, &my_explosions, explosions_texture);
    }
    for (CompanionPickup& companionpickup : all_my_companionspickup)
    {
        companionpickup.update(deltaTime, &player_entity, &my_explosions, explosions_texture);
    }
    for (Companion& companions : all_my_companions)
    {
        companions.update(deltaTime);
    }
    for (Drone& drone : all_my_drones)
    {
        drone.update(deltaTime, &player_entity, &my_explosions, explosions_texture);
    }
}

void presentScene(void)
{
    SDL_RenderPresent(app.renderer);
}

int main(int argc, char* argv[])
{
    memset(&app, 0, sizeof(App));
    memset(&app, 0, sizeof(App));

    init();

    atexit(cleanup);

    while (1)
    {
        prevTime = currentTime;
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - prevTime) / 1000.0f;

        prepareScene();

        doInput(deltaTime);

        gameLogic();

        presentScene();

        SDL_Delay(16);
    }

    return 0;
}