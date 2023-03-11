#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include <string>
#include "GameWorld.h"
#include "Board.h"
//#include "Actor.h"
#include <vector>

class GameObject;
class Actor;
class Player;

class StudentWorld : public GameWorld
{
public:
  StudentWorld(std::string assetPath);
  virtual ~StudentWorld(); // added
  virtual int init();
  virtual int move();
  virtual void cleanUp();
  bool isEmpty(double x, double y); // added
  Board getBoard() { // added
	  return bd;
  }

  // Given a player object pointer, returns a pointer to the other player object. Used for swapping
  // actions.
  Player* get_other_player(Player* p) const {
	  if (p == m_peach)
		  return m_yoshi;
	  else
		  return m_peach;
  }
  
  Player* determineWinner();
  
  Player* getPeach() const {
	  return m_peach;
  }
  Player* getYoshi() const {
	  return m_yoshi;
  }

  // Bank Square Functions
  int getBalance() const {
	  return balance;
  }
  void setBalance(int amt) {
	  balance = amt;
  }
  void addBalance(int amt) {
	  balance += amt;
  }
  void withdrawBalance(int amt) {
	  if (balance < amt)
		  balance = 0;
	  else
		  balance -= amt;
  }
  void updateStatus();

  void replaceWithDroppingSquare(Actor* dropSquare);

  // Vortex Functions
  void addVortex(Actor* vortex);
  bool doesVortexOverlap(Actor* vortex, Actor* obj);
  bool objectOverlappingWithVortexHIT(Actor* vortex);
  
private:
	std::vector<Actor*> m_actors;
	Player* m_peach;
	Player* m_yoshi;
	Board bd;
	int balance;
};

#endif // STUDENTWORLD_H_
