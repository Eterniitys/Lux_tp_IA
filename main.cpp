#include "lux/kit.hpp"
#include "lux/define.cpp"
#include <string.h>
#include <vector>
#include <set>
#include <stdio.h>

using namespace std;
using namespace lux;

void WriteLog(string log, bool reset = false)
{
  FILE *pFile;
  if (reset)
    pFile = fopen("logFile.txt", "w");
  else
    pFile = fopen("logFile.txt", "a");
  fprintf(pFile, "%s\n", log.c_str());
  fclose(pFile);
}

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

vector<Position> setPosToAvoid(GameMap gameMap, Player player, bool avoidCity = false, bool avoidUnit = false)
{
  vector<Position> toAvoid = vector<Position>();
  if (avoidCity)
  {
    for (int y = 0; y < gameMap.height; y++)
    {
      for (int x = 0; x < gameMap.width; x++)
      {
        Cell *cell = gameMap.getCell(x, y);
        if (cell->citytile != nullptr)
        {
          toAvoid.push_back(cell->pos);
        }
      }
    }
  }
  if (avoidUnit)
  {
    for (int i = 0; i < player.units.size(); i++)
    {
      toAvoid.push_back(player.units[i].pos);
    }
  }
  return toAvoid;
}

// # # # # # # # # # # # # # # # # # # # #
// Game functions please code above me =)
// # # # # # # # # # # # # # # # # # # # #

void ActOnDay(kit::Agent &gameState, vector<string> &actions)
{
  Player &player = gameState.players[gameState.id];
  Player &opponent = gameState.players[(gameState.id + 1) % 2];

  GameMap &gameMap = gameState.map;

  vector<Cell *> resourceTiles = vector<Cell *>();
  resourceTiles = GetResourceTiles(gameMap);

  map<string, City>::iterator it;
  for (it = player.cities.begin(); it != player.cities.end(); it++)
  {
    for (int i = 0; i < it->second.citytiles.size(); i++)
    {
      if (it->second.citytiles[i].canAct())
      {
        if (it->second.citytiles.size() < player.units.size())
          actions.push_back(it->second.citytiles[i].research());
        else
          actions.push_back(it->second.citytiles[i].buildWorker());
      }
    }
  }
  vector<Position> vec = setPosToAvoid(gameMap, player, true, true);
  for (size_t i = 0; i < vec.size(); i++)
  {
    WriteLog("p" + to_string(player.team) + " " + to_string(vec[i].x) + "_" + to_string(vec[i].y));
  }

  for (int i = 0; i < player.units.size(); i++)
  {
    // WriteLog("going through unit " + i);
    Unit unit = player.units[i];
    if (unit.isWorker() && unit.canAct())
    {
      if (unit.getCargoSpaceLeft() > 0)
      {
        // if the unit is a worker and we have space in cargo, lets find the nearest resource tile and try to mine it
        Cell *closestResourceTile;
        if (player.researchedCoal())
          closestResourceTile = GetClosestResource(unit, resourceTiles, player, ResourceType::coal);
        else
          closestResourceTile = GetClosestResource(unit, resourceTiles, player);

        if (closestResourceTile != nullptr)
        {
          auto dir = unit.pos.directionTo(closestResourceTile->pos);
          actions.push_back(unit.move(dir));
        }
      }
      else
      {
        if (player.cities.size() > 0)
        {
          auto city_iter = player.cities.begin();
          auto &city = city_iter->second;

          CityTile *closestCityTile;
          closestCityTile = GetClosestCityTile(unit, city);
          if (closestCityTile != nullptr)
          {
            if (i == 0 && ShouldBuildCity(unit) && city.fuel > 500)
            {
              Cell *buildLocation = GetClosestEmptyTile(gameMap, unit, player, true);
              WriteLog("p" + to_string(player.team) + " build to" + to_string(buildLocation->pos.x) + "_" + to_string(buildLocation->pos.y));
              if (unit.canBuild(gameMap) && buildLocation->pos == unit.pos)
              {
                actions.push_back(unit.buildCity());
              }
              else
              {
                auto dir = unit.pos.directionTo(buildLocation->pos, setPosToAvoid(gameMap, player, true, false));
                actions.push_back(unit.move(dir));
              }
            }
            else
            {

              auto dir = unit.pos.directionTo(closestCityTile->pos);
              actions.push_back(unit.move(dir));
            }
          }
        }
      }
    }
  }
}

void ActOnDawn(kit::Agent &gameState, vector<string> &actions)
{
  ActOnDay(gameState, actions);
}

void ActOnNight(kit::Agent &gameState, vector<string> &actions)
{
  ActOnDay(gameState, actions);
}

// # # # # # # # # # # # # # # # # # # # # # # #

int main()
{
  kit::Agent gameState = kit::Agent();
  // initialize
  gameState.initialize();
  WriteLog("newGame", true);
  while (true)
  {
    /** Do not edit! **/
    // wait for updates
    gameState.update();

    vector<string> actions = vector<string>();

    /** AI Code Goes Below! **/
    WriteLog("# # # # # # # # # # # # # # # # # # # # ");
    WriteLog("p" + to_string(gameState.id) + " " + "Turn " + to_string(gameState.turn));

    if (gameState.turn % 40 <= 25)
    {
      ActOnDay(gameState, actions);
    }
    else if (gameState.turn % 40 < 30)
    {
      ActOnDawn(gameState, actions);
    }
    else
    {
      ActOnNight(gameState, actions);
    }

    // you can add debug annotations using the methods of the Annotate class.
    // actions.push_back(Annotate::circle(0, 0));

    /** AI Code Goes Above! **/

    /** Do not edit! **/
    for (int i = 0; i < actions.size(); i++)
    {
      if (i != 0)
        cout << ",";
      cout << actions[i];
    }
    cout << endl;
    // end turn
    gameState.end_turn();
  }

  return 0;
}
