#include "power_up.hpp"
#include "game_interface.hpp"
#include "game_properties.hpp"

PowerUp::PowerUp(jt::tilemap::InfoRect const& rect) { m_info = rect; }

void PowerUp::doCreate()
{
    // TODO new sprite
    auto const type = m_info.properties.strings.at("type");
    m_animation = std::make_shared<jt::Animation>();
    m_animation->loadFromAseprite("assets/soap.aseprite", textureManager());
    m_animation->play("idle");
    m_animation->setOffset(jt::OffsetMode::CENTER);

    m_animation->setPosition(m_info.position);
}

void PowerUp::doUpdate(float const elapsed) { m_animation->update(elapsed); }

void PowerUp::doDraw() const
{
    if (m_pickedUp) {
        return;
    }
    if (!isAlive()) {
        return;
    }
    m_animation->draw(renderTarget());
}

void PowerUp::checkIfPlayerIsInPowerUp(
    jt::Vector2f const& playerPosition, std::function<void(ePowerUpType)> callback)
{
    if (m_pickedUp) {

        return;
    }

    jt::Rectf const exitRect { m_info.position.x, m_info.position.y, m_info.size.x, m_info.size.y };

    for (auto const& positionToCheck : {
             // clang-format off
    playerPosition,
    playerPosition + jt::Vector2f{GP::PlayerSize().x *0.0f, GP::PlayerSize().y *0.5f},
    playerPosition + jt::Vector2f{GP::PlayerSize().x *0.0f, -GP::PlayerSize().y *0.5f},
    playerPosition + jt::Vector2f{GP::PlayerSize().x *0.5f, GP::PlayerSize().y *0.0f},
    playerPosition + jt::Vector2f{-GP::PlayerSize().x *0.5f, -GP::PlayerSize().y *0.0f},

    playerPosition + jt::Vector2f{GP::PlayerSize().x *0.5f, GP::PlayerSize().y *0.5f},
    playerPosition + jt::Vector2f{GP::PlayerSize().x *0.5f, -GP::PlayerSize().y *0.5f},
    playerPosition + jt::Vector2f{-GP::PlayerSize().x *0.5f, GP::PlayerSize().y *0.5f},
    playerPosition + jt::Vector2f{-GP::PlayerSize().x *0.5f, -GP::PlayerSize().y *0.5f}
             // clang-format on
         }) {
        if (jt::MathHelper::checkIsIn(exitRect, positionToCheck)) {
            m_pickedUp = true;
            callback(m_type);
        }
    }
}

ePowerUpType PowerUp::getPowerUpType() const { return m_type; }
