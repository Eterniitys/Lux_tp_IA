#include "lux/kit.hpp"
#include "lux/define.cpp"
#include <string.h>
#include <vector>
#include <set>
#include <stdio.h>

using namespace std;
using namespace lux;

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

Cell *GetClosestResource(Unit unit, vector<Cell *> resourceTiles, Player player)
{
  Cell *closestResourceTile;
  float closestDist = 9999999;
  for (auto it = resourceTiles.begin(); it != resourceTiles.end(); it++)
  {
    auto cell = *it;
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

void ActOnDay(kit::Agent &gameState, vector<string> &actions)
{

  Player &player = gameState.players[gameState.id];
  Player &opponent = gameState.players[(gameState.id + 1) % 2];

  GameMap &gameMap = gameState.map;

  vector<Cell *> resourceTiles = vector<Cell *>();
  resourceTiles = GetResourceTiles(gameMap);

  for (int i = 0; i < player.units.size(); i++)
  {
    Unit unit = player.units[i];
    if (unit.isWorker() && unit.canAct())
    {
      if (unit.getCargoSpaceLeft() > 0)
      {
        // if the unit is a worker and we have space in cargo, lets find the nearest resource tile and try to mine it
        Cell *closestResourceTile;
        closestResourceTile = GetClosestResource(unit, resourceTiles, player);

        if (closestResourceTile != nullptr)
        {
          auto dir = unit.pos.directionTo(closestResourceTile->pos);
          actions.push_back(unit.move(dir));
        }
      }
      else
      {
        // if unit is a worker and there is no cargo space left, and we have cities, lets return to them
        if (player.cities.size() > 0)
        {
          auto city_iter = player.cities.begin();
          auto &city = city_iter->second;

          CityTile *closestCityTile;
          closestCityTile = GetClosestCityTile(unit, city);

          if (closestCityTile != nullptr)
          {
            auto dir = unit.pos.directionTo(closestCityTile->pos);
            actions.push_back(unit.move(dir));
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

int main()
{
  kit::Agent gameState = kit::Agent();
  // initialize
  gameState.initialize();

  while (true)
  {
    /** Do not edit! **/
    // wait for updates
    gameState.update();

    vector<string> actions = vector<string>();

    /** AI Code Goes Below! **/

    // Player &player = gameState.players[gameState.id];
    // Player &opponent = gameState.players[(gameState.id + 1) % 2];

    // GameMap &gameMap = gameState.map;

    // vector<Cell *> resourceTiles = vector<Cell *>();
    // resourceTiles = GetResourceTiles(gameMap);

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
