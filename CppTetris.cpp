#include <iostream>
#include <thread>
#include <vector>
#include <Windows.h>

using namespace std;

wstring tetromino[7];
int nFieldWidth = 12;	// Classic 12 * 8 playfield 
int nFieldHeight = 18;
unsigned char* pField = nullptr;

// Console screen X and Y sizes, default of cmd prompt
int nScreenWidth = 80;
int nScreenHeight = 30;

// Handle rotations of tetrominos
int Rotate(int px, int py, int r)
{
	switch (r % 4)
	{
	case 0:
		return py * 4 + px;			// 0 degrees
	case 1:
		return 12 + py - (px * 4);	// 90 degrees
	case 2:
		return 15 - (py * 4) - px;	// 180 degrees
	case 3:
		return 3 - py + (px * 4);	// 270 degrees
	}
	return 0;
}

// Check if a tetromino collides in a pos
bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	// iteration by 4*4, for each square of tetromino
	for (int px = 0; px < 4; px++)
	{
		for (int py = 0; py < 4; py++)
		{
			// Get index into piece
			int pi = Rotate(px, py, nRotation);

			// Get index into field
			// ...top left coords (nPosY, nPosX) plus index will translate
			// ...the tetromino into the field array
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			// Out of bounds checks
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
					// If a tetromino cell is over a non-empty space cell...
					if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
						// ...return false, there is a collision
						return false;
				}
			}
		}
	}

	return true;
}

int main()
{
	// Create assets
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");

	tetromino[1].append(L"..X.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L".X..");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X..");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"..X.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"....");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L"....");

	tetromino[4].append(L"..X.");
	tetromino[4].append(L".XX.");
	tetromino[4].append(L"..X.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"....");
	tetromino[5].append(L".XX.");
	tetromino[5].append(L"..X.");
	tetromino[5].append(L"..X.");

	tetromino[6].append(L"....");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L".X..");
	tetromino[6].append(L".X..");

	// Initialise new array
	// First, play field buffer
	pField = new unsigned char[nFieldHeight * nFieldHeight];
	// Second, board boundary
	for (int x = 0; x < nFieldWidth; x++)
	{
		for (int y = 0; y < nFieldHeight; y++)
		{
			// Init everything to 0, unless on the sides or bottom (border)
			pField[y * nFieldWidth + x] =
				(x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	// Using <Windows.h>, make dmd a screen buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	// Fill screen (sizes defined above) with blank wchars
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
	{
		screen[i] = L' ';
	}
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	// hConsole will be the screen buffer, cout will no longer operate
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Game state variables
	int nCurrentPiece = 1;
	int nCurrentRotation = 0;	// angle piece is rotated
	int nCurrentX = nFieldWidth / 2;	// piece starts at top-middle of board
	int nCurrentY = 0;
	bool bGameOver = false;	// Game loop state

	// Store keys - left, right, down, z (rotate)
	bool bKey[4];
	bool bRotateHold = false; // Flag for whether z is held

	// Game speed counter
	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;	// Flag for whether tet should be pushed
	int nPieceCount = 0;	// Increase speed every ten pieces made
	int nScore = 0;

	vector<int> vLines;

	// Game loop
	while (!bGameOver)
	{
		// ===========
		// Game timing 
		// ===========

		// Block execution of current thread for specified duration
		this_thread::sleep_for(50ms);
		// Increment speed counter (aiming for 1/second)
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);

		// =====
		// Input
		// =====

		for (int k = 0; k < 4; k++)
			// GetAsyncKey returns true if the specified key is pressed
			// And hexidecimal literals for the arrows mentioned above, and Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

		// ==========
		// Game logic
		// ==========

		// Left key
		if (bKey[1]) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) {
				nCurrentX--;
			}
		}
		// Right key
		if (bKey[0]) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) {
				nCurrentX++;
			}
		}
		// Down key
		if (bKey[2]) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
				nCurrentY++;
			}
		}
		// Rotate (z)
		if (bKey[3] && !bRotateHold) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) {
				nCurrentRotation++;
			}
			bRotateHold = true;
		}
		else {
			bRotateHold = false;
		}

		// Handle moving the piece down automatically
		if (bForceDown) {
			// Check if current piece can be moved further down
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
				nCurrentY++; // Move it down
			}
			else {
				// Lock current piece in the field
				for (int px = 0; px < 4; px++)
				{
					for (int py = 0; py < 4; py++)
					{
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X') {
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
						}
					}
				}

				// Increase piece count and difficulty
				nPieceCount++;
				if (nPieceCount % 10 == 0)
					if (nSpeed >= 10) nSpeed--;


				// Check for full horizontal lines
				for (int py = 0; py < 4; py++)
				{
					if (nCurrentY + py < nFieldHeight - 1) {
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++)
						{
							// `a &= b` is equivalent to `a = a & b`
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;
						}
						if (bLine) {
							// Eliminate the line, set to `=`
							for (int px = 1; px < nFieldWidth - 1; px++)
							{
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;
							}
							// Add current row to be erased later
							vLines.push_back(nCurrentY + py);
						}
					}
				}

				nScore += 25;
				// Increase score exponentially when lines are cleared
				if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;

				// Choose next piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7; // Choose random piece this time

				// If piece does not fit, game over
				bGameOver =
					!DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

			}
			// Last, reset speed counter
			nSpeedCounter = 0;
		}

		// =============
		// Render output
		// =============

		// Draw field 
		for (int x = 0; x < nFieldWidth; x++)
		{
			for (int y = 0; y < nFieldHeight; y++)
			{
				// The '+2' is an offset to prevent the field being drawn unaesthetically in the corner 
				// ...The character is chosen from the string array L" ABCDEFG=#"
				// ...Note that position 0 is empty space, and 9 is for the border
				screen[(y + 2) * nScreenWidth + (x + 2)] =
					L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
			}
		}

		// Draw current piece, interating over 4x4 tetromino
		for (int px = 0; px < 4; px++)
		{
			for (int py = 0; py < 4; py++)
			{
				// Find by which index is rotated, and if cell is an 'x'
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
					// Offset by current piece position and '+2', the board offset
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] =
					nCurrentPiece + 65;
				// Adding 65 to offset the character, this is the letter A in ascii
			}
		}

		// Draw score
		swprintf_s(&screen[2 & nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

		if (!vLines.empty()) {
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
			// Delay so that player notices a line has been drawn
			this_thread::sleep_for(400ms);
			// Iterate through the vector, and go column by column moving pieces over the row
			for (auto& v : vLines)
			{
				for (int px = 1; px < nFieldWidth - 1; px++) {
					for (int py = v; py > 0; py--)
					{
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					}
					// Set top row to empty space
					pField[px] = 0;
				}
			}
			vLines.clear();
		}

		// Display frame
		// ...hConsole draws screen to the predefined width * height, starting at 0,0
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
	}

	// On gameover, the above loop will exit
	// Provide a game over message
	CloseHandle(hConsole);
	cout << "Game Over! Thanks for playing Tetris!" << endl;
	cout << "\t Score: " << nScore << endl;


	return 0;
}