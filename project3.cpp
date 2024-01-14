#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;

// Enum for the possible states of a tile
enum class TileState {
    Hidden,
    Revealed,
    Number,
    Flag,
    Mine
};

// Class representing a tile on the Minesweeper board
class Tile {
public:
    Tile();
    void setState(TileState newState, int newNumber = 0);
    TileState getState() const;
    void draw(sf::RenderWindow& window, int x, int y);

private:
    TileState state;
    TileState originalState; // storing the original state for flags
    int number;
    sf::Texture hiddenTexture;
    sf::Texture revealedTexture;
    vector<sf::Texture> numberTextures;
    sf::Texture flagTexture;
    sf::Texture mineTexture;
    sf::Sprite sprite;
};

Tile::Tile() : state(TileState::Hidden), number(0) {
    hiddenTexture.loadFromFile("images/tile_hidden.png");
    revealedTexture.loadFromFile("images/tile_revealed.png");
    flagTexture.loadFromFile("images/flag.png");  
    mineTexture.loadFromFile("images/mine.png");

    // Load number textures (1-8)
    numberTextures.resize(8);
    for (int i = 0; i < 8; ++i) {
        numberTextures[i].loadFromFile("images/number_" + to_string(i + 1) + ".png");
    }

    sprite.setTexture(hiddenTexture);
}

void Tile::setState(TileState newState, int newNumber) {
    state = newState;
    number = newNumber;

    switch (state) {
    case TileState::Hidden:
        sprite.setTexture(hiddenTexture);
        break;
    case TileState::Revealed:
        sprite.setTexture(revealedTexture);
        break;
    case TileState::Number:
        if (number >= 1 && number <= 8) {
            sprite.setTexture(numberTextures[number]);
        }
        break;
    case TileState::Flag:
        sprite.setTexture(flagTexture);  
        break;
    case TileState::Mine:
        sprite.setTexture(mineTexture);
        break;
    }
}

TileState Tile::getState() const {
    return state;
}


void Tile::draw(sf::RenderWindow& window, int x, int y) {
    sf::RectangleShape tileBackground(sf::Vector2f(32.f, 32.f));
    tileBackground.setPosition(static_cast<float>(x * 32), static_cast<float>(y * 32));

    switch (state) {
    case TileState::Hidden:
        tileBackground.setTexture(&hiddenTexture);
        window.draw(tileBackground);
        break;
    case TileState::Revealed: {
        tileBackground.setTexture(&revealedTexture);
        window.draw(tileBackground);
        if (number >= 1 && number <= 8) {
            // Draw the revealed number on top of the revealed tile
            sf::Sprite numberSprite;
            numberSprite.setTexture(numberTextures[number - 1]);
            numberSprite.setPosition(static_cast<float>(x * 32), static_cast<float>(y * 32));
            window.draw(numberSprite);
        }
        break;
    }
    case TileState::Number: {
        // Draw the revealed texture for the background
        tileBackground.setTexture(&revealedTexture);
        window.draw(tileBackground);

        // Draw the number on top of the revealed tile
        sf::Sprite numberSprite;
        numberSprite.setTexture(numberTextures[number - 1]);
        numberSprite.setPosition(static_cast<float>(x * 32), static_cast<float>(y * 32));
        window.draw(numberSprite);
        break;
    }
    case TileState::Flag: {
        // Draw the revealed texture for the background of flags
        tileBackground.setTexture(&revealedTexture);
        window.draw(tileBackground);

        // Draw the flag on top of the revealed tile
        sf::Sprite flagSprite;
        flagSprite.setTexture(flagTexture);
        flagSprite.setPosition(static_cast<float>(x * 32), static_cast<float>(y * 32));
        window.draw(flagSprite);
        break;
    }
    case TileState::Mine:
        tileBackground.setTexture(&revealedTexture); 
        window.draw(tileBackground);
        sprite.setPosition(static_cast<float>(x * 32), static_cast<float>(y * 32));
        window.draw(sprite);
        break;
    }
}


// Struct for configuration values
struct ConfigValues {
    int columns;
    int rows;
    int mines;
};

// read configuration values from file
bool readConfigFile(const string& filename, ConfigValues& config) {
    ifstream configFile(filename);
    if (!configFile.is_open()) {
        return false;
    }

    string line;

    if (getline(configFile, line)) {
        stringstream(line) >> config.columns;
    }
    else {
        return false;
    }

    if (getline(configFile, line)) {
        stringstream(line) >> config.rows;
    }
    else {
        return false;
    }

    if (getline(configFile, line)) {
        stringstream(line) >> config.mines;
    }
    else {
        return false;
    }

    configFile.close();
    return true;
}

// read board files
bool readBoardFile(const  string& filename, vector<vector<bool>>& boardLayout) {
    ifstream boardFile(filename);
    if (!boardFile.is_open()) {
        return false;
    }

    boardLayout.clear();

    string line;
    while (getline(boardFile, line)) {
        vector<bool> row;
        for (char ch : line) {
            if (ch == '0') {
                row.push_back(false);
            }
            else if (ch == '1') {
                row.push_back(true);
            }
            // Ignore other characters for simplicity
        }
        boardLayout.push_back(row);
    }

    boardFile.close();
    return true;
}

// Class representing the Minesweeper board
class Board {
public:
    Board(const ConfigValues& config, int width, int height);
    void draw(sf::RenderWindow& window);
    void handleLeftClick(sf::Vector2i position);
    void handleRightClick(sf::Vector2i position);
    void printNumbers() const;
    bool isGameOver() const;
    bool isGameWon() const;
    int getRemainingMines() const;
    void reset();
    bool isDebugMode;
    void initializeBoardFromLayout(const vector<vector<bool>>& boardLayout);

private:
    int columns;
    int rows;
    int mines;
    int originalMines;
    int flagsPlaced;
    int windowWidth;
    int windowHeight;
    bool gameWon;
    bool gameLost;

    vector<vector<Tile>> grid;
    vector<vector<bool>> hasMine;
    vector<vector<int>> adjacentMineCounts;
    vector<sf::Texture> digitTextures;

    sf::Texture happyFaceTexture;
    sf::Texture winFaceTexture;
    sf::Texture loseFaceTexture;
    sf::Sprite faceSprite;
    sf::Texture debugButtonTexture;
    sf::Sprite debugButtonSprite;
    sf::Texture test1ButtonTexture;
    sf::Sprite test1ButtonSprite;
    sf::Texture test2ButtonTexture;
    sf::Sprite test2ButtonSprite;
    sf::Texture test3ButtonTexture;
    sf::Sprite test3ButtonSprite;

    void initializeDebugButton();
    void drawDebugButton(sf::RenderWindow& window);
    void handleDebugButtonClick(sf::Vector2i position);
    void initializeBoard();
    void placeMines();
    void countAdjacentMines();
    int getMineCount(int x, int y);
    bool isMine(int x, int y) const;
    void revealEmptyTiles(int x, int y);
    void revealAllMinesFlags(int a);
    void initializeFaceTextures();
    void updateFaceTexture();
    void hideAllMines();
};

// Constructor for the Board class
Board::Board(const ConfigValues& config, int width, int height)
    : columns(config.columns), rows(config.rows), mines(config.mines), originalMines(config.mines), flagsPlaced(0), gameWon(false), gameLost(false), windowWidth(width), windowHeight(height), isDebugMode(false) {

    grid.resize(columns, vector<Tile>(rows));

    initializeBoard();
    placeMines();
    countAdjacentMines();

    // Load digit textures
    sf::Texture digitTexture;
    digitTexture.loadFromFile("images/digits.png");  

    // Split the digit texture into separate textures for each digit
    int digitWidth = digitTexture.getSize().x / 10;  
    int digitHeight = digitTexture.getSize().y;
    digitTextures.resize(10);

    for (int i = 0; i < 10; ++i) {
        digitTextures[i].loadFromImage(digitTexture.copyToImage(), sf::IntRect(i * digitWidth, 0, digitWidth, digitHeight));
    }

    initializeFaceTextures();
    initializeDebugButton();
    updateFaceTexture();
}

void Board::initializeDebugButton() {
    //debug button
    debugButtonTexture.loadFromFile("images/debug.png");
    debugButtonSprite.setTexture(debugButtonTexture);
    debugButtonSprite.setPosition(static_cast<float>(windowWidth / 2 + faceSprite.getGlobalBounds().width * 2 + 16), static_cast<float>(windowHeight - 1.25 * (debugButtonSprite.getGlobalBounds().height)));

    //test buttons
    test1ButtonTexture.loadFromFile("images/test_1.png");
    test1ButtonSprite.setTexture(test1ButtonTexture);
    test1ButtonSprite.setPosition(static_cast<float>(windowWidth / 2 + faceSprite.getGlobalBounds().width * 3 + 16), static_cast<float>(windowHeight - 1.25 * (test1ButtonSprite.getGlobalBounds().height)));

    test2ButtonTexture.loadFromFile("images/test_2.png");
    test2ButtonSprite.setTexture(test2ButtonTexture);
    test2ButtonSprite.setPosition(static_cast<float>(windowWidth / 2 + faceSprite.getGlobalBounds().width * 4 + 16), static_cast<float>(windowHeight - 1.25 * (test2ButtonSprite.getGlobalBounds().height)));

    test3ButtonTexture.loadFromFile("images/test_3.png");
    test3ButtonSprite.setTexture(test3ButtonTexture);
    test3ButtonSprite.setPosition(static_cast<float>(windowWidth / 2 + faceSprite.getGlobalBounds().width * 5 + 16), static_cast<float>(windowHeight - 1.25 * (test3ButtonSprite.getGlobalBounds().height)));
}

void Board::drawDebugButton(sf::RenderWindow& window) {
    window.draw(debugButtonSprite);

    //test buttons
    window.draw(test1ButtonSprite);
    window.draw(test2ButtonSprite);
    window.draw(test3ButtonSprite);
}

void Board::hideAllMines() {
    for (int x = 0; x < columns; ++x) {
        for (int y = 0; y < rows; ++y) {
            if (hasMine[x][y]) {
                // If a tile contains a mine, set its state to Mine
                grid[x][y].setState(TileState::Hidden);
            }
        }
    }
}

void Board::initializeFaceTextures() {
    happyFaceTexture.loadFromFile("images/face_happy.png");
    winFaceTexture.loadFromFile("images/face_win.png");
    loseFaceTexture.loadFromFile("images/face_lose.png");
    faceSprite.setTexture(happyFaceTexture);
}

void Board::updateFaceTexture() {
    if (gameWon) {
        faceSprite.setTexture(winFaceTexture);
    }
    else if (gameLost) {
        faceSprite.setTexture(loseFaceTexture);
    }
    else {
        faceSprite.setTexture(happyFaceTexture);
    }

    // Set the position of the face sprite at the bottom center of the window
    faceSprite.setPosition(static_cast<float>(windowWidth / 2 - faceSprite.getGlobalBounds().width / 2), static_cast<float>(windowHeight - 1.25 * (faceSprite.getGlobalBounds().height)));
}

void Board::draw(sf::RenderWindow& window) {
    // Draw the board
    for (int x = 0; x < columns; ++x) {
        for (int y = 0; y < rows; ++y) {
            grid[x][y].draw(window, x, y);
        }
    }

    // Draw the happy face
    window.draw(faceSprite);

    // Draw the debug button
    drawDebugButton(window);
}

// initialize the Minesweeper board
void Board::initializeBoard() {
    // Set all tiles to Hidden initially
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < columns; ++x) {
            grid[x][y].setState(TileState::Hidden);
        }
    }
}

// initializes the board that was loaded from one of the files
void Board::initializeBoardFromLayout(const vector<vector<bool>>& boardLayout) {
    // Set all tiles to Hidden initially
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < columns; ++x) {
            grid[x][y].setState(TileState::Hidden);
        }
    }

    // Resize and initialize hasMine vector
    hasMine.resize(columns, vector<bool>(rows, false));

    // Set mine and non-mine tiles based on the layout
    int mineCount = 0; //count the number of mines

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < columns; ++x) {
            if (boardLayout[y][x]) {
                // Set as mine
                hasMine[x][y] = true;
                mineCount++;
            }
            else {
                // Clear mine
                hasMine[x][y] = false;
            }
        }
    }

    countAdjacentMines(); 
    mines = mineCount; // Update the total number of mines
}

// Function to place mines randomly on the board
void Board::placeMines() {
    srand(static_cast<unsigned>(time(0))); // Seed the random number generator

    hasMine.resize(columns, vector<bool>(rows, false));

    int minesToPlace = mines;
    while (minesToPlace > 0) {
        int x = rand() % columns;
        int y = rand() % rows;

        if (!hasMine[x][y]) {
            hasMine[x][y] = true;
            --minesToPlace;
        }
    }
}

// Function to count the number of adjacent mines for each tile
void Board::countAdjacentMines() {
    adjacentMineCounts.resize(columns, vector<int>(rows, 0));

    for (int x = 0; x < columns; ++x) {
        for (int y = 0; y < rows; ++y) {
            if (!hasMine[x][y]) {
                int mineCount = getMineCount(x, y);
                adjacentMineCounts[x][y] = mineCount;
            }
        }
    }
}

// Function to print out the numbers for each tile, for debugging purposes
void Board::printNumbers() const {
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < columns; ++x) {
            cout << adjacentMineCounts[x][y] << " ";
        }
        cout << endl;
    }
}

// Function to get the number of adjacent mines for a given tile
int Board::getMineCount(int x, int y) {
    int count = 0;

    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < columns && ny >= 0 && ny < rows && hasMine[nx][ny]) {
                count++;
            }
        }
    }
    return count;
}

int Board::getRemainingMines() const {
    // Calculate remaining mines by subtracting flagsPlaced from the total mines
    return mines - flagsPlaced;
}

// Function to check if a given tile is a mine
bool Board::isMine(int x, int y) const {
    return (grid[x][y].getState() == TileState::Mine);
}

// Function to reveal empty tiles and adjacent tiles
void Board::revealEmptyTiles(int x, int y) {
    if (x < 0 || x >= columns || y < 0 || y >= rows || grid[x][y].getState() != TileState::Hidden) {
        // Check if the coordinates are within the valid range and the tile is hidden
        return;
    }

    Tile& currentTile = grid[x][y];

    if (currentTile.getState() == TileState::Hidden) {
        // Reveal the current tile and its number
        currentTile.setState(TileState::Revealed, adjacentMineCounts[x][y]);

        // Check if the current tile has no adjacent mines
        if (getMineCount(x, y) == 0) {
            // Reveal adjacent tiles
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    int nx = x + dx;
                    int ny = y + dy;
                    // Only reveal the adjacent tile if it's hidden and not a mine
                    if (nx >= 0 && nx < columns && ny >= 0 && ny < rows && !hasMine[nx][ny]) {
                        revealEmptyTiles(nx, ny);
                    }
                }
            }
        }
    }
}

// Function to handle left-click events on the Minesweeper board
void Board::handleLeftClick(sf::Vector2i position) {
    // Check if the happy face is clicked
    sf::FloatRect faceBounds = faceSprite.getGlobalBounds();
    if (faceBounds.contains(static_cast<float>(position.x), static_cast<float>(position.y))) {
        reset();
        isDebugMode = false;
        return;
    }

    // Check if the debug button is clicked
    sf::FloatRect debugButtonBounds = debugButtonSprite.getGlobalBounds();
    if (debugButtonBounds.contains(static_cast<float>(position.x), static_cast<float>(position.y))) {
        if (gameWon || gameLost) {
            return;
        }
        // Toggle debug mode
        isDebugMode = !isDebugMode;

        if (isDebugMode) {
            // If entering debug mode, reveal all mines, 1 is for mines 0 for flags
            revealAllMinesFlags(1);
        }
        else {
            // If exiting debug mode, hide all mines
            hideAllMines();
        }
        return; 
    }

    sf::FloatRect test1ButtonBounds = test1ButtonSprite.getGlobalBounds();
    if (test1ButtonBounds.contains(static_cast<float>(position.x), static_cast<float>(position.y))) {
        // Load and initialize the first test board (testboard1.brd)
        vector<vector<bool>> boardLayout;
        if (readBoardFile("boards/testboard1.brd", boardLayout)) {
            reset();
            initializeBoardFromLayout(boardLayout);

        }
        else {
            cerr << "Failed to load Test Board 1." << endl;
        }
        return;
    }

    sf::FloatRect test2ButtonBounds = test2ButtonSprite.getGlobalBounds();
    if (test2ButtonBounds.contains(static_cast<float>(position.x), static_cast<float>(position.y))) {
        // Load and initialize the second test board (testboard2.brd)
        vector<vector<bool>> boardLayout;
        if (readBoardFile("boards/testboard2.brd", boardLayout)) {
            reset();
            initializeBoardFromLayout(boardLayout);

        }
        else {
            cerr << "Failed to load Test Board 2." << endl;
        }
        return;
    }

    sf::FloatRect test3ButtonBounds = test3ButtonSprite.getGlobalBounds();
    if (test3ButtonBounds.contains(static_cast<float>(position.x), static_cast<float>(position.y))) {
        // Load and initialize the third test board (testboard3.brd)
        vector<vector<bool>> boardLayout;
        if (readBoardFile("boards/testboard3.brd", boardLayout)) {
            reset();
            initializeBoardFromLayout(boardLayout);

        }
        else {
            cerr << "Failed to load Test Board 3." << endl;
        }
        return;
    }

    int x = position.x / 32;
    int y = position.y / 32;

    if (x >= 0 && x < columns && y >= 0 && y < rows) {
        if (gameWon || gameLost) {
            return;
        }
        Tile& clickedTile = grid[x][y];

        if (clickedTile.getState() == TileState::Hidden) {
            if (hasMine[x][y]) {
                // Reveal the mine
                clickedTile.setState(TileState::Mine);
                revealAllMinesFlags(1); // Reveal all mines on the board
                gameLost = true;
                updateFaceTexture();

            }
            else {
                if (getMineCount(x, y) == 0) {
                    // reveals tiles which are not adjacent to mines
                    revealEmptyTiles(x, y);
                }
                else {
                    // reveals the number
                    int number = adjacentMineCounts[x][y];
                    clickedTile.setState(TileState::Revealed, number);
                }
                // Check for winning condition
                int revealedTiles = 0;
                for (int i = 0; i < columns; ++i) {
                    for (int j = 0; j < rows; ++j) {
                        TileState state = grid[i][j].getState();

                        // Check if the game is won when all non-mine tiles are revealed
                        if (!hasMine[i][j] && (state == TileState::Revealed || state == TileState::Flag)) {
                            revealedTiles++;
                        }
                    }
                }
                if (revealedTiles == (columns * rows - mines)) {
                    gameWon = true;
                    //reveals flags because 0 is for flags and 1 is for mines
                    revealAllMinesFlags(0);
                    updateFaceTexture();
                }
            }
        }
    }
}

//function to handle right clicks - for flags 
void Board::handleRightClick(sf::Vector2i position) {
    if (gameWon || gameLost) {
        return;
    }

    int x = position.x / 32;
    int y = position.y / 32;

    if (x >= 0 && x < columns && y >= 0 && y < rows) {
        Tile& clickedTile = grid[x][y];

        // Toggle between Flag and Hidden states on right-click
        if (clickedTile.getState() == TileState::Hidden) {
            // Toggle to Flag state
            clickedTile.setState(TileState::Flag);
            flagsPlaced++;
        }
        else if (clickedTile.getState() == TileState::Flag) {
            // Toggle back to Hidden state
            clickedTile.setState(TileState::Hidden);
            flagsPlaced--;
        }
    }
}

//function to reveal all flags or mines depending on conditions
void Board::revealAllMinesFlags(int a) {
    for (int x = 0; x < columns; ++x) {
        for (int y = 0; y < rows; ++y) {
            if (hasMine[x][y] && a == 1) {
                // If a tile contains a mine, set its state to Revealed
                grid[x][y].setState(TileState::Mine);
            }
            else if (hasMine[x][y] && a == 0) {
                grid[x][y].setState(TileState::Flag);
            }
        }
    }
}

void Board::reset() {
    // Reset game state
    flagsPlaced = 0;
    gameWon = false;
    gameLost = false;

    //rest to number of mines determined by config
    mines = originalMines;

    // Clear existing data
    grid.clear();
    hasMine.clear();
    adjacentMineCounts.clear();

    // Resize and reinitialize the grid
    grid.resize(columns, vector<Tile>(rows));
    hasMine.resize(columns, vector<bool>(rows, false));

    initializeBoard();
    placeMines();
    countAdjacentMines();

    // Reset face texture
    updateFaceTexture();
}

int main() {
    //configuration object
    ConfigValues config;

    if (!readConfigFile("boards/config.cfg", config)) {
        cerr << "Failed to read configuration file." << endl;
        return 1;
    }

    int windowWidth = config.columns * 32;
    int windowHeight = config.rows * 32 + 100;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Minesweeper");

    Board minesweeper(config, windowWidth, windowHeight);  

    // Load digit texture
    sf::Texture digitTexture;
    if (!digitTexture.loadFromFile("images/digits.png")) {
        cerr << "Failed to load digit texture." << endl;
        return 1;
    }

    // Set digit width and height based on the digit texture
    int digitWidth = 21;
    int digitHeight = 32; 

    sf::Sprite digitSprite;
    digitSprite.setTexture(digitTexture);

    minesweeper.reset();  

    // Run the program as long as the window is open
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    minesweeper.handleLeftClick(static_cast<sf::Vector2i>(sf::Mouse::getPosition(window)));
                }
                else if (event.mouseButton.button == sf::Mouse::Right) {
                    minesweeper.handleRightClick(static_cast<sf::Vector2i>(sf::Mouse::getPosition(window)));
                }
            }
        }

        // Clear the window with a white background
        window.clear(sf::Color::White);

        // Draw the board and options space
        minesweeper.draw(window);

        // Draw the remaining mines count
        string minesString = to_string(minesweeper.getRemainingMines());
        int numDigits = minesString.size();
        int totalWidth = digitWidth * numDigits;

        // Center the counter between board and bottom of the window
        int offsetX = 10; 
        int offsetY = static_cast<int>((windowHeight - digitHeight * 2));

        // Draw the negative sign if the number is negative
        if (minesweeper.getRemainingMines() < 0) {
            digitSprite.setTextureRect(sf::IntRect(10 * digitWidth, 0, digitWidth, digitHeight));
            digitSprite.setPosition(static_cast<float>(offsetX), static_cast<float>(offsetY));
            window.draw(digitSprite);
            offsetX += digitWidth;
        }

        for (size_t i = 0; i < numDigits; ++i) {
            int digitValue = minesString[i] - '0';
            digitSprite.setTextureRect(sf::IntRect(digitValue * digitWidth, 0, digitWidth, digitHeight));

            digitSprite.setPosition(static_cast<float>(offsetX + i * digitWidth), static_cast<float>(offsetY));
            window.draw(digitSprite);
        }

        // Display the contents of the window
        window.display();
    }

    return 0;
}