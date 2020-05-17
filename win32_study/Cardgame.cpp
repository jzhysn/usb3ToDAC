
#include "Cardgame.h"
#include <iostream>
using namespace std;

Cardgame::Cardgame(int p)
{
	players = p;
	totalparticipants += p;
	cout <<players <<" player have  started a new game. There are now "
		<<totalparticipants<<" players in tatal."<<endl;
}

Cardgame::~Cardgame(void)
{
	totalparticipants -= players;
	cout<< players <<" player have finished their game.There are now "
		<<totalparticipants<<" players in total."<<endl;
}
