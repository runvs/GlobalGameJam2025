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
        m_drawableL = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 48, 144, 16, 16 }, textureManager());
        m_drawableM = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 64, 144, 16, 16 }, textureManager());
        m_drawableR = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 80, 144, 16, 16 }, textureManager());
    } else if (m_type == "spike_up") {
        m_drawableL = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 0, 128, 16, 16 }, textureManager());
        m_drawableM = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 16, 128, 16, 16 }, textureManager());
        m_drawableR = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 32, 128, 16, 16 }, textureManager());
    } else if (m_type == "spike_left") {
        m_drawableT = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 96, 96, 16, 16 }, textureManager());
        m_drawableM = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 96, 112, 16, 16 }, textureManager());
        m_drawableB = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 96, 128, 16, 16 }, textureManager());
    } else if (m_type == "spike_right") {
        m_drawableT = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 112, 96, 16, 16 }, textureManager());
        m_drawableM = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 112, 112, 16, 16 }, textureManager());
        m_drawableB = std::make_shared<jt::Sprite>(
            "assets/tileset.png", jt::Recti { 112, 128, 16, 16 }, textureManager());
    }
}

void Killbox::doUpdate(float const elapsed)
{
    if (m_drawableM) {
        m_drawableM->update(elapsed);
    }
}

void Killbox::doDraw() const
{
    if (m_type == "spike_down" || m_type == "spike_up") {
        auto numberOfMiddlePartsY = static_cast<int>(m_rect.height) / 16;
        for (int j = 0; j != numberOfMiddlePartsY; ++j) {
            m_drawableL->setPosition(jt::Vector2f { m_rect.left + j * 16.0f, m_rect.top + 0.0f });
            m_drawableL->update(0.0f);
            m_drawableL->draw(renderTarget());

            auto numberOfPartsX = static_cast<int>(m_rect.width) / 16;
            auto numberOfMiddlePartsX = std::max(0, numberOfPartsX - 2);

            for (int i = 0; i != numberOfMiddlePartsX; ++i) {
                m_drawableM->setPosition(
                    jt::Vector2f { m_rect.left + (i + 1) * 16.0f, m_rect.top + 0.0f });
                m_drawableM->update(0.0f);
                m_drawableM->draw(renderTarget());
            }

            if (numberOfPartsX <= 1) {
                break;
            }
            m_drawableR->setPosition(jt::Vector2f {
                m_rect.left + (numberOfMiddlePartsX + 1) * 16.0f, m_rect.top + 0.0f });
            m_drawableR->update(0.0f);
            m_drawableR->draw(renderTarget());
        }
    } else if (m_type == "spike_left" || m_type == "spike_right") {
        auto numberOfMiddlePartsX = static_cast<int>(m_rect.width) / 16;
        for (int i = 0; i != numberOfMiddlePartsX; ++i) {
            m_drawableT->setPosition(jt::Vector2f { m_rect.left + i * 16.0f, m_rect.top + 0.0f });
            m_drawableT->update(0.0f);
            m_drawableT->draw(renderTarget());

            auto numberOfPartsY = static_cast<int>(m_rect.height) / 16;
            auto numberOfMiddlePartsY = std::max(0, numberOfPartsY - 2);

            for (int j = 0; j != numberOfMiddlePartsY; ++j) {
                m_drawableM->setPosition(
                    jt::Vector2f { m_rect.left + i * 16.0f, m_rect.top + (j + 1) * 16.0f });
                m_drawableM->update(0.0f);
                m_drawableM->draw(renderTarget());
            }

            if (numberOfPartsY <= 1) {
                break;
            }
            m_drawableB->setPosition(jt::Vector2f {
                m_rect.left + i * 16.0f, m_rect.top + (numberOfMiddlePartsY + 1) * 16.0f });
            m_drawableB->update(0.0f);
            m_drawableB->draw(renderTarget());
        }
    }
    //            m_drawable->draw(renderTarget());
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
