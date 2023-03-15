#include "lux/kit.hpp"
#include "lux/define.cpp"
#include "lib/tools.cpp"
#include "lib/components.cpp"
#include <string.h>
#include <vector>
#include <set>
#include <stdio.h>

using namespace std;
using namespace lux;

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
  vector<Position> allCityTiles = getAllCityTilesPos(gameMap);
  vector<Position> unitsNextPos = vector<Position>();
  if (gameState.id == 0)
  {
    string ch = "P" + to_string(gameState.id);
    for (size_t i = 0; i < allCityTiles.size(); i++)
    {
      ch += " " + (string)allCityTiles[i];
    }
    WriteLog(ch);
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
          actions.push_back(unit.moveTo(closestResourceTile->pos));
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
              if (gameState.id == 0)
                WriteLog("p" + to_string(player.team) + " build to" + (string)buildLocation->pos);
              if (unit.canBuild(gameMap) && buildLocation->pos == unit.pos)
              {
                actions.push_back(unit.buildCity());
              }
              else
              {
                actions.push_back(unit.moveTo(buildLocation->pos, MergeVecs(allCityTiles, unitsNextPos)));
              }
            }
            else
            {
              actions.push_back(unit.moveTo(closestCityTile->pos));
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
    if (gameState.id == 0)
    {

      WriteLog("# # # # # # # # # # # # # # # # # # # # ");
      WriteLog("p" + to_string(gameState.id) + " " + "Turn " + to_string(gameState.turn));
    }

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
