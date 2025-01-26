#ifndef JAMTEMPLATE_KILLBOX_HPP
#define JAMTEMPLATE_KILLBOX_HPP

#include "box2dwrapper/box2d_object.hpp"
#include "box2dwrapper/box2d_world_interface.hpp"
#include "graphics/drawable_interface.hpp"
#include <game_object.hpp>
#include <rect.hpp>
#include <vector.hpp>
#include <functional>

// TODO think about interface
class Killbox : public jt::GameObject {
public:
    Killbox(jt::Rectf const& rect, std::string const& name, std::string const& type,
        std::weak_ptr<jt::Box2DWorldInterface> world);
    void checkIfPlayerIsInKillbox(
        jt::Vector2f const& playerPosition, std::function<void(void)> callback) const;

    std::string getName() const override;

    jt::Vector2f getPosition() const;
    void setPosition(jt::Vector2f const& pos);

private:
    mutable std::shared_ptr<jt::DrawableInterface> m_drawableL { nullptr };
    mutable std::shared_ptr<jt::DrawableInterface> m_drawableM { nullptr };
    mutable std::shared_ptr<jt::DrawableInterface> m_drawableR { nullptr };
    mutable std::shared_ptr<jt::DrawableInterface> m_drawableT { nullptr };
    mutable std::shared_ptr<jt::DrawableInterface> m_drawableB { nullptr };
    jt::Rectf m_rect {};

    std::string m_name { "" };
    std::string m_type { "" };
    std::weak_ptr<jt::Box2DWorldInterface> m_world {};
    std::shared_ptr<jt::Box2DObject> m_physicsObject { nullptr };

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;
};

#endif // JAMTEMPLATE_KILLBOX_HPP
