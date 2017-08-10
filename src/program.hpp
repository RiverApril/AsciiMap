//
//  program.hpp
//  AsciiMap
//
//  Created by Braeden Atlee on 7/27/17.
//  Copyright © 2017 Braeden Atlee. All rights reserved.
//

#ifndef program_hpp
#define program_hpp

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <ctime>
#include <vector>
#include <string>

using namespace std;


#define fontW 7
#define fontH 12

extern string initalOpenPath;

extern int activeLayer;
extern int activeColorLayer;
extern vector<bool> visibleLayers;
extern vector<vector<vector<char>>> displayBuffer;
extern vector<vector<char>> clipboard;

extern vector<vector<vector<vector<char>>>> undoMem;
extern int undoPoint;


extern SDL_Window* mainWindow;
extern SDL_Renderer* mainRenderer;

extern SDL_Texture* fontTexture;
extern SDL_Texture* uiTexture;

extern int editorX;
extern int editorY;
extern int editorW;
extern int editorH;
extern int editorScrollX;
extern int editorScrollY;

extern int selectAX;
extern int selectAY;
extern int selectBX;
extern int selectBY;
extern int selectCX;
extern int selectCY;
extern int hoverX;
extern int hoverY;

extern int mapW;
extern int mapH;

extern int mouseX;
extern int mouseY;

void refresh();

bool init();
bool update();
void cleanup();

void mouseButtonUp(int button, int x, int y);
void mouseButtonDown(int button, int x, int y);
void mouseButtonMove(int x, int y);
void keyTyped(char c);
void moveSelect(int dx, int dy);
void scroll(int sx, int sy);
void draw();

void copySelect();
void deleteSelect();
void pasteSelect();
void pasteSelectIgnore(char ignoreChar);

void debug(const char* fmt, ...);
string format(const char* fmt, ...);


struct Color{
    int r, g, b;
};

void setUiColor(Color c);
void setTextColor(Color c);
void drawChar(char c, int x, int y);
void drawText(string s, int x, int y);
void drawRect(int x, int y, int w, int h);
void drawUi(int sx, int sy, int dx, int dy, int w, int h);

void promptIfNeverSavedAndSaveFile();
void promptAndSaveFile();
void promptAndLoadFile();

void saveFile();
void loadFile();


void undo();
void redo();



#endif /* program_hpp */
