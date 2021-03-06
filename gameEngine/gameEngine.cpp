
#include "stdafx.h"
#include <SFML/Graphics.hpp>
#include <iostream> 
#include "view.h"//podłączamy kamere
#include "mission.h"
#include "level.h"
#include <vector>
#include <list>
using namespace sf;  
using namespace std;

					 ////////////////////////////////////////////////////GŁÓWNY KLAS////////////////////////
class Entity {
public:
	vector<Object> obj;																	//wektor obiektów mapy
	float dx, dy, x, y, speed, moveTimer;												
	int w, h, health;
	bool life, isMove, onGround;
	Texture texture;
	Sprite sprite;
	String name;																		//nazwa dla przeciwnika
	Entity(Image &image, String Name, float X, float Y, int W, int H) {
		x = X; y = Y; w = W; h = H; name = Name; moveTimer = 0;
		speed = 0; health = 100; dx = 0; dy = 0;
		life = true; onGround = false; isMove = false;
		texture.loadFromImage(image);
		sprite.setTexture(texture);
		sprite.setOrigin(w / 2, h / 2);
	}
	FloatRect getRect() {																// f dla rectangle, dotycze przeciwnika
		return FloatRect(x, y, w, h);													// sprawdzanie uderzenia
	}

	virtual void update(float time) = 0;												// wszystkie klasy dzidziczą update
};

////////////////////////////////////////////////////KLAS GRACZA////////////////////////
class Player :public Entity {
public:
	enum { left, right, up, down, jump, stay, right_Top } state;						// stan obiekta
	int playerScore;																	// illośc punktów
	bool isShoot;

	Player(Image &image, String Name, Level &lev, float X, float Y, int W, int H) :Entity(image, Name, X, Y, W, H) {
		playerScore = isShoot = 0; state = stay; obj = lev.GetAllObjects();				// dzialanie obiekta z mapą
		if (name == "Player1") {
			sprite.setTextureRect(IntRect(4, 19, w, h));
		}
	}
	void control() {																	// blok kierowania
		if (Keyboard::isKeyPressed) {														
			if (Keyboard::isKeyPressed(Keyboard::Left)) {												
				state = left; speed = 0.1;
			}
			if (Keyboard::isKeyPressed(Keyboard::Right)) {
				state = right, speed = 0.1;
			}
			if ((Keyboard::isKeyPressed(Keyboard::Up)) && (onGround)) {
				state = jump; dy = -0.6; onGround = false;								// zwiekszamy wysokośc skakania
			}
			if (Keyboard::isKeyPressed(Keyboard::Down)) {
				state = down;
			}
			if ((Keyboard::isKeyPressed(Keyboard::Right)) && (Keyboard::isKeyPressed(Keyboard::Up))) {
				state = right_Top;
			}
																						///// strzal
			if (Keyboard::isKeyPressed(Keyboard::Space)) {
				isShoot = true;
			}
		}
	}
	void checkCollisionWithMap(float Dx, float Dy)										// f granicy mapy
	{
		for (int i = 0; i<obj.size(); i++)												// przechodzimy przez obiekt
			if (getRect().intersects(obj[i].rect))										// sprawdzamy uderzenia gracza i obiektów
			{
				if (obj[i].name == "solid")												// jezli mamy przegrode
				{
					if (Dy>0) { y = obj[i].rect.top - h;  dy = 0; onGround = true; }
					if (Dy<0) { y = obj[i].rect.top + obj[i].rect.height;   dy = 0; }
					if (Dx>0) { x = obj[i].rect.left - w; }
					if (Dx<0) { x = obj[i].rect.left + obj[i].rect.width; }
				}
			}
	}

	void update(float time)
	{
		control();
		switch (state)																	// dzieje w zaleznosci od stanu gracza
		{
		case right:dx = speed; break;
		case left:dx = -speed; break;
		case up: break;
		case down: dx = 0; break;
		case stay: break;	
		case right_Top:dx = speed; break;
		}
		x += dx * time;
		checkCollisionWithMap(dx, 0);													// uderzenia po x i y
		y += dy * time;
		checkCollisionWithMap(0, dy);
		sprite.setPosition(x + w / 2, y + h / 2); 
		if (health <= 0) { life = false; }
		if (!isMove) { speed = 0; }
		if (life) { setPlayerCoordinateForView(x, y); }
		dy = dy + 0.0015*time;															// zawsze wracamy na ziemie
	}
};

///////////////////////////////////////KLAS PRZECIWNIK /////////////////////////////////////////
class Enemy :public Entity {
public:
	Enemy(Image &image, String Name, Level &lvl, float X, float Y, int W, int H) :Entity(image, Name, X, Y, W, H) {
		obj = lvl.GetObjects("solid");
		if (name == "EasyEnemy") {
			sprite.setTextureRect(IntRect(0, 0, w, h));
			dx = 0.1;
		}
	}

	void checkCollisionWithMap(float Dx, float Dy)
	{
		for (int i = 0; i<obj.size(); i++)
			if (getRect().intersects(obj[i].rect))
			{
				if (Dy>0) { y = obj[i].rect.top - h;  dy = 0; onGround = true; }
				if (Dy<0) { y = obj[i].rect.top + obj[i].rect.height;   dy = 0; }
				if (Dx>0) { x = obj[i].rect.left - w;  dx = -0.1; sprite.scale(-1, 1); }
				if (Dx<0) { x = obj[i].rect.left + obj[i].rect.width; dx = 0.1; sprite.scale(-1, 1); }
			}
	}

	void update(float time)
	{
		if (name == "EasyEnemy") {
			checkCollisionWithMap(dx, 0);
			x += dx * time;
			sprite.setPosition(x + w / 2, y + h / 2);
			if (health <= 0) { life = false; }
		}
	}
};

////////////////////////////////////////////////////PLATFORMA////////////////////////
class MovingPlatform : public Entity {
public:
	MovingPlatform(Image &image, String Name, Level &lvl, float X, float Y, int W, int H):Entity(image, Name, X, Y, W, H) {			// konstruktor
		sprite.setTextureRect(IntRect(0, 0, W, H));																					// rectangle 
		dx = 0.08;																													// początkowa szybkośc po x
	}

	void update(float time)
	{
		x += dx * time;																												// przemieszczenie po x
		moveTimer += time;																											// przespieszenie
		if (moveTimer>2000) { dx *= -1; moveTimer = 0; }																			// zmiana kierunku ruchu
		sprite.setPosition(x + w / 2, y + h / 2);
	}
};

////////////////////////////////////////////////////KLAS KULI////////////////////////
class Bullet :public Entity {
	int direction;

	Bullet(Image &image, String Name, Level &lvl, float X, float Y, int W, int H, int dir) :Entity(image, Name, X, Y, W, H) {
		obj = lvl.GetObjects("solid");
		x = X;
		y = Y;
		direction = dir;
		speed = 0.8;
		w = h = 16;
		life = true;
	}

	void update(float time)
	{
		switch (direction)
		{
		case 0: dx = -speed; dy = 0;   break;
		case 1: dx = speed; dy = 0;    break;
		case 2: dx = 0; dy = -speed;   break;
		case 3: dx = 0; dy = -speed;   break;
		case 4: dx = 0; dy = -speed;   break;
		case 5: dx = 0; dy = -speed;   break;
		}

		x += dx * time;
		y += dy * time;

		if (x <= 0) x = 1;
		if (y <= 0) y = 1;

		for (int i = 0; i < obj.size(); i++) {
			if (getRect().intersects(obj[i].rect)) 
			{
				life = false;
			}
		}

		sprite.setPosition(x + w / 2, y + h / 2);
	}
};

int main()
	{
		RenderWindow window(VideoMode(640, 480), "Super Game 2D Engine");
		view.reset(FloatRect(0, 0, 640, 480));

		Level lvl;																										// egzemplar klasa Level
		lvl.LoadFromFile("images/map.tmx");
		
		Image heroImage;
		heroImage.loadFromFile("images/MilesTailsPrower.gif");
		
		Image easyEnemyImage;
		easyEnemyImage.loadFromFile("images/shamaich.png");
		easyEnemyImage.createMaskFromColor(Color(255, 0, 0));
		
		Image movePlatformImage;
		movePlatformImage.loadFromFile("images/MovingPlatform.png");

		Image BulletImage;																								// kula
		BulletImage.loadFromFile("images/bullet.png");
		BulletImage.createMaskFromColor(Color(0, 0, 0));

		list<Entity*> entities;																							// tworzymmy liste i tutaj wrzucamy przeciwników
		list<Entity*>::iterator it;																						// aby przechodzić po liscie
		list<Entity*>::iterator it2;																					// miedzy obiektami listy

		vector<Object> e = lvl.GetObjects("EasyEnemy");																	// wszystkie obiekty przeciwników trzymamy w tym wektorze

		for (int i = 0; i < e.size(); i++) entities.push_back(new Enemy(easyEnemyImage, "EasyEnemy", lvl, e[i].rect.left, e[i].rect.top, 200, 97)); //wrzucamy do listy naszych przeciwników, przechodzi,y po elementam listy

		Object player = lvl.GetObject("player");

		Player p(heroImage, "Player1", lvl, player.rect.left, player.rect.top, 40, 30);
		
		e = lvl.GetObjects("MovingPlatform");

		for (int i = 0; i < e.size(); i++) entities.push_back(new MovingPlatform(movePlatformImage, "MovingPlatform", lvl, e[i].rect.left, e[i].rect.top, 95, 22));
		Clock clock;

	while (window.isOpen())
	{
		float time = clock.getElapsedTime().asMicroseconds();
		clock.restart();
		time = time / 800;

		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)																			// warunek jeżeli zamkniete okno oraz event
				window.close();
			if (p.isShoot == true) { p.isShoot = false; entities.push_back(new Bullet(BulletImage, "Bullet", lvl, p.x, p.y, 16, 16, p.state)); }	// enum przekazujemy jak int i strzelamy
		}
		for (it = entities.begin(); it != entities.end();)
		{
			Entity *b = *it;																							
			b->update(time);																							// update dla wszystkich zywych obiektów
			if (b->life == false) { it = entities.erase(it); delete b; }												// usuwamy martwy obiekt
			else it++;																									// i tak sprawdzamy kazdy obiekt
		}
		////////////////////////////////////////////////////WSPóŁPRACA Z MOVING PLATFORM////////////////////////
		for (it = entities.begin(); it != entities.end(); it++)
		{
			if (((*it)->name == "MovingPlatform") && ((*it)->getRect().intersects(p.getRect())))						// wspolpraca z obiektem mowing platfrom
			{
				Entity *movPlat = *it;
				if ((p.dy>0) || (p.onGround == false))																	// i gracz przy tym pada na dół
					if (p.y + p.h<movPlat->y + movPlat->h)	
					{
						p.y = movPlat->y - p.h + 3; p.x += movPlat->dx*time; p.dy = 0; p.onGround = true;				// wyrzucamy graca tak zeby on stal na platformie
					}
			}
			////////////////////////////////////////////////////WSPÓŁ PRACA Z ENEMY////////////////////////
			if (((*it)->name == "EasyEnemy") && ((*it)->getRect().intersects(p.getRect())))
			{
																														////////wyrzucanie przeciwnika
				if ((*it)->dx>0)																						//jezeli idzie w prawo
				{
					cout << "(*it)->x" << (*it)->x << "\n";
					cout << "p.x" << p.x << "\n\n";

					(*it)->x = p.x - (*it)->w;													
					(*it)->dx = 0;

					cout << "new (*it)->x" << (*it)->x << "\n";
					cout << "new p.x" << p.x << "\n\n";
				}
				if ((*it)->dx < 0)
				{
					(*it)->x = p.x + p.w; 
					(*it)->dx = 0;
				}
						
				if (p.dx < 0) { p.x = (*it)->x + (*it)->w; }
				if (p.dx > 0) { p.x = (*it)->x - p.w; }		
			}

			for (it2 = entities.begin(); it2 != entities.end(); it2++) {
				if ((*it)->getRect() != (*it2)->getRect())
					if (((*it)->getRect().intersects((*it2)->getRect())) && ((*it)->name == "EasyEnemy") && ((*it2)->name == "EasyEnemy"))	// uderzenia dwoch przeciwników obiektów
					{
						(*it)->dx *= -1;																									// zmieniamy ich jierunek
						(*it)->sprite.scale(-1, 1);																							// pokazujemy sprite po x
					}
			}
		}

	/////////////////////////////RYSUJEMY MAPE/////////////////////
		p.update(time);																											// Player update function	
		window.setView(view);
		window.clear(Color(77, 83, 140));
		lvl.Draw(window);
		
		for (it = entities.begin(); it != entities.end(); it++) { window.draw((*it)->sprite); }
		window.draw(p.sprite);																									// malujemy Sprajt i klas Player
		window.display();
	}

	return 0;
}
