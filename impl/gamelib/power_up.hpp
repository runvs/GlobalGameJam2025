#ifndef POWER_UP_HPP
#define POWER_UP_HPP

#include "animation.hpp"
#include "tilemap/info_rect.hpp"
#include <game_object.hpp>
#include <sprite.hpp>
#include <cstdint>

enum class ePowerUpType : std::uint8_t {
    SOAP,
};

class PowerUp : public jt::GameObject {
public:
    explicit PowerUp(jt::tilemap::InfoRect const& rect);

    void checkIfPlayerIsInPowerUp(
        jt::Vector2f const& playerPosition, std::function<void(ePowerUpType, PowerUp*)> callback);

    [[nodiscard]] ePowerUpType getPowerUpType() const;
    std::shared_ptr<jt::DrawableInterface> getDrawable();

    bool m_pickedUp { false };

private:
    jt::tilemap::InfoRect m_info {};
    ePowerUpType m_type { ePowerUpType::SOAP };
    std::shared_ptr<jt::Animation> m_animation { nullptr };

    void doCreate() override;
    void doUpdate(float elapsed) override;
    void doDraw() const override;
};

#endif // POWER_UP_HPP
