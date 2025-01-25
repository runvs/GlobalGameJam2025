#include "state_game.hpp"

#include "tweens/tween_scale.hpp"
#include <box2dwrapper/box2d_contact_manager.hpp>
#include <box2dwrapper/box2d_world_impl.hpp>
#include <box2dwrapper/logging_box2d_contact_manager.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <input/input_manager.hpp>
#include <random/random.hpp>
#include <state_menu.hpp>
#include <tweens/tween_alpha.hpp>
#include <tweens/tween_position.hpp>
#include <tweens/tween_rotation.hpp>

StateGame::StateGame(std::string const& levelName) { m_levelName = levelName; }

void StateGame::onCreate()
{
    auto contactManager = std::make_shared<jt::Box2DContactManager>();
    auto loggingContactManager
        = std::make_shared<jt::LoggingBox2DContactManager>(contactManager, getGame()->logger());
    m_world
        = std::make_shared<jt::Box2DWorldImpl>(jt::Vector2f { 0.0f, 0.0f }, loggingContactManager);

    loadLevel();

    CreatePlayer();
    auto const playerGroundContactListener = std::make_shared<ContactCallbackPlayerGround>();
    playerGroundContactListener->setPlayer(m_player);
    m_world->getContactManager().registerCallback("player_ground0", playerGroundContactListener);

    auto playerEnemyContactListener = std::make_shared<ContactCallbackPlayerEnemy>();
    playerEnemyContactListener->setPlayer(m_player);
    m_world->getContactManager().registerCallback("player_enemy1", playerEnemyContactListener);

    m_vignette = std::make_shared<jt::Vignette>(GP::GetScreenSize());
    add(m_vignette);
    setAutoDraw(false);
}

void StateGame::onEnter() { }

void StateGame::loadLevel()
{
    m_level = std::make_shared<Level>("assets/" + m_levelName, m_world);
    add(m_level);
}

void StateGame::onUpdate(float const elapsed)
{

    if (!m_ending && !getGame()->stateManager().getTransition()->isInProgress()) {
        std::int32_t const velocityIterations = 20;
        std::int32_t const positionIterations = 20;
        m_world->step(elapsed, velocityIterations, positionIterations);

        if (!m_player->isAlive()) {

            endGame();
        }

        m_level->checkIfPlayerIsInKillbox(m_player->getPosition(), [this]() {
            auto const dieSound = getGame()->audio().addTemporarySound("event:/death-by-spikes-p1");
            dieSound->play();
        });

        handleCameraScrolling(elapsed);
    }
    if (getGame()->input().keyboard()->justPressed(jt::KeyCode::F1)
        || getGame()->input().keyboard()->justPressed(jt::KeyCode::Escape)
        || getGame()->input().gamepad(0)->justPressed(jt::GamepadButtonCode::GBBack)
        || getGame()->input().gamepad(1)->justPressed(jt::GamepadButtonCode::GBBack)) {
        getGame()->stateManager().switchState(std::make_shared<StateMenu>());
    }
}

void StateGame::endGame()
{
    if (!m_ending) {
        m_ending = true;
        getGame()->stateManager().switchState(std::make_shared<StateGame>(m_levelName));
    }
}

void StateGame::handleCameraScrolling(float const elapsed)
{
    // TODO add looking forward
    auto const ps = m_player->getPosOnScreen();

    float const topMargin = 100.0f;
    float const botMargin = 100.0f;
    float const rightMargin = 120.0f;
    float const leftMargin = 120.0f;

    auto& cam = getGame()->gfx().camera();

    auto const cp = cam.getCamOffset();

    auto const dif = cp - ps;
    auto const dist = jt::MathHelper::length(dif);
    float const scrollSpeed = 200.0f;

    auto const screenWidth = GP::GetScreenSize().x;
    auto const screenHeight = GP::GetScreenSize().y;
    if (ps.x < leftMargin) {
        cam.move(jt::Vector2f { -scrollSpeed * elapsed, 0.0f });
        if (ps.x < rightMargin / 2) {
            cam.move(jt::Vector2f { -scrollSpeed * elapsed, 0.0f });
        }
    } else if (ps.x > screenWidth - rightMargin) {
        cam.move(jt::Vector2f { scrollSpeed * elapsed, 0.0f });
        if (ps.x > screenWidth - rightMargin / 3 * 2) {
            cam.move(jt::Vector2f { scrollSpeed * elapsed, 0.0f });
        }
    }

    if (ps.y < topMargin) {
        cam.move(jt::Vector2f { 0.0f, -scrollSpeed * elapsed });
        if (ps.y < rightMargin / 2) {
            cam.move(jt::Vector2f { 0.0f, -scrollSpeed * elapsed });
        }
    } else if (ps.y > screenHeight - botMargin) {
        cam.move(jt::Vector2f { 0.0f, scrollSpeed * elapsed });
        if (ps.y > screenWidth - rightMargin / 3 * 2) {
            cam.move(jt::Vector2f { 0.0f, scrollSpeed * elapsed });
        }
    }

    // clamp camera to level bounds
    auto offset = cam.getCamOffset();
    if (offset.x < 0) {
        offset.x = 0;
    }
    if (offset.y < 0) {
        offset.y = 0;
    }
    auto const levelWidth = m_level->getLevelSizeInPixel().x;
    auto const levelHeight = m_level->getLevelSizeInPixel().y;
    auto const maxCamPositionX = levelWidth - screenWidth;

    auto const maxCamPositionY = levelHeight - screenHeight;
    if (offset.x > maxCamPositionX) {
        offset.x = maxCamPositionX;
    }
    if (offset.y > maxCamPositionY) {
        offset.y = maxCamPositionY;
    }
    cam.setCamOffset(offset);
}

void StateGame::onDraw() const
{
    m_level->draw();

    m_player->draw();
    m_walkParticles->draw();
    m_playerJumpParticles->draw();
    m_vignette->draw();
}

void StateGame::CreatePlayer()
{
    m_player = std::make_shared<Player>(m_world);
    m_player->setPosition(m_level->getPlayerStart());
    m_player->setLevelSize(m_level->getLevelSizeInPixel());
    add(m_player);

    getGame()->gfx().camera().setCamOffset(m_level->getPlayerStart() - GP::GetScreenSize() * 0.5f);

    createPlayerWalkParticles();
    createPlayerJumpParticleSystem();
}

void StateGame::createPlayerJumpParticleSystem()
{
    m_playerJumpParticles = jt::ParticleSystem<jt::Shape, 50>::createPS(
        [this]() {
            auto s = std::make_shared<jt::Shape>();
            if (jt::Random::getChance()) {
                s->makeRect(jt::Vector2f { 1.0f, 1.0f }, textureManager());
            } else {
                s->makeRect(jt::Vector2f { 2.0f, 2.0f }, textureManager());
            }
            s->setColor(jt::colors::White);
            s->setPosition(jt::Vector2f { -50000, -50000 });
            s->setOrigin(jt::Vector2f { 1.0f, 1.0f });
            return s;
        },
        [this](auto s, auto p) {
            s->setPosition(p);
            s->update(0.0f);
            auto const totalTime = jt::Random::getFloat(0.2f, 0.3f);

            auto twa = jt::TweenAlpha::create(s, totalTime / 2.0f, 255, 0);
            twa->setStartDelay(totalTime / 2.0f);
            add(twa);

            auto const startPos = p;
            auto const endPos = p
                + jt::Vector2f { jt::Random::getFloatGauss(0, 4.5f),
                      jt::Random::getFloat(-2.0f, 0.0f) };
            auto twp = jt::TweenPosition::create(s, totalTime, startPos, endPos);
            add(twp);

            float minAngle = 0.0f;
            float maxAngle = 360.0f;
            if (endPos.x < startPos.x) {
                minAngle = 360.0f;
                maxAngle = 0.0f;
            }
            auto twr = jt::TweenRotation::create(s, totalTime, minAngle, maxAngle);
            add(twr);
        });
    add(m_playerJumpParticles);
    m_player->setJumpParticleSystem(m_playerJumpParticles);
}

void StateGame::createPlayerWalkParticles()
{
    m_walkParticles = jt::ParticleSystem<jt::Shape, 50>::createPS(
        [this]() {
            auto s = std::make_shared<jt::Shape>();
            s->makeRect(jt::Vector2f { 1.0f, 1.0f }, textureManager());
            s->setColor(jt::colors::Black);
            s->setPosition(jt::Vector2f { -50000, -50000 });
            return s;
        },
        [this](auto s, auto p) {
            s->setPosition(p);

            auto twa = jt::TweenAlpha::create(s, 1.5f, 255, 0);
            add(twa);

            auto const rp
                = p + jt::Vector2f { 0, 4 } + jt::Vector2f { jt::Random::getFloat(-4, 4), 0 };

            auto topPos = rp;
            auto botPos = rp;
            auto const maxHeight = jt::Random::getFloat(2.0f, 7.0f);
            auto const maxWidth = jt::Random::getFloat(2.0f, 6.0f);
            if (jt::Random::getChance()) {
                topPos = rp + jt::Vector2f { maxWidth / 2, -maxHeight };
                botPos = rp + jt::Vector2f { maxWidth, 0 };
            } else {
                topPos = rp + jt::Vector2f { -maxWidth / 2, -maxHeight };
                botPos = rp + jt::Vector2f { -maxWidth, 0 };
            }
            auto const totalTime = jt::Random::getFloat(0.3f, 0.6f);
            std::shared_ptr<jt::Tween> twp1
                = jt::TweenPosition::create(s, totalTime / 2.0f, rp, topPos);
            add(twp1);
            twp1->addCompleteCallback([this, topPos, botPos, s, totalTime]() {
                auto twp2 = jt::TweenPosition::create(s, totalTime / 2.0f, topPos, botPos);
                add(twp2);
            });
        });
    add(m_walkParticles);
    m_player->setWalkParticleSystem(m_walkParticles);
}

std::string StateGame::getName() const { return "Game"; }
