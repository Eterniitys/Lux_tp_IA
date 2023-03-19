#include "lux/kit.hpp"
#include "lux/define.cpp"
#include "lib/tools.cpp"
#include "lib/components.cpp"
#include <string.h>
#include <set>
#include <stdio.h>

using namespace std;
using namespace lux;

void ActOnDay(kit::Agent &gameState, vector<string> &actions, bool actOnDawn = false, bool actOnNight = false)
{
  Player &player = gameState.players[gameState.id];
  Player &opponent = gameState.players[(gameState.id + 1) % 2];

  GameMap &gameMap = gameState.map;

  vector<Cell *> resourceTiles = vector<Cell *>();
  resourceTiles = GetResourceTiles(gameMap);

  map<string, City>::iterator it;

  // We iterate in each city to research or create workers when certain conditions are met :
  for (it = player.cities.begin(); it != player.cities.end(); it++)
  {
    for (int i = 0; i < it->second.citytiles.size(); i++)
    {
      if (it->second.citytiles[i].canAct())
      {
        if (it->second.citytiles.size() <= player.units.size())
          actions.push_back(it->second.citytiles[i].research());
        else
        {
          if (player.units.size() <= 0)
          {
            actions.push_back(it->second.citytiles[i].buildWorker());
          }
          else if (player.researchedCoal() && player.units.size() < 3)
          {
            actions.push_back(it->second.citytiles[i].buildWorker());
          }
          else if (player.researchedUranium() && player.units.size() < 4)
          {
            actions.push_back(it->second.citytiles[i].buildWorker());
          }
          else
          {
            actions.push_back(it->second.citytiles[i].research());
          }
        }
      }
    }
  }

  vector<Position> allCityTiles = getAllCityTilesPos(gameMap);
  vector<Position> unitsNextPos = vector<Position>();

  {
    string ch = "P" + to_string(gameState.id);
    for (size_t i = 0; i < allCityTiles.size(); i++)
    {
      ch += " " + (string)allCityTiles[i];
    }
  }

  for (int i = 0; i < player.units.size(); i++)
  {
    Unit unit = player.units[i];
    if (unit.isWorker() && unit.canAct())
    {
      if (unit.getCargoSpaceLeft() > 0)
      {
        // if the unit is a worker and we have space in cargo, lets find the nearest resource tile and try to mine it
        Cell *closestResourceTile;
        vector<Cell *> resourceTiles = vector<Cell *>();
        resourceTiles = GetResourceTiles(gameMap);
        if (i == 0)
          closestResourceTile = GetClosestResource(unit, resourceTiles, player, ResourceType::wood);
        else if (i == 1)
          closestResourceTile = GetClosestResource(unit, resourceTiles, player, ResourceType::coal);
        else if (i == 2)
          closestResourceTile = GetClosestResource(unit, resourceTiles, player, ResourceType::wood);
        else
          closestResourceTile = GetClosestResource(unit, resourceTiles, player, ResourceType::uranium);

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

          // Get the closest CityTile
          CityTile *closestCityTile;
          closestCityTile = GetClosestCityTile(unit, player);

          if (closestCityTile != nullptr)
          {

            // If the worker can build : 
            if (i == 0 && ShouldBuildCity(unit, player) && !actOnDawn && !actOnNight)
            {
              Cell *buildLocation = GetBuildTile(gameMap, unit, city, player);

              if (unit.canBuild(gameMap) && buildLocation->pos == unit.pos)
              {
                actions.push_back(unit.buildCity());
              }
              else
              {
                actions.push_back(GetBestPathTo(gameMap, unit, buildLocation->pos));
              }
            }
            // else the unit go back to deposit its ressources to the closest city.
            else
            {
              CityTile *closestCityTile;
              closestCityTile = GetClosestCityTile(unit, player);

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

  while (true)
  {
    /** Do not edit! **/
    // wait for updates
    gameState.update();

    vector<string> actions = vector<string>();

    /** AI Code Goes Below! **/

    // Check the cycle we're in (day, dawn, night)
    if (gameState.turn % 40 <= 20)
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
