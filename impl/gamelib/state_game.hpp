#ifndef STATE_GAME
#define STATE_GAME

#include "hud/hud.hpp"
#include <box2dwrapper/box2d_object.hpp>
#include <box2dwrapper/box2d_world_interface.hpp>
#include <contact_callback_player_enemy.hpp>
#include <contact_callback_player_ground.hpp>
#include <game_state.hpp>
#include <level.hpp>
#include <particle_system.hpp>
#include <player.hpp>
#include <screeneffects/vignette.hpp>
#include <shape.hpp>
#include <tilemap/tile_layer.hpp>
#include <vector.hpp>

class StateGame : public jt::GameState {
public:
    explicit StateGame(std::string const& levelName = "level_01.json");

private:
    std::shared_ptr<jt::Box2DWorldInterface> m_world { nullptr };
    std::shared_ptr<jt::ParticleSystem<jt::Animation, 100>> m_particlesBubbleExhaust;

    std::string m_levelName { "" };

    std::shared_ptr<Hud> m_hud { nullptr };

    std::shared_ptr<Level> m_level { nullptr };
    std::shared_ptr<Player> m_player { nullptr };
    std::shared_ptr<jt::Shape> m_overlay { nullptr };
    std::shared_ptr<jt::Vignette> m_vignette { nullptr };

    bool m_cheatsActive { false };
    bool m_ending { false };

    std::string getName() const override;

    void onCreate() override;
    void onEnter() override;
    void clampCameraToScreen();
    void onUpdate(float const /*elapsed*/) override;
    void onDraw() const override;

    void createPlayer();
    void loadLevel();
    void handleReturnToMainMenu(float const elapsed);
    void handleCheatModeActivation(float const elapsed);
    void handleCheats(float const elapsed);
    void handleCameraScrolling(float const elapsed);
    void restartLevel();
    void transitionToLevel(std::string const& newLevelName);
};

#endif // STATE_GAME
