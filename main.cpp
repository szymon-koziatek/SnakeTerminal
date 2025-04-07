#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <iomanip>
#include <ctime>

bool GameOver;
const int WIDTH = 20;
const int HEIGHT = 20;
int x, y, fruitX, fruitY, score;
int tailX[100], tailY[100];
int nTail;
int speed = 500000;

enum eDirection {STOP = 0, LEFT, RIGHT, UP, DOWN};
eDirection dir;

void setup(){
    GameOver = false;
    dir = STOP;
    x = WIDTH / 2;
    y = HEIGHT / 2;
    fruitX = std::rand() % WIDTH;
    fruitY = std::rand() % HEIGHT;
    score = 0;
}

void draw(){
    system("clear");
    for (int i = 0; i < WIDTH+2; i++)
        std::cout << "-";    
    std::cout << std::endl;

    for (int i = 0; i < HEIGHT; i++){
        for (int j = 0; j < WIDTH; j++){
            if (j == 0)
                std::cout << "|";
            if (i == y && j == x)
                std::cout << "O";
            else if (i == fruitY && j == fruitX)
                std::cout << "X";
            else{
                bool print = false;
                for (int k = 0; k < nTail; k++){
                    
                    if(tailX[k] == j && tailY[k] == i){
                        std::cout << "O";
                        print = true;
                    }
                    
                }
                if (!print)
                    std::cout << " ";
            }


            if (j == WIDTH-1)
                std::cout << "|";
        }
        std::cout << std::endl;
    }

    for (int i = 0; i < WIDTH+2; i++)
        std::cout << "-";
    std::cout << std::endl;
    std::cout << "Score:" << score << std::endl;
    float displaySpeed = 1.0 / static_cast<float>(speed) * 500000;
    std::cout << "Speed: " << std::fixed << std::setprecision(2) << displaySpeed << std::endl;

}

int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);  // Get the current terminal settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);  // Turn off canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // Set the new terminal settings
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);  // Get current file descriptor flags
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);  // Make the input non-blocking

    ch = getchar();  // Try to get a character

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // Restore the old terminal settings
    fcntl(STDIN_FILENO, F_SETFL, oldf);  // Restore the old file descriptor flags

    if (ch != EOF) {
        ungetc(ch, stdin);  // Put the character back if a key was pressed
        return 1;
    }

    return 0;  // No key was pressed
}


void input() {
    if (kbhit()) {
        char ch = getchar();

        // Check for out-of-bounds before allowing direction change
        if (ch == 'a' && x > 0) {  // Move left, but not past the left boundary
            dir = LEFT;
        } else if (ch == 'd' && x < WIDTH - 1) {  // Move right, but not past the right boundary
            dir = RIGHT;
        } else if (ch == 's' && y < HEIGHT - 1) {  // Move down, but not past the bottom boundary
            dir = DOWN;
        } else if (ch == 'w' && y > 0) {  // Move up, but not past the top boundary
            dir = UP;
        } else if (ch == 'x') {  // End the game
            GameOver = true;
        }
    }
}



void logic(){
    int prevX = tailX[0];
    int prevY = tailY[0];
    int prev2X, prev2Y;
    tailX[0] = x;
    tailY[0] = y;
    for (int i = 1; i < nTail; i++){
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }
    switch (dir){
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
    if (x > WIDTH || x < 0 || y > HEIGHT || y < 0)
        GameOver = true;
    for (int i = 0; i < nTail; i++)
        if (tailX[i] == x && tailY[i] == y)
            GameOver = true;

    if (x == fruitX && y == fruitY){
        score += 10;
        speed = static_cast<int>(speed * 0.95);
        fruitX = std::rand() % WIDTH;
        fruitY = std::rand() % HEIGHT;
        nTail++;
    }
}

int main(){
    std::srand(static_cast<unsigned int>(std::time(0)));
    setup();
    while(!GameOver){
        draw();
        input();
        logic();
        usleep(speed);
    }

    return 0;
}
