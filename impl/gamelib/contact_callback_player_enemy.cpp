
#include "contact_callback_player_enemy.hpp"
#include <user_data_entries.hpp>

#include <iostream>

void ContactCallbackBubbleKillbox::setPlayer(std::weak_ptr<Player> player) { m_player = player; }

void ContactCallbackBubbleKillbox::setEnabled(bool enabled) { m_enabled = enabled; }

bool ContactCallbackBubbleKillbox::getEnabled() const { return m_enabled; }

void ContactCallbackBubbleKillbox::onBeginContact(b2Contact* contact)
{
    auto p = m_player.lock();
    if (!p) {
        return;
    }

    auto const fa = contact->GetFixtureA();
    auto const fb = contact->GetFixtureB();

    bool optionA = isBubbleFixture(fa) && isKillboxFixture(fb);
    bool optionB = isBubbleFixture(fb) && isKillboxFixture(fa);

    if (optionA || optionB) {

        p->popBubble();
    }
}

void ContactCallbackBubbleKillbox::onEndContact(b2Contact* /*contact*/) { }

bool ContactCallbackBubbleKillbox::isBubbleFixture(b2Fixture* fa) const
{
    void* fixtureUserData = fa->GetUserData();
    return (std::uint64_t)fixtureUserData == g_userDataPlayerBubbleID;
}

bool ContactCallbackBubbleKillbox::isKillboxFixture(b2Fixture* fa) const
{
    void* fixtureUserData = fa->GetUserData();
    return (std::uint64_t)fixtureUserData == g_userDataKillboxID;
}
