#include "exit.hpp"

#include "game_properties.hpp"
#include "math_helper.hpp"
#include <game_interface.hpp>

Exit::Exit(jt::tilemap::InfoRect const& rect) { m_info = rect; }

void Exit::doCreate()
{
    m_sprite = std::make_shared<jt::Sprite>("assets/exit.png", textureManager());

    m_sprite->setPosition(m_info.position);
}

void Exit::doUpdate(float const elapsed) { m_sprite->update(elapsed); }

void Exit::doDraw() const { m_sprite->draw(renderTarget()); }

void Exit::checkIfPlayerIsInExit(
    jt::Vector2f const& playerPosition, std::function<void(std::string const&)> callback)
{

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

        if (jt::MathHelper::checkIsIn(exitRect, playerPosition)) {
            auto const nextLevelName = m_info.properties.strings["next_level"];
            getGame()->logger().info("switch to next level: " + nextLevelName, { "platformer" });
            callback(nextLevelName);
        }
    }
}
