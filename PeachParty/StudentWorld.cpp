#include "StudentWorld.h"
#include "Actor.h"
#include "GameConstants.h"
#include <string>
#include <vector>
#include <sstream>  
#include <math.h>

using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    balance = 0;
}

int StudentWorld::init()
{
    char boardnum = '0' + GameWorld::getBoardNumber();
    string board_file = assetPath() + "board0" +  boardnum + ".txt";
    Board::LoadResult result = bd.loadBoard(board_file);

    if (result == Board::load_fail_file_not_found) {
        cerr << "Could not find board01.txt data file\n";
        return GWSTATUS_BOARD_ERROR;
    }
    else if (result == Board::load_fail_bad_format) {
        cerr << "Your board was improperly formatted\n";
        return GWSTATUS_BOARD_ERROR;
    }
    else if (result == Board::load_success) 
        cerr << "Successfully loaded board\n";

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            Board::GridEntry ge = bd.getContentsOf(i, j); 
            switch (ge) {
            case Board::empty:
                break;
            case Board::boo: // size = 1, depth  = 0 for player, bowser, boo, vortex
                m_actors.push_back(new Boo(this, IID_BOO, i * SPRITE_WIDTH, j * SPRITE_HEIGHT));
                m_actors.push_back(new CoinSquare(this, IID_BLUE_COIN_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, true));
                break;
            case Board::bowser:
                m_actors.push_back(new Bowser(this, IID_BOWSER, i * SPRITE_WIDTH, j * SPRITE_HEIGHT));
                m_actors.push_back(new CoinSquare(this, IID_BLUE_COIN_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, true));
                break;
            case Board::player:
                m_peach = new Player(this, IID_PEACH, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 0, 0, 1.0, 1);
                m_yoshi = new Player(this, IID_YOSHI, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 0, 0, 1.0, 2);
                m_actors.push_back(new CoinSquare(this, IID_BLUE_COIN_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, true));
                break;
            case Board::red_coin_square:
                m_actors.push_back(new CoinSquare(this, IID_RED_COIN_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, false));
                break;
            case Board::blue_coin_square:
                m_actors.push_back(new CoinSquare(this, IID_BLUE_COIN_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, true));
                break;
            case Board::up_dir_square:
                m_actors.push_back(new DirectionalSquare(this, IID_DIR_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 90)); // DIR dir, int angle
                break;
            case Board::down_dir_square:
                m_actors.push_back(new DirectionalSquare(this, IID_DIR_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 270));
                break;
            case Board::left_dir_square:
                m_actors.push_back(new DirectionalSquare(this, IID_DIR_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT,180));
                break;
            case Board::right_dir_square:
                m_actors.push_back(new DirectionalSquare(this, IID_DIR_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 0));
                break;
            case Board::event_square:
                m_actors.push_back(new EventSquare(this, IID_EVENT_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT));
                break;
            case Board::bank_square:
                m_actors.push_back(new BankSquare(this, IID_BANK_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT));
                break;
            case Board::star_square:
                m_actors.push_back(new StarSquare(this, IID_STAR_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT));
                break;
            default:
                break;
            }
        }
    }  
	startCountdownTimer(99);  
    return GWSTATUS_CONTINUE_GAME;
}

/*
* Each time the move() method is called, it must run a single tick of the
game. This means that it is responsible for asking each of the game actors (e.g., Peach,
Yoshi, each Bowser, Boo, Square, Vortex, etc.) to try to do something: e.g., move
themselves and/or perform their specified behavior. This method might also introduce
new actors into the game, for instance adding a new Vortex projectile next to Peach if she
fires. Finally, this method is responsible for disposing of (i.e., deleting) actors that need
to disappear during a given tick (e.g., a Vortex that runs into a baddie and disappears).
For example, if Bowser is shot by Peach’s Vortex, then the Vortex object's state should
be set to dead, and then after all of the alive actors in the game get a chance to do
something during the tick, the move() method should remove that Vortex object from the
game world (by deleting its object and removing any reference to the object from the
StudentWorld’s data structures). The move() method will automatically be called once
during each tick of the gam
*/
int StudentWorld::move() 
{
    for (unsigned i = 0; i < m_actors.size(); i++) {
        if (m_actors[i]->getActive())
            m_actors[i]->doSomething();
    }
    m_peach->doSomething();
    m_yoshi->doSomething();

    //Remove newly-inactive actors after each tick
    vector<Actor*>::iterator p = m_actors.begin();
    for (; p != m_actors.end();) {
        if (!(*p)->getActive()) {
            delete* p;
            p = m_actors.erase(p);
        }
        else
            p++;
    }

    updateStatus();
        
    if (timeRemaining() <= 0)
    {
        playSound(SOUND_GAME_FINISHED);
        if (m_yoshi == determineWinner())
        {
            setFinalScore(m_yoshi->get_stars(), m_yoshi->get_coins());
            return GWSTATUS_YOSHI_WON;
        }
        else // peach won
        {
            setFinalScore(m_peach->get_stars(), m_peach->get_coins());
            return GWSTATUS_PEACH_WON;
        }
    }
     //the game isn't over yet so continue playing
    return GWSTATUS_CONTINUE_GAME;
}

/*
* The cleanUp() method is called by our framework when Peach and Yoshi complete the
current game. The cleanUp() method is responsible for deleting all remaining actors (e.g.,
Peach and Yoshi, squares, baddies, Vortexes, etc.) that are currently managed by
StudentWorld to prevent memory leaks. This includes all actors created during either the
init() method or introduced during subsequent game play by the actors in the game (e.g.,
a Vortex that was added to the board by Yoshi when he fires) that have not yet been
removed from the game. 
*/
void StudentWorld::cleanUp()
{
    delete m_peach;
    delete m_yoshi;
    for (unsigned i = 0; i < m_actors.size(); i++) {
        delete m_actors[i];
    }
    m_actors.clear();
    m_peach = nullptr;
    m_yoshi = nullptr;
}

StudentWorld::~StudentWorld() {
    cleanUp();
}

bool StudentWorld::isEmpty(double x, double y) {
    if (x < 0 || y < 0)
        return true;
    return (bd.getContentsOf(x / 16, y / 16) == bd.empty);
}

Player* StudentWorld::determineWinner() {
    if (m_yoshi->get_stars() > m_peach->get_stars()) 
        return m_yoshi;
    else if (m_yoshi->get_stars() < m_peach->get_stars()) 
        return m_peach;
    else { // use coins to determine winner
        if (m_yoshi->get_coins() > m_peach->get_coins())
            return m_yoshi;
        else if (m_yoshi->get_coins() < m_peach->get_coins())
            return m_peach;
        else { // neither coins or stars, random
            int chance = randInt(0, 1);
            if (chance)
                return m_peach;
            return m_yoshi;
        }
    }
}

void StudentWorld::updateStatus() {
    ostringstream text;
    //P1 Roll : 3 Stars : 2 $$ : 15 | Time : 75 | Bank : 9 | P2 Roll : 0 Stars : 1 $$ : 22 VOR
    string vor1;
    string vor2;
    if (m_peach->has_vortex())
        vor1 += " VOR";
    else
        vor1 = "";
    if (m_yoshi->has_vortex())
        vor2 += " VOR";
    else
        vor2 = "";

    text << "P1 Roll: " << m_peach->currentDieRoll() << " Stars: " << m_peach->get_stars()
        << " $$: " << m_peach->get_coins() << vor1 << " | Time: " << timeRemaining() << " | Bank: " << getBalance()
        << " | P2 Roll: " << m_yoshi->currentDieRoll() << " Stars: " << m_yoshi->get_stars() <<
        " $$: " << m_yoshi->get_coins() << vor2 << endl;
    setGameStatText(text.str());
}

void StudentWorld::replaceWithDroppingSquare(Actor* dropSquare) {
    for (unsigned i = 0; i < m_actors.size(); i++) {
        if (m_actors[i]->getX() == dropSquare->getX() &&
            m_actors[i]->getY() == dropSquare->getY() && m_actors[i]->is_a_square())
            m_actors[i]->setActive(false);
    }
    m_actors.push_back(dropSquare);
}

void StudentWorld::addVortex(Actor* vortex) {
    m_actors.push_back(vortex);
}

bool StudentWorld::doesVortexOverlap(Actor* vortex, Actor* obj) {
    int x = abs(vortex->getX() - obj->getX());
    int y = abs(vortex->getY() - obj->getY());

    if (x <= 15 && y <= 15)
        return true;
    return false;
}

bool StudentWorld::objectOverlappingWithVortexHIT(Actor* vortex) {
    for (int i = 0; i < m_actors.size(); i++) {
        if (m_actors[i]->can_be_hit_by_vortex() && doesVortexOverlap(vortex, m_actors[i])) {
            m_actors[i]->hit_by_vortex();
            return true;
        }
    }
    return false;
}