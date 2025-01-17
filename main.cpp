#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <deque>
#include <string>
#include <ctime>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int SNAKE_SIZE = 20;
const int MAX_OBSTACLES = 5;
const int WALL_THICKNESS = 20;

// Directions
enum Direction { UP, DOWN, LEFT, RIGHT };
Direction dir = RIGHT;

struct SnakeSegment {
    int x, y;
};

std::deque<SnakeSegment> snake;
int appleX, appleY;
int score = 0;
bool gameOver = false;
bool gameStarted = false;
SDL_Rect obstacles[MAX_OBSTACLES];
SDL_Rect startButton = {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 25, 200, 50};

void spawnApple() {
    bool validPosition;
    do {
        validPosition = true;
        appleX = (rand() % ((WINDOW_WIDTH - 2 * WALL_THICKNESS) / SNAKE_SIZE) + 1) * SNAKE_SIZE;
        appleY = (rand() % ((WINDOW_HEIGHT - 2 * WALL_THICKNESS) / SNAKE_SIZE) + 1) * SNAKE_SIZE;

        // Ensure apple is not on any obstacle
        for (int i = 0; i < MAX_OBSTACLES; ++i) {
            if (appleX == obstacles[i].x && appleY == obstacles[i].y) {
                validPosition = false;
                break;
            }
        }
    } while (!validPosition || appleX < WALL_THICKNESS || appleX >= WINDOW_WIDTH - WALL_THICKNESS ||
             appleY < WALL_THICKNESS || appleY >= WINDOW_HEIGHT - WALL_THICKNESS);
}

void spawnObstacles() {
    for (int i = 0; i < MAX_OBSTACLES; ++i) {
        int x, y;
        bool validPosition;
        do {
            validPosition = true;
            x = (rand() % ((WINDOW_WIDTH - 2 * WALL_THICKNESS) / SNAKE_SIZE) + 1) * SNAKE_SIZE;
            y = (rand() % ((WINDOW_HEIGHT - 2 * WALL_THICKNESS) / SNAKE_SIZE) + 1) * SNAKE_SIZE;

            // Ensure obstacle is not on the apple
            if (x == appleX && y == appleY) {
                validPosition = false;
            }

            // Ensure obstacle is not on another obstacle
            for (int j = 0; j < i; ++j) {
                if (x == obstacles[j].x && y == obstacles[j].y) {
                    validPosition = false;
                    break;
                }
            }
        } while (!validPosition || x < WALL_THICKNESS || x >= WINDOW_WIDTH - WALL_THICKNESS ||
                 y < WALL_THICKNESS || y >= WINDOW_HEIGHT - WALL_THICKNESS);

        obstacles[i] = {x, y, SNAKE_SIZE, SNAKE_SIZE};
    }
}

void initGame() {
    snake.clear();
    snake.push_front({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2});
    dir = RIGHT;
    score = 0;
    gameOver = false;

    spawnApple();
    spawnObstacles();
}

void handleEvents(SDL_Event &e) {
    if (e.type == SDL_QUIT) {
        gameOver = true;
    } else if (e.type == SDL_KEYDOWN && gameStarted) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:    if (dir != DOWN) dir = UP; break;
            case SDLK_DOWN:  if (dir != UP) dir = DOWN; break;
            case SDLK_LEFT:  if (dir != RIGHT) dir = LEFT; break;
            case SDLK_RIGHT: if (dir != LEFT) dir = RIGHT; break;
        }
    } else if (e.type == SDL_MOUSEBUTTONDOWN && !gameStarted) {
        if (e.button.x >= startButton.x && e.button.x <= startButton.x + startButton.w &&
            e.button.y >= startButton.y && e.button.y <= startButton.y + startButton.h) {
            gameStarted = true;
            initGame();
        }
    }
}

bool checkCollisionWithSelf() {
    for (size_t i = 1; i < snake.size(); ++i) {
        if (snake[i].x == snake.front().x && snake[i].y == snake.front().y) return true;
    }
    return false;
}

bool checkCollisionWithObstacles() {
    for (int i = 0; i < MAX_OBSTACLES; ++i) {
        if (snake.front().x == obstacles[i].x && snake.front().y == obstacles[i].y) return true;
    }
    return false;
}

bool checkCollisionWithApple() {
    return snake.front().x == appleX && snake.front().y == appleY;
}

bool checkCollisionWithWalls() {
    return (snake.front().x < WALL_THICKNESS || snake.front().x >= WINDOW_WIDTH - WALL_THICKNESS ||
            snake.front().y < WALL_THICKNESS || snake.front().y >= WINDOW_HEIGHT - WALL_THICKNESS);
}

void updateSnake() {
    SnakeSegment newHead = snake.front();

    switch (dir) {
        case UP: newHead.y -= SNAKE_SIZE; break;
        case DOWN: newHead.y += SNAKE_SIZE; break;
        case LEFT: newHead.x -= SNAKE_SIZE; break;
        case RIGHT: newHead.x += SNAKE_SIZE; break;
    }

    snake.push_front(newHead);

    if (checkCollisionWithApple()) {
        spawnApple();
        score++;
    } else {
        snake.pop_back();
    }

    if (checkCollisionWithWalls() || checkCollisionWithSelf() || checkCollisionWithObstacles()) {
        gameOver = true;
    }
}

void drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string &text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect destRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void drawGame(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw boundary walls
    SDL_SetRenderDrawColor(renderer, 150, 0, 0, 255);
    SDL_Rect topWall = {0, 0, WINDOW_WIDTH, WALL_THICKNESS};
    SDL_Rect bottomWall = {0, WINDOW_HEIGHT - WALL_THICKNESS, WINDOW_WIDTH, WALL_THICKNESS};
    SDL_Rect leftWall = {0, 0, WALL_THICKNESS, WINDOW_HEIGHT};
    SDL_Rect rightWall = {WINDOW_WIDTH - WALL_THICKNESS, 0, WALL_THICKNESS, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &topWall);
    SDL_RenderFillRect(renderer, &bottomWall);
    SDL_RenderFillRect(renderer, &leftWall);
    SDL_RenderFillRect(renderer, &rightWall);

    // Draw snake
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (const auto &segment : snake) {
        SDL_Rect rect = {segment.x, segment.y, SNAKE_SIZE, SNAKE_SIZE};
        SDL_RenderFillRect(renderer, &rect);
    }

    // Draw apple
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect appleRect = {appleX, appleY, SNAKE_SIZE, SNAKE_SIZE};
    SDL_RenderFillRect(renderer, &appleRect);

    // Draw obstacles
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    for (int i = 0; i < MAX_OBSTACLES; ++i) {
        SDL_RenderFillRect(renderer, &obstacles[i]);
    }

    // Display score
    SDL_Color white = {255, 255, 255, 255};
    drawText(renderer, font, "Score: " + std::to_string(score), 10, 10, white);

    SDL_RenderPresent(renderer);
}

void drawStartScreen(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw "Start" button
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &startButton);

    SDL_Color white = {255, 255, 255, 255};
    drawText(renderer, font, "Start", startButton.x + 60, startButton.y + 10, white);

    SDL_RenderPresent(renderer);
}

void drawGameOverScreen(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color white = {255, 255, 255, 255};
    drawText(renderer, font, "Game Over", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 50, white);
    drawText(renderer, font, "Final Score: " + std::to_string(score), WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2, white);

    SDL_RenderPresent(renderer);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", 24); // Replace with actual font path
    if (!font) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    srand(static_cast<unsigned int>(time(0)));

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            handleEvents(e);
            if (e.type == SDL_QUIT) quit = true;
        }

        if (gameStarted) {
            if (!gameOver) {
                updateSnake();
                drawGame(renderer, font);
            } else {
                drawGameOverScreen(renderer, font);
                gameStarted = false;
            }
        } else {
            drawStartScreen(renderer, font);
        }

        SDL_Delay(100);  // Adjust delay for smoother animations
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
