#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include <iostream>
using namespace std;

// Movable 
bool Movable::isDirectionValid(int dir) {
	int xnew, ynew; 
	getPositionInThisDirection(dir, SPRITE_WIDTH, xnew, ynew); // update walk dir
	if (getWorld()->isEmpty(xnew, ynew))
		return false;
	return true;
}

int Movable::getRandomValidDirection() {
	int dir;
	do {
		dir = randInt(0, 3) * 90;
	} while (!isDirectionValid(dir));
	
	return dir;
}

void Movable::chooseNewDir() {
	/*if (getDir() == left) {
		xnew += SPRITE_WIDTH - 1;
	}
	if (getDir() == down) {
		ynew += SPRITE_WIDTH - 1;
	}*/
	
	if (getDir() == right || getDir() == left) { // move up or down
		int xup, yup; // up case 
		int xdown, ydown; // down case
		getPositionInThisDirection(up, SPRITE_WIDTH, xup, yup);
		getPositionInThisDirection(down, SPRITE_WIDTH, xdown, ydown);
		if (!getWorld()->isEmpty(xup, yup)) {
			setDir(up);
			setDirection(0); // update sprite dir
		}
		else if (!getWorld()->isEmpty(xdown, ydown)) {
			setDir(down);
			setDirection(0); // update sprite dir
		}
	}
	else if (getDir() == up || getDir() == down) { // move left or right
		int xright, yright; // right case 
		int xleft, yleft; // left case
		getPositionInThisDirection(right, SPRITE_WIDTH, xright, yright);
		getPositionInThisDirection(left, SPRITE_WIDTH, xleft, yleft);
		if (!getWorld()->isEmpty(xright, yright)) {
			setDir(right);
			setDirection(0); // update sprite dir
		}

		else if (!getWorld()->isEmpty(xleft, yleft)) {
			setDir(left);
			setDirection(180); // update sprite dir
		}
	}
}

bool Movable::directlyOnTopOfSquare(int x, int y) {
	if (!getWorld()->isEmpty(x, y) && x % 16 == 0 && y % 16 == 0)
		return true;
	return false;
}

bool Movable::atFork(int x, int y) {
	int count = 0;
	for (int i = 0; i < 4; i++) {
		int x, y;
		getPositionInThisDirection(90*i, SPRITE_WIDTH, x, y);
		if (!getWorld()->isEmpty(x, y))
			count++;
	}
	//cout << "count: " << count << endl;
	return count >= 3;
}

void Movable::teleport_me_to_random_sq() {
	// finding random coordinates
	//cerr << "In teleport" << endl;
	int x, y;
	do {
		x = randInt(0, 15) * 16;
		y = randInt(0, 15) * 16;
	} while (!directlyOnTopOfSquare(x, y));
	//cerr << "out of while" << endl;
	moveTo(x, y);
}

// Player implementations
void Player::doSomething() {
	if (isWaiting()) { // waiting to roll state
		if (!isDirectionValid(getDir())) { // taking care of invalid dir when teleported
			chooseNewDir();
		}
		int act = getWorld()->getAction(getPlayerNum());
		if (act != ACTION_NONE) { // check if user pressed key
			if (act == ACTION_ROLL) {
				dieRoll = randInt(1, 10);
				setTicksToMove(dieRoll * 8);
				setWaiting(false);
			}
			if (act == ACTION_FIRE && vortex) {
				equip_with_vortex_projectile();
				getWorld()->playSound(SOUND_PLAYER_FIRE);
				setVortex(false);
			}
		}
		// if Action Fire
		else if (act == ACTION_NONE /*|| (act != ACTION_ROLL && act != ACTION_FIRE)*/)
			return;
	}
	if (!isWaiting()) { // in walking state
		
		if (atFork(getX(), getY()) && !isDirectionForced() && directlyOnTopOfSquare(getX(), getY())) {
			/*cerr << "isAtFork" << endl;*/
			// check if user pressed key
			int act = getWorld()->getAction(getPlayerNum());

			if (act != ACTION_NONE) {
				if (act == ACTION_RIGHT && isDirectionValid(right)) {
					if (getDir() == left)
						return;
					setDir(right);
					setDirection(0);
				}
				if (act == ACTION_LEFT && isDirectionValid(left)) {
					if (getDir() == right)
						return;
					setDir(left);
					setDirection(180);
				}
				if (act == ACTION_UP && isDirectionValid(up)) {
					if (getDir() == down)
						return;
					setDir(up);
					setDirection(0);
				}
				if (act == ACTION_DOWN && isDirectionValid(down)) {
					if (getDir() == up)
						return;
					setDir(down);
					setDirection(0);
				}
			}
			else if (act == ACTION_NONE || (act != ACTION_RIGHT && act != ACTION_LEFT
				&& act != ACTION_DOWN && act != ACTION_UP))
				return;
		}
		

		// player can't move in current direction 2C
		int xnew, ynew;
		getPositionInThisDirection(getDir(), SPRITE_WIDTH, xnew, ynew); // update walk dir
		
		if (getWorld()->isEmpty(xnew, ynew) && xnew % 16 == 0 && ynew % 16 == 0) 
			chooseNewDir(); 

		moveAtAngle(getDir(), 2);
		setTicksToMove(getTicksToMove() - 1);
		// when walking, set Newness = true
		setNewNess(true);
		// when walking, no longer forced direction
		resetDirectionForced(false);

		if (getTicksToMove() == 0)
			setWaiting(true);
		return;
	}
}


void Player::equip_with_vortex_projectile() {
	int xnew, ynew;
	getPositionInThisDirection(getDir(), 16, xnew, ynew);
	Actor* votex = new Vortex(getWorld(), IID_VORTEX, xnew, ynew, getDir());
	getWorld()->addVortex(votex);
}

void Player::swapPlayer() {
	swap_positions();
	// swap ticks
	int thisTicks = getTicksToMove();
	setTicksToMove(getWorld()->get_other_player(this)->getTicksToMove());
	getWorld()->get_other_player(this)->setTicksToMove(thisTicks);
	// swap dir
	int dir = getDir();
	setDir(getWorld()->get_other_player(this)->getDir()); // walking dir
	getWorld()->get_other_player(this)->setDir(dir);
	// swap sprite dir
	int spritedir = getDirection();
	setDirection(getWorld()->get_other_player(this)->getDirection()); // sprite dir
	getWorld()->get_other_player(this)->setDirection(spritedir);
	// swap walk state
	bool waiting = isWaiting();
	setWaiting(getWorld()->get_other_player(this)->isWaiting());
	getWorld()->get_other_player(this)->setWaiting(waiting);
}

void Player::swap_positions() {  // with other player
	int thisX = getX();
	int thisY = getY();
	// changes for THIS player
	int x = getWorld()->get_other_player(this)->getX();
	int y = getWorld()->get_other_player(this)->getY();
	moveTo(x, y);
	//changes for OTHER player
	getWorld()->get_other_player(this)->moveTo(thisX, thisY);
}

// Enemy
void Enemy::doSomething() {
	//cerr << getWorld()->getPeach()->getNewNess() << endl;
	if (!getWalking()) {
		if (getWorld()->getPeach()->getX() == getX() // checking for Peach
			&& getWorld()->getPeach()->getY() == getY() // if they are on the same square
			&& getWorld()->getPeach()->isWaiting() 
			&& getPeachNew()) {
			doActivity1(1); // Boo vs. Bowser
			setPeachNew(false);
		}
		if (getWorld()->getYoshi()->getX() == getX() // checking for Yoshi ^
			&& getWorld()->getYoshi()->getY() == getY()
			&& getWorld()->getYoshi()->isWaiting() && getYoshiNew()) {
			doActivity1(2);
			setYoshiNew(false);
		}
		setPauseCounter(getPauseCounter() - 1); // decrement pause counter

		if (getPauseCounter() == 0) {
			doActivity2(); // num doesn't matter here
			setTicksToMove(getSquaresToMove() * 8);
			int dir = getRandomValidDirection();
			setDir(dir);
			if (dir == left)
				setDirection(180);
			else
				setDirection(0);
			setWalking(true);
		}
	}
	if (getWalking()) {
		if (atFork(getX(), getY()) && directlyOnTopOfSquare(getX(),getY())) {
			int dir = getRandomValidDirection();
			setDir(dir);
			if (dir == left)
				setDirection(180);
			else
				setDirection(0);
		}
		else if (directlyOnTopOfSquare(getX(), getY()) && !isDirectionValid(getDir())) {
			chooseNewDir();
		}

		moveAtAngle(getDir(), 2);
		setPeachNew(true);
		setYoshiNew(true);
		setTicksToMove(getTicksToMove() - 1);
		if (getTicksToMove() == 0) {
			setWalking(false);
			setPauseCounter(180);
			doActivity3();
		}
	}
}

void Enemy::hit_by_vortex() {
	teleport_me_to_random_sq();
	setDir(right);
	setDirection(0);
	setWalking(false);
	setPauseCounter(180);
}

void Bowser::doActivity1(int playNum) {
	if (playNum == 1) { // peach
		int chance = randInt(0, 1); // 50% player loses all the coins + stars
		if (chance) {
			getWorld()->getPeach()->reset_coins();
			getWorld()->getPeach()->reset_stars();
			getWorld()->playSound(SOUND_BOWSER_ACTIVATE);
		}
	}
	if (playNum == 2) { // Yoshi
		int chance = randInt(0, 1);
		if (chance) {
			getWorld()->getYoshi()->reset_coins();
			getWorld()->getYoshi()->reset_stars();
			getWorld()->playSound(SOUND_BOWSER_ACTIVATE);
		}
	}
}
void Bowser::doActivity2() {
	setSquaresToMove(randInt(1, 10));
}

void Bowser::doActivity3() {
	int chance = randInt(1, 4);
	if (chance == 1) { // 25% chance
		Actor* dropSquare = new DroppingSquare(getWorld(), IID_DROPPING_SQUARE, getX(), getY());
		getWorld()->replaceWithDroppingSquare(dropSquare);
		getWorld()->playSound(SOUND_DROPPING_SQUARE_CREATED);
	}
}

void Boo::doActivity1(int playNum) {
	//cerr << "in Activity 1" << endl;
	if (playNum == 1) { // peach
		int chance = randInt(0, 1); 
		if (chance) {
			//cerr << "swap coins" << endl;
			getWorld()->getPeach()->swap_coins();
		}
		else {
			//cerr << "swap stars" << endl;
			getWorld()->getPeach()->swap_stars();
		}
		getWorld()->playSound(SOUND_BOO_ACTIVATE);
	}
	if (playNum == 2) { // Yoshi
		int chance = randInt(0, 1); // 50% 
		if (chance) {
			//cerr << "swap coins" << endl;
			getWorld()->getYoshi()->swap_coins();
		}
		else {
			//cerr << "swap stars" << endl;
			getWorld()->getYoshi()->swap_stars();
		}
		getWorld()->playSound(SOUND_BOO_ACTIVATE);
	}
}
void Boo::doActivity2() {
	//cerr << "in Activity 2" << endl;
	setSquaresToMove(randInt(1, 3));
}

void Boo::doActivity3() {
	//cerr << "in Activity 3" << endl;
	return;
}

// Activate on Player
void ActivateOnPlayer::doSomething() {
	if (getWorld()->getPeach()->getX() == getX() && getWorld()->getPeach()->getY() == getY()
		&& getWorld()->getPeach()->getNewNess()) {
		doActivity(1);
		getWorld()->getPeach()->setNewNess(false);
	}
	if (getWorld()->getYoshi()->getX() == getX() && getWorld()->getYoshi()->getY() == getY()
		&& getWorld()->getYoshi()->getNewNess()) {
		doActivity(2);
		getWorld()->getYoshi()->setNewNess(false);
	}
}


// Coin
void CoinSquare::doActivity(int num) { // TEST WORKS
	if (!getActive())
		return;
	
	// if players are walking, dont do anything
	if (!getWorld()->getPeach()->isWaiting() || !getWorld()->getYoshi()->isWaiting())
		return;

	// check if PEACH LANDED on square
	if (num == 1) {
		if (getWorld()->getPeach()->isWaiting() && getWorld()->getPeach()->getNewNess()) {
			if (isBlue()) { // gives coins!
				getWorld()->getPeach()->addCoins(3);
				//cerr << "PEACH coins after blue: " << getWorld()->getPeach()->get_coins() << endl;
				getWorld()->playSound(SOUND_GIVE_COIN);
			}
			else if (isRed()) { // takes coins!
				getWorld()->getPeach()->deductCoins(3);
				//cerr << "PEACH coins after RED: " << getWorld()->getPeach()->get_coins() << endl;
				getWorld()->playSound(SOUND_TAKE_COIN);
			}
		}
	}

	// check if Yoshi landed on square
	if (num == 2) {
		if (getWorld()->getYoshi()->isWaiting() && getWorld()->getYoshi()->getNewNess()) {
			if (isBlue()) { // gives coins!
				getWorld()->getYoshi()->addCoins(3);
				//cerr << "Yoshi coins after blue: " << getWorld()->getYoshi()->get_coins() << endl;
				getWorld()->playSound(SOUND_GIVE_COIN);
			}
			else if (isRed()) { // takes coins!
				getWorld()->getYoshi()->deductCoins(3);
				//cerr << "YOSHI coins after RED: " << getWorld()->getYoshi()->get_coins() << endl;
				getWorld()->playSound(SOUND_TAKE_COIN);
			}
		}
	}
}

// Star
void StarSquare::doActivity(int num) { // TEST WORKS
	if (!getActive())
		return;
	// peach
	if (num == 1) {
		if (getWorld()->getPeach()->get_coins() < 20)
			return;
		else {
			getWorld()->getPeach()->deductCoins(20);
			getWorld()->getPeach()->addStars(1);
			getWorld()->playSound(SOUND_GIVE_STAR);
		}
	}

	// Yoshi
	if (num == 2) {
		if (getWorld()->getYoshi()->get_coins() < 20)
			return;
		else {
			getWorld()->getYoshi()->deductCoins(20);
			getWorld()->getYoshi()->addStars(1);
			getWorld()->playSound(SOUND_GIVE_STAR);
		}
	}
}

void DirectionalSquare::doActivity(int num) { // TEST WORKS
	if (!getActive())
		return;
	if (num == 1) {
		getWorld()->getPeach()->force_walk_direction(forceDir);
		if (getWorld()->getPeach()->getDir() == left)
			getWorld()->getPeach()->setDirection(180);
		if(getWorld()->getPeach()->getDir() == right || getWorld()->getPeach()->getDir() == up
			|| getWorld()->getPeach()->getDir() == down)
			getWorld()->getPeach()->setDirection(0);
	}
	if (num == 2) {
		getWorld()->getYoshi()->force_walk_direction(forceDir);
		if (getWorld()->getYoshi()->getDir() == left)
			getWorld()->getYoshi()->setDirection(180);
		if (getWorld()->getYoshi()->getDir() == right || getWorld()->getYoshi()->getDir() == up
			|| getWorld()->getYoshi()->getDir() == down)
			getWorld()->getYoshi()->setDirection(0);
	}
}

void BankSquare::doActivity(int num) { // TEST WORKS
	if (!getActive())
		return;
	// PEACH
	if (num == 1) {
		if (getWorld()->getPeach()->isWaiting()) { // landed on
			getWorld()->getPeach()->addCoins(getWorld()->getBalance());
			getWorld()->setBalance(0);
			getWorld()->playSound(SOUND_WITHDRAW_BANK);
		}
		else if (!getWorld()->getPeach()->isWaiting()) { // passes over
			int deducted = getWorld()->getPeach()->deductCoins(5);
			getWorld()->addBalance(deducted);
			getWorld()->playSound(SOUND_DEPOSIT_BANK);
		}
	}

	// Yoshi
	if (num == 2) {
		if (getWorld()->getYoshi()->isWaiting()) { // landed on
			getWorld()->getYoshi()->addCoins(getWorld()->getBalance());
			getWorld()->setBalance(0);
			getWorld()->playSound(SOUND_WITHDRAW_BANK);
		}
		else if (!getWorld()->getYoshi()->isWaiting()) { // passes over
			int deducted = getWorld()->getYoshi()->deductCoins(5);
			getWorld()->addBalance(deducted);
			getWorld()->playSound(SOUND_DEPOSIT_BANK);
		}
	}
}

void EventSquare::doActivity(int num) {
	if (!getActive())
		return;
	// PEACH
	if (num == 1) {
		if (getWorld()->getPeach()->isWaiting()) { // landed on
			int choice = randInt(1, 3);
			if (choice == 1) {
				//cerr << "PEACH In Event Square Teleport" << endl;
				getWorld()->getPeach()->teleport_me_to_random_sq();
				getWorld()->playSound(SOUND_PLAYER_TELEPORT);
			}
			else if (choice == 2) {
				//cerr << "PEACH In Event Square Swap Player" << endl;
				getWorld()->getPeach()->swapPlayer();
				getWorld()->playSound(SOUND_PLAYER_TELEPORT);
			}
			else {
				//cerr << "PEACH In Event Square Projectile" << endl;
				getWorld()->getPeach()->setVortex(true);
				getWorld()->playSound(SOUND_GIVE_VORTEX);
			}
		}
	}
	// YOSHI
	if (num == 2) {
		if (getWorld()->getYoshi()->isWaiting()) { // landed on
			int choice = randInt(1, 3);
			if (choice == 1) {
				//cerr << "YOSHI In Event Square Teleport" << endl;
				getWorld()->getYoshi()->teleport_me_to_random_sq();
				getWorld()->playSound(SOUND_PLAYER_TELEPORT);
			}
			else if (choice == 2) {
				//cerr << "YOSHI In Event Square SWAP" << endl;
				getWorld()->getYoshi()->swapPlayer();
				getWorld()->playSound(SOUND_PLAYER_TELEPORT);
			}
			else {
				//cerr << "YOSHI In Event Square PROJECTILE" << endl;
				getWorld()->getYoshi()->setVortex(true);
				getWorld()->playSound(SOUND_GIVE_VORTEX);
			}
		}
	}
}

void DroppingSquare::doActivity(int num) {
	if (!getActive())
		return;

	// PEACH
	if (num == 1) {
		if (getWorld()->getPeach()->isWaiting()) { // landed on
			int choice = randInt(1, 2);
			if (choice == 1) {
				getWorld()->getPeach()->deductCoins(10);
			}
			else if (choice == 2) {
				getWorld()->getPeach()->deductStars(1);
			}
			getWorld()->playSound(SOUND_DROPPING_SQUARE_ACTIVATE);
		}
	}

	// Yoshi
	if (num == 2) {
		if (getWorld()->getYoshi()->isWaiting()) { // landed on
			int choice = randInt(1, 2);
			if (choice == 1) {
				getWorld()->getYoshi()->deductCoins(10);
			}
			else
				getWorld()->getYoshi()->deductStars(1);
			
			getWorld()->playSound(SOUND_DROPPING_SQUARE_ACTIVATE);
		}
	}
}

void Vortex::doSomething() {
	//cerr << "Vortex do Something called" << endl;
	if (!getActive())
		return;
	moveAtAngle(getDir(), 2);
	if (getX() < 0 || getX() >= VIEW_WIDTH || getY() < 0 || getY() >= VIEW_HEIGHT)
		setActive(false);
	if (getWorld()->objectOverlappingWithVortexHIT(this)) {
		setActive(false);
		getWorld()->playSound(SOUND_HIT_BY_VORTEX);
	}
}

