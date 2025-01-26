#include "level.hpp"

#include "game_properties.hpp"
#include "power_up.hpp"
#include <game_interface.hpp>
#include <math_helper.hpp>
#include <strutils.hpp>
#include <tilemap/tileson_loader.hpp>
#include <Box2D/Box2D.h>

Level::Level(std::string const& fileName, std::weak_ptr<jt::Box2DWorldInterface> world)
{
    m_fileName = fileName;
    m_world = world;
}

void Level::doCreate()
{
    m_flatColorBackground = std::make_shared<jt::Shape>();
    m_flatColorBackground->makeRect(GP::GetScreenSize(), textureManager());

    m_flatColorBackground->setCamMovementFactor(0.0f);
    m_background = std::make_shared<jt::Sprite>("assets/background.aseprite", textureManager());

    auto c = m_background->getColor();
    c.a = 50;
    m_background->setColor(c);
    m_background->setCamMovementFactor(0.3f);

    jt::tilemap::TilesonLoader loader { getGame()->cache().getTilemapCache(), m_fileName };

    loadLevelSize(loader);
    loadLevelSettings(loader);
    loadLevelTileLayer(loader);
    loadLevelCollisions(loader);
    loadLevelKillboxes(loader);
    loadLevelPowerups(loader);
    loadMovingPlatforms(loader);
}

void Level::loadMovingPlatforms(jt::tilemap::TilesonLoader& loader)
{
    auto const platform_infos = loader.loadObjectsFromLayer("platforms");

    std::map<std::string, std::pair<jt::Vector2f, float>> allPositionsInLevel;
    for (auto const& p : platform_infos) {

        if (!p.properties.strings.contains("type")) {
            float waitTime = 0.0f;
            if (p.properties.floats.contains("wait")) {
                waitTime = p.properties.floats.at("wait");
            }
            allPositionsInLevel[p.name] = std::make_pair(p.position, waitTime);
        }
    }
    for (auto const& p : platform_infos) {
        if (!p.properties.strings.empty()) {
            std::vector<std::pair<jt::Vector2f, float>> currentPlatformPositions;
            auto const positionsString = p.properties.strings.at("positions");
            auto const individualPositionStrings = strutil::split(positionsString, ",");
            for (auto const& ps : individualPositionStrings) {
                if (!allPositionsInLevel.contains(ps)) {
                    getGame()->logger().warning("position not found in level: " + ps, { "level" });
                }
                currentPlatformPositions.push_back(allPositionsInLevel[ps]);
            }
            float timeoffset = 0.0f;
            if (p.properties.floats.count("timeoffset") == 1) {
                timeoffset = p.properties.floats.at("timeoffset");
            }
            auto platform = std::make_shared<MovingPlatform>(m_world.lock(), p.size,
                currentPlatformPositions, p.properties.floats.at("velocity"), timeoffset,
                p.properties.strings.at("type"));
            std::string linkedKillbox { "" };
            if (p.properties.strings.count("linked_killbox") == 1) {
                linkedKillbox = p.properties.strings.at("linked_killbox");
            }
            if (!linkedKillbox.empty()) {
                std::shared_ptr<Killbox> ptr { nullptr };
                for (auto kb : m_killboxes) {
                    if (kb->getName() == linkedKillbox) {
                        ptr = kb;
                        break;
                    }
                }

                if (ptr != nullptr) {
                    platform->setLinkedKillbox(ptr);
                }
            }
            platform->setGameInstance(getGame());
            platform->create();
            m_movingPlatforms.push_back(platform);
        }
    }
}

void Level::loadLevelSize(jt::tilemap::TilesonLoader const& loader)
{
    auto const sizeInTiles = loader.getMapSizeInTiles();
    m_levelSizeInPixel = jt::Vector2f { 16.0f * sizeInTiles.x, 16.0f * sizeInTiles.y };
}

void Level::loadLevelKillboxes(jt::tilemap::TilesonLoader& loader)
{
    auto const killboxInfos = loader.loadObjectsFromLayer("killboxes");
    for (auto const& i : killboxInfos) {
        std::string name { i.name };
        std::string type { "" };
        if (i.properties.strings.count("type") == 1) {
            type = i.properties.strings.at("type");
        }
        auto kb = std::make_shared<Killbox>(
            jt::Rectf { i.position.x, i.position.y, i.size.x, i.size.y }, name, type);
        kb->setGameInstance(getGame());
        kb->create();
        m_killboxes.push_back(kb);
    }
}

void Level::loadLevelPowerups(jt::tilemap::TilesonLoader& loader)
{
    auto const powerUpInfos = loader.loadObjectsFromLayer("powerups");
    for (auto const& i : powerUpInfos) {
        auto pu = std::make_shared<PowerUp>(i);
        pu->setGameInstance(getGame());
        pu->create();
        m_powerUps.push_back(pu);
    }
}

void Level::loadLevelCollisions(jt::tilemap::TilesonLoader& loader)
{
    auto tileCollisions = loader.loadCollisionsFromLayer("ground");

    tileCollisions.refineColliders(16);
    for (auto const& r : tileCollisions.getRects()) {
        b2BodyDef bodyDef;
        bodyDef.fixedRotation = true;
        bodyDef.type = b2_staticBody;
        bodyDef.position.Set(r.left + r.width / 2.0f, r.top + r.height / 2.0f);

        b2FixtureDef fixtureDef;
        b2PolygonShape boxCollider {};
        boxCollider.SetAsBox(r.width / 2.0f, r.height / 2.0f);
        fixtureDef.shape = &boxCollider;

        auto collider = std::make_shared<jt::Box2DObject>(m_world.lock(), &bodyDef);
        collider->getB2Body()->CreateFixture(&fixtureDef);

        m_colliders.push_back(collider);
    }
}

void Level::loadLevelTileLayer(jt::tilemap::TilesonLoader& loader)
{
    m_tileLayerGround = std::make_shared<jt::tilemap::TileLayer>(
        loader.loadTilesFromLayer("ground", textureManager(), "assets/"));
}

void Level::loadLevelSettings(jt::tilemap::TilesonLoader& loader)
{
    auto settings = loader.loadObjectsFromLayer("settings");
    for (auto const& info : settings) {

        if (info.name == "map_settings") {
            m_flatColorBackground->setColor(
                jt::Color { static_cast<uint8_t>(info.properties.ints.at("bg_r")),
                    static_cast<uint8_t>(info.properties.ints.at("bg_g")),
                    static_cast<uint8_t>(info.properties.ints.at("bg_b")) });
            if (info.properties.ints.contains("initial_patches")) {
                m_initiallyAvailablePatches = info.properties.ints.at("initial_patches");
            }
        } else if (info.name == "player_start") {
            m_playerStart = info.position;
        } else if (info.name == "exit") {
            auto exit = Exit { info };
            exit.setGameInstance(getGame());
            exit.create();
            m_exits.emplace_back(exit);
        }
    }
}

void Level::doUpdate(float const elapsed)
{
    m_flatColorBackground->update(elapsed);
    m_background->update(elapsed);
    m_tileLayerGround->update(elapsed);

    for (auto& exit : m_exits) {
        exit.update(elapsed);
    }
    for (auto& p : m_movingPlatforms) {
        p->update(elapsed);
    }
    for (auto& kb : m_killboxes) {
        kb->update(elapsed);
    }
    for (auto& pu : m_powerUps) {
        pu->update(elapsed);
    }
}

void Level::doDraw() const
{
    m_flatColorBackground->draw(renderTarget());
    float const w = m_background->getLocalBounds().width;
    float const h = m_background->getLocalBounds().height;
    for (auto i = -2; i != 3; ++i) {
        for (auto j = -2; j != 3; ++j) {
            m_background->setPosition(jt::Vector2f { i * w, j * h });
            m_background->update(0.0f);
            m_background->draw(renderTarget());
        }
    }

    m_tileLayerGround->draw(renderTarget());

    for (auto const& exit : m_exits) {
        exit.draw();
    }
    for (auto const& p : m_movingPlatforms) {
        p->draw();
    }
    for (auto const& kb : m_killboxes) {
        kb->draw();
    }
    for (auto& pu : m_powerUps) {
        pu->draw();
    }
}

jt::Vector2f Level::getPlayerStart() const { return m_playerStart; }

void Level::checkIfPlayerIsInKillbox(
    jt::Vector2f const& playerPosition, std::function<void(void)> callback) const
{
    for (auto const& kb : m_killboxes) {
        kb->checkIfPlayerIsInKillbox(playerPosition, callback);
    }
}

void Level::checkIfPlayerIsInExit(
    jt::Vector2f const& playerPosition, std::function<void(std::string const&)> callback)
{
    for (auto& exit : m_exits) {
        exit.checkIfPlayerIsInExit(playerPosition, callback);
        break;
    }
}

void Level::checkIfPlayerIsInPowerup(
    jt::Vector2f const& playerPosition, std::function<void(ePowerUpType, PowerUp*)> const& callback)
{
    for (auto p : m_powerUps) {
        if (!p->isAlive()) {
            continue;
        }
        p->checkIfPlayerIsInPowerUp(playerPosition, callback);
    }
}

jt::Vector2f Level::getLevelSizeInPixel() const { return m_levelSizeInPixel; }

int Level::getNumberOfInitiallyAvailablePatches() const { return m_initiallyAvailablePatches; }
