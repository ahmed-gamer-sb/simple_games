#include <bits/fs_fwd.h>
#include <cstddef>
#include <iostream>
#include <deque>
#include <raylib.h>
#include <raymath.h>

using namespace std;

int score{};
const int cell = 30;
const int cellSize = 25;
const Color green{173, 204, 96, 255};
const Color dark_green{43, 51, 24, 255};
const Color debug_color{255, 51, 24, 255};
const Color score_color{255, 255, 255, 155};

double lastUpdateTime{};

bool addSegment = false;
bool hold = false;
const Font font{};
const float font_size = 16.0f;


bool TrigerEvent(double interval) {
	double current = GetTime();
	if (current - lastUpdateTime > interval) {lastUpdateTime = current; return true;}
	
	return false;
}


bool ElementInDeque(Vector2 pos, deque<Vector2> deq) {
	for (size_t i{}; i < deq.size(); i++) {
		if (Vector2Equals(pos, deq[i]))  return true;
	}
	return false;
}


void CheckForHold(Vector2 head) {
	if (head.x < 0 or head.x >= cell or head.y < 0 or head.y >= cell) {
		hold = true;
	} else {
		hold = false;
	}
}


struct Snake {

	void Draw() {
		float x = body[0].x;
		float y = body[0].y;
		Rectangle rectangle{x * cellSize, y * cellSize, cellSize, cellSize};
		DrawRectangleRounded(rectangle, .5f, 10.f, debug_color);
		for (size_t i{1}; i < body.size(); i++) {
			x = body[i].x;
			y = body[i].y;
			Rectangle rectangle{x * cellSize, y * cellSize, cellSize, cellSize};
			DrawRectangleRounded(rectangle, .5f, 10.f, dark_green);
		}
	}

	void update() {
		direction = hold_direction;
		body.push_front(Vector2Add(body[0], direction));
		if (addSegment == true) {addSegment = false; }
		else { body.pop_back(); }

		if (body[0].x < -1) { body[0] = Vector2{cell + 2, body[0].y};}
		else if (body[0].x > cell + 1) { body[0] = Vector2{-2, body[0].y};}

		if (body[0].y < -1) { body[0] = Vector2{body[0].x, cell + 2};}
		else if (body[0].y > cell + 1) { body[0] = Vector2{body[0].x, -2};}
	}

	bool death() {
		for (size_t i{}; i < body.size(); i++) {
			if (Vector2Equals(body[0], body[i]) && i != 0) {
				return true;
				//cout << "Hello you eat you self\n";
			}
		}
		return false;
	}

	deque<Vector2> initial_body{ Vector2{8, 9}, Vector2{7, 9}, Vector2{6, 9}};
	deque<Vector2> body{ initial_body };
	Vector2 direction{1, 0};
	Vector2 hold_direction{1, 0};

};


struct Food {
public:
	Food(Snake& snake) {
		pos = GenrateRandomPos();
		snake = snake;
		Image image = LoadImage("./data/imgs/food.png");
		texture = LoadTextureFromImage(image);
		UnloadImage(image);
	}

	~Food() {
		UnloadTexture(texture);
	}

	Vector2 GenerateRandomCell() {
		int x = GetRandomValue(0, cell - 1);
		int y = GetRandomValue(0, cell - 1);
		return Vector2{(float)x, (float)y};
	}

	Vector2 GenrateRandomPos() {
		Vector2 position = GenerateRandomCell();
		while (ElementInDeque(position, snake.body)) {
			position = GenerateRandomCell();
		}
		//cout << position.x << " " << position.y << "\n";
		return position;
	}

	void Draw() {
		DrawTexture(texture, pos.x * cellSize, pos.y * cellSize, WHITE);
	}

	Vector2 pos{};
	Texture2D texture;
	Snake snake;
};


struct Game{
	Game(): food{snake} {
		InitAudioDevice();
		eat_sound = LoadSound("./data/sounds/eat.mp3");
		wall_sound = LoadSound("./data/sounds/wall.mp3");
	}
	
	~Game() {
		UnloadSound(eat_sound);
		UnloadSound(wall_sound);
		CloseAudioDevice();
	}
	
	bool CheckCollisionFood() {
		if (Vector2Equals(food.pos, snake.body[0])) {
			food.pos = food.GenrateRandomPos();
			score++;
			PlaySound(eat_sound);
			
			return true;
		}
		return false;
	}

	void GameOver(Snake& snake, Food& food) {
		PlaySound(wall_sound);
		WaitTime(0.3);
		score = 0;
		food.pos = food.GenrateRandomPos();
		snake.body = snake.initial_body;
		snake.hold_direction = Vector2{1, 0};
		this->render();
		WaitTime(0.5);
	}

	void handle_events() {
		CheckForHold(snake.body[0]);

		if (IsKeyDown(KEY_LEFT)) {
			if (snake.direction.x != 1 && !hold ) { snake.hold_direction = Vector2{-1, 0};}
		}
		if (IsKeyDown(KEY_RIGHT)) {
			if (snake.direction.x != -1 && !hold ) { snake.hold_direction = Vector2{1, 0}; }
		}
		if (IsKeyDown(KEY_UP)) {
			if (snake.direction.y != 1 && !hold )  { snake.hold_direction = Vector2{0, -1}; }
		}
		if (IsKeyDown(KEY_DOWN)) {
			if (snake.direction.y != -1&& !hold ) { snake.hold_direction = Vector2{0, 1}; }
		}
		/*if (IsKeyPressed(KEY_SPACE)) {
			for (size_t i{}; i < snake.body.size(); i++) {
				cout<< snake.body[i].x << ";" << snake.body[i].y << "  ";
			}
			cout<< endl;
		}
		*/
	}

	void update() {
		addSegment = CheckCollisionFood();	
		snake.update();
		if (snake.death()) {
			GameOver(snake, food);
		}

	}

	void render() {
		BeginDrawing();
			ClearBackground(green);
			food.Draw();
			snake.Draw();
			DrawText(TextFormat("Score: %d", score), (cell / 2) * cellSize - 40, 5, 30, score_color);
			DrawFPS(5, 5);
		EndDrawing();
	}

	void run() {
		while (!WindowShouldClose()) {
			this->handle_events();
			this->render();
			if (TrigerEvent(0.2)) {
				hold = false;
				this->update();
			}
		}
	}

private:
	float dt{};
	Snake snake;
	Food food;
	Sound eat_sound;
	Sound wall_sound;
};

int main() {
	InitWindow(cell * cellSize, cell * cellSize, "My first cpp game");
	SetTargetFPS(60);
	Game game{};
	game.run();
	CloseWindow();


	return 0;
}
