#include "state_menu.hpp"
#include <build_info.hpp>
#include <color/color.hpp>
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <input/input_manager.hpp>
#include <lerp.hpp>
#include <log/license_info.hpp>
#include <screeneffects/vignette.hpp>
#include <state_game.hpp>
#include <state_manager/state_manager_transition_fade_to_black.hpp>
#include <text.hpp>
#include <tweens/tween_alpha.hpp>
#include <tweens/tween_color.hpp>
#include <tweens/tween_position.hpp>
#include <algorithm>

void StateMenu::onCreate()
{
    createMenuText();
    createShapes();
    createVignette();

    add(std::make_shared<jt::LicenseInfo>());

    getGame()->stateManager().setTransition(std::make_shared<jt::StateManagerTransitionFadeToBlack>(
        GP::GetScreenSize(), textureManager()));

    try {
        auto bgm = getGame()->audio().getPermanentSound("music");
        if (bgm == nullptr) {
            bgm = getGame()->audio().addPermanentSound("bgm", "event:/music");
            bgm->play();
        }
    } catch (std::exception const& e) {
        getGame()->logger().error(e.what(), { "menu", "music" });
    }
}

void StateMenu::onEnter()
{
    createTweens();
    m_started = false;
}

void StateMenu::createVignette()
{
    m_vignette = std::make_shared<jt::Vignette>(GP::GetScreenSize());
    add(m_vignette);
}

void StateMenu::createShapes()
{
    m_backgroundFlat
        = jt::dh::createShapeRect(GP::GetScreenSize(), jt::colors::Black, textureManager());
    m_backgroundFlat->setCamMovementFactor(0.0f);
    m_logo = std::make_shared<jt::Sprite>("assets/menu.aseprite", textureManager());
    m_logo->setCamMovementFactor(0.0f);
    m_overlay = jt::dh::createShapeRect(GP::GetScreenSize(), jt::colors::Black, textureManager());
    m_overlay->setCamMovementFactor(0.0f);
    m_background = std::make_shared<jt::Sprite>("assets/background.aseprite", textureManager());

    auto c = m_background->getColor();
    c.a = 50;
    m_background->setColor(c);
}

void StateMenu::createMenuText()
{
    createTextTitle();
    createTextStart();
    createTextExplanation();
    createTextCredits();
}

void StateMenu::createTextExplanation() { }

void StateMenu::createTextCredits()
{
    m_textVersion = jt::dh::createText(renderTarget(), "", 14u, jt::colors::White);
    m_textVersion->setCamMovementFactor(0.0f);
    if (jt::BuildInfo::gitTagName() != "") {
        m_textVersion->setText(jt::BuildInfo::gitTagName());
    } else {
        m_textVersion->setText(
            jt::BuildInfo::gitCommitHash().substr(0, 6) + " " + jt::BuildInfo::timestamp());
    }
    m_textVersion->setTextAlign(jt::Text::TextAlign::RIGHT);
    m_textVersion->setPosition({ GP::GetScreenSize().x - 5.0f, GP::GetScreenSize().y - 20.0f });
    m_textVersion->setShadow(jt::colors::Gray, jt::Vector2f { 1, 1 });
}

void StateMenu::createTextStart() { }

void StateMenu::createTextTitle() { }

void StateMenu::createTweens()
{
    createTweenOverlayAlpha();
    createTweenTitleAlpha();
    createTweenCreditsPosition();
    createTweenExplanation();
}

void StateMenu::createInstructionTweenColor1() { }

void StateMenu::createInstructionTweenColor2() { }

void StateMenu::createTweenExplanation() { }

void StateMenu::createTweenTitleAlpha() { }

void StateMenu::createTweenOverlayAlpha()
{
    auto const tween
        = jt::TweenAlpha::create(m_overlay, 0.5f, std::uint8_t { 255 }, std::uint8_t { 0 });
    tween->setSkipTicks();
    add(tween);
}

void StateMenu::createTweenCreditsPosition() { }

void StateMenu::onUpdate(float const elapsed)
{
    getGame()->gfx().camera().move(jt::Vector2f { 2.0f, 3.0f } * elapsed);
    updateDrawables(elapsed);
    checkForTransitionToStateGame();
}

void StateMenu::updateDrawables(float const& elapsed)
{
    m_backgroundFlat->update(elapsed);
    m_background->update(elapsed);
    m_logo->update(elapsed);

    m_textVersion->update(elapsed);
    m_overlay->update(elapsed);
    m_vignette->update(elapsed);
}

void StateMenu::checkForTransitionToStateGame()
{
    auto const& kb = getGame()->input().keyboard();
    auto const& gp = getGame()->input().gamepad(GP::GamepadIndex());

    auto const keysToTriggerTransition = { jt::KeyCode::Space, jt::KeyCode::Enter };

    if (std::any_of(std::begin(keysToTriggerTransition), std::end(keysToTriggerTransition),
            [&kb](auto const k) { return kb->justPressed(k); })) {
        startTransitionToStateGame();
    }

    auto const buttonsToTriggerTransition = { jt::GamepadButtonCode::GBA };

    if (std::any_of(std::begin(buttonsToTriggerTransition), std::end(buttonsToTriggerTransition),
            [&gp](auto const k) { return gp->justPressed(k); })) {
        startTransitionToStateGame();
    }
}

void StateMenu::startTransitionToStateGame()
{
    if (!m_started) {
        m_started = true;
        getGame()->stateManager().storeCurrentState("menu");
        getGame()->stateManager().switchState(std::make_shared<StateGame>());
    }
}

void StateMenu::onDraw() const
{
    m_backgroundFlat->draw(renderTarget());

    float const w = m_background->getLocalBounds().width;
    float const h = m_background->getLocalBounds().height;
    for (auto i = -70; i != 70; ++i) {
        for (auto j = -70; j != 70; ++j) {
            m_background->setPosition(jt::Vector2f { i * w, j * h });
            m_background->update(0.0f);
            m_background->draw(renderTarget());
        }
    }

    m_logo->draw(renderTarget());

    m_textVersion->draw(renderTarget());

    m_overlay->draw(renderTarget());
    m_vignette->draw();
}

std::string StateMenu::getName() const { return "State Menu"; }
