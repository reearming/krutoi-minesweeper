#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


void set_mode(int raw) {
    static struct termios old_t, new_t;
    if (raw) {
        // Get current terminal attributes
        tcgetattr(STDIN_FILENO, &old_t); 
        new_t = old_t;
        
        // Disable canonical mode (line buffering) and local echo
        new_t.c_lflag &= ~(ICANON | ECHO); 
        
        // Set minimum characters to read to 1, and timeout to 0
        new_t.c_cc[VMIN] = 1;
        new_t.c_cc[VTIME] = 0;
        
        // Apply settings immediately
        tcsetattr(STDIN_FILENO, TCSANOW, &new_t);
    } else {
        // Restore original terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
    }
}

int main() 
{
    char map[100][100]; // Assuming a maximum size for the map
    char marked[100][100]; // Marked cells
    char bombs[100][100]; // Bomb locations
    char open[100][100]; // Cells with adjacent bomb counts
    int bombCount = 0; // Count of bombs will be placed
    int bombPercentage = 0; // Percentage of the map to be filled with bombs
    printf("Enter map width and height (e.g., 3x3). Must be less than 100x100 (default 25x10): ");
    int width, height;
    scanf("%dx%d", &width, &height);
    if (width > 0 && width <= 100 && height > 0 && height <= 100) {
        printf("Map size set to %dx%d.\n", width, height);
    }
    else if (width == '\0' || height == '\0') {
        printf("Invalid input. Using default size 25x10.\n");
        width = 25;
        height = 10;
    }
    else {
        printf("Invalid dimensions. Using default size 25x10.\n");
        width = 25;
        height = 10;
    }
    printf("Enter percent of the map will be replaced with bombs (0-100, default 16): ");
    scanf("%d", &bombPercentage);
    if (bombPercentage > 0 && bombPercentage <= 100) {
        printf("Bomb percentage set to %d%%.\n", bombPercentage);
    }
    else if (bombPercentage == '\0') {
        printf("Invalid input. Using default 16.\n");
        bombPercentage = 16;
    }
    else {
        printf("Invalid bomb percentage. Using default 16%%.\n");
        bombPercentage = 16;
    }
    bombCount = (width * height * bombPercentage) / 100; // Calculate number of bombs based on percentage
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            map[i][j] = '*'; // Initialize map with unopened cells
            open[i][j] = '0'; // Initialize cells with adjacent bomb counts
            bombs[i][j] = '\0'; // Initialize bomb locations
            marked[i][j] = '\0'; // Initialize marked cells
        }
    }

    for (int i = 0; i < bombCount; i++) {
        srand(time(NULL) + i); // Seed random number generator
        int bombX = rand() % width; // Random x-coordinate for bomb
        int bombY = rand() % height; // Random y-coordinate for bomb
        bombs[bombY][bombX] = 'd'; // Place bomb at random location
    }
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (bombs[i][j] == 'd') {
                open[i - 1][j - 1] = (char)((int)open[i - 1][j - 1] + 1); // Add to the count of adjacent bombs for the top-left cell
                open[i - 1][j] = (char)((int)open[i - 1][j] + 1); // Add to the count of adjacent bombs for the top cell
                open[i - 1][j + 1] = (char)((int)open[i - 1][j + 1] + 1); // Add to the count of adjacent bombs for the top-right cell
                open[i][j - 1] = (char)((int)open[i][j - 1] + 1); // Add to the count of adjacent bombs for the left cell
                open[i][j + 1] = (char)((int)open[i][j + 1] + 1); // Add to the count of adjacent bombs for the right cell
                open[i + 1][j - 1] = (char)((int)open[i + 1][j - 1] + 1); // Add to the count of adjacent bombs for the bottom-left cell
                open[i + 1][j] = (char)((int)open[i + 1][j] + 1); // Add to the count of adjacent bombs for the bottom cell
                open[i + 1][j + 1] = (char)((int)open[i + 1][j + 1] + 1); // Add to the count of adjacent bombs for the bottom-right cell
            }
        }
    }
    
    int c;   
    static struct termios oldt, newt;
    
    int selected = 0;

    set_mode(1); // Set terminal to raw mode

    bool allMatch = false; // Flag to check if all marked cells match bomb locations
    while (true) {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (i * width + j != selected) {
                    if (marked[i][j] == 'd') {
                        printf("F"); // Display flagged cell
                    } else if (marked[i][j] == open[i][j]) {
                        printf("%c", marked[i][j]); // Display opened cell
                    } else {
                    printf("%c", map[i][j]);
                    }
                }
                else {
                    if (marked[i][j] == 'd') {
                        printf("\033[31mF\033[0m"); // Display flagged cell in red
                    }
                    else if (marked[i][j] == open[i][j]) {
                        printf("\033[31m%c\033[0m", marked[i][j]); // Display opened cell in red
                    }
                    else {
                        printf("\033[31m%c\033[0m", map[i][j]); // Display unopened cell in red
                    }
                }
            }
            
            printf("\n"); // Print newline at the end of each row
        }

        printf("\n q-uit, h-left, j-down, k-up, l-right, x-open, d-mark\n");
        if (allMatch) {
            printf("Congratulations! You've marked all bombs correctly!\n");
            break; // Exit the game loop on successful marking
        }
        char input = getchar();
        //map[selected / width][selected % width] = '*'; // Reset previous selection
        int moveDirection = 0; // Variable to track movement direction
        if (input == 'q') {
            break;
        }
        else if (input == 'k' && selected > width - 1) {
            selected -= width;
            moveDirection = -2; // Moving up
        }
        else if (input == 'j' && selected < width * height - width) {
            selected += width;
            moveDirection = 2; // Moving down
        }
        else if (input == 'h' && selected % width > 0) {
            selected--;
            moveDirection = -1; // Moving left
        }
        else if (input == 'l' && selected % width < width - 1) {
            selected++;
            moveDirection = 1; // Moving right
        }
        else if (input == 'x') {
            if (marked[selected / width][selected % width] != 'd') { // Only open if not marked
                if (bombs[selected / width][selected % width] != 'd') { // boom
                    map[selected / width][selected % width] = open[selected / width][selected % width]; // Open selected cell
                }
                else {
                    printf("Boom! You hit a bomb at %dx%d\n", selected % width + 1, selected / width + 1);
                    break; // Exit the game loop on hitting a bomb
                }
            }
        }
        else if (input == 'd') {
            if (map[selected / width][selected % width] == '*') { // Only mark if not already opened
            
                if (marked[selected / width][selected % width] != 'd') {
                    marked[selected / width][selected % width] = 'd'; // Mark new selection
                }
                else {
                    marked[selected / width][selected % width] = '\0'; // Clear marked cell
                }
            }
        }

        // if (marked[selected / width][selected % width] != 'd') {
        //     map[selected / width][selected % width] = 'x'; // Mark new selection
        // }
        // else if (marked[selected / width][selected % width] == '0') {
        //     map[selected / width][selected % width] = 'o'; // Mark new selection
        // }
        // else if (marked[selected / width][selected % width] == 'd') {
        //     map[selected / width][selected % width] = 'f'; // Mark new selection
        // }
        printf("Selected: %dx%d\n", selected % width + 1, selected / width + 1); // Display selected position (1-based index)

        allMatch = true; // Reset match flag before checking
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                // Break early and return false on the first mismatch
                if (marked[i][j] != bombs[i][j]) {
                    allMatch = false;
                    break;
                }
            }
        }
        
    }
    set_mode(0); // Restore terminal to normal mode
    return 0;
};

