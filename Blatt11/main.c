#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tetris.h"


#ifdef __MINGW32__
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#endif

#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <SDL2_mixer/SDL_mixer.h>
#endif


#define RESOLUTION_WIDTH 1280
#define RESOLUTION_HEIGHT 720
#define FPS 60
#define STARTLEVEL 0
#define MAXLEVEL 20
#define DIFFICULTY 1

void renderNumbers(int, float, float, float, SDL_Renderer*, SDL_Texture*[]);
	
typedef struct 
{
	int Stelle_1;
	int Stelle_2;
	int Stelle_3;
	int Stelle_4;
	int Stelle_5;
	int Stelle_6;
} Stellen;

typedef struct
{
	int Stelle_1;
	int Stelle_2;
	int Stelle_3;
} StellenLines;

typedef struct
{
	int Stelle_1;
	int Stelle_2;
} StellenTetris;
	
	
int main(int argc, char *argv[])
{
	// Pixels per Block
	float PpB = RESOLUTION_HEIGHT / 25;
	srand(time(NULL));
	
	printf("argc = %d, name of exe: %s\n\n", argc, argv[0]);

	Stellen scoreAusgabe;
	StellenLines linesAusgabe;
	StellenTetris tetrisAusgabe;
	tetrisGame game;
	tetrisGame* pointGame;
	pointGame = &game;
	
	game.rows = 20;
	game.columns = 10;
	game.score = 0;
	game.level = STARTLEVEL;
	game.lines = 0;
	game.tetris = 0;
	
	block fall;
	fall.type = 0;
	game.falling = fall;
	
	block nex;
	nex.type = 0;
	game.next = nex;
	
	
	FILE *print;
	print = fopen("output.txt", "w");
	
	int *LinesNeeded = (int*) malloc((MAXLEVEL + 1)*sizeof(int));
	for (int i = 0, k = 10; i < MAXLEVEL; i++, k+=10)
	{
		LinesNeeded[i] = k;
	fprintf(print, "LinesNeeded[%d]: %d\n", i, LinesNeeded[i]);
	}

	int* pLinesNeeded = LinesNeeded;
	for (int i = 0; i < STARTLEVEL; i++, pLinesNeeded++);
	fprintf(print, "pLinesNeeded: %d\n", *pLinesNeeded);
	
	int *GravityArray = (int*) malloc(40*sizeof(int));
	
	for (int i = 0, k = 48; i < 9; i++, k -= 5)
	{
		GravityArray[i] = k;  // 0-8 filled
	}
	GravityArray[9] = 6;
	for (int i = 10; i < 13; i++)
	{
		GravityArray[i] = 5;
	}
	for (int i = 13; i < 16; i++)
	{
		GravityArray[i] = 4;
	}
	for (int i = 16; i < 19; i++)
	{
		GravityArray[i] = 3;
	}
	for (int i = 19; i < 29; i++)
	{
		GravityArray[i] = 2;
	}
	for (int i = 29; i < 40; i ++)
	{
		GravityArray[i] = 1;
	}
	int* pGravityArray = GravityArray;
	for (int i = 0; i < STARTLEVEL; i++, pGravityArray++);
	
	int renderMap[game.rows][game.columns];

	tetrisGetNextBlock (pointGame);
	tetrisGetNextBlock (pointGame);

	/* Init map */
	for (int x = 0; x < game.columns; x++)
	{
		for (int y = 0; y < game.rows; y++)
		{
			game.map[y][x] = 0;
		}
	}

	for (int x = 0; x < game.columns; x++)
	{
		for (int y = 0; y < game.rows; y++)
		{
			renderMap[y][x] = 0;
		}
	}


	/* Anfang Basic Framework */
	if (SDL_Init( SDL_INIT_VIDEO| SDL_INIT_AUDIO | SDL_INIT_TIMER ) < 0)
	{
		printf("Fehler beim Initialisierungsprozesses von SDL: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window* win = SDL_CreateWindow("Tetris NES",
										SDL_WINDOWPOS_CENTERED,
										SDL_WINDOWPOS_CENTERED,
										RESOLUTION_WIDTH,
										RESOLUTION_HEIGHT, 0);
	if (!win)
	{
		printf("Fehler beim Erstellen des Fensters: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);

	if (!rend)
	{
		printf("Fehler beim Initialisieren des Renderers: %s\n", SDL_GetError());
		SDL_DestroyWindow(win);
		SDL_Quit();
		return 1;
	}

	SDL_Surface* BlockI = IMG_Load("textures/TexBlockI.png");
	SDL_Surface* BlockL = IMG_Load("textures/TexBlockL.png");
	SDL_Surface* BlockJ = IMG_Load("textures/TexBlockJ.png");
	SDL_Surface* BlockO = IMG_Load("textures/TexBlockO.png");
	SDL_Surface* BlockS = IMG_Load("textures/TexBlockS.png");
	SDL_Surface* BlockT = IMG_Load("textures/TexBlockT.png");
	SDL_Surface* BlockZ = IMG_Load("textures/TexBlockZ.png");
	SDL_Surface* Edges = IMG_Load("textures/Edges.png");
	SDL_Surface* BackgroundImage = IMG_Load("textures/Background.jpg");
    SDL_Surface* GameOver = IMG_Load("textures/GameOver.jpg");
	
	SDL_Texture* texNum[10];
	
	for(int i = 0; i < 10; i++) 
	{
		char buf[100];
		sprintf(buf, "textures/Tex%d.png", i);
		SDL_Surface* temp = IMG_Load(buf);
		texNum[i] = SDL_CreateTextureFromSurface(rend, temp);
		SDL_FreeSurface(temp);
	}
	
	if (!(BlockI || BlockL || BlockJ || BlockO || BlockS || BlockT || BlockZ || Edges || BackgroundImage))
	{
		printf("Fehler beim Initialisieren von mindestens einer der Oberflaechen: %s\n", SDL_GetError());
		SDL_DestroyRenderer(rend);
		SDL_DestroyWindow(win);
		SDL_Quit();
		return 1;
	}


	SDL_Texture* texBlockI = SDL_CreateTextureFromSurface(rend, BlockI);
	SDL_Texture* texBlockL = SDL_CreateTextureFromSurface(rend, BlockL);
	SDL_Texture* texBlockJ = SDL_CreateTextureFromSurface(rend, BlockJ);
	SDL_Texture* texBlockO = SDL_CreateTextureFromSurface(rend, BlockO);
	SDL_Texture* texBlockS = SDL_CreateTextureFromSurface(rend, BlockS);
	SDL_Texture* texBlockT = SDL_CreateTextureFromSurface(rend, BlockT);
	SDL_Texture* texBlockZ = SDL_CreateTextureFromSurface(rend, BlockZ);
	SDL_Texture* texEdges = SDL_CreateTextureFromSurface(rend, Edges);
	SDL_Texture* texBackgroundImage = SDL_CreateTextureFromSurface(rend, BackgroundImage);
    SDL_Texture* textGameOver = SDL_CreateTextureFromSurface(rend, GameOver);
	SDL_FreeSurface(BlockI);
	SDL_FreeSurface(BlockL);
	SDL_FreeSurface(BlockJ);
	SDL_FreeSurface(BlockO);
	SDL_FreeSurface(BlockS);
	SDL_FreeSurface(BlockT);
	SDL_FreeSurface(BlockZ);
	SDL_FreeSurface(Edges);
	SDL_FreeSurface(BackgroundImage);
    SDL_FreeSurface(GameOver);

    

	/* Fehlercode an die neuen Texturen anpassen */
	if (!(texBlockI || texBlockL || texBlockJ || texBlockS || texBlockT || texBlockO || texBlockZ || texEdges || texBackgroundImage))
	{
		printf("Fehler beim Erstellen der Textur: %s\n", SDL_GetError());
		SDL_DestroyRenderer(rend);
		SDL_DestroyWindow(win);
		SDL_Quit();
		return 1;
	}
	/* Ende Basic Framework */

	/* Music */
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	Mix_Music* BackgroundMusic = Mix_LoadMUS("music/BackgroundMusic1.mp3");
	Mix_PlayMusic(BackgroundMusic, -1);


	//Struct der Position und Groesse des Sprites
	SDL_Rect RBlockI;
	SDL_Rect RBlockL;
	SDL_Rect RBlockJ;
	SDL_Rect RBlockO;
	SDL_Rect RBlockS;
	SDL_Rect RBlockT;
	SDL_Rect RBlockZ;
	SDL_Rect REdges;


	//Dimension der Textur
	SDL_QueryTexture(texBlockI, NULL, NULL, &RBlockI.w, &RBlockI.h);
	SDL_QueryTexture(texBlockL, NULL, NULL, &RBlockL.w, &RBlockL.h);
	SDL_QueryTexture(texBlockJ, NULL, NULL, &RBlockJ.w, &RBlockJ.h);
	SDL_QueryTexture(texBlockO, NULL, NULL, &RBlockO.w, &RBlockO.h);
	SDL_QueryTexture(texBlockS, NULL, NULL, &RBlockS.w, &RBlockS.h);
	SDL_QueryTexture(texBlockT, NULL, NULL, &RBlockT.w, &RBlockT.h);
	SDL_QueryTexture(texBlockZ, NULL, NULL, &RBlockZ.w, &RBlockZ.h);
	SDL_QueryTexture(texEdges, NULL, NULL, &REdges.w, &REdges.h);

	// Bewegungsrichtungen
	int down = 0;
	int oldlines = 0;
	int oldlines2 = 0;
	int oldtetris = 0;
	int maxlevel = MAXLEVEL;
	int oldscore = 0;
	int timer = 0;
	int tempTimer = 0;
	int Framecounter = 0;
	int levelCheck = STARTLEVEL;
	int z = game.level;
	int tempTetris = 0;
	
	scoreAusgabe.Stelle_1 = 0;	
	scoreAusgabe.Stelle_2 = 0;
	scoreAusgabe.Stelle_3 = 0;
	scoreAusgabe.Stelle_4 = 0;
	scoreAusgabe.Stelle_5 = 0;
	scoreAusgabe.Stelle_6 = 0;
	
	linesAusgabe.Stelle_1 = 0;
	linesAusgabe.Stelle_2 = 0;
	linesAusgabe.Stelle_3 = 0;
	
	tetrisAusgabe.Stelle_1 = 0;
	tetrisAusgabe.Stelle_2 = 0;
	
	
	game.running = true;
   
	/* Animation */
	while (game.running)
	{
        
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
				if (event.type == SDL_QUIT)
				{
					game.running = false;
				}

				else if (event.type == SDL_KEYDOWN)
				{
					switch (event.key.keysym.scancode)
					{
						case SDL_SCANCODE_ESCAPE:
							game.running = false;
							break;
						case SDL_SCANCODE_A:
						case SDL_SCANCODE_LEFT:
							tetrisMoveLeft(pointGame);
							break;
						case SDL_SCANCODE_S:
						case SDL_SCANCODE_DOWN:
							down = 1;
							break;
						case SDL_SCANCODE_D:
						case SDL_SCANCODE_RIGHT:
							tetrisMoveRight(pointGame);
							break;
						case SDL_SCANCODE_Q:
							tetrisTurnBlockLeft(pointGame);
							break;
						case SDL_SCANCODE_E:
							tetrisTurnBlockRight(pointGame);
							break;
						default:
							break;
					}
				}

				else if (event.type == SDL_KEYUP)
				{
					switch (event.key.keysym.scancode)
					{
						case SDL_SCANCODE_S:
						case SDL_SCANCODE_DOWN:
							down = 0;
							break;
						default:
							break;
					}
				}

		}


		if (down)
		{
			tetrisApplyGravity(pointGame);
		}

		
		/* Gravity */
		if (levelCheck != game.level)
		{
			pGravityArray++;
		}
		levelCheck = game.level;
		if (Framecounter >= (*pGravityArray))
		{
			tetrisApplyGravity(pointGame);
			Framecounter -= (*pGravityArray);
		}

		// Leere das Fenster
		SDL_RenderClear(rend);

		/* Neue Texturen auf die Map */
		for (int x = 0; x < game.columns; x++){
			for (int y = 0; y < game.rows; y++){
				renderMap[y][x] = game.map[y][x];
			}
		}

		for (int i = 0; i < 4; i++){
			int checkX = tetrisGetArrayInfo(game.falling.type, game.falling.orientation, i).x + game.falling.pos.x;
			int checkY = tetrisGetArrayInfo(game.falling.type, game.falling.orientation, i).y + game.falling.pos.y;
			renderMap[checkY][checkX] = game.falling.type + 1;
		}

		/* Background */
		SDL_RenderCopy(rend, texBackgroundImage, NULL, NULL);


		/* Edges Start */
		/* Main */
		for (int y = 0; y < game.rows; y++)
		{
			REdges.x = -1 * PpB + (RESOLUTION_WIDTH/2) - 5*PpB;
			REdges.y = y * PpB + 3*PpB;
				SDL_RenderCopy(rend, texEdges, NULL, &REdges);
		}
		for (int y = 0; y < game.rows; y++)
		{
			REdges.x = game.columns * PpB + (RESOLUTION_WIDTH/2) - 5*PpB;
			REdges.y = y * PpB + 3*PpB;
				SDL_RenderCopy(rend, texEdges, NULL, &REdges);
		}
		for (int x = -1; x < (game.columns + 1); x++)
		{
			REdges.x = x * PpB + (RESOLUTION_WIDTH/2) - 5*PpB;
			REdges.y = -1 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texEdges, NULL, &REdges);
		}
		for (int x = -1; x < (game.columns + 1); x++)
		{
			REdges.x = x * PpB + (RESOLUTION_WIDTH/2) - 5*PpB;
			REdges.y = game.rows * PpB + 3*PpB;
				SDL_RenderCopy(rend, texEdges, NULL, &REdges);
		} /* Main End */
		
		/* Next Block */
		for (int y = 0; y < 3; y++)
		{
			REdges.x = -1 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
			REdges.y = y * PpB + 3*PpB;
				SDL_RenderCopy(rend, texEdges, NULL, &REdges);
		}
		for (int y = 0; y < 3; y++)
		{
			REdges.x = 5 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
			REdges.y = y * PpB + 3*PpB;
				SDL_RenderCopy(rend, texEdges, NULL, &REdges);
		}
		for (int x = -1; x < (4 + 2); x++)
		{
			REdges.x = x * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
			REdges.y = -1 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texEdges, NULL, &REdges);
		}
		for (int x = -1; x < (4 + 2); x++)
		{
			REdges.x = x * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
			REdges.y = 3 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texEdges, NULL, &REdges);
		}
		/* Next Block End */
		/* Edges End */
		
		
		/* Lines Ausgabe */
		int tempLinesCounter = 0;
		
		if (game.lines != oldlines2)
		{
			fprintf(print, "game.lines: %d\noldlines2: %d\n", game.lines, oldlines2);
			linesAusgabe.Stelle_1 = 0;
			linesAusgabe.Stelle_2 = 0;
			linesAusgabe.Stelle_3 = 0;
			tempLinesCounter = game.lines;
		}
		
		if (tempLinesCounter != 0)
		{ 
			fprintf(print, "tempLinesCounter: %d\n", tempLinesCounter);
		}
		
		while (tempLinesCounter >= 100)
		{
			linesAusgabe.Stelle_1++;
			tempLinesCounter -= 100;
		}
		
		while (tempLinesCounter >= 10)
		{
			linesAusgabe.Stelle_2++;
			tempLinesCounter -= 10;
		}
		
		while (tempLinesCounter >= 1)
		{
			linesAusgabe.Stelle_3++;
			tempLinesCounter -= 1;
		}
		
		oldlines2 = game.lines;
		
		
			switch (linesAusgabe.Stelle_1)
			{
				case 0:
					renderNumbers(0, 29.7, 10, PpB, rend, texNum);
					break;
				case 1: 
					renderNumbers(1, 29.7, 10, PpB, rend, texNum);
					break;
				case 2:
					renderNumbers(2, 29.7, 10, PpB, rend, texNum);
					break;
				case 3:
					renderNumbers(3, 29.7, 10, PpB, rend, texNum);
					break;
				case 4:
					renderNumbers(4, 29.7, 10, PpB, rend, texNum);
					break;
				case 5:
					renderNumbers(5, 29.7, 10, PpB, rend, texNum);
					break;
				case 6:
					renderNumbers(6, 29.7, 10, PpB, rend, texNum);
					break;
				case 7:
					renderNumbers(7, 29.7, 10, PpB, rend, texNum);
					break;
				case 8:
					renderNumbers(8, 29.7, 10, PpB, rend, texNum);
					break;
				case 9:
					renderNumbers(9, 29.7, 10, PpB, rend, texNum);
					break;
				default:
					break;
			}
			
			switch (linesAusgabe.Stelle_2)
			{
				case 0:
					renderNumbers(0, 30.7, 10, PpB, rend, texNum);
					break;
				case 1: 
					renderNumbers(1, 30.7, 10, PpB, rend, texNum);
					break;
				case 2:
					renderNumbers(2, 30.7, 10, PpB, rend, texNum);
					break;
				case 3:
					renderNumbers(3, 30.7, 10, PpB, rend, texNum);
					break;
				case 4:
					renderNumbers(4, 30.7, 10, PpB, rend, texNum);
					break;
				case 5:
					renderNumbers(5, 30.7, 10, PpB, rend, texNum);
					break;
				case 6:
					renderNumbers(6, 30.7, 10, PpB, rend, texNum);
					break;
				case 7:
					renderNumbers(7, 30.7, 10, PpB, rend, texNum);
					break;
				case 8:
					renderNumbers(8, 30.7, 10, PpB, rend, texNum);
					break;
				case 9:
					renderNumbers(9, 30.7, 10, PpB, rend, texNum);
					break;
				default:
					break;
			}
			
			switch (linesAusgabe.Stelle_3)
			{
				case 0:
					renderNumbers(0, 31.7, 10, PpB, rend, texNum);
					break;
				case 1: 
					renderNumbers(1, 31.7, 10, PpB, rend, texNum);
					break;
				case 2:
					renderNumbers(2, 31.7, 10, PpB, rend, texNum);
					break;
				case 3:
					renderNumbers(3, 31.7, 10, PpB, rend, texNum);
					break;
				case 4:
					renderNumbers(4, 31.7, 10, PpB, rend, texNum);
					break;
				case 5:
					renderNumbers(5, 31.7, 10, PpB, rend, texNum);
					break;
				case 6:
					renderNumbers(6, 31.7, 10, PpB, rend, texNum);
					break;
				case 7:
					renderNumbers(7, 31.7, 10, PpB, rend, texNum);
					break;
				case 8:
					renderNumbers(8, 31.7, 10, PpB, rend, texNum);
					break;
				case 9:
					renderNumbers(9, 31.7, 10, PpB, rend, texNum);
					break;
				default:
					break;
			}
			
		
		/* Score Ausgabe */

		int tempScore = 0;
		if (game.score != oldscore)
		{
			scoreAusgabe.Stelle_1 = 0;
			scoreAusgabe.Stelle_2 = 0;
			scoreAusgabe.Stelle_3 = 0;
			scoreAusgabe.Stelle_4 = 0;
			scoreAusgabe.Stelle_5 = 0;
			scoreAusgabe.Stelle_6 = 0;
			tempScore = game.score;
		}
		
		while (tempScore >= 100000)
		{
			scoreAusgabe.Stelle_1++;
			tempScore -= 100000;
		}
		while (tempScore >= 10000)
		{
			scoreAusgabe.Stelle_2++;
			tempScore -= 10000;
		}
		while (tempScore >= 1000)
		{
			scoreAusgabe.Stelle_3++;
			tempScore -= 1000;
		}
		while (tempScore >= 100)
		{
			scoreAusgabe.Stelle_4++;
			tempScore -= 100;
		}
		while (tempScore >= 10)
		{
			scoreAusgabe.Stelle_5++;
			tempScore -= 10;
		}
		while (tempScore >= 1)
		{
			scoreAusgabe.Stelle_6++;
			tempScore -= 1;
		}
		if (oldscore != game.score)
		{
			fprintf(print, "1: %d\n2: %d\n3: %d\n4: %d\n5: %d\n6: %d\n", scoreAusgabe.Stelle_1, scoreAusgabe.Stelle_2, scoreAusgabe.Stelle_3, scoreAusgabe.Stelle_4, scoreAusgabe.Stelle_5, scoreAusgabe.Stelle_6);
		}
		
		oldscore = game.score;
		
		
		switch (scoreAusgabe.Stelle_1)
		{
			case 0:
				renderNumbers(0, 29.7, 8, PpB, rend, texNum);
				break;
			case 1: 
				renderNumbers(1, 29.7, 8, PpB, rend, texNum);
				break;
			case 2:
				renderNumbers(2, 29.7, 8, PpB, rend, texNum);
				break;
			case 3:
				renderNumbers(3, 29.7, 8, PpB, rend, texNum);
				break;
			case 4:
				renderNumbers(4, 29.7, 8, PpB, rend, texNum);
				break;
			case 5:
				renderNumbers(5, 29.7, 8, PpB, rend, texNum);
				break;
			case 6:
				renderNumbers(6, 29.7, 8, PpB, rend, texNum);
				break;
			case 7:
				renderNumbers(7, 29.7, 8, PpB, rend, texNum);
				break;
			case 8:
				renderNumbers(8, 29.7, 8, PpB, rend, texNum);
				break;
			case 9:
				renderNumbers(9, 29.7, 8, PpB, rend, texNum);
				break;
			default:
				break;
		}
		switch (scoreAusgabe.Stelle_2)
		{
			case 0:
				renderNumbers(0, 30.7, 8, PpB, rend, texNum);
				break;
			case 1: 
				renderNumbers(1, 30.7, 8, PpB, rend, texNum);
				break;
			case 2:
				renderNumbers(2, 30.7, 8, PpB, rend, texNum);
				break;
			case 3:
				renderNumbers(3, 30.7, 8, PpB, rend, texNum);
				break;
			case 4:
				renderNumbers(4, 30.7, 8, PpB, rend, texNum);
				break;
			case 5:
				renderNumbers(5, 30.7, 8, PpB, rend, texNum);
				break;
			case 6:
				renderNumbers(6, 30.7, 8, PpB, rend, texNum);
				break;
			case 7:
				renderNumbers(7, 30.7, 8, PpB, rend, texNum);
				break;
			case 8:
				renderNumbers(8, 30.7, 8, PpB, rend, texNum);
				break;
			case 9:
				renderNumbers(9, 30.7, 8, PpB, rend, texNum);
				break;
			default:
				break;
		}
		switch (scoreAusgabe.Stelle_3)
		{
			case 0:
				renderNumbers(0, 31.7, 8, PpB, rend, texNum);
				break;
			case 1: 
				renderNumbers(1, 31.7, 8, PpB, rend, texNum);
				break;
			case 2:
				renderNumbers(2, 31.7, 8, PpB, rend, texNum);
				break;
			case 3:
				renderNumbers(3, 31.7, 8, PpB, rend, texNum);
				break;
			case 4:
				renderNumbers(4, 31.7, 8, PpB, rend, texNum);
				break;
			case 5:
				renderNumbers(5, 31.7, 8, PpB, rend, texNum);
				break;
			case 6:
				renderNumbers(6, 31.7, 8, PpB, rend, texNum);
				break;
			case 7:
				renderNumbers(7, 31.7, 8, PpB, rend, texNum);
				break;
			case 8:
				renderNumbers(8, 31.7, 8, PpB, rend, texNum);
				break;
			case 9:
				renderNumbers(9, 31.7, 8, PpB, rend, texNum);
				break;
			default:
				break;
		}
		switch (scoreAusgabe.Stelle_4)
		{
			case 0:
				renderNumbers(0, 32.7, 8, PpB, rend, texNum);
				break;
			case 1: 
				renderNumbers(1, 32.7, 8, PpB, rend, texNum);
				break;
			case 2:
				renderNumbers(2, 32.7, 8, PpB, rend, texNum);
				break;
			case 3:
				renderNumbers(3, 32.7, 8, PpB, rend, texNum);
				break;
			case 4:
				renderNumbers(4, 32.7, 8, PpB, rend, texNum);
				break;
			case 5:
				renderNumbers(5, 32.7, 8, PpB, rend, texNum);
				break;
			case 6:
				renderNumbers(6, 32.7, 8, PpB, rend, texNum);
				break;
			case 7:
				renderNumbers(7, 32.7, 8, PpB, rend, texNum);
				break;
			case 8:
				renderNumbers(8, 32.7, 8, PpB, rend, texNum);
				break;
			case 9:
				renderNumbers(9, 32.7, 8, PpB, rend, texNum);
				break;
			default:
				break;
		}
		switch (scoreAusgabe.Stelle_5)
		{
			case 0:
				renderNumbers(0, 33.7, 8, PpB, rend, texNum);
				break;
			case 1: 
				renderNumbers(1, 33.7, 8, PpB, rend, texNum);
				break;
			case 2:
				renderNumbers(2, 33.7, 8, PpB, rend, texNum);
				break;
			case 3:
				renderNumbers(3, 33.7, 8, PpB, rend, texNum);
				break;
			case 4:
				renderNumbers(4, 33.7, 8, PpB, rend, texNum);
				break;
			case 5:
				renderNumbers(5, 33.7, 8, PpB, rend, texNum);
				break;
			case 6:
				renderNumbers(6, 33.7, 8, PpB, rend, texNum);
				break;
			case 7:
				renderNumbers(7, 33.7, 8, PpB, rend, texNum);
				break;
			case 8:
				renderNumbers(8, 33.7, 8, PpB, rend, texNum);
				break;
			case 9:
				renderNumbers(9, 33.7, 8, PpB, rend, texNum);
				break;
			default:
				break;
		}
		switch (scoreAusgabe.Stelle_6)
		{
			case 0:
				renderNumbers(0, 34.7, 8, PpB, rend, texNum);
				break;
			case 1: 
				renderNumbers(1, 34.7, 8, PpB, rend, texNum);
				break;
			case 2:
				renderNumbers(2, 34.7, 8, PpB, rend, texNum);
				break;
			case 3:
				renderNumbers(3, 34.7, 8, PpB, rend, texNum);
				break;
			case 4:
				renderNumbers(4, 34.7, 8, PpB, rend, texNum);
				break;
			case 5:
				renderNumbers(5, 34.7, 8, PpB, rend, texNum);
				break;
			case 6:
				renderNumbers(6, 34.7, 8, PpB, rend, texNum);
				break;
			case 7:
				renderNumbers(7, 34.7, 8, PpB, rend, texNum);
				break;
			case 8:
				renderNumbers(8, 34.7, 8, PpB, rend, texNum);
				break;
			case 9:
				renderNumbers(9, 34.7, 8, PpB, rend, texNum);
				break;
			default:
				break;
		}
		/* Score Rendering End */
		
		/* Tetris Counter Rendering */
		if (oldtetris != game.tetris)
		{
			tetrisAusgabe.Stelle_1 = 0;
			tetrisAusgabe.Stelle_2 = 0;
			tempTetris = game.tetris;
		}
		
		if (tempTetris >= 10)
		{
			tetrisAusgabe.Stelle_1++;
			tempTetris -= 10;
		}
		if (tempTetris >= 1)
		{
			tetrisAusgabe.Stelle_2++;
			tempTetris -= 1;
		}
		
		switch (tetrisAusgabe.Stelle_1)
		{
			case 0:
				renderNumbers(0, 29.7, 12, PpB, rend, texNum);
				break;
			case 1: 
				renderNumbers(1, 29.7, 12, PpB, rend, texNum);
				break;
			case 2:
				renderNumbers(2, 29.7, 12, PpB, rend, texNum);
				break;
			case 3:
				renderNumbers(3, 29.7, 12, PpB, rend, texNum);
				break;
			case 4:
				renderNumbers(4, 29.7, 12, PpB, rend, texNum);
				break;
			case 5:
				renderNumbers(5, 29.7, 12, PpB, rend, texNum);
				break;
			case 6:
				renderNumbers(6, 29.7, 12, PpB, rend, texNum);
				break;
			case 7:
				renderNumbers(7, 29.7, 12, PpB, rend, texNum);
				break;
			case 8:
				renderNumbers(8, 29.7, 12, PpB, rend, texNum);
				break;
			case 9:
				renderNumbers(9, 29.7, 12, PpB, rend, texNum);
				break;
			default:
				break;
		}
		
		switch (tetrisAusgabe.Stelle_2)
		{
			case 0:
				renderNumbers(0, 30.7, 12, PpB, rend, texNum);
				break;
			case 1: 
				renderNumbers(1, 30.7, 12, PpB, rend, texNum);
				break;
			case 2:
				renderNumbers(2, 30.7, 12, PpB, rend, texNum);
				break;
			case 3:
				renderNumbers(3, 30.7, 12, PpB, rend, texNum);
				break;
			case 4:
				renderNumbers(4, 30.7, 12, PpB, rend, texNum);
				break;
			case 5:
				renderNumbers(5, 30.7, 12, PpB, rend, texNum);
				break;
			case 6:
				renderNumbers(6, 30.7, 12, PpB, rend, texNum);
				break;
			case 7:
				renderNumbers(7, 30.7, 12, PpB, rend, texNum);
				break;
			case 8:
				renderNumbers(8, 30.7, 12, PpB, rend, texNum);
				break;
			case 9:
				renderNumbers(9, 30.7, 12, PpB, rend, texNum);
				break;
			default:
				break;
		}
		/* Tetris Counter Rendering End */
		
		/* Next Block Rendering */
		switch(game.next.type + 1)
		{
			case 1:
				RBlockI.x = 0.5 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockI.y = 1 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockI, NULL, &RBlockI);
				RBlockI.x = 1.5 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockI.y = 1 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockI, NULL, &RBlockI);
				RBlockI.x = 2.5 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockI.y = 1 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockI, NULL, &RBlockI);
				RBlockI.x = 3.5 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockI.y = 1 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockI, NULL, &RBlockI);
				break;
			case 2:
				RBlockL.x = 1 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockL.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockL, NULL, &RBlockL);
				RBlockL.x = 2 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockL.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockL, NULL, &RBlockL);
				RBlockL.x = 3 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockL.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockL, NULL, &RBlockL);
				RBlockL.x = 1 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockL.y = 1.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockL, NULL, &RBlockL);
				break;
			case 3:
				RBlockJ.x = 1 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockJ.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockJ, NULL, &RBlockJ);
				RBlockJ.x = 2 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockJ.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockJ, NULL, &RBlockJ);
				RBlockJ.x = 3 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockJ.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockJ, NULL, &RBlockJ);
				RBlockJ.x = 3 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockJ.y = 1.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockJ, NULL, &RBlockJ);
				break;
			case 4:
				RBlockO.x = 1.5 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockO.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockO, NULL, &RBlockO);
				RBlockO.x = 1.5 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockO.y = 1.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockO, NULL, &RBlockO);
				RBlockO.x = 2.5 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockO.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockO, NULL, &RBlockO);
				RBlockO.x = 2.5 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockO.y = 1.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockO, NULL, &RBlockO);
				break;
			case 5:
				RBlockS.x = 1 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockS.y = 1.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockS, NULL, &RBlockS);
				RBlockS.x = 2 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockS.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockS, NULL, &RBlockS);
				RBlockS.x = 3 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockS.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockS, NULL, &RBlockS);
				RBlockS.x = 2 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockS.y = 1.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockS, NULL, &RBlockS);
				break;
			case 6:
				RBlockT.x = 1 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockT.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockT, NULL, &RBlockT);
				RBlockT.x = 2 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockT.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockT, NULL, &RBlockT);
				RBlockT.x = 3 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockT.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockT, NULL, &RBlockT);
				RBlockT.x = 2 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockT.y = 1.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockT, NULL, &RBlockT);
				break;
			case 7:
				RBlockZ.x = 1 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockZ.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockZ, NULL, &RBlockZ);
				RBlockZ.x = 2 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockZ.y = 0.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockZ, NULL, &RBlockZ);
				RBlockZ.x = 3 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockZ.y = 1.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockZ, NULL, &RBlockZ);
				RBlockZ.x = 2 * PpB + (3*RESOLUTION_WIDTH/4) - 4*PpB;
				RBlockZ.y = 1.5 * PpB + 3*PpB;
				SDL_RenderCopy(rend, texBlockZ, NULL, &RBlockZ);
				break;
			default:
				break;
		} /* Next Block Rendering End */
		
		/* Main Rendering */
		for (int x = 0; x < game.columns; x++)
		{
			for (int y = 0; y < game.rows; y++)
			{
				switch (renderMap[y][x])
				{
					case 0: break;
					case 1: RBlockI.x = x * PpB + (RESOLUTION_WIDTH/2) - 5*PpB;
							RBlockI.y = y * PpB + 3*PpB;
								SDL_RenderCopy(rend, texBlockI, NULL, &RBlockI);
							break;
					case 2: RBlockL.x = x * PpB + (RESOLUTION_WIDTH/2) - 5*PpB;
							RBlockL.y = y * PpB + 3*PpB;
								SDL_RenderCopy(rend, texBlockL, NULL, &RBlockL);
							break;
					case 3: RBlockJ.x = x * PpB + (RESOLUTION_WIDTH/2) - 5*PpB;
							RBlockJ.y = y * PpB + 3*PpB;
								SDL_RenderCopy(rend, texBlockJ, NULL, &RBlockJ);
							break;
					case 4: RBlockO.x = x * PpB + (RESOLUTION_WIDTH/2) - 5*PpB;
							RBlockO.y = y * PpB + 3*PpB;
								SDL_RenderCopy(rend, texBlockO, NULL, &RBlockO);
							break;
					case 5: RBlockS.x = x * PpB + (RESOLUTION_WIDTH/2) - 5*PpB;
							RBlockS.y = y * PpB + 3*PpB;
								SDL_RenderCopy(rend, texBlockS, NULL, &RBlockS);
							break;
					case 6: RBlockT.x = x * PpB + (RESOLUTION_WIDTH/2) - 5*PpB;
							RBlockT.y = y * PpB + 3*PpB;
								SDL_RenderCopy(rend, texBlockT, NULL, &RBlockT);
							break;
					case 7: RBlockZ.x = x * PpB + (RESOLUTION_WIDTH/2) - 5*PpB;
							RBlockZ.y = y * PpB + 3*PpB;
								SDL_RenderCopy(rend, texBlockZ, NULL, &RBlockZ);
							break;
					default: break;
				}
			}
		} /* Main Rendering End */
		
		
		oldtetris = game.tetris;
		
		pLinesNeeded = tetrisCheckLines(pointGame, pLinesNeeded, maxlevel);
		
			
		if (oldlines != game.lines)
		{
			fprintf(print, "Lines: %d\n", game.lines);
			fprintf(print, "Score: %d\n", game.score);
			fprintf(print, "Level: %d\n\n", game.level);
		}
		
		oldlines = game.lines;
		
		tempTimer += 1;
		if (tempTimer >= FPS)
		{
			timer += 1;
			tempTimer -= FPS;
		}
		
		Framecounter += DIFFICULTY;
        /* Musik depending on level*/
        if(game.level!=z){
            switch(game.level){
                case 5: Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
                        Mix_Music* BackgroundMusic = Mix_LoadMUS("music/OcarinaOfTime");
                        Mix_PlayMusic(BackgroundMusic, -1);
                        z=game.level;
                        break;
                case 10: Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
                        Mix_Music* BackgroundMusic2 = Mix_LoadMUS("music/SuperMarioLand.mp3");
                        Mix_PlayMusic(BackgroundMusic2, -1);
                        z=game.level;
                        break;
                
            }
        }
            /*Musik depending on level ende*/
		SDL_RenderPresent(rend);


		/* On every Frame */
		SDL_Delay(1000/FPS);



	} /* Animation Ende */
    
    SDL_RenderCopy(rend, textGameOver, NULL, NULL);
    SDL_Delay(10000);
/*    SDL_Event event1;
    while(1){
        SDL_RenderCopy(rend, textGameOver, NULL, NULL);
  while (SDL_PollEvent(&event1))
    {
        if (event1.type == SDL_QUIT)
        {
            game.running = false;
        }
        
        else if (event1.type == SDL_KEYDOWN)
        {
            switch (event1.key.keysym.scancode)
            {
                case SDL_SCANCODE_ESCAPE:
                    break;
                default:
                    break;
            }
        }
        

        }
        break;
    }
 
    }*/
	/* Score Documentation */
	FILE* Scoreboard;
	Scoreboard = fopen("scoreboard.txt", "a");
	fprintf(Scoreboard, "Score: %d, achievedTetris: %d, reachedLevel: %d, while surviving for %d seconds.\n", game.score, game.tetris, game.level, timer);
	
	/* Resourcen schliessen */
	SDL_DestroyTexture(texBlockI);
	SDL_DestroyTexture(texBlockL);
	SDL_DestroyTexture(texBlockJ);
	SDL_DestroyTexture(texBlockS);
	SDL_DestroyTexture(texBlockT);
	SDL_DestroyTexture(texBlockO);
	SDL_DestroyTexture(texBlockZ);
	SDL_DestroyTexture(texEdges);
	SDL_DestroyTexture(texBackgroundImage);
	for (int i = 0; i < 10; i++)
	{
		SDL_DestroyTexture(texNum[i]);
	}
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	SDL_Quit();
	free(LinesNeeded);
	free(GravityArray);
	fclose(print);
	fclose(Scoreboard);
	
	return 0;
}

// renderNumbers(int zahl, float x_offset, float y_offset, PpB, rend, texNum);

void renderNumbers(int zahl, float x_offset, float y_offset, float PpB, SDL_Renderer* rend, SDL_Texture* texNum[])
{
	
	SDL_Rect Rrender;
	Rrender.x = x_offset*PpB;
	Rrender.y = y_offset*PpB;
	Rrender.w = PpB;
	Rrender.h = PpB;
	
	switch(zahl)
	{
	case 0:
		SDL_RenderCopy(rend, texNum[0], NULL, &Rrender);
		break;
	case 1:
		SDL_RenderCopy(rend, texNum[1], NULL, &Rrender);
		break;
	case 2:
		SDL_RenderCopy(rend, texNum[2], NULL, &Rrender);
		break;
	case 3:
		SDL_RenderCopy(rend, texNum[3], NULL, &Rrender);
		break;
	case 4:
		SDL_RenderCopy(rend, texNum[4], NULL, &Rrender);
		break;
	case 5:
		SDL_RenderCopy(rend, texNum[5], NULL, &Rrender);
		break;
	case 6:
		SDL_RenderCopy(rend, texNum[6], NULL, &Rrender);
		break;
	case 7:
		SDL_RenderCopy(rend, texNum[7], NULL, &Rrender);
		break;
	case 8:
		SDL_RenderCopy(rend, texNum[8], NULL, &Rrender);
		break;
	case 9:
		SDL_RenderCopy(rend, texNum[9], NULL, &Rrender);
		break;
	default: 
		break;
	}
}
