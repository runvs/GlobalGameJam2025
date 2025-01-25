﻿#ifndef JAMTEMPLATE_DEMO_PLATFORM_PLAYER
#define JAMTEMPLATE_DEMO_PLATFORM_PLAYER

#include "line.hpp"
#include <animation.hpp>
#include <box2dwrapper/box2d_object.hpp>
#include <game_object.hpp>
#include <particle_system.hpp>
#include <shape.hpp>
#include <Box2D/Box2D.h>
#include <memory>

class Player : public jt::GameObject {
public:
    using Sptr = std::shared_ptr<Player>;
    explicit Player(std::shared_ptr<jt::Box2DWorldInterface> world);

    ~Player() override = default;

    std::shared_ptr<jt::Animation> getAnimation();
    b2Body* getB2Body();

    void setTouchesGround(bool touchingGround);

    jt::Vector2f getPosOnScreen() const;
    void setPosition(jt::Vector2f const& pos);
    jt::Vector2f getPosition() const;

    void setLevelSize(jt::Vector2f const& levelSizeInTiles);

    void resetVelocity() const;

private:
    std::shared_ptr<jt::Animation> m_animation;

    std::shared_ptr<jt::Shape> m_indicator;
    std::shared_ptr<jt::Shape> m_punctureIndicator;
    std::shared_ptr<jt::Box2DObject> m_physicsObject;

    bool m_isTouchingGround { false };
    bool m_wasTouchingGroundLastFrame { false };

    bool m_isMoving { false };
    jt::Vector2f m_levelSizeInTiles { 0.0f, 0.0f };

    float m_lastTouchedGroundTimer { 0.0f };

    float m_soundTimerWalk { 0.0f };
    float m_soundTimerJump { 0.0f };

    jt::Vector2f m_indicatorVec;
    float m_punctureTimer { 0.0f };
    std::vector<jt::Vector2f> m_velocities;

    void doCreate() override;

    void doUpdate(float const elapsed) override;
    void doDraw() const override;

    void handleMovement(float const elapsed);
    void updateAnimation(float elapsed);
    void clampPositionToLevelSize(jt::Vector2f& currentPosition) const;
    bool m_horizontalMovement { false };
};

#endif // JAMTEMPLATE_DEMO_PLATFORM_PLAYER
