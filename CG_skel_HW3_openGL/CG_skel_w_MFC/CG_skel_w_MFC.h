#pragma once

#include "resource.h"

void display( void );
void idle();
void reshape( int width, int height );
void keyboard( unsigned char key, int x, int y );
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void special(int key, int x, int y);
void mouseWheel(int wheel, int direction, int x, int y);
void fileMenu(int id);
void mainMenu(int id);
void newMenuCallback(int id);
void setMenuCallback(int id);
void shadingMenuCallback(int id);
void setMenuCallback(int id);
void toggleMenuCallback(int id);

void initMenu();