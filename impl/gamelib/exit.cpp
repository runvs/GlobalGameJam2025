#include "exit.hpp"

#include "game_properties.hpp"
#include "math_helper.hpp"
#include <game_interface.hpp>

Exit::Exit(jt::tilemap::InfoRect const& rect) { m_info = rect; }

void Exit::doCreate()
{
    m_animation = std::make_shared<jt::Animation>();
    m_animation->loadFromAseprite("assets/goal.aseprite", textureManager());
    m_animation->play("idle");
    m_animation->setPosition(m_info.position);
}

void Exit::doUpdate(float const elapsed) { m_animation->update(elapsed); }

void Exit::doDraw() const { m_animation->draw(renderTarget()); }

void Exit::checkIfPlayerIsInExit(
    jt::Vector2f const& playerPosition, std::function<void(std::string const&)> callback)
{

    jt::Rectf const exitRect { m_info.position.x + 8, m_info.position.y + 8, m_info.size.x - 16,
        m_info.size.y - 16 };

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
            auto const nextLevelName = m_info.properties.strings["next_level"];
            getGame()->logger().info("switch to next level: " + nextLevelName, { "platformer" });
            m_animation->play("expl");
            callback(nextLevelName);
        }
    }
}
