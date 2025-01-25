#ifndef JAMTEMPLATE_CONTACT_CALLBACK_PLAYER_ENEMY_HPP
#define JAMTEMPLATE_CONTACT_CALLBACK_PLAYER_ENEMY_HPP

#include "player.hpp"
#include <box2dwrapper/box2d_contact_callback_interface.hpp>
#include <memory>

class ContactCallbackPlayerEnemy : public jt::Box2DContactCallbackInterface {
public:
    void setPlayer(std::weak_ptr<Player> player);
    void setEnabled(bool enabled) override;
    bool getEnabled() const override;

private:
    // TODO make dependent on Interface, not on concrete class
    std::weak_ptr<Player> m_player;
    bool m_enabled { true };

    /// Called when two fixtures begin to touch.
    void onBeginContact(b2Contact* contact) override;

    /// Called when two fixtures cease to touch.
    void onEndContact(b2Contact* contact) override;

    bool isPlayerFixture(b2Fixture* fa) const;
    bool isEnemyFixture(b2Fixture* fa) const;
};

#endif // JAMTEMPLATE_CONTACT_CALLBACK_PLAYER_ENEMY_HPP
