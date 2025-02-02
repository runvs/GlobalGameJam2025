﻿#include "state_game.hpp"

#include "drawable_helpers.hpp"
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

StateGame::StateGame(std::string const& levelName) { m_levelName = levelName; }

void StateGame::onCreate()
{
    auto contactManager = std::make_shared<jt::Box2DContactManager>();
    auto loggingContactManager
        = std::make_shared<jt::LoggingBox2DContactManager>(contactManager, getGame()->logger());
    m_world
        = std::make_shared<jt::Box2DWorldImpl>(jt::Vector2f { 0.0f, 0.0f }, loggingContactManager);

    m_hud = std::make_shared<Hud>();
    add(m_hud);
    loadLevel();

    m_particlesBubbleExhaust = jt::ParticleSystem<jt::Animation, 200>::createPS(
        [this]() {
            auto a = std::make_shared<jt::Animation>();
            a->loadFromAseprite("assets/particles.aseprite", textureManager());

            a->play(std::to_string(jt::Random::getInt(0, 6)));

            a->setPosition({ -2000, -2000 });
            a->setOrigin(jt::OriginMode::CENTER);
            return a;
        },
        [this](auto& a, auto pos) {
            a->setPosition(pos);
            a->update(0.0f);
            auto const playerPosition = this->m_player->getAnimation()->getPosition();
            auto direction = pos - playerPosition;
            jt::MathHelper::normalizeMe(direction);
            add(jt::TweenPosition::create(
                a, 1.0, pos, pos + direction * 20 + jt::Random::getRandomPointInCircle(8)));
            add(jt::TweenAlpha::create(a, 1.0, 255, 0));
            add(jt::TweenScale::create(a, 1.0, { .5, .5 }, { 1.0, 1.0 }));
        });
    add(m_particlesBubbleExhaust);

    createPlayer();

    getGame()->gfx().camera().setCamOffset(m_player->getPosition() - GP::GetScreenSize() * 0.5f);
    clampCameraToScreen();

    auto const playerGroundContactListener = std::make_shared<ContactCallbackPlayerGround>();
    playerGroundContactListener->setPlayer(m_player);

    m_world->getContactManager().registerCallback("player_ground0", playerGroundContactListener);

    auto playerEnemyContactListener = std::make_shared<ContactCallbackBubbleKillbox>();
    playerEnemyContactListener->setPlayer(m_player);
    m_world->getContactManager().registerCallback("player_enemy1", playerEnemyContactListener);

    auto color = jt::colors::Black;
    color.a = 0;
    m_overlay = jt::dh::createShapeRect(GP::GetScreenSize(), color, textureManager());
    m_overlay->setIgnoreCamMovement(true);

    m_vignette = std::make_shared<jt::Vignette>(GP::GetScreenSize());
    add(m_vignette);

    setAutoDraw(false);
}

void StateGame::onEnter() { }

void StateGame::clampCameraToScreen()
{
    auto& cam = getGame()->gfx().camera();

    auto const screenWidth = GP::GetScreenSize().x;
    auto const screenHeight = GP::GetScreenSize().y;

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

void StateGame::loadLevel()
{
    m_level = std::make_shared<Level>("assets/" + m_levelName, m_world);
    add(m_level);
    m_hud->setPatches(m_level->getNumberOfInitiallyAvailablePatches());

    clampCameraToScreen();
}

void StateGame::onUpdate(float const elapsed)
{
    m_overlay->update(elapsed);

    handleReturnToMainMenu(elapsed);

    m_hud->update(elapsed);

    if (!m_ending && !getGame()->stateManager().getTransition()->isInProgress()) {
        handleCheatModeActivation(elapsed);
        handleCheats(elapsed);

        m_world->step(elapsed, GP::PhysicVelocityIterations(), GP::PhysicPositionIterations());

        if (!m_player->isAlive()) {
            restartLevel();
        }

        m_level->checkIfPlayerIsInExit(
            m_player->getPosition(), [this](std::string const& newLevelName) {
                if (!m_ending) {
                    auto snd = getGame()->audio().addTemporarySound("event:/sfx/goal");
                    snd->play();

                    transitionToLevel(newLevelName);
                }
            });

        m_level->checkIfPlayerIsInKillbox(m_player->getPosition(), [this]() {
            auto const dieSound = getGame()->audio().addTemporarySound("event:/death-by-spikes-p1");
            dieSound->play();
            restartLevel();
        });
        m_level->checkIfPlayerIsInPowerup(
            m_player->getPosition(), [this](ePowerUpType t, PowerUp* pu) {
                if (pu->getPowerUpType() == ePowerUpType::SOAP) {
                    m_player->pickUpBubble();
                } else if (pu->getPowerUpType() == ePowerUpType::PATCH) {
                    m_player->addPatches();
                    m_hud->addPatches(
                        GP::NumberOfPatchesPerPowerUp(), [this](std::shared_ptr<jt::Sprite> p) {
                            add(jt::TweenAlpha::create(p, 0.4, 0, 255));
                            add(jt::TweenScale::create(
                                p, 0.4, jt::Vector2f { 1.5f, 1.5f }, jt::Vector2f { 1.0f, 1.0f }));
                        });
                }

                auto snd = getGame()->audio().addTemporarySound("event:/sfx/pickup");
                snd->play();

                add(jt::TweenScale::create(
                    pu->getDrawable(), 0.5f, jt::Vector2f { 1.0f, 1.0f }, { 2.0f, 2.0f }));
                auto twa = jt::TweenAlpha::create(pu->getDrawable(), 0.5f, 255u, 0u);
                twa->addCompleteCallback([pu]() { pu->kill(); });
                add(twa);
            });

        handleCameraScrolling(elapsed);
    }
}

void StateGame::restartLevel()
{
    if (!m_ending) {
        m_ending = true;
        auto const tween
            = jt::TweenAlpha::create(m_overlay, 1.0f, std::uint8_t { 0 }, std::uint8_t { 255 });
        tween->addCompleteCallback([this]() {
            getGame()->stateManager().switchState(std::make_shared<StateGame>(m_levelName));
        });
        add(tween);
    }
}

void StateGame::transitionToLevel(std::string const& newLevelName)
{
    if (!m_ending) {
        m_ending = true;
        auto const tween
            = jt::TweenAlpha::create(m_overlay, 1.0f, std::uint8_t { 0 }, std::uint8_t { 255 });
        tween->addCompleteCallback([this, newLevelName]() {
            getGame()->stateManager().switchState(std::make_shared<StateGame>(newLevelName));
        });
        add(tween);
    }
}

void StateGame::handleReturnToMainMenu(float const elapsed)
{
    if (getGame()->input().keyboard()->justPressed(jt::KeyCode::F1)
        || getGame()->input().keyboard()->justPressed(jt::KeyCode::Escape)
        || getGame()
            ->input()
            .gamepad(GP::GamepadIndex())
            ->justPressed(jt::GamepadButtonCode::GBBack)) {
        getGame()->stateManager().switchState(std::make_shared<StateMenu>());
    }
}

void StateGame::handleCheatModeActivation(float const elapsed)
{
    auto& input = getGame()->input();

    if (input.keyboard()->justPressed(jt::KeyCode::F8)) {
        m_cheatsActive = !m_cheatsActive;
        std::cerr << "cheats " << (m_cheatsActive ? "on" : "off") << std::endl;
        if (m_cheatsActive) {
            std::cerr << "\t(J) jump to mouse position" << std::endl;
            std::cerr << "\t(P) add patches" << std::endl;
            std::cerr << "\t(O) patch all holes" << std::endl;
            std::cerr << "\t(B) smallest bubble" << std::endl;
            std::cerr << "\t(N) half bubble" << std::endl;
            std::cerr << "\t(M) full bubble" << std::endl;
        }
    }
}

void StateGame::handleCheats(float const elapsed)
{
    if (!m_cheatsActive)
        return;

    auto const& keyboard = getGame()->input().keyboard();
    auto const& mouse = getGame()->input().mouse();

    if (keyboard->justPressed(jt::KeyCode::J)) {
        std::cerr << "cheat (J) jump to mouse position" << std::endl;
        auto const worldPos = mouse->getMousePositionWorld();
        m_player->resetVelocity();
        m_player->setPosition(worldPos);
    }

    if (keyboard->justPressed(jt::KeyCode::P)) {
        std::cerr << "cheat (P) add patches" << std::endl;
        m_player->addPatches();
        m_hud->addPatches(GP::NumberOfPatchesPerPowerUp(), [this](std::shared_ptr<jt::Sprite> p) {
            add(jt::TweenAlpha::create(p, 0.4, 0, 255));
            add(jt::TweenScale::create(
                p, 0.4, jt::Vector2f { 1.5f, 1.5f }, jt::Vector2f { 1.0f, 1.0f }));
        });
    }

    if (keyboard->justPressed(jt::KeyCode::O)) {
        std::cerr << "cheat (O) patch all holes" << std::endl;
        m_player->resetPuncturePoints();
    }

    if (keyboard->justPressed(jt::KeyCode::B)) {
        std::cerr << "cheat (B) smallest bubble" << std::endl;
        m_player->setBubbleVolume(0.0f);
    }

    if (keyboard->justPressed(jt::KeyCode::N)) {
        std::cerr << "cheat (N) half bubble" << std::endl;
        m_player->setBubbleVolume(0.5f);
    }

    if (keyboard->justPressed(jt::KeyCode::M)) {
        std::cerr << "cheat (M) full bubble" << std::endl;
        m_player->setBubbleVolume(1.0f);
    }
}

void StateGame::handleCameraScrolling(float const elapsed)
{
    // TODO add looking forward
    auto& cam = getGame()->gfx().camera();

    auto const screenWidth = GP::GetScreenSize().x;
    auto const screenHeight = GP::GetScreenSize().y;

    cam.setCamOffset(m_player->getPosition() - jt::Vector2f { screenWidth / 2, screenHeight / 2 });

    clampCameraToScreen();
}

void StateGame::onDraw() const
{
    m_level->draw();

    m_player->draw();
    m_particlesBubbleExhaust->draw();
    m_vignette->draw();
    m_hud->draw();
    m_overlay->draw(renderTarget());
}

void StateGame::createPlayer()
{
    m_player = std::make_shared<Player>(m_world, m_particlesBubbleExhaust);
    m_player->setPosition(m_level->getPlayerStart());
    m_player->setLevelSize(m_level->getLevelSizeInPixel());
    m_player->setAvailablePatches(m_level->getNumberOfInitiallyAvailablePatches());
    add(m_player);

    getGame()->gfx().camera().setCamOffset(m_level->getPlayerStart() - GP::GetScreenSize() * 0.5f);
    m_player->setPatchUsedCallback([this]() {
        m_hud->removePatch([this](std::shared_ptr<jt::Sprite> p) {
            p->flash(0.3f, { jt::Color { 255, 255, 255, 100 } });
            add(jt::TweenAlpha::create(p, 0.4f, 255, 0));
            add(jt::TweenScale::create(
                p, 0.4f, jt::Vector2f { 1.0f, 1.0f }, jt::Vector2f { 1.5f, 1.5f }));
        });
    });
}

std::string StateGame::getName() const { return "Game"; }
