#include <iostream>
#include <SDL.h>
#include <list>
#include <box2d/box2d.h>
#include <cmath>
#define PTM_RATIO 32.0f


typedef struct {
    SDL_Renderer* renderer;
    SDL_Window* window;
} App;

class Entity {
public:
    b2Body* body;
    Entity() : texture(nullptr), maxStatesW(0), currentStateW(0), maxStatesH(0), currentStateH(0) {}
    Entity(int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP) {
        if (currentStateW >= maxStatesW || currentStateH >= maxStatesH) {
            std::cout << "Invalid entity states" << std::endl;
            exit(1);
        }

        int textureWidth, textureHeight;
        position.x = x;
        position.y = y;
        position.w = position.h = 32;
        this->texture = texture;
        this->maxStatesW = maxStatesW;
        this->currentStateW = currentStateW;
        this->maxStatesH = maxStatesH;
        this->currentStateH = currentStateH;
        this->HP = HP;

        SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);
        state.w = textureWidth / maxStatesW;
        state.h = textureHeight / maxStatesH;
        state.x = currentStateW * state.w;
        state.y = currentStateH * state.h;
    }

    bool isColliding(Entity other) {
        b2Fixture* fixtureA = body->GetFixtureList();
        b2Fixture* fixtureB = other.body->GetFixtureList();

        while (fixtureA != nullptr) {
            while (fixtureB != nullptr) {
                if (b2TestOverlap(fixtureA->GetShape(), 0, fixtureB->GetShape(), 0, body->GetTransform(), other.body->GetTransform())) {
                    return true;
                }
                fixtureB = fixtureB->GetNext();
            }
            fixtureA = fixtureA->GetNext();
        }

        return false;
    }

    void nextWstate(bool upState) {
        if (upState && maxStatesW - 1 > currentStateW)
        {
            currentStateW++;

        }
        else if (0 < currentStateW)
        {
            currentStateW--;
        }
        state.x = currentStateW * state.w;
    }
    void nextHstate(bool upState) {
        if (upState && maxStatesH - 1 > currentStateH)
        {
            currentStateH++;

        }
        else if (0 < currentStateH)
        {
            currentStateH--;
        }
        state.y = currentStateH * state.h;
    }
    virtual void clean()
    {
        SDL_DestroyTexture(texture);
        texture = NULL;

    }

    bool OOB() {
        return position.y + state.h < -100 || position.y + state.h > 580 || position.x + state.w > 740 || position.w + state.w < -100;
    }

    virtual void renderToApplication(App app) {
        SDL_RenderCopyF(app.renderer, texture, &state, &position);
    }
    virtual void move(float speedX, float speedY, float deltaTime, int direction) {}

    virtual void update(float deltatime) {}

    void initPhysics(b2World* world) {
        bodyDef.position.Set(position.x, position.y);
        bodyDef.type = b2_dynamicBody;
        body = world->CreateBody(&bodyDef);

        dynamicBox.SetAsBox(state.w / 2 / PTM_RATIO, state.h / 2 / PTM_RATIO);

        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.3f;

        body->CreateFixture(&fixtureDef);
    }

    const SDL_FRect& getPosition() const {
        return position;
    }
    const SDL_Rect& getState() const {
        return state;
    }
    void onHit(int hitDamage) {
        HP -= hitDamage;
        isDead();
    }

    bool isDead() {
        if (HP <= 0)
        {
            return true;
        }
        else return false;
    }
protected:
    b2FixtureDef fixtureDef;
    b2BodyDef bodyDef;
    b2PolygonShape dynamicBox;
    SDL_FRect position;
    SDL_Rect state;
    int maxStatesW;
    int currentStateW;
    int maxStatesH;
    int currentStateH;
    float elapsedTime = 0;
    SDL_Texture* texture;
    int HP;
};

class Explosion :public Entity
{
public:
    Explosion() : Entity(), nextState(true), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f) {}
    Explosion(int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP) :
        Entity(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP), nextState(true), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f)
    {
        isFinished = false;
        state2 = 0;
    }

    void update(float deltaTime) {
        timeSinceLastFrameChange += deltaTime;
        if (timeSinceLastFrameChange >= frameChangeTime) {
            currentStateW++;
            if (currentStateW >= maxStatesW) {
                currentStateW = 0;
                currentStateH++;
                if (currentStateH >= maxStatesH) {
                    currentStateH = 0;
                }
            }
            state.x = currentStateW * state.w;
            state.y = currentStateH * state.h;
            timeSinceLastFrameChange = 0.0f;
        }
        isFinished = (currentStateH == 1 && currentStateW == 4);
    }
    bool isAnimationFinished() {
        return isFinished;
    }
private:
    bool isFinished;
    int state2;
    bool nextState;
    float timeSinceLastFrameChange;
    float frameChangeTime;
};

class Projectile : public Entity {
public:
    int state2;
    int velX, velY;
    Projectile() : Entity(), nextState(true), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f) {}
    Projectile(int velX, int velY, int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH,
        int HP, b2World* world) :
        Entity(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP), nextState(true), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f)
    {
        state2 = 0;
        this->velX = velX;
        this->velY = velY;
        initPhysics(world);
    }


    void move(float deltaTime) {
        position.x -= velX * deltaTime;
        position.y -= velY * deltaTime;
        nextWstate(nextState);
        nextState = !nextState;

    }
    bool OOB() {
        return position.y + state.h < -20 || position.y + state.h > 500 || position.x + state.w > 660 || position.w + state.w < -20;
    }
    void update(float deltaTime) {
        if (body) {
            body->SetTransform(b2Vec2(position.x / PTM_RATIO, position.y / PTM_RATIO), body->GetAngle());
        }
        timeSinceLastFrameChange += deltaTime;
        if (timeSinceLastFrameChange >= frameChangeTime) {
            state2++;
            currentStateW = state2 % maxStatesW;
            currentStateH = (state2 / maxStatesW) % maxStatesH;
            timeSinceLastFrameChange = 0.0f;
        }

    }
    void renderToApplication(App app) {
        SDL_RenderCopyF(app.renderer, texture, &state, &position);
    }


private:
    bool nextState;
    float timeSinceLastFrameChange;
    float frameChangeTime;
};

class Enemy : public Entity
{
public:
    Enemy() : Entity() {}
    Enemy(int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP, int hitdamage)
        : Entity(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP) {}

    virtual void move(float speedX, float speedY, float deltaTime) {}

};

class Companion : public Entity
{
    float timeSinceLastFrameChange;
    float frameChangeTime;
    int state2;
    SDL_Texture* projectile;
    int upgradesPickedUp = 0;
    int maxHP;
    int HP;
public:
    Companion() : Entity() {}
    Companion(SDL_Texture* texture, int x, int y, SDL_Texture* projectile_texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, 
        int HP, b2World* world) :
        Entity(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f)
    {
        projectile = projectile_texture;
        state2 = 0;
        this->HP=HP;
        maxHP = HP;
        initPhysics(world);
    }

   void move(float speedX, float speedY, float deltaTime) {
        position.x += speedX * deltaTime;
        position.y += speedY * deltaTime;
    }

    void update(float deltatime) {
        if (body) {
            body->SetTransform(b2Vec2(position.x / PTM_RATIO, position.y / PTM_RATIO), body->GetAngle());
        }
        timeSinceLastFrameChange += deltatime;
        if (timeSinceLastFrameChange >= frameChangeTime) {
            state2++;
            currentStateW = state2 % maxStatesW;
            currentStateH = (state2 / maxStatesW) % (maxStatesH - 1);
            timeSinceLastFrameChange = 0.0f;
        }

        state.x = currentStateW * state.w;
        state.y = currentStateH * state.h;
    }
    const int getUpgradesPickedUp(){
        return upgradesPickedUp;
    }

    void pickupDamageUpgrade() {
        ++upgradesPickedUp;
    }
    void pickupHealthPickup(int healthPickup) {
        if (HP < maxHP)
        {
            HP += healthPickup;

            if (HP > maxHP)
            {
                HP = maxHP;
            }

        }
    }




};

class Player : public Entity
{
public:

    Player() : Entity() {}
    Player(SDL_Texture* p, int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP, 
        SDL_Texture* c, b2World* world) :
        Entity(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP) {
        projectile_texture = p;
        shootCooldown = 0;
        damageCooldown = 0;
        companion_texture = c;
        this->HP = HP;
        maxHP = HP;
        initPhysics(world);
        this->world = world;
    }
    bool doCollision(Entity my_enemy) {
        for (auto it = projectiles.begin(); it != projectiles.end();)
        {
            if (it->isColliding(my_enemy))
            {
                it = projectiles.erase(it);

                return true;
            }
            else ++it;
        }
        return false;
    }
    void move(float speedX, float speedY, float deltaTime, int direction) {
        elapsedTime += deltaTime;
        position.x += speedX * deltaTime;
        position.y += speedY * deltaTime;

        if (direction == 1 && elapsedTime >= 0.1) {
            nextWstate(true);
            elapsedTime = 0;
        }
        else if (direction == 0 && elapsedTime >= 0.1)
        {
            nextWstate(false);
            elapsedTime = 0;
        }
        for (Companion& i : companions) {
            i.move(speedX, speedY, deltaTime);
        }
    }
    void stop(float deltaTime) {
        elapsedTime += deltaTime;
        if (elapsedTime >= 0.1)
        {
            if (currentStateW > 3)
            {
                currentStateW--;
            }
            else if (currentStateW < 3)
            {
                currentStateW++;
            }
            state.x = currentStateW * state.w;
            elapsedTime = 0;
        }

    }

    void killCompanion(std::list<Explosion>* explosions, SDL_Texture* explosions_texture) {
        for (auto& i : companions) {
            i.clean();
            explosions->push_back(Explosion(i.getPosition().x, i.getPosition().y,
                explosions_texture, 5, 0, 2, 0, 0));;
        }
        companions.clear();
    }

    void shoot()
    {

        if (shootCooldown <= 0.8f)
        {
            return;
        }
        shootCooldown = 0;
        for (Companion& i : companions) {
            if (i.getUpgradesPickedUp() <= 0)
            {
                
                Projectile new_projectile(0, 150, i.getPosition().x, i.getPosition().y, projectile_texture, 2, 0, 3, 0, 1, world);
                projectiles.push_front(new_projectile);
            }
            else if (i.getUpgradesPickedUp() == 1)
            {
                Projectile new_projectile(0, 150, i.getPosition().x, i.getPosition().y, projectile_texture, 2, 0, 3, 1, 1, world);
                projectiles.push_front(new_projectile);
            }
            else
            {
                Projectile new_projectile(0, 150, i.getPosition().x, i.getPosition().y, projectile_texture, 2, 0, 3, 2, 1, world);
                projectiles.push_front(new_projectile);
            }
        }


        if (upgradesPickedUp <= 0)
        {
            Projectile new_projectile(0, 150, position.x, position.y, projectile_texture, 2, 0, 3, 0, 1, world);
            projectiles.push_front(new_projectile);
            

        }
        else if (upgradesPickedUp == 1)
        {
            Projectile new_projectile(0, 150, position.x, position.y, projectile_texture, 2, 0, 3, 1, 1, world);
            projectiles.push_front(new_projectile);
        }
        else
        {
            Projectile new_projectile(0, 150, position.x, position.y, projectile_texture, 2, 0, 3, 2, 1, world);
            projectiles.push_front(new_projectile);
        }


    }
    void renderToApplication(App app, b2World *world) {
        SDL_RenderCopyF(app.renderer, texture, &state, &position);
        for (Projectile i : projectiles)
        {
            i.renderToApplication(app);
        }
        for (Companion i : companions)
            i.renderToApplication(app);
    }

    void clean() {
        SDL_DestroyTexture(texture);
        texture = NULL;
        SDL_DestroyTexture(projectile_texture);
        projectile_texture = NULL;
    }

    void update(float deltatime) {
        shootCooldown += deltatime;
        damageCooldown += deltatime;

        if (body) {
            body->SetTransform(b2Vec2(position.x / PTM_RATIO, position.y / PTM_RATIO), body->GetAngle());
        }

        for (Projectile& i : projectiles)
        {
            i.move(deltatime);
            i.update(deltatime);
        }
        for (Companion& i : companions) {
            i.update(deltatime);
        }
        projectiles.remove_if([](Projectile i) {return i.OOB(); });
    }

    std::list<Projectile>& getProjectiles()
    {
        return projectiles;
    }

    void SetPosition(int x, int y)
    {
        position.x = x;
        position.y = y;
    }

    const int getDamage()
    {
        return hitDamage;
    }

    void upgradeDamage()
    {
        hitDamage *= 2;
    }
    void pickupDamageUpgrade()
    {
        if (upgradesPickedUp <= 2)
        {
            upgradeDamage();
            upgradesPickedUp++;

        }
    }
    void pickupHealthPickup(int healthPickup)
    {
        if (HP < maxHP)
        {
            HP += healthPickup;

            if (HP > maxHP)
            {
                HP = maxHP;
            }

        }
    }
    void pickupCompanion()
    {
        if (currentCompanion <= maxCompanion)
        {
            ++currentCompanion;
            if (currentCompanion == 1)
            {

                Companion new_companion(companion_texture, position.x + 35, position.y + 5, projectile_texture, 4, 0, 5, 0, 3, world);
                companions.push_front(new_companion);
            }

            if (currentCompanion == 2)
            {

                Companion new_companion(companion_texture, position.x - 35, position.y + 5, projectile_texture, 4, 0, 5, 0, 3, world);
                companions.push_front(new_companion);
            }


            
        }

       
    }

    void onHit(int hitDamage) {
        if (damageCooldown >= 2.0f)
        {
            HP -= hitDamage;
            damageCooldown = 0;
        }

    }
    bool isDead() {
        return HP <= 0;
    }

    std::list<Companion>* getCompanions()
    {
        return &companions;
    }

private:
    std::list<Enemy> enemies;
    std::list<Projectile> projectiles;
    std::list<Companion> companions;
    SDL_Texture* projectile_texture;
    SDL_Texture* companion_texture;
    float shootCooldown;
    float damageCooldown;
    int hitDamage = 1;
    int upgradesPickedUp = 0;
    int HP;
    int maxHP;
    int currentCompanion = 0;
    int maxCompanion = 2;
    b2World* world;
};

class Rusher : public Enemy {
public:
    float timeSinceLastFrameChange;
    float frameChangeTime;
    int state2;

    Rusher() : Enemy() {}
    Rusher(int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP, int hitDamage, b2World *world) :
        Enemy(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP, hitDamage), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f)
    {
        state2 = 0;
        this->hitDamage = hitDamage;
        initPhysics(world);
    }

    void move(float speedX, float speedY, float deltaTime) {
        position.x += speedX * deltaTime;
        position.y += speedY * deltaTime;


        timeSinceLastFrameChange += deltaTime;
        if (timeSinceLastFrameChange >= frameChangeTime) {
            state2++;
            currentStateW = state2 % maxStatesW;
            currentStateH = (state2 / maxStatesW) % maxStatesH;
            timeSinceLastFrameChange = 0.0f;
        }

        state.x = currentStateW * state.w;
        state.y = currentStateH * state.h;

    }

    void update(float deltatime, Player* player, std::list<Explosion>* explosions, SDL_Texture* explosions_texture) {
        if (body) {
            body->SetTransform(b2Vec2(position.x / PTM_RATIO, position.y / PTM_RATIO), body->GetAngle());
        }

        if (this->isColliding(*player)) {
            player->onHit(hitDamage);
            if (player->isDead())
            {
                explosions->push_back(Explosion(player->getPosition().x, player->getPosition().y,
                    explosions_texture, 5, 0, 2, 0, 0));
                player->clean();
                player->killCompanion(explosions, explosions_texture);
                player->SetPosition(10000, 10000);
            }

        }
        if (this->OOB())
        {
            this->clean();
        }
        move(0, 100, deltatime);
    }
    const int getDamage() {
        return hitDamage;
    }

protected:
    int hitDamage;
};

class Loner : public Enemy {
public:
    float timeSinceLastFrameChange;
    float frameChangeTime;
    int state2;
    float initialpositionX;

    Loner() : Enemy() {}
    Loner(SDL_Texture* p, int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP, 
        int hitDamage, b2World* world, float speedX, float speedY) :
        Enemy(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP, hitDamage), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f)
    {
        state2 = 0;
        enemy_projectile_texture = p;
        enemy_shootCooldown = 0;
        initialpositionX = x;
        this->hitDamage = hitDamage;
        this->world = world;
        this->speedX = speedX;
        this->speedY = speedY;
        initPhysics(world);
    }

    void move(float speedX, float speedY, float deltaTime) {
        position.x += speedX * deltaTime;
        position.y += speedY * deltaTime;
        timeSinceLastFrameChange += deltaTime;
        if (timeSinceLastFrameChange >= frameChangeTime) {
            state2++;
            currentStateW = state2 % maxStatesW;
            currentStateH = (state2 / maxStatesW) % maxStatesH;
            timeSinceLastFrameChange = 0.0f;
        }

        state.x = currentStateW * state.w;
        state.y = currentStateH * state.h;
    }

    void shoot(Player* player)
    {

        if (enemy_shootCooldown <= 1.5f)
        {
            return;
        }
        enemy_shootCooldown = 0;
        int directionX = position.x - player->getPosition().x;
        int directionY = position.y - player->getPosition().y;
        double magnitude = std::sqrt(directionX * directionX + directionY * directionY) / 100;
        if (magnitude == 0) magnitude = 1.0f;
        Projectile new_projectile(directionX / magnitude, directionY / magnitude, position.x, position.y, enemy_projectile_texture, 8, 0, 1, 0, 1, world);
        enemy_projectiles.push_front(new_projectile);

    }

    void update(float deltatime, Player* player, App app, std::list<Explosion>* explosions, SDL_Texture* explosions_texture) {
        enemy_shootCooldown += deltatime;
        if (body) {
            body->SetTransform(b2Vec2(position.x / PTM_RATIO, position.y / PTM_RATIO), body->GetAngle());
        }
        for (auto it = enemy_projectiles.begin(); it != enemy_projectiles.end();) {
            it->move(deltatime);
            if (it->OOB()) {
                it = enemy_projectiles.erase(it);

            }
            else {
                bool collision = false;
                if (it->isColliding(*player)) {
                    player->onHit(hitDamage);
                    if (player->isDead())
                    {
                        explosions->push_back(Explosion(player->getPosition().x, player->getPosition().y,
                            explosions_texture, 5, 0, 2, 0, 0));
                        player->clean();
                        player->killCompanion(explosions, explosions_texture);
                        player->SetPosition(10000, 10000);
                    }
                    collision = true;
                    break;
                }

                if (collision) {
                    it = enemy_projectiles.erase(it);
                }
                else {
                    ++it;
                }
            }
        }
        for (auto& projectile : enemy_projectiles) {
            projectile.renderToApplication(app);
            projectile.update(deltatime);
        }
        if (this->isColliding(*player)) {
            player->onHit(hitDamage);
            if (player->isDead())
            {
                explosions->push_back(Explosion(player->getPosition().x, player->getPosition().y,
                    explosions_texture, 5, 0, 2, 0, 0));
                player->killCompanion(explosions, explosions_texture);
                player->clean();
                player->SetPosition(10000, 10000);
            }

        }

        shoot(player);
        move(-speedX, speedY, deltatime);
    }


    std::list<Projectile>& getProjectiles() {
        return enemy_projectiles;
    }
private:
    std::list<Projectile> enemy_projectiles;
    SDL_Texture* enemy_projectile_texture;
    float enemy_shootCooldown;
    float distanceX;
    float distanceY;
    float speedX;
    float speedY;
    b2World* world;
protected:
    int hitDamage;
};

class BigStoneDebris :public Entity
{
    float timeSinceLastFrameChange;
    float frameChangeTime;
    int state2;
public:
    BigStoneDebris() : Entity() {}
    BigStoneDebris(int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP, int hitDamage, b2World* world) :
        Entity(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f)
    {
        state2 = 0;
        this->hitDamage = hitDamage;
        initPhysics(world);
    }

    bool OOBDebris() {
        return position.y + state.h > 600;
    }

    void move(float speedX, float speedY, float deltaTime) {
        position.x += speedX * deltaTime;
        position.y += speedY;
        if (OOBDebris())
        {
            this->clean();
        }

        timeSinceLastFrameChange += deltaTime;
        if (timeSinceLastFrameChange >= frameChangeTime) {
            state2++;
            currentStateW = state2 % maxStatesW;
            currentStateH = (state2 / maxStatesW) % maxStatesH;
            timeSinceLastFrameChange = 0.0f;
        }

        state.x = currentStateW * state.w;
        state.y = currentStateH * state.h;
    }

    void update(float deltatime, Player* player, std::list<Explosion>* explosions, SDL_Texture* explosions_texture) {
        if (body) {
            body->SetTransform(b2Vec2(position.x / PTM_RATIO, position.y / PTM_RATIO), body->GetAngle());
        }
        if (this->isColliding(*player)) {
            player->onHit(hitDamage);
            if (player->isDead())
            {
                explosions->push_back(Explosion(player->getPosition().x, player->getPosition().y,
                    explosions_texture, 5, 0, 2, 0, 0));
                player->killCompanion(explosions, explosions_texture);
                player->clean();
                player->SetPosition(10000, 10000);
            }
        }
        move(0, 1, deltatime);
    }
protected:
    int hitDamage;
};

class MediumStoneDebris :public Entity
{
    float timeSinceLastFrameChange;
    float frameChangeTime;
    int state2;
    bool left;
public:
    MediumStoneDebris() : Entity() {}
    MediumStoneDebris(bool left, int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP,
        int hitDamage, b2World* world) :
        Entity(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f)
    {
        this->left = left;
        state2 = 0;
        this->hitDamage = hitDamage;
        initPhysics(world);
    }

    bool OOBDebris() {
        return position.y + state.h > 600;
    }

    void move(float speedX, float speedY, float deltaTime) {
        position.x += speedX * deltaTime;
        position.y += speedY;
        if (OOBDebris())
        {
            this->clean();
        }

        timeSinceLastFrameChange += deltaTime;
        if (timeSinceLastFrameChange >= frameChangeTime) {
            state2++;
            currentStateW = state2 % maxStatesW;
            currentStateH = (state2 / maxStatesW) % maxStatesH;
            timeSinceLastFrameChange = 0.0f;
        }

        state.x = currentStateW * state.w;
        state.y = currentStateH * state.h;
    }

    void update(float deltatime, Player* player, std::list<Explosion>* explosions, SDL_Texture* explosions_texture) {
        if (body) {
            body->SetTransform(b2Vec2(position.x / PTM_RATIO, position.y / PTM_RATIO), body->GetAngle());
        }
        if (this->isColliding(*player)) {
            player->onHit(hitDamage);
            if (player->isDead())
            {
                explosions->push_back(Explosion(player->getPosition().x, player->getPosition().y,
                    explosions_texture, 5, 0, 2, 0, 0));
                player->killCompanion(explosions, explosions_texture);
                player->clean();
                player->SetPosition(10000, 10000);
            }
        }
        move(left ? -30 : 30, 1, deltatime);
    }
protected:
    int hitDamage;
};

class SmallStoneDebris :public Entity
{
    float timeSinceLastFrameChange;
    float frameChangeTime;
    int state2;
    bool left;
public:
    SmallStoneDebris() : Entity() {}
    SmallStoneDebris(bool left, int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP, 
        int hitDamage, b2World* world) :
        Entity(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f)
    {
        this->left = left;
        state2 = 0;
        this->hitDamage = hitDamage;
        initPhysics(world);
    }

    bool OOBDebris() {
        return position.y + state.h > 600;
    }

    void move(float speedX, float speedY, float deltaTime) {
        position.x += speedX * deltaTime;
        position.y += speedY;
        if (OOBDebris())
        {
            this->clean();
        }

        timeSinceLastFrameChange += deltaTime;
        if (timeSinceLastFrameChange >= frameChangeTime) {
            state2++;
            currentStateW = state2 % maxStatesW;
            currentStateH = (state2 / maxStatesW) % maxStatesH;
            timeSinceLastFrameChange = 0.0f;
        }

        state.x = currentStateW * state.w;
        state.y = currentStateH * state.h;
    }

    void update(float deltatime, Player* player, std::list<Explosion>* explosions, SDL_Texture* explosions_texture) {
        if (body) {
            body->SetTransform(b2Vec2(position.x / PTM_RATIO, position.y / PTM_RATIO), body->GetAngle());
        }
        if (this->isColliding(*player)) {
            player->onHit(hitDamage);
            if (player->isDead())
            {
                explosions->push_back(Explosion(player->getPosition().x, player->getPosition().y,
                    explosions_texture, 5, 0, 2, 0, 0));
                player->killCompanion(explosions, explosions_texture);
                player->clean();
                player->SetPosition(10000, 10000);
            }
        }
        move(left ? -30 : 30, 1, deltatime);
    }
protected:
    int hitDamage;
};

class MetalDebris :public Entity
{
    float timeSinceLastFrameChange;
    float frameChangeTime;
    int state2;
public:
    MetalDebris() : Entity() {}
    MetalDebris(int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP, 
        int hitDamage, b2World* world) :
        Entity(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f)
    {
        state2 = 0;
        this->hitDamage = hitDamage;
        initPhysics(world);
    }

    bool OOBDebris() {
        return position.y + state.h > 600;
    }

    void move(float speedX, float speedY, float deltaTime) {
        position.x += speedX * deltaTime;
        position.y += speedY;
        if (OOBDebris())
        {
            this->clean();
        }

        timeSinceLastFrameChange += deltaTime;
        if (timeSinceLastFrameChange >= frameChangeTime) {
            state2++;
            currentStateW = state2 % maxStatesW;
            currentStateH = (state2 / maxStatesW) % maxStatesH;
            timeSinceLastFrameChange = 0.0f;
        }

        state.x = currentStateW * state.w;
        state.y = currentStateH * state.h;
    }

    void update(float deltatime, Player* player, std::list<Explosion>* explosions, SDL_Texture* explosions_texture) {
        if (body) {
            body->SetTransform(b2Vec2(position.x / PTM_RATIO, position.y / PTM_RATIO), body->GetAngle());
        }
        if (this->isColliding(*player)) {
            player->onHit(hitDamage);
            if (player->isDead())
            {
                explosions->push_back(Explosion(player->getPosition().x, player->getPosition().y,
                    explosions_texture, 5, 0, 2, 0, 0));
                player->killCompanion(explosions, explosions_texture);
                player->clean();
                player->SetPosition(10000, 10000);
            }
        }
        move(0, 1, deltatime);
    }
protected:
    int hitDamage;
};

class Drone : public Enemy {
public:
    float timeSinceLastFrameChange;
    float frameChangeTime;
    int state2;
    float initialpositionX;

    Drone() : Enemy() {}
    Drone(int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP, int hitDamage, 
        int TimeCreated, b2World* world) :
        Enemy(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP, hitDamage), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f)
    {
        state2 = 0;
        initialpositionX = x;
        this->hitDamage = hitDamage;
        this->TimeCreated = TimeCreated;
        initPhysics(world);
    }

    void move(float speedX, float speedY, float deltaTimeCurrent) {
        float amplitude = 35.0f;
        float frequency = 0.0035f;
        float displacement = amplitude * sin((SDL_GetTicks() - TimeCreated) * frequency);
        position.x = initialpositionX + displacement;
        position.y += speedY * deltaTimeCurrent;

        timeSinceLastFrameChange += deltaTimeCurrent;
        if (timeSinceLastFrameChange >= frameChangeTime) {
            state2++;
            currentStateW = state2 % maxStatesW;
            currentStateH = (state2 / maxStatesW) % maxStatesH;
            timeSinceLastFrameChange = 0.0f;
        }

        state.x = currentStateW * state.w;
        state.y = currentStateH * state.h;
    }

    void update(float deltatime, Player* player, std::list<Explosion>* explosions, SDL_Texture* explosions_texture) {
        if (body) {
            body->SetTransform(b2Vec2(position.x / PTM_RATIO, position.y / PTM_RATIO), body->GetAngle());
        }
        if (this->isColliding(*player)) {
            player->onHit(hitDamage);
            if (player->isDead())
            {
                explosions->push_back(Explosion(player->getPosition().x, player->getPosition().y,
                    explosions_texture, 5, 0, 2, 0, 0));
                player->killCompanion(explosions, explosions_texture);
                player->clean();
                player->SetPosition(10000, 10000);
            }
        }
        move(0, 100, deltatime);
    }

protected:
    int hitDamage;
    int TimeCreated;
};

class WeaponUpgrade : public Entity
{
public:
    float timeSinceLastFrameChange;
    float frameChangeTime;
    int state2;
    WeaponUpgrade() : Entity() {}
    WeaponUpgrade(int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP, b2World* world) :
        Entity(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f)
    {
        state2 = 0;
        collided = false;
        initPhysics(world);
    }

    bool OOB() {
        return position.y + state.h > 600;
    }

    void move(float speedX, float speedY, float deltaTime) {
        position.x += speedX * deltaTime;
        position.y += speedY;
        if (OOB())
        {
            this->clean();
        }


        timeSinceLastFrameChange += deltaTime;
        if (timeSinceLastFrameChange >= frameChangeTime) {
            state2++;
            currentStateW = state2 % maxStatesW;
            currentStateH = (state2 / maxStatesW) % maxStatesH;
            timeSinceLastFrameChange = 0.0f;
        }

        state.x = currentStateW * state.w;
        state.y = currentStateH * state.h;
    }

    void update(float deltatime, Player* player, std::list<Explosion>* explosions, SDL_Texture* explosions_texture) {
        if (body) {
            body->SetTransform(b2Vec2(position.x / PTM_RATIO, position.y / PTM_RATIO), body->GetAngle());
        }
        if (this->isColliding(*player)) {
            if (player->isDead() == false)
            {
                player->pickupDamageUpgrade();
                collided = true;
                this->clean();

            }
        }

        std::list<Companion>* lista = player->getCompanions();

        for (Companion &companion : *lista) {
            if (this->isColliding(companion)) {
                if (companion.isDead() == false)
                {
                    companion.pickupDamageUpgrade();
                    collided = true;
                    this->clean();

                }
            }
        }
        move(0, 1, deltatime);
    }
    bool getCollision()
    {
        return collided;
    }
private:
    bool collided;

};

class HealthPickup : public Entity
{
public:
    float timeSinceLastFrameChange;
    float frameChangeTime;
    int state2;
    HealthPickup() : Entity() {}
    HealthPickup(int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP, 
        int HPReplenish, b2World* world) :
        Entity(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f)
    {
        state2 = 0;
        collided = false;
        this->HPReplenish = HPReplenish;
        initPhysics(world);
    }

    bool OOB() {
        return position.y + state.h > 600;
    }

    void move(float speedX, float speedY, float deltaTime) {
        position.x += speedX * deltaTime;
        position.y += speedY;
        if (OOB())
        {
            this->clean();
        }

        timeSinceLastFrameChange += deltaTime;
        if (timeSinceLastFrameChange >= frameChangeTime) {
            state2++;
            currentStateW = state2 % maxStatesW;
            currentStateH = (state2 / maxStatesW) % maxStatesH;
            timeSinceLastFrameChange = 0.0f;
        }

        state.x = currentStateW * state.w;
        state.y = currentStateH * state.h;
    }

    void update(float deltatime, Player* player, std::list<Explosion>* explosions, SDL_Texture* explosions_texture) {
        if (body) {
            body->SetTransform(b2Vec2(position.x / PTM_RATIO, position.y / PTM_RATIO), body->GetAngle());
        }
        if (this->isColliding(*player)) {
            if (player->isDead() == false)
            {
                player->pickupHealthPickup(HPReplenish);
                collided = true;
                this->clean();

            }
        }
        std::list<Companion>* lista = player->getCompanions();

        for (Companion &companion : *lista) {
            if (this->isColliding(companion)) {
                if (companion.isDead() == false)
                {
                    companion.pickupDamageUpgrade();
                    collided = true;
                    this->clean();

                }
            }
        }
        move(0, 1, deltatime);
    }
    bool getCollision()
    {
        return collided;
    }
private:
    bool collided;
    int HPReplenish;
};

class CompanionPickup : public Entity
{
public:
    float timeSinceLastFrameChange;
    float frameChangeTime;
    int state2;
    CompanionPickup() : Entity() {}
    CompanionPickup(int x, int y, SDL_Texture* texture, int maxStatesW, int currentStateW, int maxStatesH, int currentStateH, int HP, b2World* world) :
        Entity(x, y, texture, maxStatesW, currentStateW, maxStatesH, currentStateH, HP), timeSinceLastFrameChange(0.0f), frameChangeTime(0.1f)
    {
        state2 = 0;
        collided = false;
        initPhysics(world);
    }

    bool OOB() {
        return position.y + state.h > 600;
    }

    void move(float speedX, float speedY, float deltaTime) {
        position.x += speedX * deltaTime;
        position.y += speedY;
        if (OOB())
        {
            this->clean();
        }

        timeSinceLastFrameChange += deltaTime;
        if (timeSinceLastFrameChange >= frameChangeTime) {
            state2++;
            currentStateW = state2 % maxStatesW;
            currentStateH = (state2 / maxStatesW) % (maxStatesH-1);
            timeSinceLastFrameChange = 0.0f;
        }

        state.x = currentStateW * state.w;
        state.y = currentStateH * state.h;
    }

    void update(float deltatime, Player* player, std::list<Explosion>* explosions, SDL_Texture* explosions_texture) {
        if (body) {
            body->SetTransform(b2Vec2(position.x / PTM_RATIO, position.y / PTM_RATIO), body->GetAngle());
        }
        if (this->isColliding(*player)) {
            if (player->isDead() == false)
            {
                player->pickupCompanion();
                collided = true;
                this->clean();

            }
        }
        move(0, 1, deltatime);
    }
    bool getCollision()
    {
        return collided;
    }
private:
    bool collided;
};