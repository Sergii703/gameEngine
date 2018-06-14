#pragma once
using namespace std;
#include <string>
///////////////////////////////////NUMER MISJI//////////////////////////////////

int getCurrentMission(int x)					//numer misji się zmienia w zależności od koordynat gracza
{
	int mission = 0;
	if ((x>0) && (x<400)) { mission = 0; }		//misję
	if (x>400) { mission = 1; }					
	if (x>700) { mission = 2; }					
	if (x>2200) { mission = 3; }				

	return mission;								//funkcja zwraca numer misji
}



/////////////////////////////////////TEKST MISJI/////////////////////////////////

string getTextMission(int currentMission) {

	string missionText = "";					//tekst misji i jego inicjalizacja

	switch (currentMission)						//przjmuje numer misji
	{
	case 0: missionText = "\nBeginning \ninstruction"; break;
	case 1: missionText = "\nMission 1\n\nThe first mission\n"; break;
	case 2: missionText = "\nMission 2\n The second mission\n"; break;
	}

	return missionText;							//funkcja zwraca tekst
};