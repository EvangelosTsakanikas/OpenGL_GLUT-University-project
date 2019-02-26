#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <GL/gl.h>     // openGL header
#include <GL/glu.h>   // glut header
#include <GL/glut.h>   // glut header
#include <GL/freeglut.h>

#define WIDTH 1000
#define HEIGHT 1000

#define KEY_W (key == 'W' || key == 'w')
#define KEY_S (key == 'S' || key == 's')
#define KEY_A (key == 'A' || key == 'a')
#define KEY_D (key == 'D' || key == 'd')
#define KEY_Q (key == 'Q' || key == 'q')
#define KEY_E (key == 'E' || key == 'e')
#define KEY_R (key == 'R' || key == 'r')
#define KEY_SPACEBAR (key == 32)
#define KEY_ESC (key == 27)
#define KEY_V (key == 'V' || key == 'v')

int id;
int N, x, y, z, scoreLimit;
int ***grid;
int stateAfterPressingR;
int stateBeforePressingR;
int directionOfMovement = 0;
int directionOfRotation = 0;
int drops = 0;
int score = 50;
int stock = 0;
int lives = 3;
int level = 0;
int movement = 1;
int zFallLimit = -10;

double angle = 0;

char gameStatus[40];

bool reachedLevelNForTheFirstTime = true;
bool lookStraightAhead = true;
bool freeFall = false;
bool victory = false;

void window_display();
void window_idle();
void render_scene();
void changeSize(int w, int h);
void keyboardNormal(unsigned char key, int x, int y);
void mouseButton(int button, int state, int x, int y);
void constructGrid();
void setCamera();
void showDetailsForLivesPointsStockColor();
void renderBitmapString2D(float x, float y, void *font, char *string);
void updateInfoAfterPressingLeftClick(int i, int j, int levelAfterTheFall);
void updateScoreAfterTheFall(int levelAfterTheFall);
void doTheFall(int i, int j);
void destroyAllFrontCubes(int i, int j);
void dropAllCubes();
void initLights();
void printGameResult(char* gameStatus);

int checkIfYouCanClimb(int i, int j);
int searchForEmptyZLevelUpwards(int i, int j);
int searchForEmptyZLevelDownwards(int i, int j);
int searchForConsecutiveCubes(int i, int j);

void renderBitmapString2D(float x, float y, void *font, char *string) 
{   
    char *character;
	glRasterPos2i(x, y);
    for (character = string; *character != '\0'; character++) 
	{
        glutBitmapCharacter(font, *character);
    }
}

void showDetailsForLivesPointsStockColor()
{
	char buffer[256];
	char colorBuffer[20];
	char levelBuffer[5];
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glPushMatrix();
		
		gluOrtho2D(0.0, WIDTH, 0.0, HEIGHT);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
				
		sprintf(buffer, "Lives: %d , Score: %d , Stock: %d", lives, score, stock);
		sprintf(levelBuffer, "Level: %d", level);
		
		glColor3f(1.0f, 1.0f, 1.0f);
		renderBitmapString2D(10, 980, GLUT_BITMAP_HELVETICA_18, buffer);
		renderBitmapString2D(50, 950, GLUT_BITMAP_TIMES_ROMAN_24, levelBuffer);
		
		switch(grid[x][y][z-1])
		{
			case 0: 
				glColor3f(1.0f, 0.0f, 0.0f); // red
				sprintf(colorBuffer, "Current Cube Color: %s", "red");
				break;				
			case 1: 
				glColor3f(0.0f, 0.0f, 1.0f); //blue
				sprintf(colorBuffer, "Current Cube Color: %s", "blue");
				break;
			case 2: 
				glColor3f(0.0f, 1.0f, 0.0f); //green
				sprintf(colorBuffer, "Current Cube Color: %s", "green");
				break;
			case 3: 
				glColor3f(1.0f, 1.0f, 0.0f); //yellow
				sprintf(colorBuffer, "Current Cube Color: %s", "yellow");
				break;
			case 101: 
				glColor3f(1.0f, 0.0f, 1.0f); //magenta
				sprintf(colorBuffer, "Current Cube Color: %s", "magenta");
				break;
		}
		
		renderBitmapString2D((WIDTH/2.0f) - 80, 970, GLUT_BITMAP_TIMES_ROMAN_24, colorBuffer);		
		
	glPopMatrix();
}

void constructGrid()
{				
	srand(time(NULL));
	
	grid = new int**[N];
	for (int i = 0; i < N; i++)
	{
		grid[i] = new int*[N];
		for (int j = 0; j < N; j++)
		{
			grid[i][j] = new int[N];
			for (int k = 0; k < N; k++)
			{
				if (k != 0)
				{
					grid[i][j][k] = -1;
				}
				else
				{
					if (i == round((N-1) / 2.0) && (j == round((N-1) / 2.0)))
					{
						grid[i][j][k] = 101; // 1 0 1 sthn glColor3f dinei to magenta
					}						
					else
					{
						grid[i][j][k] = rand() % 4;
					}
				}
			}
		}
	}
}

void changeSize(int w, int h)
{
	if (h == 0)
		h = 1;
	float ratio =  w * 1.0 / h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);		
	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(110.0f, ratio, 0.3f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
}

void initLights()
{	
	glEnable(GL_COLOR_MATERIAL);	
	glShadeModel(GL_SMOOTH);
	
	GLfloat leftNearLight_Position[] = {0.0f, 0.0f, N, 1.0f };
	GLfloat rightNearLight_Position[] = { N-1, 0.0f, N, 1.0f };
	GLfloat lefFarLight_Position[] = {  0.0f, N-1, N, 1.0f };	
	GLfloat rightFarLight_Position[] = { N-1, N-1, N,  1.0f };
	
	GLfloat ambientLight[] = {0.0f, 0.0f, 0.0f, 1.0f};
	GLfloat diffuseLight[] = {1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat specularLight[] = {1.0f, 1.0f, 1.0f, 1.0f };	
			
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_LIGHT3);
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0,GL_DIFFUSE, diffuseLight);	
	glLightfv(GL_LIGHT0,GL_SPECULAR, specularLight);
	glLightfv(GL_LIGHT0, GL_POSITION, rightNearLight_Position);
	
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specularLight);		
	glLightfv(GL_LIGHT1, GL_POSITION, leftNearLight_Position);
	
	glLightfv(GL_LIGHT2, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT2, GL_SPECULAR, specularLight);	
	glLightfv(GL_LIGHT2, GL_POSITION, lefFarLight_Position);

	glLightfv(GL_LIGHT3, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT3, GL_SPECULAR, specularLight);
	glLightfv(GL_LIGHT3, GL_POSITION, rightFarLight_Position);
}

void setCamera()
{
	if (lookStraightAhead == true)
	{
		glRotatef(-90, 1, 0, 0);
	}
	else
	{
		glRotatef(-50, 1, 0, 0);
	}
		
	if (directionOfRotation != angle){
		if (directionOfRotation < angle)
		{
			angle -= 18;
		}
		else
		{
			angle += 18;
		}
	}
	else
	{
		movement = 1; // enables movement when turning finished
	}
	
	glRotatef(angle, 0.0, 0.0, 1.0);		
	glTranslatef(-(x), -(y), -(z+1));
}

void window_display()
{		
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);	
		
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth(1);
	
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
    gluPerspective(110.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.3, 1000.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	render_scene();
	
	glutSwapBuffers();
}

void render_scene()
{								
	setCamera();
			
	if (lives >= 0)
	{
		initLights();
		for (int i=0; i<N; i++)
		{
			for (int j=0; j<N; j++)
			{
				for (int k=0; k<N; k++)
				{
					if (grid[i][j][k]!=-1)
					{
						glPushMatrix();							
							glTranslatef(i, j, k);
							switch (grid[i][j][k])
							{
								case 0:		
									glColor3f(1.0f, 0.0f, 0.0f);	//red															
									break;
								case 1:
									glColor3f(0.0f, 0.0f, 1.0f);	//blue								
									break;
								case 2:
									glColor3f(0.0f, 1.0f, 0.0f);	//green	
									break;
								case 3:
									glColor3f(1.0f, 1.0f, 0.0f);	//yellow					
									break;
								case 101:
									glColor3f(1.0f, 0.0f, 1.0f);	//magenta
									break;
							}
							glutSolidCube(1.0f);
									
							glColor3f(0.0f, 0.0f, 0.0f);
							
							glutWireCube(1.0f);
							glutWireCube(1.001f);
							glutWireCube(1.002f);
							glutWireCube(1.003f);
							glutWireCube(0.999f);
							glutWireCube(0.998f);
							glutWireCube(0.997f);
								
						glPopMatrix();
					}
				}
			}
		}
	}
	else
	{
		glColor3f(1.0f, 0.0f, 0.0f);
		sprintf(gameStatus, "%s", "DEFEATED");
		printGameResult(gameStatus);		
		return;
	}
	
	if (score >= scoreLimit)
	{
		glPushMatrix();
			victory = true;
			glColor3f(0.0f, 1.0f, 0.0f);
			sprintf(gameStatus, "%s", "VICTORIOUS");
			printGameResult(gameStatus);
			showDetailsForLivesPointsStockColor();
		glPopMatrix();
	}
	
	if (victory == true)
	{
		glColor3f(0.0f, 1.0f, 0.0f);
		printGameResult(gameStatus);
		showDetailsForLivesPointsStockColor();
	}
	
	if (drops != 0)
	{
		stateBeforePressingR = 0;
		dropAllCubes();	
	}
	
	if(freeFall == true)
	{		
		z--;
		if (z == zFallLimit)
		{
			freeFall = false;
			x = round((N-1)/2.0);
			y = round((N-1)/2.0);
			z = 1;
			grid[x][y][z] = -1;
			grid[x][y][z+1] = -1;
		}
	}
	
	if (stateBeforePressingR != stateAfterPressingR && drops == 0) // vale 10 sto score mono otan epese kouti me to R, alliws oxi.
	{		
		score += 10;
		stateAfterPressingR = 0;
	}
	glDisable(GL_LIGHTING);
	showDetailsForLivesPointsStockColor();
	glEnable(GL_LIGHTING);
}

void printGameResult(char* gameStatus)
{
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glPushMatrix();
	
		gluOrtho2D(0.0, WIDTH, 0.0, HEIGHT);
	
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		renderBitmapString2D((WIDTH/2.0f) - 50, (HEIGHT/2.0f), GLUT_BITMAP_TIMES_ROMAN_24, gameStatus);
		
	glPopMatrix();
	glEnable(GL_LIGHTING);	
}

void dropAllCubes()
{
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			for (int k = 1; k < N; k++)
			{
				if (grid[i][j][k] != -1 && grid[i][j][k-1]==-1)
				{	
					
					if (i == x && j == y && k-1 == z+1)
					{						
						continue;
					}
					grid[i][j][k-1] = grid[i][j][k];
					grid[i][j][k] = -1;
					
					stateAfterPressingR = 1;
					
					if (grid[x][y][z-1] == -1 && z > 1)
					{
						z -= 1;
						level--;
					}
					
					if (k == 1)
					{
						grid[i][j][k-1] = -1;
						if (grid[x][y][z-1] == -1)
						{	
							freeFall = true;							
						}						
					}	
				}								
			}			
			glutPostRedisplay();				
		}
	}
	drops --;
}

void keyboardNormal(unsigned char key, int xp, int yp)
{
	int levelAfterTheFall;
	if (KEY_W)
	{
		switch(directionOfMovement)
		{
			case 0: // forward			
				if (y + 1 < N && movement == 1)
				{
					if (checkIfYouCanClimb(x, y+1))
					{
						y += 1;
						z += 1;
						score += 5;
					}
					else if (grid[x][y+1][z] == -1 && z < N)
					{
						if (grid[x][y+1][z+1] == -1 && z < N-1)
						{
							y += 1;							
							doTheFall(x, y);							
						}
						if (z == N - 1)
						{
							y += 1;
							doTheFall(x, y);
						}												
					}
					else if (z == N)
					{
						y += 1;
						doTheFall(x, y);						
					}
				}													
				break;
								
			case 90: // right
			case -270: //			
				if (x + 1 < N && movement == 1)
				{
					if (checkIfYouCanClimb(x+1, y))																
					{
						x += 1;
						z += 1;
						score += 5;
					}
					else if (grid[x+1][y][z] == -1 && z < N)
					{
						if (grid[x+1][y][z+1] == -1 && z < N-1)
						{
							x += 1;
							doTheFall(x, y);							
						}
						if (z == N - 1)
						{
							x += 1;
							doTheFall(x, y);							
						}												
					}
					else if (z == N)
					{
						x += 1;
						doTheFall(x, y);						
					}
				}				
				break;				
				
			case 180: // rear
			case -180: //			
				if (y - 1 >= 0 && movement == 1)
				{
					if (checkIfYouCanClimb(x, y-1))
					{
						y -= 1;
						z += 1;
						score += 5;
					}
					else if (grid[x][y-1][z] == -1 && z < N)
					{
						if (grid[x][y-1][z+1] == -1 && z < N-1)
						{
							y -= 1;
							doTheFall(x, y);							
						}
						if (z == N - 1)
						{
							y -= 1;
							doTheFall(x, y);							
						}												
					}
					else if (z == N)
					{
						y -= 1;
						doTheFall(x, y);						
					}
				}				
				break;
				
			case 270: // left
			case -90: //
				if (x - 1 >= 0 && movement == 1)
				{
					if (checkIfYouCanClimb(x-1, y))
					{
						x -= 1;
						z += 1;
						score += 5;
					}
					else if (grid[x-1][y][z] == -1 && z < N)
					{
						if (grid[x-1][y][z+1] == -1 && z < N-1)
						{
							x -= 1;
							doTheFall(x, y);							
						}
						if (z == N - 1)
						{
							x -= 1;
							doTheFall(x, y);							
						}												
					}
					else if (z == N)
					{
						x -= 1;
						doTheFall(x, y);						
					}
				}				
				break;
		}		
		level = z;
		if (level == N && reachedLevelNForTheFirstTime == true)
		{
			reachedLevelNForTheFirstTime = false;
			lives++;
			score += 100;
		}		
	}

	if (KEY_S)
	{		
		switch(directionOfMovement)
		{
			case 0:
				if (y - 1 >= 0 && movement == 1)
				{
					if (checkIfYouCanClimb(x, y-1))
					{
						y -= 1;
						z += 1;
						score += 5;
					}
					else if (grid[x][y-1][z] == -1 && z < N)
					{
						if (grid[x][y-1][z+1] == -1 && z < N-1)
						{
							y -= 1;
							doTheFall(x, y);						
						}
						if (z == N - 1)
						{
							y -= 1;
							doTheFall(x, y);							
						}												
					}
					else if (z == N)
					{
						y -= 1;
						doTheFall(x, y);						
					}
				}				
				break;
			case 90:
			case -270:
				if (x - 1 >= 0 && movement == 1)
				{
					if (checkIfYouCanClimb(x-1, y))
					{
						x -= 1;
						z += 1;
						score += 5;
					}
					else if (grid[x-1][y][z] == -1 && z < N)
					{
						if (grid[x-1][y][z+1] == -1 && z < N-1)
						{
							x -= 1;
							doTheFall(x, y);							
						}
						if (z == N - 1)
						{
							x -= 1;
							doTheFall(x, y);							
						}												
					}
					else if (z == N)
					{
						x -= 1;
						doTheFall(x, y);						
					}
				}				
				break;
			case 180:
			case -180:
				if (y + 1 < N && movement == 1)
				{
					if (checkIfYouCanClimb(x, y+1))
					{
						y += 1;
						z += 1;
						score += 5;
					}
					else if (grid[x][y+1][z] == -1 && z < N)
					{
						if (grid[x][y+1][z+1] == -1 && z < N-1)
						{
							y += 1;
							doTheFall(x, y);						
						}
						if (z == N - 1)
						{
							y += 1;
							doTheFall(x, y);	
						}												
					}
					else if (z == N)
					{
						y += 1;
						doTheFall(x, y);					
					}
				}					
				break;
			case 270:
			case -90:
				if (x + 1 < N && movement == 1)
				{
					if (checkIfYouCanClimb(x+1, y))																
					{
						x += 1;
						z += 1;
						score += 5;
					}
					else if (grid[x+1][y][z] == -1 && z < N)
					{
						if (grid[x+1][y][z+1] == -1 && z < N-1)
						{
							x += 1;
							doTheFall(x, y);						
						}
						if (z == N - 1)
						{
							x += 1;
							doTheFall(x, y);							
						}												
					}
					else if (z == N)
					{
						x += 1;
						doTheFall(x, y);
					}
				}				
				break;				
		}
		level = z;
		if (level == N && reachedLevelNForTheFirstTime == true)
		{
			reachedLevelNForTheFirstTime = false;
			lives++;
			score += 100;
		}		
	}
	
	if (KEY_A && movement == 1)
	{
		movement = 0;
		
		directionOfMovement -= 90;		
		directionOfMovement = directionOfMovement % 360;
		
		if ((int)angle % 90 == 0)
		{
			directionOfRotation -= 90;
			if (directionOfRotation < 0)
			{
				angle = 360;
				directionOfRotation = 360 + directionOfRotation;
			}
		}	
	}
	if (KEY_D && movement == 1)
	{
		movement = 0;
		
		directionOfMovement += 90;
		directionOfMovement = directionOfMovement % 360;
		
		if ((int)angle % 90 == 0)
		{
			if (directionOfRotation == 360)
			{
				directionOfRotation = 0;
				angle = 0;
			}
			directionOfRotation += 90;
		}		
	}
	
	if (KEY_E)
	{
		switch(directionOfMovement)
		{
			case 0:			
				if (y + 1 < N)
				{		
					destroyAllFrontCubes(x, y+1);					
				}
				break;				
			case 90:
			case -270:
				if (x + 1 < N)
				{
					destroyAllFrontCubes(x+1, y);
				}	
				break;
			case 180:
			case -180:
				if (y - 1 >= 0)
				{
					destroyAllFrontCubes(x, y-1);
				}
				break;
			case 270:
			case -90:
				if (x - 1 >= 0 )
				{
					destroyAllFrontCubes(x-1, y);
				}
				break;				
		}
	}
	
	if (KEY_Q)
	{
		switch(directionOfMovement)
		{
			case 0:			
				if (y + 1 < N)
				{		
					grid[x][y+1][z] = -1;					
				}
				break;				
			case 90:
			case -270:
				if (x + 1 < N)
				{
					grid[x+1][y][z] = -1;
				}	
				break;
			case 180:
			case -180:
				if (y - 1 >= 0)
				{
					grid[x][y-1][z] = -1;
				}
				break;
			case 270:
			case -90:
				if (x - 1 >= 0 )
				{
					grid[x-1][y][z] = -1;
				}
				break;				
		}
	}
	
	if (KEY_SPACEBAR)
	{
		switch(grid[x][y][z-1]){
			case 0: // red
				if (score >= 5)
				{
					stock++;				
					score -= 5;
					grid[x][y][z-1] = 3; // yellow	
				}							
				break;
			case 2:	// green
				if (score >= 5)
				{
					stock++;
					score -= 5;
					grid[x][y][z-1] = 0; // red
				}
	
				break;
			case 3: // yellow
				if (score >= 5)
				{
					stock++;
					score -= 5;
					grid[x][y][z-1] = 1; // blue
				}

				break;
		}
	}
	
	if (KEY_R)
	{	
		if (drops == 0)
		{
			drops = 10;
		}
	}
	
	if(KEY_ESC)
	{
		glutDestroyWindow(id);
	}
	
	if (KEY_V)
	{
		lookStraightAhead = !lookStraightAhead;
	}
}

int checkIfYouCanClimb(int i, int j)
{
	if (grid[i][j][z] != -1 && grid[i][j][z+1] == -1 && grid[i][j][z+2] == -1)
	{
		return 1;
	}
	if (z >= N-2 && z < N && grid[i][j][z] != -1)
	{
		return 1;
	}
	return 0;
}

void doTheFall(int i, int j)
{
	int levelAfterTheFall = searchForEmptyZLevelDownwards(i, j);
	if (levelAfterTheFall == 1)
	{							
		if (grid[i][j][levelAfterTheFall-1] == -1)
		{	
			lives--;			
			freeFall = true;																
		}
		else
		{
			updateScoreAfterTheFall(levelAfterTheFall);
			z = levelAfterTheFall;
		}		
	}
	else
	{
		updateScoreAfterTheFall(levelAfterTheFall);	
		z = levelAfterTheFall;
	}	
}
				
int searchForEmptyZLevelDownwards(int i, int j)
{
	for (int k = z; k >= 1; k--)
	{
		if (grid[i][j][k] != -1 && k < N)
		{
			return k+1;
		}
	}
	return 1;
}

void updateScoreAfterTheFall(int levelAfterTheFall)
{
	if (z - levelAfterTheFall > 1)
	{
		score -= ((z-levelAfterTheFall-1)) * 5;
		if (score < 0)
		{			
			lives--;
			score = 0;
		}
	}
}

void destroyAllFrontCubes(int i, int j)
{
	int counter = 0;
	for (int k = 0; k < N; k++)
	{	
		if (grid[i][j][k] != -1)
		{
			counter++;
		}					
	}
	if (counter > 0 && score >= 20)
	{
		for (int k = N; k >= 0; k--)
		{
			if (grid[i][j][k] != 101)
			{
				grid[i][j][k] = -1;
			}
			else
			{
				return;
			}				
		}
		lives++;
		score -= 20;
	}	
}

void mouseButton(int button, int state, int x1, int y1)
{
	int levelToAddNewCube;
	
	if (button == GLUT_LEFT_BUTTON && (state == GLUT_DOWN) && stock > 0)
	{		
		switch(directionOfMovement)
		{
			case 0:
				if (y + 1 < N && movement == 1)
				{
					levelToAddNewCube = searchForEmptyZLevelUpwards(x, y+1);
					if (levelToAddNewCube > -1)
					{
						updateInfoAfterPressingLeftClick(x, y+1, levelToAddNewCube);
					}
				}				
				break;
				
			case 90:
			case -270:
				if (x + 1 < N && movement == 1)
				{
					levelToAddNewCube = searchForEmptyZLevelUpwards(x+1, y);
					if (levelToAddNewCube > -1)
					{
						updateInfoAfterPressingLeftClick(x+1, y, levelToAddNewCube);
					}		
				}										
				break;
				
			case 180:
			case -180:
				if (y - 1 >= 0 && movement == 1)
				{
					levelToAddNewCube = searchForEmptyZLevelUpwards(x, y-1);
					if (levelToAddNewCube > -1)
					{					
						updateInfoAfterPressingLeftClick(x, y-1, levelToAddNewCube);	
					}			
				}										
				break;
				
			case 270:
			case -90:
				if (x - 1 >= 0 && movement == 1)
				{
					levelToAddNewCube = searchForEmptyZLevelUpwards(x-1, y);
					if (levelToAddNewCube > -1)
					{	
						updateInfoAfterPressingLeftClick(x-1, y, levelToAddNewCube);					
					}							
				}				
				break;
		}
	}
	
	if (button == GLUT_RIGHT_BUTTON && (state == GLUT_DOWN))
	{
		int consecutiveCubes = 0;
		int var = 0;
		switch(directionOfMovement)
		{
			case 0:
				for (int j = y+1; j < N; j++)
				{
					if (grid[x][j][z] != -1)
					{
						consecutiveCubes++;
					}
					else
					{
						break;
					}
				}
				var = y+1 + consecutiveCubes;
				if (var < N)
				{
					for (int j = var; j >= y+1; j--)
					{
						grid[x][j][z] = grid[x][j-1][z];
					}
				}
				else
				{
					for (int j = var-1; j >= y+1; j--)
					{
						grid[x][j][z] = grid[x][j-1][z];
					}
				}				
				break;
			
			case 90:
			case -270:			
				for (int i = x+1; i < N; i++)
				{
					if (grid[i][y][z] != -1)
					{
						consecutiveCubes++;
					}
					else
					{
						break;
					}
				}
				var = x+1 + consecutiveCubes;
				if (var < N)
				{
					for (int i = var; i >= x+1; i--)
					{
						grid[i][y][z] = grid[i-1][y][z];
					}
				}
				else
				{
					for (int i = var-1; i >= x+1; i--)
					{
						grid[i][y][z] = grid[i-1][y][z];
					}
				}
				break;
				
			case 180:
			case -180:				
				for (int j = y-1; j >= 0; j--)
				{
					if (grid[x][j][z] != -1)
					{
						consecutiveCubes++;
					}
					else
					{
						break;
					}
				}
				
				var = y-1 - consecutiveCubes;
				if (var >= 0)
				{
					for (int j = var; j <= y-1; j++)
					{
						grid[x][j][z] = grid[x][j+1][z];
					}
				}
				else
				{
					for (int j = var+1; j <= y-1; j++)
					{
						grid[x][j][z] = grid[x][j+1][z];
					}
				}
				break;
				
			case 270:
			case -90:
				for (int i = x-1; i >= 0; i--)
				{
					if (grid[i][y][z] != -1)
					{
						consecutiveCubes++;
					}
					else
					{
						break;
					}
				}
				var = x-1 - consecutiveCubes;
				if (var >= 0)
				{
					for (int i = var; i <= x+1; i++)
					{
						grid[i][y][z] = grid[i+1][y][z];
					}
				}
				else
				{
					for (int i = var+1; i <= x+1; i++)
					{
						grid[i][y][z] = grid[i+1][y][z];
					}
				}
				break;
		}
	}
}

int searchForEmptyZLevelUpwards(int i, int j)
{
	
	for (int k = z; k <= N; k++)
	{
		if (grid[i][j][k] == -1)
		{
			return k;
		}
	}
	return -1;
}

void updateInfoAfterPressingLeftClick(int i, int j, int levelAfterTheFall)
{
	if (levelAfterTheFall < N)
	{
		grid[i][j][levelAfterTheFall] = rand() % 4;
		score += 5;
		stock--;
	}	
}

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		printf("Den exeis dwsei swsta th diastash tou plegmatos(N) kai to score pou prepei na ftaseis.\nKsanaprospathise dinontas p.x. : ./Project 10 250\n");
	}
	else
	{
		N = atoi(argv[1]);
		scoreLimit = atoi(argv[2]);
		x = round((N-1)/2.0);
		y = round((N-1)/2.0);		
		z = 1;
		level = z;
		
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
		//glutInitWindowPosition(100,100);
		glutInitWindowSize(WIDTH, HEIGHT);
		id = glutCreateWindow("Project - GLUT ");
					
		glutDisplayFunc(window_display);
		glutIdleFunc(window_display);
		glutReshapeFunc(changeSize);
		
		glutKeyboardFunc(keyboardNormal);
		glutMouseFunc(mouseButton);
		
		constructGrid();
				
		glutMainLoop();
	}	
	return 1;
}