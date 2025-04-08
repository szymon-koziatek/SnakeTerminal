#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

bool GameOver;
const int WIDTH = 20;
const int HEIGHT = 20;
int x, y, fruitX, fruitY, score;
int tailX[100], tailY[100];
int nTail;
int speed = 500000;

enum eDirection {STOP = 0, LEFT, RIGHT, UP, DOWN};
eDirection dir;

Mix_Chunk *deathSound = nullptr;
Mix_Chunk *scoreSound = nullptr;
Mix_Music *backgroundMusic = nullptr;

void setup() {
    GameOver = false;
    dir = STOP;
    x = WIDTH / 2;
    y = HEIGHT / 2;
    fruitX = std::rand() % WIDTH;
    fruitY = std::rand() % HEIGHT;
    score = 0;
    nTail = 0;       // Reset tail length
    speed = 500000;  // Reset speed to initial value
}

bool initSDL() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (Mix_Init(MIX_INIT_MP3) != MIX_INIT_MP3) {
        std::cerr << "Mix_Init failed: " << Mix_GetError() << std::endl;
        return false;
    }

    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        std::cerr << "Mix_OpenAudio failed: " << Mix_GetError() << std::endl;
        return false;
    }

    return true;
}

void playMusic() {
    if (backgroundMusic) {
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }
    backgroundMusic = Mix_LoadMUS("assets/music.wav");
    if (!backgroundMusic) {
        std::cerr << "Failed to load background music: " << Mix_GetError() << std::endl;
        return;
    }
    Mix_PlayMusic(backgroundMusic, -1);
}

void stopMusic() {
    if (backgroundMusic) {
        Mix_HaltMusic();  // Stop the music when the game ends or restarts
    }
}

void loadSoundEffects() {
    deathSound = Mix_LoadWAV("assets/death.mp3");
    if (!deathSound) {
        std::cerr << "Error loading death sound: " << Mix_GetError() << std::endl;
    }

    scoreSound = Mix_LoadWAV("assets/score.mp3");
    if (!scoreSound) {
        std::cerr << "Error loading score sound: " << Mix_GetError() << std::endl;
    }
}

void playDeathSound() {
    if (deathSound) {
        Mix_PlayChannel(-1, deathSound, 0);  // Play the death sound
    }
}

void playScoreSound() {
    if (scoreSound) {
        Mix_PlayChannel(-1, scoreSound, 0);  // Play the score sound
    }
}

void closeSDL() {
    Mix_FreeChunk(deathSound);
    Mix_FreeChunk(scoreSound);
    Mix_FreeMusic(backgroundMusic);
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
}

void displayMenu() {
    std::cout << "\033[2J\033[1;1H";  // Clear the screen, preserving colors
    std::cout << "Snake Game!" << std::endl;
    std::cout << "---------------------" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "W - Move Up" << std::endl;
    std::cout << "A - Move Left" << std::endl;
    std::cout << "S - Move Down" << std::endl;
    std::cout << "D - Move Right" << std::endl;
    std::cout << "Q - Quit" << std::endl;
    std::cout << "Press Enter to Start" << std::endl;
}

void displayGameOverMenu() {
    std::cout << "\033[2J\033[1;1H";  // Clear the screen, preserving colors
    std::cout << "Game Over!" << std::endl;
    std::cout << "Score: " << score << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << "Press R to restart or Q to quit" << std::endl;
}

void draw() {
    std::cout << "\033[2J\033[1;1H";  // Clear screen but preserve color
    for (int i = 0; i < WIDTH + 2; i++) std::cout << "-";
    std::cout << std::endl;

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (j == 0)
                std::cout << "|";
            if (i == y && j == x)
                std::cout << "\033[1;32mÖ\033[0m";  // Snake head in green
            else if (i == fruitY && j == fruitX)
                std::cout << "\033[1;31mX\033[0m";  // Fruit in red
            else {
                bool print = false;
                for (int k = 0; k < nTail; k++) {
                    if (tailX[k] == j && tailY[k] == i) {
                        std::cout << "\033[1;33mo\033[0m";  // Tail in yellow
                        print = true;
                    }
                }
                if (!print)
                    std::cout << " ";
            }
            if (j == WIDTH - 1)
                std::cout << "|";
        }
        std::cout << std::endl;
    }

    for (int i = 0; i < WIDTH + 2; i++) std::cout << "-";
    std::cout << std::endl;
    std::cout << "Score: " << score << std::endl;
    std::cout << "Speed: " << std::fixed << std::setprecision(2) << 1.0 / static_cast<float>(speed) * 500000 << std::endl;
}

int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void input() {
    if (kbhit()) {
        char ch = getchar();

        if (ch == 'a') {
            dir = LEFT;
        } else if (ch == 'd') {
            dir = RIGHT;
        } else if (ch == 's') {
            dir = DOWN;
        } else if (ch == 'w') {
            dir = UP;
        } else if (ch == 'x') {
            GameOver = true;
        }
    }
}

void logic() {
    int prevX = tailX[0];
    int prevY = tailY[0];
    int prev2X, prev2Y;
    tailX[0] = x;
    tailY[0] = y;
    for (int i = 1; i < nTail; i++) {
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    switch (dir) {
    case LEFT:
        x--;
        break;
    case RIGHT:
        x++;
        break;
    case UP:
        y--;
        break;
    case DOWN:
        y++;
        break;
    default:
        break;
    }

    if (x >= WIDTH || x < 0 || y >= HEIGHT || y < 0)
        GameOver = true;
    for (int i = 0; i < nTail; i++)
        if (tailX[i] == x && tailY[i] == y)
            GameOver = true;

    if (x == fruitX && y == fruitY) {
        score += 10;
        speed = static_cast<int>(speed * 0.95);
        fruitX = std::rand() % WIDTH;
        fruitY = std::rand() % HEIGHT;
        nTail++;
        playScoreSound();  // Play score sound when eating fruit
    }
}

int main() {
    if (!initSDL()) {
        return -1;  // Exit if initialization fails
    }

    loadSoundEffects();  // Load sound effects
    displayMenu();

    // Wait for user input to start the game
    while (true) {
        if (kbhit()) {
            char ch = getchar();
            if (ch == '\n') break;  // Start game when Enter is pressed
            else if (ch == 'q') return 0;  // Quit the game when Q is pressed
        }
    }

    playMusic();  // Start playing background music after starting the game

    setup();  // Initialize game variables

    // Game loop
    while (!GameOver) {
        draw();
        input();
        logic();

        if (GameOver) {
            stopMusic();  // Stop the background music
            playDeathSound();  // Play the death sound
            displayGameOverMenu();
            while (true) {
                if (kbhit()) {
                    char ch = getchar();
                    if (ch == 'r') {
                        setup();  // Restart the game
                        playMusic();  // Restart music after reset
                        break;  // Exit game over loop
                    } else if (ch == 'q') {
                        closeSDL();  // Quit the game
                        return 0;
                    }
                }
            }
        }

        usleep(speed);  // Control the game speed
    }

    closeSDL();  // Clean up SDL resources
    return 0;
}
