#ifndef GAME_HUD_HPP
#define GAME_HUD_HPP

#include <game_object.hpp>
#include <sprite.hpp>
#include <functional>
#include <memory>

class ScoreDisplay;

class Hud : public jt::GameObject {
public:
    void setPatches(int p);
    void addPatches(int p, std::function<void(std::shared_ptr<jt::Sprite>)> const& cb);
    void removePatch(std::function<void(std::shared_ptr<jt::Sprite>)> const&);

private:
    int m_numberOfAvailablePatches;

    std::vector<std::shared_ptr<jt::Sprite>> m_patches;

    void doCreate() override;

    void doUpdate(float const elapsed) override;

    void doDraw() const override;
};

#endif
