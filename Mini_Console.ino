// Below are links I used for testing the display and RFID, any code I referenced from them will have a comment added to them
// TFT Display
// https://youtu.be/R_V_lzAbnb8
// https://www.programmingboss.com/2021/01/arduino-mega-tft-lcd-color-display-shield.html#gsc.tab=0
// MFRC522 RFID
// https://youtu.be/hxQYIwdZRng
#include <SPI.h>
#include <MFRC522.h>
#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_KBV.h> //Hardware-specific library

LCDWIKI_KBV mylcd(ILI9486, 40, 38, 39, -1, 41); //model,cs,cd,wr,rd,reset

// Pre-defined colors for display
#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  0xFBE4
#define SKYBLUE 0x867D
#define INDIGO  0x4008

#define TFT_CS    10
#define TFT_DC    9
#define RFID_CS   8

#define SS_PIN 53
#define RST_PIN 49

MFRC522 rfid(SS_PIN, RST_PIN);

// Joystick related variables
const int xAxis = A0;
const int yAxis = A1;
const int stickclick = 2;

const int upright = 900;
const int downleft = 100;

bool analogG = true;
bool up = false;
bool down = false;
bool left = false;
bool right = false;


// RFID related variables
String uid = "";
const String game1Scan = "7ae7d280";
const String game2Scan = "a9586c16";

// Display related variables
int countdown = 0;
bool once = true;
bool menu = true;

// Game related variables
bool end_game = false;
// Tic-Tac-Toe
bool game_1 = false;
bool first = true;
bool draw = false;
bool comp_first = false;
bool pvp = false;
bool pvc = false;
bool AI = false;
int TTT_Row = 0;
int TTT_Col = 0;
int temp_Row = 0;
int temp_Col = 0;
const int MAX_Depth = 9;
String pieceAI = "";
String pieceP = "";
bool Board[3][3] = { { false, false, false },
                     { false, false, false },
                     { false, false, false } };
bool X[3][3] = { { false, false, false },
                 { false, false, false },
                 { false, false, false } };
bool O[3][3] = { { false, false, false },
                 { false, false, false },
                 { false, false, false } };
const int xBoard[3] = { 153, 225, 295 };
const int yBoard[3] = { 50, 140, 230 };
// Snake
bool game_2 = false;
const int snakeLen = 5;
int score = 0;
int snakeRowPos[snakeLen] = {};
int snakeColPos[snakeLen] = {};
int direction = 1;
int s_Column = 20;
int s_Row = 20;
int tempCol = 0;
int tempRow = 0;
int foodRow = 0;
int foodCol = 0;

// Joystick related helper functions
// Direction input from Joystick
void joystick_direction() {
    up = false;
    down = false;
    left = false;
    right = false;

    if (analogRead(yAxis) < downleft) { //left
        left = true;
        if (analogRead(xAxis) > upright) { //up-left
            up = true;
        } else if (analogRead(xAxis) < downleft) { //down-left
            down = true;
        }
    } else if (analogRead(yAxis) > upright) { //right
        right = true;
        if (analogRead(xAxis) > upright) { //up-right
            up = true;
        }
        else if (analogRead(xAxis) < downleft) { //down-right
            down = true;
        }
    } else {
        if (analogRead(xAxis) > upright) { //up
            up = true;
        }
        else if (analogRead(xAxis) < downleft) { //down
            down = true;
        }
    }
}

// RFID related helper functions
// Reads in RFID when scanned
void rfidScan() {
    uid = "";

    // Got this part from the video of the MFRC522
    for (byte i = 0; i < rfid.uid.size; i++) {
        uid += String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        uid += String(rfid.uid.uidByte[i], HEX);
    }
    uid.replace(" ", "");
    Serial.println(uid);

    if (uid == game1Scan) {
        game_1 = true;
        once = true;
        menu = false;
    } else if (uid == game2Scan) {
        game_2 = true;
        once = true;
        menu = false;
    }
    else {
        mylcd.Fill_Screen(BLACK);
        mylcd.Set_Text_colour(RED);
        mylcd.Set_Text_Size(6);
        mylcd.Print_String("Invalid", 150, 45);
        mylcd.Print_String("Game", 150, 100);

        countdown++;
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}

// Tic-Tac-Toe related helper functions
// Check to see if either player has won
bool checkWin(bool validating[3][3]) {
    // check if win by row
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (!validating[i][j]) break;

            if (j == 2) return true;
        }
    }

    // check if win by col
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (!validating[j][i]) break;

            if (j == 2) return true;
        }
    }

    // check if win by diagonal
    for (int i = 0; i < 3; i++) {
        if (!validating[i][i]) break;

        if (i == 2) return true;
    }
    for (int i = 0; i < 3; i++) {
        if (!validating[i][2 - i]) break;

        if (i == 2) return true;
    }

    return false;
}

// Checks to see if a player can move depending on the joystick input
void TTT_Move(String turn) {
    temp_Row = TTT_Row;
    temp_Col = TTT_Col;

    if (up) {
        if (TTT_Row > 0 &&
           (!Board[TTT_Row - 1][TTT_Col] ||
           (left && ((TTT_Col > 0 && !Board[TTT_Row - 1][TTT_Col - 1]) || (TTT_Col > 1 && !Board[TTT_Row - 1][TTT_Col - 2]))) ||
           (right && ((TTT_Col < 2 && !Board[TTT_Row - 1][TTT_Col + 1]) || (TTT_Col < 1 && !Board[TTT_Row - 1][TTT_Col + 2]))))) {
            once = true;
            TTT_Row--;
        } else if (TTT_Row > 1 &&
                  (!Board[TTT_Row - 2][TTT_Col] ||
                  (left && ((TTT_Col > 0 && !Board[TTT_Row - 2][TTT_Col - 1]) || (TTT_Col > 1 && !Board[TTT_Row - 2][TTT_Col - 2]))) ||
                  (right && ((TTT_Col < 2 && !Board[TTT_Row - 2][TTT_Col + 1]) || (TTT_Col < 1 && !Board[TTT_Row - 2][TTT_Col + 2]))))) {
            once = true;
            TTT_Row -= 2;
        }
    } else if (down) {
        if (TTT_Row < 2 &&
           (!Board[TTT_Row + 1][TTT_Col] ||
           (left && ((TTT_Col > 0 && !Board[TTT_Row + 1][TTT_Col - 1]) || (TTT_Col > 1 && !Board[TTT_Row + 1][TTT_Col - 2]))) ||
           (right && ((TTT_Col < 2 && !Board[TTT_Row + 1][TTT_Col + 1]) || (TTT_Col < 1 && !Board[TTT_Row + 1][TTT_Col + 2]))))) {
            once = true;
            TTT_Row++;
        } else if (TTT_Row < 1 &&
                  (!Board[TTT_Row + 2][TTT_Col] ||
                  (left && ((TTT_Col > 0 && !Board[TTT_Row + 2][TTT_Col - 1]) || (TTT_Col > 1 && !Board[TTT_Row + 2][TTT_Col - 2]))) ||
                  (right && ((TTT_Col < 2 && !Board[TTT_Row + 2][TTT_Col + 1]) || (TTT_Col < 1 && !Board[TTT_Row + 2][TTT_Col + 2]))))) {
            once = true;
            TTT_Row += 2;
        }
    }
    if (left) {
        if (TTT_Col > 0 && !Board[TTT_Row][TTT_Col - 1]) {
            once = true;
            TTT_Col--;
        } else if (TTT_Col > 1 && !Board[TTT_Row][TTT_Col - 2]) {
            once = true;
            TTT_Col -= 2;
        }
    } else if (right) {
        if (TTT_Col < 2 && !Board[TTT_Row][TTT_Col + 1]) {
            once = true;
            TTT_Col++;
        } else if (TTT_Col < 1 && !Board[TTT_Row][TTT_Col + 2]) {
            once = true;
            TTT_Col += 2;
        }
    }

    if (once) {
        mylcd.Set_Text_colour(BLACK);
        mylcd.Print_String(turn, xBoard[temp_Col], yBoard[temp_Row]);

        mylcd.Set_Text_colour(YELLOW);
        mylcd.Print_String(turn, xBoard[TTT_Col], yBoard[TTT_Row]);

        once = false;
    }
}

// Checks to see if the board is filled up after seeing no winner
bool TTT_Draw() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (!Board[i][j]) return false;

            if (i == 2 && j == 2) return true;
        }
    }
}

// Moves cursor to next available spot after a turn
void TTT_Spot() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (!Board[i][j]) {
                TTT_Row = i;
                TTT_Col = j;
                break;
            }
        }
        if (!Board[TTT_Row][TTT_Col]) break;
    }
}

// AI helper functions
void moveAI() {
    int bestScore = -1000;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (!Board[i][j]) {
                Board[i][j] = true;
                X[i][j] = true;
                int score = minimaxAI(0, false);
                Board[i][j] = false;
                X[i][j] = false;
                if (score > bestScore) {
                    bestScore = score;
                    TTT_Row = i;
                    TTT_Col = j;
                }
            }
        }
    }
}

int minimaxAI(int depth, bool isMaximizing) {
    if (checkWin(X)) {
        return 1;
    } else if (checkWin(O)) {
        return -1;
    } else if (TTT_Draw()) {
        return 0;
    } else if (depth == MAX_Depth) {
        return 0;
    }

    if (isMaximizing) {
        int bestScore = -1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (!Board[i][j]) {
                    Board[i][j] = true;
                    X[i][j] = true;
                    int score = minimaxAI(depth + 1, false);
                    Board[i][j] = false;
                    X[i][j] = false;
                    bestScore = max(score, bestScore);
                }
            }
        }
        return bestScore;
    } else {
        int worstScore = 1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (!Board[i][j]) {
                    Board[i][j] = true;
                    O[i][j] = true;
                    int score = minimaxAI(depth + 1, true);
                    Board[i][j] = false;
                    O[i][j] = false;
                    worstScore = min(score, worstScore);
                }
            }
        }
        return worstScore;
    }
}

// Snake related helper functions
// Spwans the food for snake one board that's not on top of the body
void spawnFood() {
    bool check = true;

    while (check) {
        foodCol = (millis() % 23) * 20;
        foodRow = (millis() % 15) * 20;
        check = false;

        if (s_Column == foodCol && s_Row == foodRow) check = true;
        if (!check && score != 0) {
            for (int i = 0; i < score; i++) {
                if (snakeRowPos[i] == foodRow && snakeColPos[i] == foodCol) {
                    check = true;
                    break;
                }
            }
        }
    }

    mylcd.Set_Text_colour(RED);
    mylcd.Print_String("*", foodCol, foodRow);
}

// Redraws the snake every run as it keeps moving
void redrawSnake() {
    mylcd.Set_Text_colour(ORANGE);
    mylcd.Print_String("*", s_Column, s_Row);

    mylcd.Set_Text_colour(BLACK);
    if (score == 0) {
        mylcd.Print_String("*", tempCol, tempRow);
    }
    else {
        mylcd.Print_String("*", snakeColPos[score - 1], snakeRowPos[score - 1]);

        for (int i = score - 1; i > 0; i--) {
            snakeRowPos[i] = snakeRowPos[i - 1];
            snakeColPos[i] = snakeColPos[i - 1];
        }
        snakeRowPos[0] = tempRow;
        snakeColPos[0] = tempCol;
    }
}

// Keeps moving the snake in the direction based on the last input
void SNAKE_Move() {
    if (down && direction != 2) {
        direction = 1;
    }
    else if (up && direction != 1) {
        direction = 2;
    }
    else if (right && direction != 4) {
        direction = 3;
    }
    else if (left && direction != 3) {
        direction = 4;
    }

    tempRow = s_Row;
    tempCol = s_Column;

    if (direction == 1) {
        s_Row += 20;
    }
    else if (direction == 2) {
        s_Row -= 20;
    }
    else if (direction == 3) {
        s_Column += 20;
    }
    else if (direction == 4) {
        s_Column -= 20;
    }
}

// Checks if the head of the snake reached the food
void checkIfScored() {
    if (s_Column == foodCol && s_Row == foodRow) {
        score++;
        once = true;
    }
}

// Check if the player lost the game (either out of bounds or hit it's own body
void lostgame() {
    if (s_Row < 0 || s_Row > 300 || s_Column < 0 || s_Column > 460) {
        end_game = true;
        once = true;
        return;
    }

    for (int i = 0; i < score; i++) {
        if (s_Column == snakeColPos[i] && s_Row == snakeRowPos[i]) {
            end_game = true;
            once = true;
            return;
        }
    }
}

typedef struct task {
    int state;
    unsigned long period;
    unsigned long elapsedTime;
    int (*TickFct)(int);

} task;

const unsigned short tasksNum = 5;
task tasks[tasksNum];

// Joystick related SM
enum JS_States { JS_INIT, JS_Read };
int JS_TICK(int state) {
    switch (state) { // State transitions
    case JS_INIT:
        state = JS_Read;
        break;
    case JS_Read:
        state = JS_Read;
        break;
    default:
        state = JS_INIT;
        break;
    }
    switch (state) { // State Action
    case JS_Read:
        joystick_direction();
        break;
    default:
        break;
    }

    return state;
}

// RFID related SM
enum RFID_States { RFID_INIT, RFID_Read, RFID_Wait };
int RFID_Tick(int state) {
    //Read thing
    switch (state) { // State transitions
    case RFID_INIT:
        state = RFID_Read;
        break;
    case RFID_Read:
        if (!menu) {
            state = RFID_Wait;
        }
        break;
    case RFID_Wait:
        if (menu) {
            state = RFID_Read;
        }
    default:
        state = RFID_INIT;
        break;
    }
    switch (state) { // State Action
    case RFID_Read:
        if (rfid.PICC_IsNewCardPresent()
            && rfid.PICC_ReadCardSerial()) {
            rfidScan();
        }
        break;
    default:
        break;
    }

    return state;
}

// Display menu related SM
enum MAIN_States { MAIN_INIT, MAIN_Wait, MAIN_Standby };
int MAIN_Tick(int state) {
    switch (state) { // State transitions
    case MAIN_INIT:
        state = MAIN_Wait;
        break;
    case MAIN_Wait:
        if (!menu) {
            state = MAIN_Standby;
        }
        break;
    case MAIN_Standby:
        if (menu) {
            state = MAIN_Wait;
        }
        break;
    default:
        state = MAIN_INIT;
        break;
    }
    switch (state) { // State Action
    case MAIN_Wait:
        if (once || countdown >= 8) {
            mylcd.Fill_Screen(WHITE);
            mylcd.Set_Text_colour(GREEN);
            mylcd.Set_Text_Size(6);
            mylcd.Print_String("Scan a", 150, 45);
            mylcd.Print_String("Game", 150, 100);

            once = false;
            countdown = 0;
        }
        if (countdown > 0) countdown++;
        break;
    default:
        break;
    }

    return state;
}

// Tic-Tac-Toe related SM
enum TTT_States { TTT_INIT, TTT_Wait, TTT_Menu, TTT_PVP, TTT_PVC, TTT_Win };
int TTT_Tick(int state) {
    switch (state) { // State transitions
    case TTT_INIT:
        state = TTT_Wait;
        break;
    case TTT_Wait:
        if (game_1) {
            state = TTT_Menu;
        }
        break;
    case TTT_Menu:
        if (pvp) {
            state = TTT_PVP;
        } else if (pvc) {
            state = TTT_PVC;
        }
        break;
    case TTT_PVP:
        if (end_game || draw) {
            state = TTT_Win;
        }
        break;
    case TTT_PVC:
        if (end_game || draw) {
            state = TTT_Win;
        }
        break;
    case TTT_Win:
        if (countdown >= 8) {
            state = TTT_INIT;
        }
        break;
    default:
        state = TTT_INIT;
        break;
    }
    switch (state) { // State Action
    case TTT_INIT:
        mylcd.Fill_Screen(BLACK);
        game_1 = false;
        end_game = false;
        first = true;
        draw = false;
        comp_first = false;
        once = true;
        pvp = false;
        pvc = false;
        AI = false;
        menu = true;
        TTT_Row = 0;
        TTT_Col = 0;
        temp_Row = 0;
        temp_Col = 0;
        countdown = 0;
        pieceAI = "";
        pieceP = "";
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                Board[i][j] = false;
                X[i][j] = false;
                O[i][j] = false;
            }
        }
        break;
    case TTT_Menu:
        if (once) {
            mylcd.Set_Text_Size(4);
            mylcd.Fill_Screen(BLACK);
            mylcd.Set_Text_colour(SKYBLUE);
            mylcd.Print_String("Tic-Tac-Toe", 100, 50);
            mylcd.Set_Text_colour(WHITE);
            mylcd.Print_String("Choose mode", 100, 100);
            mylcd.Print_String("PvAI", 100, 200);
            mylcd.Set_Text_colour(MAGENTA);
            mylcd.Print_String(">", 75, 150);
            mylcd.Print_String("PvP", 100, 150);

            AI = false;
            once = false;
        }

        if (up) {
            mylcd.Set_Text_colour(WHITE);
            mylcd.Print_String("PvAI", 100, 200);
            mylcd.Set_Text_colour(MAGENTA);
            mylcd.Print_String(">", 75, 150);
            mylcd.Print_String("PvP", 100, 150);
            mylcd.Set_Text_colour(BLACK);
            mylcd.Print_String(">", 75, 200);
            AI = false;
        }

        if (down) {
            mylcd.Set_Text_colour(WHITE);
            mylcd.Print_String("PvP", 100, 150);
            mylcd.Set_Text_colour(MAGENTA);
            mylcd.Print_String(">", 75, 200);
            mylcd.Print_String("PvAI", 100, 200);
            mylcd.Set_Text_colour(BLACK);
            mylcd.Print_String(">", 75, 150);
            AI = true;
        }

        if (!digitalRead(2)) {
            if (AI) {
                pvc = true;
                if (millis() % 2 == 0) {
                    comp_first = true;
                    pieceAI = "X";
                    pieceP = "O";
                } else {
                    comp_first = false;
                    pieceP = "X";
                    pieceAI = "O";
                }
            } else {
                pvp = true;
            }
            mylcd.Fill_Screen(BLACK);
            mylcd.Set_Text_colour(WHITE);
            mylcd.Set_Text_Size(6);
            mylcd.Print_String(" | | ", 150, 45);
            mylcd.Print_String("- - -", 150, 90);
            mylcd.Print_String(" | | ", 150, 135);
            mylcd.Print_String("- - -", 150, 180);
            mylcd.Print_String(" | | ", 150, 225);
            mylcd.Set_Text_Size(5);
            once = true;
        }
        break;
    case TTT_PVP:
        if (first) {
            TTT_Move("X");

            if (!digitalRead(2)) {
                once = true;
                mylcd.Set_Text_colour(RED);
                mylcd.Print_String("X", xBoard[TTT_Col], yBoard[TTT_Row]);

                X[TTT_Row][TTT_Col] = true;
                Board[TTT_Row][TTT_Col] = true;

                if (checkWin(X)) {
                    end_game = true;
                } else {
                    first = false;
                    draw = TTT_Draw();
                    if (!draw) TTT_Spot();
                }
            }
        } else {
            TTT_Move("O");

            if (!digitalRead(2)) {
                once = true;
                mylcd.Set_Text_colour(CYAN);
                mylcd.Print_String("O", xBoard[TTT_Col], yBoard[TTT_Row]);

                O[TTT_Row][TTT_Col] = true;
                Board[TTT_Row][TTT_Col] = true;

                if (checkWin(O)) {
                    end_game = true;
                } else {
                    first = true;
                    draw = TTT_Draw();
                    if (!draw) TTT_Spot();
                }
            }
        }
        break;
    case TTT_PVC:
        if (comp_first) {
            moveAI();

            mylcd.Set_Text_colour(ORANGE);
            mylcd.Print_String(pieceAI, xBoard[TTT_Col], yBoard[TTT_Row]);

            X[TTT_Row][TTT_Col] = true;
            Board[TTT_Row][TTT_Col] = true;

            if (checkWin(X)) {
                end_game = true;
            } else {
                comp_first = false;
                draw = TTT_Draw();
                if (!draw) TTT_Spot();
            }
        } else {
            TTT_Move(pieceP);

            if (!digitalRead(2)) {
                once = true;
                mylcd.Set_Text_colour(INDIGO);
                mylcd.Print_String(pieceP, xBoard[TTT_Col], yBoard[TTT_Row]);

                O[TTT_Row][TTT_Col] = true;
                Board[TTT_Row][TTT_Col] = true;

                if (checkWin(O)) {
                    end_game = true;
                } else {
                    comp_first = true;
                    draw = TTT_Draw();
                    if (!draw) TTT_Spot();
                }
            }
        }
        break;
    case TTT_Win:
        countdown++;
        if (once) {
            mylcd.Fill_Screen(BLACK);
            mylcd.Set_Text_Size(8);
            once = false;
        }

        if (draw) {
            // draw
            mylcd.Set_Text_colour(GREEN);
            mylcd.Print_String("Draw", 0, 0);
        } else if (!AI) {
            if (first) {
                // x wins
                mylcd.Set_Text_colour(RED);
                mylcd.Print_String("Player X", 0, 0);
                mylcd.Print_String("Wins", 0, 80);
            } else if (!first) {
                // o wins
                mylcd.Set_Text_colour(CYAN);
                mylcd.Print_String("Player O", 0, 0);
                mylcd.Print_String("Wins", 0, 80);
            }
        } else {
            if (comp_first) {
                // comp wins
                mylcd.Set_Text_colour(ORANGE);
                mylcd.Print_String("AI Wins", 0, 0);
            } else if (!comp_first) {
                // player wins
                mylcd.Set_Text_colour(INDIGO);
                mylcd.Print_String("Player", 0, 0);
                mylcd.Print_String("Wins", 0, 80);
            }
        }
        break;
    default:
        break;
    }

    return state;
}

enum SNAKE_States { SNAKE_INIT, SNAKE_Wait, SNAKE_Menu, SNAKE_Play, SNAKE_End };
int SNAKE_Tick(int state) {
    switch (state) { // State transitions
    case SNAKE_INIT:
        state = SNAKE_Wait;
        break;
    case SNAKE_Wait:
        if (game_2) {
            state = SNAKE_Menu;
        }
        break;
    case SNAKE_Menu:
        if (countdown <= 0) {
            state = SNAKE_Play;
        }
        break;
    case SNAKE_Play:
        if (score >= snakeLen || end_game) {
            state = SNAKE_End; 
        }
        break;
    case SNAKE_End:
        if (countdown >= 8) {
            state = SNAKE_INIT;
        }
        break;
    default:
        state = SNAKE_INIT;
        break;
    }
    switch (state) { // State Action
    case SNAKE_INIT:
        game_2 = false;
        menu = true;
        once = true;
        end_game = false;
        score = 0;
        direction = 1;
        foodRow = 0;
        foodCol = 0;
        countdown = 0;
        s_Column = 20;
        s_Row = 20;
        tempCol = 0;
        tempRow = 0;
        for (int i = 0; i < snakeLen; i++) {
            snakeRowPos[i] = -20;
            snakeColPos[i] = -20;
        }
        break;
    case SNAKE_Menu:
        if (once) {
            mylcd.Set_Text_Size(4);
            mylcd.Fill_Screen(BLACK);
            mylcd.Set_Text_colour(YELLOW);
            mylcd.Print_String("Snake", 100, 50);
            mylcd.Set_Text_colour(WHITE);
            mylcd.Print_String("Starting in", 100, 100);
            countdown = 6;
            once = false;
        }

        mylcd.Set_Text_colour(BLACK);
        mylcd.Print_String(String(countdown), 100, 150);
        countdown--;
        mylcd.Set_Text_colour(WHITE);
        mylcd.Print_String(String(countdown), 100, 150);

        if (countdown <= 0) {
            once = true;
            mylcd.Fill_Screen(BLACK);
            mylcd.Set_Text_Size(3);
        }
        break;
    case SNAKE_Play:
        // main game
        if (once) {
            spawnFood();
            once = false;
        }

        SNAKE_Move();
        lostgame();
        redrawSnake();
        checkIfScored();
        break;
    case SNAKE_End:
        if (once) {
            mylcd.Fill_Screen(BLACK);
            mylcd.Set_Text_Size(8);
            once = false;
        }

        if (end_game) {
            mylcd.Set_Text_colour(RED);
            mylcd.Print_String("Player", 0, 0);
            mylcd.Print_String("Lost", 0, 80);
        } else {
            mylcd.Set_Text_colour(SKYBLUE);
            mylcd.Print_String("Player", 0, 0);
            mylcd.Print_String("Wins", 0, 80);
        }
        countdown++;
        break;
    default:
        break;
    }

    return state;
}

void setup() {
    Serial.begin(9600);

    pinMode(stickclick, INPUT_PULLUP);

    mylcd.Init_LCD();
    mylcd.Fill_Screen(BLACK);
    mylcd.Set_Rotation(3);
    mylcd.Set_Text_Mode(1);
    mylcd.Set_Text_Size(3);

    SPI.begin();
    rfid.PCD_Init();

    unsigned char i = 0;
    tasks[i].state = JS_INIT;
    tasks[i].period = 250;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &JS_TICK;
    i++;
    tasks[i].state = RFID_INIT;
    tasks[i].period = 250;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &RFID_Tick;
    i++;
    tasks[i].state = MAIN_INIT;
    tasks[i].period = 250;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &MAIN_Tick;
    i++;
    tasks[i].state = TTT_INIT;
    tasks[i].period = 250;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &TTT_Tick;
    i++;
    tasks[i].state = SNAKE_INIT;
    tasks[i].period = 250;
    tasks[i].elapsedTime = 0;
    tasks[i].TickFct = &SNAKE_Tick;

}

void loop() {
    unsigned char i;

    for (i = 0; i < tasksNum; ++i) {
        if ((millis() - tasks[i].elapsedTime) >= tasks[i].period) {
            tasks[i].state = tasks[i].TickFct(tasks[i].state);
            tasks[i].elapsedTime = millis();
        }
    }
}
