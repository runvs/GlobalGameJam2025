
#include "killbox.hpp"

#include "game_properties.hpp"
#include <game_interface.hpp>
#include <math_helper.hpp>
#include <sprite.hpp>

Killbox::Killbox(jt::Rectf const& rect, std::string const& name, std::string const& type)
    : m_rect { rect }
    , m_name { name }
    , m_type { type }
{
}

void Killbox::doCreate()
{
    if (m_type == "spike_down") {
        m_drawable = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 64, 144, 8, 8 }, textureManager());
    } else if (m_type == "spike_up") {
        m_drawable = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 16, 128, 16, 16 }, textureManager());
    } else if (m_type == "spike_left") {
        m_drawable = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 96, 112, 16, 16 }, textureManager());
    } else if (m_type == "spike_right") {
        m_drawable = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 112, 112, 16, 16 }, textureManager());
    }
}

void Killbox::doUpdate(float const elapsed)
{
    if (m_drawable) {
        m_drawable->update(elapsed);
    }
}

void Killbox::doDraw() const
{
    if (m_drawable) {
        if (m_type == "spike_down" || m_type == "spike_up") {
            auto numberOfParts = static_cast<int>(m_rect.width) / 16;
            for (int i = 0; i != numberOfParts; ++i) {
                m_drawable->setPosition(
                    jt::Vector2f { m_rect.left + i * 16.0f, m_rect.top + 0.0f });
                m_drawable->update(0.0f);
                m_drawable->draw(renderTarget());
            }
        } else if (m_type == "spike_left" || m_type == "spike_right") {
            auto numberOfParts = static_cast<int>(m_rect.height) / 16;
            for (int i = 0; i != numberOfParts; ++i) {
                m_drawable->setPosition(
                    jt::Vector2f { m_rect.left + 0.0f, m_rect.top + i * 16.0f });
                m_drawable->update(0.0f);
                m_drawable->draw(renderTarget());
            }
            //            m_drawable->draw(renderTarget());
        }
    }
}

void Killbox::checkIfPlayerIsInKillbox(
    jt::Vector2f const& playerPosition, std::function<void(void)> callback) const
{
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
        if (jt::MathHelper::checkIsIn(m_rect, positionToCheck)) {
            getGame()->logger().info(
                "Player killed by killbox '" + m_name + "'", { "demo", "killbox" });
            callback();
        }
    }
}

std::string Killbox::getName() const { return m_name; }

jt::Vector2f Killbox::getPosition() const { return jt::Vector2f { m_rect.left, m_rect.top }; }

void Killbox::setPosition(jt::Vector2f const& pos)
{
    m_rect.left = pos.x;
    m_rect.top = pos.y;
}
