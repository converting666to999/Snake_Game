#include <iostream>
#include "raylib.h"
#include <deque>
#include <raymath.h>
#define GAMEPAD_PLAYER1 0

using namespace std;

// custom colors 
Color Green_BG = { 173,204,96,255 };
Color Green_Snake = { 43, 51,24, 255 };
Color neon_pink = { 255, 50, 200, 255 };

const int cellSize = 30;
const int cellCount = 25;
const int offset = 75;
double LastUpdateTime = 0;
bool Add_Segment = false;

// this checks if desired interval is passed or not
// and update it as event triggered
bool EventTriggered(double interval)
{
	double CurrentTime = GetTime();
	if(CurrentTime - LastUpdateTime >= interval)
	{
		LastUpdateTime = CurrentTime;
		return true;
	}
	return false;
}

// food class 
class Food {
public: 
	Vector2 position;
	Texture2D Food_Texture;

	// constructor 
	Food()
	{
		Image image = LoadImage("Graphics/food.png");
		Food_Texture = LoadTextureFromImage(image);
		UnloadImage(image);
		position = GetRandomFoodPos();
	}

	// destructor
	~Food()
	{
		UnloadTexture(Food_Texture);
	}

	void DrawFood()
	{
		DrawTexture(Food_Texture,offset + position.x * cellSize,offset + position.y * cellSize, WHITE);
	}

	// generate random food position
	Vector2 GetRandomFoodPos()
	{
		float x = GetRandomValue(0, cellCount - 1);
		float y = GetRandomValue(0, cellCount - 1);

		return Vector2{ x,y };
	}
};

// snake class
class Snake
{
public : 
	// using deque to define the body of the snake
	std::deque<Vector2>  Snake_body = { Vector2{6,7},Vector2{5,7},Vector2{4,7} };
	Vector2 direction = { 1,0 };

	void DrawSnake()
	{
		for(unsigned int i = 0; i < Snake_body.size(); i++)
		{
			float x = Snake_body[i].x;
			float y = Snake_body[i].y;
			Rectangle body = {offset + x * cellSize,offset + y * cellSize,cellSize,cellSize };
			DrawRectangleRounded(body, 0.8, 6, Green_Snake);
		}
	}


	void UpdateSnakePos()
	{	if(Add_Segment == true)
		{	// it adds a new segment in front of the snake(head)
			// when it collides with food
			Snake_body.push_front(Vector2Add(Snake_body[0], direction));
			Add_Segment = false;
		}
		else
		{	
			Snake_body.pop_back();
			Snake_body.push_front(Vector2Add(Snake_body[0], direction));
		}
	}

	void ResetSnakePos()
	{
		Snake_body = { Vector2{6,7},Vector2{5,7},Vector2{4,7} };
		direction = { 1,0 };
	}

};

class Game
{
public: 
	
	Food food; 
	Snake snake;
	bool running = true;
	int score = 0;
	Sound Eatsound; 
	Sound Wallsound;

	Game()
	{	
		InitAudioDevice();
		Eatsound = LoadSound("Sounds/eat.mp3");
		Wallsound = LoadSound("Sounds/wall.mp3");
	}

	~Game()
	{
		UnloadSound(Eatsound);
		UnloadSound(Wallsound);
		CloseAudioDevice();
	}

	void DrawObjects()
	{
		food.DrawFood();
		snake.DrawSnake();
	}

	// checks the collision with food
	// is true then is randomizes the food pos
	void CheckCollisionWithFood()
	{
		if(Vector2Equals(snake.Snake_body[0],food.position))
		{
			food.position = food.GetRandomFoodPos();
			Add_Segment = true;
			score++;
			PlaySound(Eatsound);
		}
	}

	// it check collision of the snake head with head
	// when the above condition is true it pause the game
	// until the player press the enter key
	void CheckCollisionWithEdges()
	{
		if (snake.Snake_body[0].x == cellCount || snake.Snake_body[0].x == -1)
		{
			GameOver();
			PlaySound(Wallsound);
			SetGamepadVibration(GAMEPAD_PLAYER1, 1.0f, 2.0f, 2); // The vibration dosent work for some reason
		}
		else if(snake.Snake_body[0].y == cellCount || snake.Snake_body[0].y == -1)
		{
			GameOver();
			PlaySound(Wallsound);
			SetGamepadVibration(GAMEPAD_PLAYER1, 2.0f, 2.0f, 2);
		}
	}

	// checks the collision with snakes body
	void CheckCollisionWithSnakeBody()
	{
		deque<Vector2> headless_body = snake.Snake_body;
		headless_body.pop_front();

		for (unsigned int i = 0; i < headless_body.size(); i++)
		{	
			if(snake.Snake_body[0] == headless_body[i])
			{
				GameOver();
				PlaySound(Wallsound);
				return;

			}
		}
	}	

	void GameOver()
	{
		snake.ResetSnakePos();
		food.position = food.GetRandomFoodPos();
		running = false;
		score = 0;
	}

	// updates the game position
	void UpdatePos()
	{   if(running)
		{
		snake.UpdateSnakePos();
		CheckCollisionWithFood();
		CheckCollisionWithEdges();
		CheckCollisionWithSnakeBody();
		}
		else
		{
		DrawText("SKILL ISSUES", 335, 360, 30, WHITE);
		DrawText("PRESS ENTER TO START AGAIN", 200, 400, 30, WHITE);
		}
	}
};


int main()
{
	
	InitWindow(2*offset + cellSize*cellCount,2*offset + cellSize*cellCount,"Snake game");
	SetTargetFPS(60);

	Game game; // creating a game object


	//Game loop
	while(WindowShouldClose() != true )
	{
		
		//updating position
		if(EventTriggered(0.2))
		{
			game.UpdatePos();
		}

		// Event Handling (Keyboard input)
		if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) && game.snake.direction.y != 1)
		{
			game.snake.direction = { 0, -1 };
		}
		else if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S) && game.snake.direction.y != -1)
		{
			game.snake.direction = { 0,1 };
		}
		else if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) && game.snake.direction.x != -1)
		{
			game.snake.direction = { 1,0 };
		}
		else if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A) && game.snake.direction.x != 1)
		{
			game.snake.direction = { -1,0 };

		}
		else if(IsKeyDown(KEY_ENTER))
		{
			game.running = true;
		}

		// Event handling gamepad
		if(IsGamepadAvailable(GAMEPAD_PLAYER1))
		{
			if(IsGamepadButtonPressed(GAMEPAD_PLAYER1,GAMEPAD_BUTTON_LEFT_FACE_UP) && game.snake.direction.y != 1)
			{
				game.snake.direction = { 0,-1 };
			}
			else if(IsGamepadButtonPressed(GAMEPAD_PLAYER1,GAMEPAD_BUTTON_LEFT_FACE_DOWN) && game.snake.direction.y != -1)
			{
				game.snake.direction = { 0,1 };
			}
			else if(IsGamepadButtonPressed(GAMEPAD_PLAYER1,GAMEPAD_BUTTON_LEFT_FACE_RIGHT) && game.snake.direction.x != -1)
			{
				game.snake.direction = { 1,0 };
			}
			else if(IsGamepadButtonPressed(GAMEPAD_PLAYER1,GAMEPAD_BUTTON_LEFT_FACE_LEFT) && game.snake.direction.x != 1)
			{
				game.snake.direction = { -1,0 };
			}
			else if(IsGamepadButtonPressed(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_MIDDLE_RIGHT))
			{
				game.running = true;
			}
		}
		


		// Drawing Object
		BeginDrawing();
		ClearBackground(neon_pink);
		DrawRectangleLinesEx(Rectangle{ offset - 5, offset - 5,cellSize*cellCount + 10, cellSize*cellCount + 10}, 5, BLACK);
		DrawText("Snake the Pussy Slayer", offset - 5, 20, 40, BLACK);
		DrawText(TextFormat("%i",game.score), offset + 105*7, 20, 40, BLACK);
		game.DrawObjects();

		EndDrawing();

	}

	CloseWindow();
	return 0;
}