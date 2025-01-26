#include "player.hpp"

#include "color/color_factory.hpp"
#include "drawable_helpers.hpp"
#include "game_properties.hpp"
#include <conversions.hpp>
#include <game_interface.hpp>
#include <math_helper.hpp>
#include <random/random.hpp>
#include <state_game.hpp>
#include <user_data_entries.hpp>

Player::Player(std::shared_ptr<jt::Box2DWorldInterface> world,
    std::weak_ptr<jt::ParticleSystem<jt::Animation, 100>> exhaustParticleSFstem,
    std::string const& currentLevelName)
    : m_exhaustParticleSystem { exhaustParticleSFstem }
    , m_currentLevelName { currentLevelName }
{
    b2BodyDef bodyDef;
    bodyDef.fixedRotation = true;
    bodyDef.type = b2_dynamicBody;

    m_physicsObject = std::make_shared<jt::Box2DObject>(world, &bodyDef);
}

Player::~Player()
{
    if (m_bubbleSounds) {
        m_bubbleSounds->stop();
    }
    if (m_bubbleSoundsStrong) {
        m_bubbleSoundsStrong->stop();
    }
}

void Player::doCreate()
{
    m_animation = std::make_shared<jt::Animation>();
    m_animation->loadFromAseprite("assets/char.aseprite", textureManager());
    m_animation->play("idle");
    m_animation->setOrigin(jt::OriginMode::CENTER);

    m_bubble = std::make_shared<jt::Animation>();
    m_bubble->loadFromAseprite("assets/bubble.aseprite", textureManager());
    m_bubble->play("b6");
    m_bubble->setOrigin(jt::OriginMode::CENTER);

    m_indicator = jt::dh::createShapeRect({ 2.0f, 2.0f }, jt::colors::Black, textureManager());
    m_indicator->setOrigin(jt::OriginMode::CENTER);

    m_punctureIndicator
        = jt::dh::createShapeRect({ 1.0f, 1.0f }, jt::colors::Red, textureManager());
    m_punctureIndicator->setOrigin(jt::OriginMode::CENTER);

    b2FixtureDef fixtureDef;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.5f;
    b2CircleShape circleCollider {};
    circleCollider.m_radius = GP::PlayerSize().x * 0.4f;
    fixtureDef.shape = &circleCollider;
    auto playerCollider = m_physicsObject->getB2Body()->CreateFixture(&fixtureDef);
    playerCollider->SetUserData((void*)(g_userDataPlayerID));

    m_bubbleSounds = getGame()->audio().addTemporarySound("event:/sfx/bubbles");
    m_bubbleSounds->setVolume(0.0f);
    m_bubbleSoundsStrong = getGame()->audio().addTemporarySound("event:/sfx/bubbles _strong");
    m_bubbleSounds->setVolume(0.0f);
}

std::shared_ptr<jt::Animation> Player::getAnimation() { return m_animation; }

void Player::doUpdate(float const elapsed)
{
    b2FixtureDef fixtureDef;
    fixtureDef.isSensor = true;
    b2PolygonShape polygonShape;

    if (m_velocities.empty()) {
        m_bubbleSounds->setVolume(0.0f);
        m_bubbleSoundsStrong->setVolume(0.0f);
    } else {
        float const v = std::clamp(static_cast<float>(m_velocities.size()) / 10.0f, 0.0f, 1.0f);

        m_bubbleSounds->setVolume(1.0f - v);
        m_bubbleSoundsStrong->setVolume(v);
    }

    auto const rotDeg = -90;
    auto const halfAxis = jt::MathHelper::rotateBy(jt::Vector2f { 3.0f, 0.2f }, rotDeg);
    auto const center = jt::MathHelper::rotateBy(jt::Vector2f { 0, 4 }, rotDeg);

    polygonShape.SetAsBox(halfAxis.x, halfAxis.y, jt::Conversion::vec(center), 0);
    fixtureDef.shape = &polygonShape;

    updateAnimation(elapsed);
    handleMovement(elapsed);

    if (++m_particleFrameCount >= 30) {
        m_particleFrameCount = 0;

        for (auto const& v : m_velocities) {
            auto const position = (m_animation->getPosition() - v * 16.0f);
            m_exhaustParticleSystem.lock()->fire(1, position);
        }
    }

    auto currentPosition = m_physicsObject->getPosition();
    clampPositionToLevelSize(currentPosition);
    m_physicsObject->setPosition(currentPosition);
    m_animation->setPosition(currentPosition);
    m_bubble->setPosition(currentPosition
        + jt::Vector2f { std::sin(getAge() * 0.4123f) * 2.0f, std::sin(getAge() * 0.5f) * 2.0f }
            * std::clamp(m_bubbleVolume * 2.0f, 0.0f, 1.0f));

    handleOutsideBubbleWithoutMovement(elapsed);

    m_previousPosition = m_animation->getPosition();
}

void Player::clampPositionToLevelSize(jt::Vector2f& currentPosition) const
{
    auto const playerWidth = m_animation->getLocalBounds().width;
    auto const playerHeight = m_animation->getLocalBounds().height;
    if (currentPosition.x < playerWidth) {
        currentPosition.x = playerWidth;
    }
    if (currentPosition.x > m_levelSizeInTiles.x - playerWidth) {
        currentPosition.x = m_levelSizeInTiles.x - playerWidth;
    }

    if (currentPosition.y < playerHeight) {
        currentPosition.y = playerHeight;
    }
    if (currentPosition.y > m_levelSizeInTiles.y - playerHeight) {
        currentPosition.y = m_levelSizeInTiles.y - playerHeight;
    }
}

void Player::updateAnimation(float const elapsed)
{
    if (m_hasStabbed) {
        m_hasStabbed = false;
        m_stabbedCooldown = 0.4f;
        if (std::abs(m_indicatorVec.x) > std::abs(m_indicatorVec.y)) {
            if (m_indicatorVec.x > 0) {
                m_animation->play("stab-right");
            } else {
                m_animation->play("stab-left");
            }
        } else {
            if (m_indicatorVec.y > 0) {
                m_animation->play("stab-down");
            } else {
                m_animation->play("stab-up");
            }
        }
    }
    if (m_stabbedCooldown > 0.0f) {
        m_stabbedCooldown -= elapsed;
        if (m_stabbedCooldown <= 0) {
            m_animation->play("idle");
        }
    }

    if (isInBubble()) {
        int const index = std::clamp(static_cast<int>(m_bubbleVolume * 7), 0, 6);
        m_bubble->play("b" + std::to_string(index));
        m_bubbleVolume -= elapsed * m_velocities.size() * GP::BubbleVolumeLossFactor();
    } else {
        if (m_bubble->getCurrentAnimationName() != "pop") {
            m_bubble->play("pop");
            m_velocities.clear();

            for (int i = 0; i < 16; ++i) {
                auto position
                    = m_animation->getPosition() + jt::Random::getRandomPointOnCircle(12.0);

                m_exhaustParticleSystem.lock()->fire(1, position);
            }
        }
    }

    m_animation->update(elapsed);
    m_bubble->update(elapsed);
    m_indicator->update(elapsed);
}

void Player::handleMovement(float const elapsed)
{
    m_punctureTimer -= elapsed;

    if (isInBubble()) {

        m_indicatorVec = jt::Vector2f { 0.0f, 0.0f };
        auto gp = getGame()->input().gamepad(GP::GamepadIndex());
        auto axis = gp->getAxis(jt::GamepadAxisCode::ALeft);
        float const l = jt::MathHelper::length(axis);
        if (l > 0.1f) {
            jt::MathHelper::normalizeMe(axis);
            m_indicatorVec = axis;

            if (m_punctureTimer <= 0) {
                if (gp->justPressed(jt::GamepadButtonCode::GBA)) {
                    m_punctureTimer = GP::PlayerInputPunctureDeadTime();
                    m_velocities.push_back(-1.0f * m_indicatorVec);
                    m_hasStabbed = true;
                    m_bubble->flash(0.3f, jt::ColorFactory::fromHexString("#d59f63"));
                    auto snd = getGame()->audio().addTemporarySound("event:/sfx/poke");
                    snd->play();
                }

                if (gp->justPressed(jt::GamepadButtonCode::GBB)) {
                    if (!m_velocities.empty()) {
                        if (m_patchesAvailable > 0) {

                            auto controllerVec = -1.0f * m_indicatorVec;

                            bool anyPatched = false;
                            std::erase_if(
                                m_velocities, [controllerVec, &anyPatched](auto const& v) {
                                    auto dist = jt::MathHelper::length(controllerVec - v);

                                    bool patchedThis = dist < 0.25f;
                                    anyPatched |= patchedThis;
                                    return patchedThis;
                                });

                            if (anyPatched) {
                                m_patchesAvailable -= 1;
                                m_punctureTimer = GP::PlayerInputPunctureDeadTime();
                                m_bubble->flash(0.3f, jt::ColorFactory::fromHexString("#00f595"));
                                auto snd = getGame()->audio().addTemporarySound("event:/sfx/patch");
                                snd->play();
                                if (m_patchUsedCallback) {
                                    m_patchUsedCallback();
                                }
                            }
                        }
                    }
                }
            }
        }

        m_indicator->setPosition(m_animation->getPosition() + axis * 16.0f);

        jt::Vector2f resultingForce { 0.0f, 0.0f };
        for (auto const& v : m_velocities) {
            resultingForce += v * GP::PlayerBlowoutForceFactor();
        }

        getB2Body()->ApplyForceToCenter(b2Vec2 { resultingForce.x, resultingForce.y }, true);

        // damp bubble movement
        auto v = m_physicsObject->getVelocity();
        v *= GP::PlayerMovementDampeningFactor();
        m_physicsObject->setVelocity(v);
    } else {
        // TODO movement for outside bubble
        getB2Body()->ApplyForceToCenter(b2Vec2 { 0.0f, 10000.0f }, true);
    }
}

void Player::handleOutsideBubbleWithoutMovement(float const elapsed)
{
    if (isInBubble() || m_previousPosition != m_animation->getPosition()) {
        m_timeWithoutBubbleOrMovement = 0.0f;
        return;
    }

    m_timeWithoutBubbleOrMovement += elapsed;

    if (m_timeWithoutBubbleOrMovement >= GP::PlayerWithoutBubbleOrMovementDeathTimer()) {
        // he's dead, Jim
        getGame()->stateManager().switchState(std::make_shared<StateGame>(m_currentLevelName));
    }
}

b2Body* Player::getB2Body() { return m_physicsObject->getB2Body(); }

void Player::doDraw() const
{
    m_animation->draw(renderTarget());
    m_bubble->draw(renderTarget());

    if (isInBubble()) {
        for (auto const& v : m_velocities) {
            m_punctureIndicator->setPosition(m_animation->getPosition() - v * 16.0f);
            m_punctureIndicator->update(0.0f);
            m_punctureIndicator->draw(renderTarget());
        }
        if (jt::MathHelper::length(m_indicatorVec) > 0.2f) {
            m_indicator->draw(renderTarget());
        }
    }
}

jt::Vector2f Player::getPosOnScreen() const { return m_animation->getScreenPosition(); }

jt::Vector2f Player::getPosition() const { return m_physicsObject->getPosition(); }

void Player::setPosition(jt::Vector2f const& pos) { m_physicsObject->setPosition(pos); }

void Player::setVelocity(jt::Vector2f const& velocity) { m_physicsObject->setVelocity(velocity); }

void Player::setLevelSize(jt::Vector2f const& levelSizeInTiles)
{
    m_levelSizeInTiles = levelSizeInTiles;
}

void Player::resetVelocity() const { m_physicsObject->setVelocity({ 0, 0 }); }

bool Player::isInBubble() const { return m_bubbleVolume >= 0.0f; }

void Player::resetBubbleVolume() { m_bubbleVolume = 1.0f; }

void Player::addPatches()
{
    m_patchesAvailable += GP::NumberOfPatchesPerPowerUp();
    m_patchesAvailable = std::clamp(m_patchesAvailable, 0, 20);
}

void Player::setAvailablePatches(int numberOfAvailablePatches)
{
    m_patchesAvailable = numberOfAvailablePatches;
}

void Player::setPatchUsedCallback(std::function<void()> const& callback)
{
    m_patchUsedCallback = callback;
}
