//
// Created by robin on 22.09.2019.
//

#include "SfmlDemoManager.h"

void SfmlDemoManager::initialize(const sf::Vector2i &windowSize, const sf::Vector2i &resolution, const std::string &title, const fs::path &basePath)
{
    m_window.create(sf::VideoMode(windowSize.x, windowSize.y), title, sf::Style::Titlebar);
    m_window.setView(sf::View(sf::FloatRect(0.f, 0.f, resolution.x, resolution.y)));
    m_basePath = basePath;
}

bool SfmlDemoManager::parseMap(const std::string &filename)
{
    tson::Tileson t;
    m_map = t.parse(fs::path(m_basePath / filename));

    if(m_map.getStatus() == tson::Map::ParseStatus::OK)
    {
        for(auto &tileset : m_map.getTilesets())
            storeAndLoadImage(tileset.getImage().u8string(), {0,0});

        return true;
    }
    else
        std::cout << "Parse error: " << m_map.getStatusMessage() << std::endl;

    return false;
}

void SfmlDemoManager::drawMap()
{
    tson::Tileset* tileset = m_map.getTileset("demo-tileset");

    for(auto &layer : m_map.getLayers())
    {
        switch (layer.getType())
        {
            case tson::Layer::Type::TileLayer:
                drawTileLayer(layer, tileset);
                break;

            case tson::Layer::Type::ObjectGroup:
                //TODO: Implement
                break;

            case tson::Layer::Type::ImageLayer:
                drawImageLayer(layer);
                break;

            case tson::Layer::Type::Group:
                //TODO: Implement
                break;

            default:
                break;
        }
    }
}

void SfmlDemoManager::run()
{
    while (m_window.isOpen())
    {
        // Process events
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            // Close m_window: exit
            if (event.type == sf::Event::Closed)
                m_window.close();
        }
        // Clear screen
        m_window.clear({35, 65, 90, 255});
        drawMap();
        m_window.display();
    }
}

void SfmlDemoManager::drawTileLayer(tson::Layer& layer, tson::Tileset* tileset)
{
    int firstId = tileset->getFirstgid(); //First tile id of the tileset
    int columns = tileset->getColumns(); //For the demo map it is 8.
    int rows = tileset->getTileCount() / columns;
    int lastId = (tileset->getFirstgid() + tileset->getTileCount()) - 1;

    //pos = position in tile units
    for (auto& [pos, tile] : layer.getTileData()) //Loops through absolutely all existing tiles
    {
        fs::path imagePath;
        std::string pathStr;
        //With this, I know that it's related to the tileset above (though I only have one tileset)
        if (tile->getId() >= firstId && tile->getId() <= lastId)
        {
            //Get position in pixel units
            tson::Vector2f position = {(float) std::get<0>(pos) * m_map.getTileSize().x, (float) std::get<1>(pos) * m_map.getTileSize().y};

            int baseTilePosition = (tile->getId() - firstId);

            int tileModX = (baseTilePosition % columns);
            int currentRow = (baseTilePosition / columns);
            int offsetX = (tileModX != 0) ? ((tileModX) * m_map.getTileSize().x) : (0 * m_map.getTileSize().x);
            int offsetY =  (currentRow < rows-1) ? (currentRow * m_map.getTileSize().y) : ((rows-1) * m_map.getTileSize().y);

            //Set sprite data to draw the tile
            sf::Sprite *sprite = storeAndLoadImage(tileset->getImage().u8string(), {0,0});
            if(sprite != nullptr)
            {
                sprite->setTextureRect({offsetX, offsetY, m_map.getTileSize().x, m_map.getTileSize().y});
                sprite->setPosition({position.x, position.y});
                m_window.draw(*sprite);
            }
        }
    }
}

void SfmlDemoManager::drawImageLayer(tson::Layer &layer)
{
    sf::Sprite *sprite = storeAndLoadImage(layer.getImage(), {layer.getOffset().x, layer.getOffset().y});
    if(sprite != nullptr)
        m_window.draw(*sprite);
}

/*!
 * Stores and loads the image if it doesn't exists, and retrieves it if it does.
 * @param image
 * @return
 */
sf::Sprite *SfmlDemoManager::storeAndLoadImage(const std::string &image, const sf::Vector2f &position)
{
    if(m_textures.count(image) == 0)
    {
        fs::path path = m_basePath / image;
        if(fs::exists(path))
        {
            std::unique_ptr<sf::Texture> tex = std::make_unique<sf::Texture>();
            tex->loadFromFile(path);
            std::unique_ptr<sf::Sprite> spr = std::make_unique<sf::Sprite>();
            spr->setTexture(*tex);
            spr->setPosition(position);
            m_textures[image] = std::move(tex);
            m_sprites[image] = std::move(spr);
        }
        else
            std::cout << "Could not find: " << path.u8string() << std::endl;
    }

    if(m_sprites.count(image) > 0)
        return m_sprites[image].get();

    return nullptr;
}