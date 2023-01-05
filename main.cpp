#include <SFML\Graphics.hpp>
#include <SFML\Audio.hpp>
#include <random>
#include <ctime>

int main ()
{
	sf::RenderWindow window (sf::VideoMode (500, 700), "ÉKARÁCSONY 2018", sf::Style::Close); // it can't be resize
	window.setFramerateLimit (60);

	sf::Texture backgroundTexture;
	sf::Texture playerTexture;
	sf::Texture platformTexture;
	sf::Texture giftTexture;
	sf::Texture elfTexture;

	backgroundTexture.loadFromFile ("images/background.png");
	playerTexture.loadFromFile ("images/doodle.png");
	platformTexture.loadFromFile ("images/platform.png");
	giftTexture.loadFromFile ("images/gift.png");
	elfTexture.loadFromFile ("images/elf.png");

	sf::Sprite background (backgroundTexture);
	sf::Sprite player (playerTexture);
	sf::Sprite platform (platformTexture);
	sf::Sprite gift (giftTexture);
	sf::Sprite elf (elfTexture);
	/*
	*      ----Size of images----
	*    background.png size: 500 * 700
	*    doodle.png     size:  80 *  80
	*    platform.png   size:  68 *  14
	*	 gift.png		size:  30 *  30
	*	 elf.png        size:  70 *  70;
	*/
newgame:
	bool hasGift = false;
	bool enableDrawGift = false;
	bool enableDrawElf = false;
	std::clock_t start;
	double duration;

	start = std::clock ();

	sf::RectangleShape gameoverBackground (sf::Vector2f (500, 700));
	gameoverBackground.setFillColor (sf::Color::White);

	sf::Font font;
	font.loadFromFile ("font/arial.ttf");
	sf::Text scoreText;
	scoreText.setFont (font);
	scoreText.setCharacterSize (50);
	scoreText.setFillColor (sf::Color::White);
	sf::Text gameoverText;
	gameoverText.setFont (font);
	gameoverText.setString ("Game Over!");
	gameoverText.setCharacterSize (80);
	gameoverText.setFillColor (sf::Color::Red);

	sf::Text newgame;
	newgame.setFont (font);
	newgame.setString ("Press N for new game");
	newgame.setCharacterSize (50);
	newgame.setFillColor (sf::Color::Red);


	// sound
	sf::SoundBuffer buffer;
	buffer.loadFromFile ("sound/jump.wav");
	sf::Sound sound;
	sound.setBuffer (buffer);

	// santa
	sf::SoundBuffer buffer2;
	buffer2.loadFromFile ("sound/santa.wav");
	sf::Sound sound2;
	sound2.setBuffer (buffer2);



	// initialize platforms
	sf::Vector2u platformPosition[10];
	std::uniform_int_distribution<unsigned> x (0, 500 - platformTexture.getSize ().x);
	std::uniform_int_distribution<unsigned> y (100, 700);
	std::default_random_engine e (time (0));
	for (size_t i = 0; i < 10; ++i)
	{
		platformPosition[i].x = x (e);
		platformPosition[i].y = y (e);
	}

	// initialize gift
	sf::Vector2u giftPosition[1];
	giftPosition[0].x = 500;
	giftPosition[0].y = 700;

	//initialize elf
	sf::Vector2u elfPosition[1];
	elfPosition[0].x = 500;
	elfPosition[0].y = 700;

	// player's positon and down velocity
	int playerX = 250;
	int playerY = 151;
	float dy = 0;
	int height = 150;
	int score = 0;

	// player's bounding box. It should modify according to the image size
	const int PLAYER_LEFT_BOUNDING_BOX = 20;
	const int PLAYER_RIGHT_BOUNDING_BOX = 60;
	const int PLAYER_BOTTOM_BOUNDING_BOX = 70;

	while (window.isOpen ())
	{

		sf::Event event;
		while (window.pollEvent (event))
		{
			if (event.type == sf::Event::Closed)
				window.close ();
		}

		if (sf::Keyboard::isKeyPressed (sf::Keyboard::A) || sf::Keyboard::isKeyPressed (sf::Keyboard::Left))
			playerX -= 4;
		if (sf::Keyboard::isKeyPressed (sf::Keyboard::D) || sf::Keyboard::isKeyPressed (sf::Keyboard::Right))
			playerX += 4;

		// player out of boundary and change the side
		// I don't know why   if (playerX > window.getSize().x)   doesn't work. so I use 500 instead of window.getSize().x
		// the second if :  if (playerX < -40)   use -40 because window.getSize().x is unsigned int, it can't be negative
		// -40 is (playerTexture x size) / 2
		if (playerX > 500)
			playerX = 0;
		if (playerX < -40)
			playerX = window.getSize ().x - playerTexture.getSize ().x;

		// score
		// dy = -1.60001 is terminal velocity that player stand on the platform and don't go up anymore
		// score can't increase in this situation
//		if (playerY == height && dy < (-1.62))
//		{
//			score += 1;
//			scoreText.setString ("Score: " + std::to_string (score));
//		}

		// player's jump mechanism
		dy += 0.2;
		playerY += dy;

		if (playerY < height)
			for (size_t i = 0; i < 10; ++i)
			{
				playerY = height;
				platformPosition[i].y -= dy;  // vertical translation
				if (platformPosition[i].y > 700) // set new platform on the top
				{
					platformPosition[i].y = 0;
					platformPosition[i].x = x (e);
				}

				duration = (std::clock () - start) / (double)CLOCKS_PER_SEC;
				if (duration > 5) {
					enableDrawGift = true;
					enableDrawElf = true;
					start = std::clock ();
				}

				if (enableDrawGift) {
					giftPosition[0].x = platformPosition[5].x + 15;
					giftPosition[0].y = platformPosition[5].y - 30;
				}

				if (enableDrawElf) {
					elfPosition[0].x = platformPosition[6].x;
					elfPosition[0].y = platformPosition[6].y - 70;
				}

			}
		// detect jump on the platform
		for (size_t i = 0; i < 10; ++i)
		{
			if ((playerX + PLAYER_RIGHT_BOUNDING_BOX > platformPosition[i].x) && (playerX + PLAYER_LEFT_BOUNDING_BOX < platformPosition[i].x + platformTexture.getSize ().x)        // player's horizontal range can touch the platform
				&& (playerY + PLAYER_BOTTOM_BOUNDING_BOX > platformPosition[i].y) && (playerY + PLAYER_BOTTOM_BOUNDING_BOX < platformPosition[i].y + platformTexture.getSize ().y)  // player's vertical   range can touch the platform
				&& (dy > 0)) // player is falling
			{
				sound.play ();
				dy = -10;
			}
		}

		// detect pickup gift
		if (!hasGift) {
			if ((playerX + PLAYER_RIGHT_BOUNDING_BOX > giftPosition[0].x) && (playerX + PLAYER_LEFT_BOUNDING_BOX < giftPosition[0].x + giftTexture.getSize ().x)        // player's horizontal range can touch the gift
				&& (playerY + PLAYER_BOTTOM_BOUNDING_BOX > giftPosition[0].y) && (playerY + PLAYER_BOTTOM_BOUNDING_BOX < giftPosition[0].y + giftTexture.getSize ().y))  // player's vertical   range can touch the gift
			{
				giftPosition[0].x = 500;
				giftPosition[0].y = 700;
				hasGift = true;
				start = std::clock ();
				enableDrawGift = false;
				enableDrawElf = false;
			}
		}

		// detect dropgift
		if (hasGift) {
			if ((playerX + PLAYER_RIGHT_BOUNDING_BOX > elfPosition[0].x) && (playerX + PLAYER_LEFT_BOUNDING_BOX < elfPosition[0].x + elfTexture.getSize ().x)        // player's horizontal range can touch the gift
				&& (playerY + PLAYER_BOTTOM_BOUNDING_BOX > elfPosition[0].y) && (playerY + PLAYER_BOTTOM_BOUNDING_BOX < elfPosition[0].y + elfTexture.getSize ().y))  // player's vertical   range can touch the gift
			{
				elfPosition[0].x = 500;
				elfPosition[0].y = 700;
				hasGift = false;

				start = std::clock ();
				enableDrawGift = false;
				enableDrawElf = false;

				score += 1;
				scoreText.setString ("Score: " + std::to_string (score));
				sound2.play ();
			}
		}

		player.setPosition (playerX, playerY);

		window.draw (background);
		window.draw (player);

		// set and draw platforms
		for (size_t i = 0; i < 10; ++i)
		{
			platform.setPosition (platformPosition[i].x, platformPosition[i].y);
			window.draw (platform);
		}

		if (!hasGift && enableDrawGift) {
			gift.setPosition (giftPosition[0].x, giftPosition[0].y);
			window.draw (gift);
		}

		if (hasGift && enableDrawElf) {
			elf.setPosition (elfPosition[0].x, elfPosition[0].y);
			window.draw (elf);
		}

		// game over
		if (playerY > 700)
		{
			gameoverText.setPosition (30, 150);
			scoreText.setPosition (150, 300);
			newgame.setPosition (0, 500);
			goto gameover;
		}
		window.draw (scoreText);
		window.display ();
	}

	// Game Over
gameover:
	while (window.isOpen ())
	{
		sf::Event event;
		while (window.pollEvent (event))
		{
			if (event.type == sf::Event::Closed)
				window.close ();
		}
		window.draw (gameoverBackground);
		window.draw (gameoverText);
		window.draw (scoreText);
		window.draw (newgame);
		window.display ();
		if (sf::Keyboard::isKeyPressed (sf::Keyboard::N))
			goto newgame;
	}
	return 0;
}