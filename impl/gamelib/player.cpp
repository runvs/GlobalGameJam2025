#include "player.hpp"
#include <conversions.hpp>
#include <game_interface.hpp>
#include <line.hpp>
#include <math_helper.hpp>
#include <user_data_entries.hpp>

Player::Player(std::shared_ptr<jt::Box2DWorldInterface> world)
{
    b2BodyDef bodyDef;
    bodyDef.fixedRotation = true;
    bodyDef.type = b2_dynamicBody;

    m_physicsObject = std::make_shared<jt::Box2DObject>(world, &bodyDef);
}

void Player::doCreate()
{
    m_animation = std::make_shared<jt::Animation>();
    m_animation->loadFromAseprite("assets/player.aseprite", textureManager());
    m_animation->play("idle");
    m_animation->setOrigin({ 2, 4 });

    b2FixtureDef fixtureDef;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.5f;
    b2CircleShape circleCollider {};
    circleCollider.m_radius = 4.0f;
    fixtureDef.shape = &circleCollider;
    auto playerCollider = m_physicsObject->getB2Body()->CreateFixture(&fixtureDef);
    playerCollider->SetUserData((void*)(g_userDataPlayerID));

    fixtureDef.isSensor = true;
    b2PolygonShape polygonShape;
    polygonShape.SetAsBox(3.0f, 0.2f, b2Vec2(0, 4), 0);
    fixtureDef.shape = &polygonShape;
    m_footSensorFixture = m_physicsObject->getB2Body()->CreateFixture(&fixtureDef);
}

std::shared_ptr<jt::Animation> Player::getAnimation() { return m_animation; }

void Player::doUpdate(float const elapsed)
{
    m_physicsObject->getB2Body()->DestroyFixture(m_footSensorFixture);
    b2FixtureDef fixtureDef;
    fixtureDef.isSensor = true;
    b2PolygonShape polygonShape;

    auto const rotDeg = -90;
    auto const halfAxis = jt::MathHelper::rotateBy(jt::Vector2f { 3.0f, 0.2f }, rotDeg);
    auto const center = jt::MathHelper::rotateBy(jt::Vector2f { 0, 4 }, rotDeg);

    polygonShape.SetAsBox(halfAxis.x, halfAxis.y, jt::Conversion::vec(center), 0);
    fixtureDef.shape = &polygonShape;
    m_footSensorFixture = m_physicsObject->getB2Body()->CreateFixture(&fixtureDef);
    m_footSensorFixture->SetUserData((void*)g_userDataPlayerFeetID);

    auto currentPosition = m_physicsObject->getPosition();
    clampPositionToLevelSize(currentPosition);
    m_physicsObject->setPosition(currentPosition);

    m_animation->setPosition(currentPosition);
    updateAnimation(elapsed);
    handleMovement(elapsed);

    m_wasTouchingGroundLastFrame = m_isTouchingGround;

    m_lastTouchedGroundTimer -= elapsed;
    m_lastJumpTimer -= elapsed;
    m_wantsToJumpTimer -= elapsed;
}

void Player::clampPositionToLevelSize(jt::Vector2f& currentPosition) const
{
    auto const playerWidth = m_animation->getLocalBounds().width;
    if (currentPosition.x < playerWidth) {
        currentPosition.x = playerWidth;
    }
    if (currentPosition.x > m_levelSizeInTiles.x - playerWidth) {
        currentPosition.x = m_levelSizeInTiles.x - playerWidth;
    }
    // TODO clamp in Y as well
}

void Player::updateAnimation(float elapsed)
{
    // TODO update animation
    auto const rotated_velocity = m_physicsObject->getVelocity();

    if (rotated_velocity.x > 0) {
        m_animation->play("left");
        m_isMoving = true;
    } else if (rotated_velocity.x < 0) {
        m_animation->play("right");
        m_isMoving = true;
    } else {
        m_isMoving = false;
    }

    m_animation->update(elapsed);
}

InputState Player::queryInput()
{
    InputState result = {};
    auto const keyboard = getGame()->input().keyboard();

    result.isLeftPressed = keyboard->pressed(jt::KeyCode::A);
    result.isRightPressed = keyboard->pressed(jt::KeyCode::D);
    result.isUpPressed = keyboard->pressed(jt::KeyCode::W);
    result.isDownPressed = keyboard->pressed(jt::KeyCode::S);

    auto const gamepad = getGame()->input().gamepad(0);

    return result;
}

void Player::handleMovement(float const elapsed)
{
    // TODO query input, then apply physics forces/velocities
    getB2Body()->ApplyForceToCenter(b2Vec2 { 0.0f, 0.0f }, true);
}

b2Body* Player::getB2Body() { return m_physicsObject->getB2Body(); }

void Player::doDraw() const { m_animation->draw(renderTarget()); }

void Player::setTouchesGround(bool touchingGround)
{
    auto const m_postDropJumpTimeFrame = 0.2f; // coyote time
    m_isTouchingGround = touchingGround;
    if (m_isTouchingGround) {
        m_lastTouchedGroundTimer = m_postDropJumpTimeFrame;
    }
}

jt::Vector2f Player::getPosOnScreen() const { return m_animation->getScreenPosition(); }

void Player::setPosition(jt::Vector2f const& pos) { m_physicsObject->setPosition(pos); }

jt::Vector2f Player::getPosition() const { return m_physicsObject->getPosition(); }

void Player::setLevelSize(jt::Vector2f const& levelSizeInTiles)
{
    m_levelSizeInTiles = levelSizeInTiles;
}

void Player::resetVelocity() const { m_physicsObject->setVelocity({ 0, 0 }); }

bool Player::canJump() const
{
    if (m_lastJumpTimer >= 0.0f) {
        return false;
    }
    if (m_isTouchingGround) {
        return true;
    }
    if (m_lastTouchedGroundTimer > 0) {
        return true;
    }
    return false;
}
