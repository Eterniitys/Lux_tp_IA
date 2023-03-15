#include "lux/kit.hpp"
#include "lux/define.cpp"
#include "lib/tools.cpp"
#include "lib/components.cpp"
#include <string.h>
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
        // if (it->second.citytiles.size() <= player.units.size())
          actions.push_back(it->second.citytiles[i].research());
        // else
        //   actions.push_back(it->second.citytiles[i].buildWorker());
      }
    }
  }

  vector<Position> allCityTiles = getAllCityTilesPos(gameMap);
  vector<Position> unitsNextPos = vector<Position>();
  if (gameState.id == 1)
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
    Unit unit = player.units[i];
    if (unit.isWorker() && unit.canAct())
    {
      if (unit.getCargoSpaceLeft() > 0)
      {
        // if the unit is a worker and we have space in cargo, lets find the nearest resource tile and try to mine it
        Cell *closestResourceTile;
        vector<Cell *> resourceTiles = vector<Cell *>();
        resourceTiles = GetResourceTiles(gameMap);
        // if (player.researchedCoal())
        //   closestResourceTile = GetClosestResource(unit, resourceTiles, player, ResourceType::coal);
        // else if (player.researchedUranium() && i > 7)
        //   closestResourceTile = GetClosestResource(unit, resourceTiles, player, ResourceType::uranium);
        // else
        closestResourceTile = GetClosestResource(unit, resourceTiles, player);

        if (closestResourceTile != nullptr)
        {
          if (gameState.id == 1)
            WriteLog(unit.id + " collect ressource -> " + (string)closestResourceTile->pos);
          vector<Position> toAvoid;
          for (auto var : player.units)
            toAvoid.push_back(var.pos);
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
            if (gameState.id == 1)
              WriteLog(ShouldBuildCity(unit, city) ? "true" : "false");
            if (i == 0 && ShouldBuildCity(unit, city))
            {
              Cell *buildLocation = GetBuildTile(gameMap, unit, city);
              if (gameState.id == 1)
                WriteLog("Want build here " + (string)buildLocation->pos);

              if (unit.canBuild(gameMap) && buildLocation->pos == unit.pos)
              {
                actions.push_back(unit.buildCity());
              }
              else
              {

                // auto nextPos = unit.moveTo(buildLocation->pos, MergeVecs(allCityTiles, unitsNextPos));
                // unitsNextPos.push_back(nextPos);
                Position diff = Position(buildLocation->pos.x - unit.pos.x, buildLocation->pos.y - unit.pos.y);

                // x < 0 ; west
                // x > 0 ; east
                // y < 0 ; north
                // y > 0 ; south
                string log = "diff ->" + (string)diff;
                if (abs(diff.x) < abs(diff.y))
                {
                  int sign = diff.y > 0 ? 1 : -1;
                  log += " s" + to_string(sign);
                  Cell *cell = gameMap.getCellByPos(Position(unit.pos.x, unit.pos.y + sign));
                  if (cell->citytile == nullptr)
                  {
                    if (sign == 1)
                    {
                      // south
                      actions.push_back(unit.move(DIRECTIONS::SOUTH));
                      log += "-> South";
                    }
                    else
                    {
                      // north
                      actions.push_back(unit.move(DIRECTIONS::NORTH));
                      log += "-> North";
                    }
                  }
                  else
                  {
                    sign = diff.x > 0 ? 1 : -1;
                    log += " s" + to_string(sign);

                    if (sign == 1)
                    {
                      // east
                      actions.push_back(unit.move(DIRECTIONS::EAST));
                      log += "-> East";
                    }
                    else
                    {
                      // west
                      actions.push_back(unit.move(DIRECTIONS::WEST));
                      log += "-> West";
                    }
                  }
                }
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
                      actions.push_back(unit.move(EAST));
                      log += "-> East";
                    }
                    else
                    {
                      // west
                      actions.push_back(unit.move(WEST));
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
                      actions.push_back(unit.move(SOUTH));
                      log += "-> South";
                    }
                    else
                    {
                      // north
                      actions.push_back(unit.move(NORTH));
                      log += "-> North";
                    }
                  }
                }
                if (gameState.id == 1)
                  WriteLog(log);
              }
            }
            else
            {
              vector<Position> toAvoid;
              for (auto var : player.units)
                toAvoid.push_back(var.pos);
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
    if (gameState.id == 1)
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
