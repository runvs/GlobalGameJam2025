#ifndef GAME_GAMEPROPERTIES_HPP
#define GAME_GAMEPROPERTIES_HPP

#include "player.hpp"
#include <color/color.hpp>
#include <color/palette.hpp>
#include <vector.hpp>
#include <string>

class GP {
public:
    GP() = delete;

    static std::string GameName() { return "PFFFFFfffffft"; }

    static std::string AuthorName() { return "SSHTM"; }

    static std::string JamName() { return "GlobalGameJam"; }

    static std::string JamDate() { return "2025-01-24 - 26"; }

    static std::string ExplanationText()
    {
        return "Win the game\n[W,A,S,D] to move \n[Space] to jump\n[M/U] to mute/unmute audio";
    }

    static jt::Vector2f GetWindowSize() { return jt::Vector2f { 1280, 720 }; }

    static float GetZoom() { return 4.0f; }

    static jt::Vector2f GetScreenSize() { return GetWindowSize() * (1.0f / GetZoom()); }

    static jt::Color PaletteBackground() { return GP::getPalette().getColor(4); }

    static jt::Color PaletteFontFront() { return GP::getPalette().getColor(0); }

    static jt::Color PalleteFrontHighlight() { return GP::getPalette().getColor(1); }

    static jt::Color PaletteFontShadow() { return GP::getPalette().getColor(2); }

    static jt::Color PaletteFontCredits() { return GP::getPalette().getColor(1); }

    static jt::Palette getPalette();

    static int PhysicVelocityIterations();
    static int PhysicPositionIterations();
    static jt::Vector2f PlayerSize();
    static float PlayerInputPunctureDeadTime();
    static float PlayerMovementDampeningFactor();
    static float PlayerBlowoutForceFactor();
    static float PlayerWithoutBubbleOrMovementDeathTimer();
    static int GamepadIndex();
    static float BubbleVolumeLossFactor();
};

#endif
