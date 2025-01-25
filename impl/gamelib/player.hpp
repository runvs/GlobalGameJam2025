#ifndef JAMTEMPLATE_DEMO_PLATFORM_PLAYER
#define JAMTEMPLATE_DEMO_PLATFORM_PLAYER

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
    Player(std::shared_ptr<jt::Box2DWorldInterface> world,
        std::weak_ptr<jt::ParticleSystem<jt::Animation, 100>> exhaustParticleSFstem,
        std::string const& currentLevelName);

    ~Player() override = default;

    std::shared_ptr<jt::Animation> getAnimation();
    b2Body* getB2Body();

    jt::Vector2f getPosOnScreen() const;
    void setPosition(jt::Vector2f const& pos);
    jt::Vector2f getPosition() const;

    void setLevelSize(jt::Vector2f const& levelSizeInTiles);

    void resetVelocity() const;

    bool isInBubble() const;

    void resetBubbleVolume();

    void addPatches();
    void setAvailablePatches(int numberOfAvailablePatches);

private:
    std::shared_ptr<jt::Animation> m_animation;
    std::shared_ptr<jt::Animation> m_bubble;
    std::shared_ptr<jt::Shape> m_indicator;
    std::shared_ptr<jt::Shape> m_punctureIndicator;
    std::shared_ptr<jt::Box2DObject> m_physicsObject;
    std::weak_ptr<jt::ParticleSystem<jt::Animation, 100>> m_exhaustParticleSystem;

    bool m_cheatsActive { false };

    int m_particleFrameCount { 0 };

    float m_bubbleVolume { 1.0 };

    bool m_isMoving { false };
    jt::Vector2f m_previousPosition { 9000.1f, 9000.1f };
    float m_timeWithoutBubbleOrMovement = 0.0f;

    jt::Vector2f m_levelSizeInTiles { 0.0f, 0.0f };
    std::string const& m_currentLevelName;

    float m_soundTimerWalk { 0.0f };
    float m_soundTimerJump { 0.0f };

    jt::Vector2f m_indicatorVec;
    float m_punctureTimer { 0.0f };
    std::vector<jt::Vector2f> m_velocities;
    bool m_hasStabbed { false };
    float m_stabbedCooldown { 0.2f };

    void doCreate() override;

    void doUpdate(float const elapsed) override;
    void doDraw() const override;

    void handleCheats(float const elapsed);
    void handleMovement(float const elapsed);
    void handleOutsideBubbleWithoutMovement(float const);
    void updateAnimation(float elapsed);
    void clampPositionToLevelSize(jt::Vector2f& currentPosition) const;
    bool m_horizontalMovement { false };

    int m_patchesAvailable { 2 };
};

#endif // JAMTEMPLATE_DEMO_PLATFORM_PLAYER
