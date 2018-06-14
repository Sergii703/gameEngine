#pragma once
#include <SFML/Graphics.hpp>
using namespace sf;

View view;											//tworzymy obiekt vie, ktory i jest kamerą

void setPlayerCoordinateForView(float x, float y) { // funkcja dla sledzenie za graczem
	float tempX = x; float tempY = y;
													// przesunięcie kamery
	if (y > 624) tempY = 624;						

	view.setCenter(tempX, tempY);					//przekazujemy koordynata gracza

}