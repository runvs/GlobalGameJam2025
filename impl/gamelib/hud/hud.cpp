#include "hud.hpp"
#include <color/color.hpp>
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <hud/score_display.hpp>

void Hud::setPatches(int p)
{
    m_numberOfAvailablePatches = p;
    m_numberOfAvailablePatches
        = std::clamp(m_numberOfAvailablePatches, 0, static_cast<int>(m_patches.size()) - 1);
    for (auto i = 0; i < m_patches.size(); i++) {
        bool visible = i <= m_numberOfAvailablePatches - 1;
        m_patches[i]->setColor(jt::Color { 255, 255, 255, std::uint8_t(visible ? 255u : 0u) });
    }
    m_numberOfAvailablePatches = m_numberOfAvailablePatches - 1;
}

void Hud::addPatches(int p, std::function<void(std::shared_ptr<jt::Sprite>)> const& cb)
{
    for (int i = 0; i != p; ++i) {
        m_numberOfAvailablePatches += 1;
        if (m_numberOfAvailablePatches < m_patches.size()) {
            cb(m_patches[m_numberOfAvailablePatches]);
        }
    }
}

void Hud::removePatch(std::function<void(std::shared_ptr<jt::Sprite>)> const& cb)
{
    cb(m_patches.at(m_numberOfAvailablePatches));
    m_numberOfAvailablePatches -= 1;
    m_numberOfAvailablePatches
        = std::clamp(m_numberOfAvailablePatches, 0, static_cast<int>(m_patches.size()) - 1);
}

void Hud::doCreate()
{
    for (auto i = 0; i != 20; ++i) {
        auto sprite = std::make_shared<jt::Sprite>(
            "assets/patch.aseprite", jt::Recti { 0, 0, 16, 16 }, textureManager());
        float const x = (i % 10) * 12 + 6;
        float const y = (i / 10) * 16 + 8;
        sprite->setPosition({ x, y });
        sprite->setIgnoreCamMovement(true);
        sprite->setOrigin(jt::OriginMode::CENTER);
        m_patches.push_back(sprite);
    }
}

void Hud::doUpdate(float const elapsed)
{
    for (auto& p : m_patches) {
        p->update(elapsed);
    }
}

void Hud::doDraw() const
{
    for (auto& p : m_patches) {
        p->draw(renderTarget());
    }
}
