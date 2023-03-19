#ifndef components_cpp
#define components_cpp

#include <vector>

// Get all the ressources on the map.
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

// Get the closest (random or specific) ressource next to the unit.
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

CityTile *GetClosestCityTile(Unit unit, Player player)
{
    CityTile *closestCityTile = nullptr;
    float closestDist = 999999;
    float leastFuel = 999999;

    // Find the city with the least amount of fuel
    City *smallestCity = nullptr;
    for (auto &cityPair : player.cities)
    {
        if (cityPair.second.fuel < leastFuel)
        {
            smallestCity = &cityPair.second;
            leastFuel = cityPair.second.fuel;
        }
    }

    // If no cities have fuel, return null pointer
    if (smallestCity == nullptr)
    {
        return nullptr;
    }

    // Find the closest city tile in the city with the least amount of fuel
    for (auto &cityTile : smallestCity->citytiles)
    {
        float dist = cityTile.pos.distanceTo(unit.pos);
        if (dist < closestDist)
        {
            closestCityTile = &cityTile;
            closestDist = dist;
        }
    }

    return closestCityTile;
}

bool ShouldBuildCity(Unit unit, Player player)
{
    // if there is no city, the unit should build a city
    if (player.cities.size() <= 0)
        return true;
    else
    {
        // check if each city has enough fuel for the night cycle with a 150 margin.
        bool hasEnoughStock = true;
        for (const auto &cityPair : player.cities)
        {
            if (cityPair.second.fuel < (cityPair.second.getLightUpkeep() * 10) + 150)
            {
                hasEnoughStock = false;
            }
        }
        // check if the unit also has enough ressources on it.
        if ((unit.cargo.wood + unit.cargo.coal + unit.cargo.uranium) >= GAME_CONSTANTS["PARAMETERS"]["CITY_BUILD_COST"] && hasEnoughStock)
            return true;
        else
            return false;
    }
}

// Check if the CityTile is adjacent to the unit.
bool IsNextToCityTile(Unit unit, CityTile cityTile)
{
    return unit.pos.distanceTo(cityTile.pos) == 1;
}

// Return the closest empty cell next to the unit.
Cell *GetClosestEmptyTile(GameMap gameMap, Unit unit, Player player)
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
            float dist = cell->pos.distanceTo(unit.pos);
            if (dist < closestDist)
            {
                closestDist = dist;
                closestEmptyTile = cell;
            }
        }
    }
    return closestEmptyTile;
}

Cell *GetBuildTile(GameMap gameMap, Unit unit, City city, Player player)
{
    Cell *closestEmptyTile = nullptr;
    float closestDist = 9999999;

    // Find all cells adjacent to all city tiles in all cities that are empty and have no citytile
    vector<Cell *> emptyAdjacentCells;
    for (const auto &cityPair : player.cities)
    {
        for (const auto &cityTile : cityPair.second.citytiles)
        {
            for (const auto dir : ALL_DIRECTIONS)
            {
                Cell *cell = gameMap.getCellByPos(cityTile.pos.translate(dir, 1));
                if (!cell->hasResource() && !cell->citytile)
                {
                    emptyAdjacentCells.push_back(cell);
                }
            }
        }
    }

    // If there are any empty adjacent cells, find the closest one to the unit
    if (!emptyAdjacentCells.empty())
    {
        for (const auto &emptyAdjacentCell : emptyAdjacentCells)
        {
            float dist = emptyAdjacentCell->pos.distanceTo(unit.pos);
            if (dist < closestDist)
            {
                closestEmptyTile = emptyAdjacentCell;
                closestDist = dist;
            }
        }
    }
    // Otherwise, find the closest empty cell on the map to the unit
    else
    {
        closestEmptyTile = GetClosestEmptyTile(gameMap, unit, player);
    }

    return closestEmptyTile;
}

// Function to get the positions of all city tiles on the game map
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

// Function to merge two vectors of type T
template <typename T>
vector<T> MergeVecs(vector<T> vec1, vector<T> vec2)
{
    vector<T> mergedVec;
    mergedVec.reserve(vec1.size() + vec2.size());
    mergedVec.insert(mergedVec.end(), vec1.begin(), vec1.end());
    mergedVec.insert(mergedVec.end(), vec2.begin(), vec2.end());
    return mergedVec;
}

string GetBestPathTo(GameMap gameMap, Unit unit, Position targetPos)
{
    // Calculate the difference between the unit's current position and the target position.
    Position diff = Position(targetPos.x - unit.pos.x, targetPos.y - unit.pos.y);

    // x < 0 ; west
    // x > 0 ; east
    // y < 0 ; north
    // y > 0 ; south

    string log = "diff ->" + (string)diff;

    // If the difference in the x-coordinate is smaller than the difference in the y-coordinate:
    if (abs(diff.x) < abs(diff.y))
    {
        // calculate the sign of the difference in the y-coordinate.
        int sign = diff.y > 0 ? 1 : -1;
        log += " s" + to_string(sign);
        Cell *cell = gameMap.getCellByPos(Position(unit.pos.x, unit.pos.y + sign));
        if (cell->citytile == nullptr)
        {
            if (sign == 1)
            {
                // south
                return unit.move(DIRECTIONS::SOUTH);
                log += "-> South";
            }
            else
            {
                // north
                return unit.move(DIRECTIONS::NORTH);
                log += "-> North";
            }
        }
        else
        {
            // calculate the sign of the difference in the x-coordinate.
            sign = diff.x > 0 ? 1 : -1;
            log += " s" + to_string(sign);

            if (sign == 1)
            {
                // east
                return unit.move(DIRECTIONS::EAST);
                log += "-> East";
            }
            else
            {
                // west
                return unit.move(DIRECTIONS::WEST);
                log += "-> West";
            }
        }
    }
    // If the difference in the y-coordinate is smaller than the difference in the x-coordinate:
    else
    {
        int sign = diff.x > 0 ? 1 : -1;
        log += " s" + to_string(sign);
        Cell *cell = gameMap.getCellByPos(Position(unit.pos.x + sign, unit.pos.y));
        if (cell->citytile == nullptr)
        {
            if (sign == 1)
            {
                // east
                return unit.move(EAST);
                log += "-> East";
            }
            else
            {
                // west
                return unit.move(WEST);
                log += "-> West";
            }
        }
        else
        {
            sign = diff.y > 0 ? 1 : -1;
            log += " s" + to_string(sign);
            if (sign == 1)
            {
                // south
                return unit.move(SOUTH);
                log += "-> South";
            }
            else
            {
                // north
                return unit.move(NORTH);
                log += "-> North";
            }
        }
    }
}

#endif