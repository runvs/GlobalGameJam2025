#ifndef JAMTEMPLATE_CONTACT_CALLBACK_PLAYER_GROUND_HPP
#define JAMTEMPLATE_CONTACT_CALLBACK_PLAYER_GROUND_HPP

#include "player.hpp"
#include <box2dwrapper/box2d_contact_callback_interface.hpp>
#include <memory>

class ContactCallbackPlayerGround : public jt::Box2DContactCallbackInterface {
public:
    void setPlayer(std::weak_ptr<Player> player);
    void setEnabled(bool enabled) override;
    bool getEnabled() const override;

private:
    // TODO make dependent on Interface, not on concrete class
    std::weak_ptr<Player> m_player;
    int m_numberOfFeetContacts { 0 };
    bool m_enabled { true };

    /// Called when two fixtures begin to touch.
    void onBeginContact(b2Contact* contact) override;

    /// Called when two fixtures cease to touch.
    void onEndContact(b2Contact* contact) override;

    bool isPlayerFeetFixture(b2Fixture* fa) const;
};

#endif // JAMTEMPLATE_CONTACT_CALLBACK_PLAYER_GROUND_HPP
