#include "../lux/kit.hpp"
#include "../lux/define.cpp"
#include "tools.cpp"
#include <string.h>
#include <vector>
#include <set>
#include <stdio.h>

vector<Cell *> GetResourceTiles(GameMap gameMap)
{
    vector<Cell *> resourceTiles = vector<Cell *>();
    for (int y = 0; y < gameMap.height; y++)
    {
        for (int x = 0; x < gameMap.width; x++)
        {
            Cell *cell = gameMap.getCell(x, y);
            if (cell->hasResource())
            {
                resourceTiles.push_back(cell);
            }
        }
    }
    return resourceTiles;
}

Cell *GetClosestResource(Unit unit, vector<Cell *> resourceTiles, Player player, ResourceType type = ResourceType::any)
{
    Cell *closestResourceTile;
    float closestDist = 9999999;
    for (auto it = resourceTiles.begin(); it != resourceTiles.end(); it++)
    {
        auto cell = *it;
        if (type != ResourceType::any && cell->resource.type != type)
            continue;
        if (cell->resource.type == ResourceType::coal && !player.researchedCoal())
            continue;
        if (cell->resource.type == ResourceType::uranium && !player.researchedUranium())
            continue;
        float dist = cell->pos.distanceTo(unit.pos);
        if (dist < closestDist)
        {
            closestDist = dist;
            closestResourceTile = cell;
        }
    }
    return closestResourceTile;
}

CityTile *GetClosestCityTile(Unit unit, City city)
{
    float closestDist = 999999;
    CityTile *closestCityTile;
    for (auto &citytile : city.citytiles)
    {
        float dist = citytile.pos.distanceTo(unit.pos);
        if (dist < closestDist)
        {
            closestCityTile = &citytile;
            closestDist = dist;
        }
    }
    return closestCityTile;
}

bool ShouldBuildCity(Unit unit)
{
    if ((unit.cargo.wood + unit.cargo.coal + unit.cargo.uranium) >= GAME_CONSTANTS["PARAMETERS"]["CITY_BUILD_COST"])
        return true;
    else
        return false;
}

bool IsNextToCityTile(Unit unit, CityTile cityTile)
{
    return unit.pos.distanceTo(cityTile.pos) == 1;
}

Cell *GetClosestEmptyTile(GameMap gameMap, Unit unit, Player player, bool nextToCityTile = false)
{
    Cell *closestEmptyTile;
    float closestDist = 9999999;

    for (int y = 0; y < gameMap.height; y++)
    {
        for (int x = 0; x < gameMap.width; x++)
        {
            Cell *cell = gameMap.getCell(x, y);
            if (cell->hasResource() || cell->citytile != nullptr)
            {
                continue;
            }
            if (nextToCityTile)
            {
                map<string, City>::iterator it;
                for (it = player.cities.begin(); it != player.cities.end(); it++)
                {
                    for (int i = 0; i < it->second.citytiles.size(); i++)
                    {
                        float dist = cell->pos.distanceTo(it->second.citytiles[i].pos);
                        if (dist < closestDist)
                        {
                            closestDist = dist;
                            closestEmptyTile = cell;
                        }
                    }
                }
            }
            else
            {
                float dist = cell->pos.distanceTo(unit.pos);
                if (dist < closestDist)
                {
                    closestDist = dist;
                    closestEmptyTile = cell;
                }
            }
        }
    }
    return closestEmptyTile;
}

vector<Position> getAllCityTilesPos(GameMap gameMap)
{
    vector<Position> tiles = vector<Position>();

    for (int y = 0; y < gameMap.height; y++)
    {
        for (int x = 0; x < gameMap.width; x++)
        {
            Cell *cell = gameMap.getCell(x, y);
            if (cell->citytile != nullptr)
            {
                tiles.push_back(cell->pos);
            }
        }
    }
    return tiles;
}

template <typename T>
vector<T> MergeVecs(vector<T> vec1, vector<T> vec2)
{
    vector<T> mergedVec;
    mergedVec.reserve(vec1.size() + vec2.size());
    mergedVec.insert(mergedVec.end(), vec1.begin(), vec1.end());
    mergedVec.insert(mergedVec.end(), vec2.begin(), vec2.end());
    return mergedVec;
}