#ifndef ACTOR_H_
#define ACTOR_H_
#include "GraphObject.h"
#include "GameConstants.h"
#include "StudentWorld.h"
//class StudentWorld; // can't do #include "StudentWorld.h" bc student world has Actor.h

class Actor : public GraphObject {
public:
	Actor(StudentWorld* world, int imageID, int startX, int startY, int startDirection, int depth, double size = 1) : // startDirec = sprite dire
		GraphObject(imageID, startX, startY, startDirection, depth, size),
		m_world(world) 
	{
		active = true;
	}
	virtual void doSomething() = 0;
	StudentWorld* getWorld() { // get_ptr_to_student_world() 
		return m_world;
	}
	bool getActive() const { // is game object still alive or should it be removed from game?
		return active;
	}
	void setActive(bool stat) {
		active = stat;
	}
	virtual bool is_a_square() const = 0;
	virtual bool can_be_hit_by_vortex() const = 0;  
	virtual void hit_by_vortex(){}  // tell an game object it has been hit by a vortex

private:
	StudentWorld* m_world;
	bool active; // inactive = false
};

// Movable CLASSES
class Movable : public Actor {
public:
	Movable(StudentWorld* world, int imageID, int startX, int startY, int startDirection, int depth = 0, double size = 1) :
		Actor(world, imageID, startX, startY, startDirection, depth, size)
	{
		ticks_to_move = 0;
		waiting = true;
		dir = right;
	}
	virtual void doSomething() = 0;
	int getTicksToMove() { 
		return ticks_to_move; 
	}
	
	void setTicksToMove(int ticks) {
		ticks_to_move = ticks;
	}
	void setWaiting(bool val) {
		waiting = val;
	}
	bool isWaiting() const { //bool is_walking() const;
		return waiting;
	}
	int getDir() {
		return dir;
	}
	void setDir(int d) { 
		dir = d;
	}

	bool isDirectionValid(int dir);
	void chooseNewDir();
	bool directlyOnTopOfSquare(int x, int y);
	bool atFork(int x, int y);

	int currentDieRoll() {
		return (ticks_to_move + 7) / 8;
	}
	int getRandomValidDirection();
	void teleport_me_to_random_sq();
	//bool can_be_hit_by_vortex() const {}
	virtual bool is_a_square() const {
		return false;
	}
	
private:
	int ticks_to_move;
	bool waiting; // walking is when waiting_to_roll = false
	int dir; // walking direction
};


class Player : public Movable {
public:
	Player(StudentWorld* world, int imageID, int startX, int startY, int startDirection, int depth, double size, int num) :
	Movable(world, imageID, startX, startY, startDirection, depth, size)
	{
		playerNum = num;
		stars = 0;
		coins = 0;
		vortex = false;
		newNess = true;
		dieRoll = 0;
		forced = false;
	}
	virtual void doSomething();

	int getPlayerNum() {
		return playerNum;
	}
	
	void reset_coins() {
		coins = 0;
	  }
	void reset_stars() {
		stars = 0;
	}
	bool is_a_square() const {
		return false;
	}
	bool can_be_hit_by_vortex() const {
		return false;
	}

	int get_stars() const {
		return stars;
	}   
	int get_coins() const {
		return coins;
	}

	void setCoins(int amt) {
		coins = amt;
	}
	void setStars(int amt) {
		stars = amt;
	}

	void addCoins(int amt) {
		coins += amt;
	}
	void addStars(int amt) {
		stars+= amt;
	}

	int deductCoins(int amt) { // return amtDeducted
		if (coins < amt) {
			int temp = coins;
			coins = 0;
			return temp;
		}
		else
			coins -= amt;
		return amt;
	}

	int deductStars(int amt) { // return amtDeducted
		if (stars < amt) {
			coins = 0;
			return stars;
		}
		else
			stars -= amt;
		return amt;
	}
	bool has_vortex() const {
		return vortex;
	}
	void setVortex(bool val) {
		vortex = val;
	}

	void force_walk_direction(int dir) {
		setDir(dir);
		forced = true;
	}	

	bool isDirectionForced() {
		return forced;
	}
	void resetDirectionForced(bool val) {
		forced = val;
	}
	void equip_with_vortex_projectile(); // IMPLEMENT
	//void teleport_me_to_random_sq(); 
	void swapPlayer(); 
	void swap_positions();
	void swap_stars() {  // with other player
		int thisS = get_stars();
		setStars(getWorld()->get_other_player(this)->get_stars());
		getWorld()->get_other_player(this)->setStars(thisS);
	}
	void swap_coins() {  // with other player
		//std::cerr << "in swap coins" << std::endl;
		int thisS = get_coins();
		int otherCoins = getWorld()->get_other_player(this)->get_coins();
		setCoins(otherCoins);
		getWorld()->get_other_player(this)->setCoins(thisS);
	}

	bool getNewNess() const {
		return newNess;
	}
	 
	void setNewNess(bool val) {
		newNess = val;
	}

private:
	int playerNum; //keep track player 1 or 2
	int stars;
	int coins;
	bool vortex;
	int dieRoll;
	bool newNess;
	bool forced;
};

class Enemy : public Movable {
public:
	Enemy(StudentWorld* sw, int imageID, int startX, int startY,
		int dir, int depth, double size, bool activate_when_go_lands, int number_of_ticks_to_pause) :
		Movable(sw, imageID, startX, startY, 0, 0, 1.0) {
		squares_to_move = 0;
		pauseCounter = 180;
		walking = false;
		peachNew = true;
		yoshiNew = true;
	}
	void doSomething();
	virtual void doActivity1(int playNum) = 0;
	virtual void doActivity2() = 0;
	virtual void doActivity3() = 0;
	virtual bool is_a_square() const {
		return false;
	}
	virtual bool can_be_hit_by_vortex() const {
		return true;
	}
	virtual void hit_by_vortex();  // called when enemy is hit by a vortex projectile (called by vortex projectile)

	int getSquaresToMove() {
		return squares_to_move;
	}

	void setSquaresToMove(int sqrs) {
		squares_to_move = sqrs;
	}

	int getPauseCounter() {
		return pauseCounter;
	}

	void setPauseCounter(int cntr) {
		pauseCounter = cntr;
	}
	bool getWalking() const {
		return walking;
	}
	void setWalking(bool val) {
		walking = val;
	}

	bool getPeachNew() const {
		return peachNew;
	}
	bool getYoshiNew() const {
		return yoshiNew;
	}
	void setPeachNew(bool val) {
		peachNew = val;
	}
	void setYoshiNew(bool val) {
		yoshiNew = val;
	}
private:
	int squares_to_move;
	int pauseCounter;
	bool walking; // not walking = paused
	bool peachNew;
	bool yoshiNew;
};

class Bowser : public Enemy {
public:
	Bowser(StudentWorld* sw, int imageID, int startX, int startY) : Enemy(sw, imageID, startX, startY,
		0, 0, 1.0, false, 180) {
	}
	void doActivity1(int playNum); // lose coins
	void doActivity2(); // finding squares to move
	void doActivity3(); // dropping square
	bool is_a_square() const {
		return false;
	}
}; 

class Boo : public Enemy {
public:
	Boo(StudentWorld* sw, int imageID, int startX, int startY) : Enemy(sw, imageID, startX, startY,
		0, 0, 1.0, false, 1) {} // NEED TO UPDATE
	void doActivity1(int playNum); // swap coins or stars
	void doActivity2(); // get squares to move
	void doActivity3(); // do nothing
	bool is_a_square() const {
		return false;
	}
};


// Activable Object classes
//class ActivatingObject : public Actor {
//public:
//	ActivatingObject(StudentWorld* sw, int imageID, int startX, int startY, int dir, int depth, double size) : 
//		Actor(sw, imageID, startX, startY, dir, depth, size) {} //here DIR represents sprite dir!
//	virtual void doSomething();
//};

class ActivateOnPlayer : public Actor {
public:
	ActivateOnPlayer(StudentWorld* sw, int imageID, int startX, int startY, int dir, int depth, double size) :
		Actor(sw, imageID, startX, startY, dir, depth, size){
		//peachNew = true;
		//yoshiNew = true;
	}
	virtual void doSomething();
	virtual void doActivity(int num) = 0;
	/*bool setPeachNew(bool val) {
		peachNew = val;
	}
	bool setYoshiNew(bool val) {
		yoshiNew = val;
	}*/
	//bool can_be_hit_by_vortex() const;

//private:
//	bool peachNew;
//	bool yoshiNew;
};


class Vortex : public Movable {
public:
	Vortex(StudentWorld* sw, int imageID, int startX, int startY, int dir) :
		Movable(sw, IID_VORTEX, startX, startY, dir, 0, 1.0){
		setDir(dir);
	}
	bool is_a_square() const {
		return false;
	}
	bool can_be_hit_by_vortex() const {
		return false;
	}
	void doSomething();
};

class StarSquare : public ActivateOnPlayer {
public:
	StarSquare(StudentWorld* sw, int imageID, int startX, int startY)
		:ActivateOnPlayer(sw, imageID, startX, startY, 0, 1, 1.0) {}
	bool is_a_square() const {
		return true;
	}
	bool can_be_hit_by_vortex() const {
		return false;
	}
	virtual void doActivity(int num);
};

class CoinSquare : public ActivateOnPlayer {
public:
	CoinSquare(StudentWorld* sw, int imageID, int startX, int startY, bool b)
		:ActivateOnPlayer(sw, imageID, startX, startY, 0, 1, 1.0){
		blue = b;
	} 
	bool is_a_square() const {
		return true;
	}
	bool can_be_hit_by_vortex() const {
		return false;
	}
	virtual void doActivity(int num);

	bool isRed() const {
		return !blue;
	}
	bool isBlue() const {
		return blue;
	}
private:
	bool blue; // if red, false
};

class DirectionalSquare : public ActivateOnPlayer {
public:
	DirectionalSquare(StudentWorld* sw, int imageID, int startX, int startY, int dir)
		:ActivateOnPlayer(sw, imageID, startX, startY, dir, 1, 1.0) {
		setDirection(dir);
		forceDir = dir;
	}
	bool is_a_square() const {
		return true;
	}
	bool can_be_hit_by_vortex() const {
		return false;
	}
	virtual void doActivity(int num);
private:
	int forceDir;
};

class BankSquare : public ActivateOnPlayer {
public:
	BankSquare(StudentWorld* sw, int imageID, int startX, int startY)
		:ActivateOnPlayer(sw, imageID, startX, startY, 0, 1, 1.0) {}
	bool is_a_square() const {
		return true;
	}
	bool can_be_hit_by_vortex() const {
		return false;
	}
	virtual void doActivity(int num);
};

class EventSquare : public ActivateOnPlayer {
public:
	EventSquare(StudentWorld* sw, int imageID, int startX, int startY)
		:ActivateOnPlayer(sw, imageID, startX, startY, 0, 1, 1.0) {}
	bool is_a_square() const {
		return true;
	}
	bool can_be_hit_by_vortex() const {
		return false;
	}
	virtual void doActivity(int num);
};


class DroppingSquare : public ActivateOnPlayer {
public:
	DroppingSquare(StudentWorld* sw, int imageID, int startX, int startY)
		:ActivateOnPlayer(sw, imageID, startX, startY, 0, 1, 1.0) {}
	bool is_a_square() const {
		return true;
	}
	bool can_be_hit_by_vortex() const {
		return false;
	}
	virtual void doActivity(int num);
};


#endif // ACTOR_H_
