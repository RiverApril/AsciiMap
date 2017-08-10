//
//  program.cpp
//  AsciiMap
//
//  Created by Braeden Atlee on 7/27/17.
//  Copyright © 2017 Braeden Atlee. All rights reserved.
//

#include <iostream>

#include "program.hpp"
#include "UIElement.hpp"

#define MAX_UNDOS 100

int activeLayer = 0;
int activeColorLayer = -1;
vector<bool> visibleLayers;
vector<vector<vector<char>>> displayBuffer; // layer, x, y
vector<vector<char>> clipboard;

vector<vector<vector<vector<char>>>> undoMem;
int undoPoint = 0;

string filename = "";

int metaDataSize = 0;
unsigned char* metaData = 0;

int editorX = 92;
int editorY = 20;
int editorW = 1;
int editorH = 1;
int editorScrollX = 0;
int editorScrollY = 0;

int selectAX = -1;
int selectAY = -1;
int selectBX = -1;
int selectBY = -1;
int selectCX = -1;
int selectCY = -1;
int hoverX = -1;
int hoverY = -1;

int mapW = 60;
int mapH = 30;

int mouseX = 0;
int mouseY = 0;

bool dragging = false;

vector<UIElement*> uiElements;

UILayers* uiLayers;

void clearUndo(){
    undoPoint = 0;
    undoMem.clear();
}

void saveUndoPoint(){
    if(undoPoint > 0){
        undoMem.erase(undoMem.end()-undoPoint, undoMem.end());
        undoPoint = 0;
    }
    undoMem.push_back(displayBuffer);
}

void undo(){
    if(((int)undoMem.size()) - undoPoint - 2 >= 0){
        displayBuffer.clear();
        displayBuffer = vector<vector<vector<char>>>(undoMem[undoMem.size() - undoPoint - 2]);
        if(displayBuffer.size() > 0){
            mapW = (int)displayBuffer[0].size();
            mapH = (int)displayBuffer[0][0].size();
        }else{
            mapW = 0;
            mapH = 0;
        }
        undoPoint++;
    }
}

void redo(){
    if(undoPoint > 0){
        undoPoint--;
        displayBuffer.clear();
        displayBuffer = vector<vector<vector<char>>>(undoMem[undoMem.size() - undoPoint - 1]);
    }
}

int makeNewLayer(){
    int layer = (int)displayBuffer.size();
    displayBuffer.push_back(vector<vector<char>>());
    visibleLayers.push_back(true);
    
    for(int i=0;i<mapW;i++){
        displayBuffer[layer].push_back(vector<char>());
        for(int j=0;j<mapH;j++){
            displayBuffer[layer][i].push_back(' ');
        }
    }
    saveUndoPoint();
    return layer;
}

void resize(int width, int height){
    for(int i = 0; i < displayBuffer.size(); i++){
        vector<vector<char>> prev;
        prev.insert(prev.begin(), displayBuffer[i].begin(), displayBuffer[i].end());
        
        displayBuffer[i].clear();
        for(int x = 0; x < width; x++){
            displayBuffer[i].push_back(vector<char>());
            for(int y = 0; y < height; y++){
                if(x < mapW && y < mapH){
                    displayBuffer[i][x].push_back(prev[x][y]);
                }else{
                    displayBuffer[i][x].push_back(' ');
                }
            }
        }
    }
    
    mapW = width;
    mapH = height;
    saveUndoPoint();
}

void actionPromptResize(UIButton* button, int mb){
    printf("\n- Resize -\n");
    printf("Current Size: %dx%d\n", mapW, mapH);
    int width, height;
    printf("Enter new width: ");
    cin >> width;
    printf("Enter new height: ");
    cin >> height;
    resize(width, height);
    printf("- Done -\n");
}

void actionOpen(UIButton* button, int mb){
    promptAndLoadFile();
}

void actionSave(UIButton* button, int mb){
    promptIfNeverSavedAndSaveFile();
}

void actionSaveAs(UIButton* button, int mb){
    promptAndSaveFile();
}

void updateLayers(){
    uiLayers->updateSize();
}

void actionChangeLayer(UIList* list, int mb){
    if(mb == SDL_BUTTON_LEFT){
        activeLayer = list->selected;
    } else if(mb == SDL_BUTTON_RIGHT){
        if(activeColorLayer == list->selected){
            activeColorLayer = -1;
        }else{
            activeColorLayer = list->selected;
        }
    }
    updateLayers();
}

void actionDeleteLayer(UIButton* button, int mb){
    displayBuffer.erase(displayBuffer.begin()+activeLayer);
    activeLayer = 0;
    updateLayers();
    saveUndoPoint();
}

void actionAddLayer(UIButton* button, int mb){
    activeLayer = makeNewLayer();
    updateLayers();
    saveUndoPoint();
}

bool init(){
    
    activeLayer = makeNewLayer();
    
    int x = 2;
    uiElements.push_back(new UIButton("Open", x, 2, actionOpen));
    x += 4+(fontW*4)+4;
    uiElements.push_back(new UIButton("Save", x, 2, actionSave));
    x += 4+(fontW*4)+4;
    uiElements.push_back(new UIButton("Save As", x, 2, actionSaveAs));
    x += 4+(fontW*7)+4;
    uiElements.push_back(new UIButton("Resize", x, 2, actionPromptResize));
    x += 4+(fontW*6)+4;
    
    int y = 2;
    y += 4+(fontH)+4;
    y += 4+(fontH)+4;
    uiElements.push_back(new UIButton("Add Layer", 2, y, actionAddLayer));
    y += 4+(fontH)+4;
    uiElements.push_back(new UIButton("Delete Layer", 2, y, actionDeleteLayer));
    y += 4+(fontH)+4;
    
    uiLayers = new UILayers(visibleLayers, activeLayer, activeColorLayer, 2, y);
    uiElements.push_back(uiLayers);
    updateLayers();
    
    if(!initalOpenPath.empty()){
        filename = initalOpenPath;
        loadFile();
    }
    
    return true;
}

bool update(){
    refresh();
    return true;
}

void draw(){
    for(UIElement* el : uiElements){
        el->draw();
    }
}

void cleanup(){
    
}

void mouseButtonDown(int button, int x, int y){
    if(button == SDL_BUTTON_LEFT){
        if(x >= editorX && y >= editorY){
            int cx = (x-editorX+editorScrollX)/fontW;
            int cy = (y-editorY+editorScrollY)/fontH;
            if(cx < mapW && cy < mapH){
                selectAX = cx;
                selectAY = cy;
                selectBX = cx;
                selectBY = cy;
                selectCX = cx;
                selectCY = cy;
            }
            dragging = true;
            return;
        }
    }
    for(UIElement* el : uiElements){
        if(x >= el->x && y >= el->y && x <= el->x+el->w && y <= el->y+el->h){
            el->onClick(button, x, y);
        }
    }
}

void mouseButtonMove(int x, int y){
    mouseX = x;
    mouseY = y;
    hoverX = -1;
    hoverY = -1;
    
    if(x >= editorX && y >= editorY){
        int cx = (x-editorX+editorScrollX)/fontW;
        int cy = (y-editorY+editorScrollY)/fontH;
        if(cx < mapW && cy < mapH){
            if(dragging){
                selectBX = cx;
                selectBY = cy;
            }
            hoverX = cx;
            hoverY = cy;
        }
    }
}

void mouseButtonUp(int button, int x, int y){
    if(button == SDL_BUTTON_LEFT){
        dragging = false;
    }
}

void clampSelects(){
    
    
    if(selectAX < 0){
        selectAX = mapW-1;
    }
    if(selectAX > mapW-1){
        selectAX = 0;
    }
    if(selectAY < 0){
        selectAY = mapH-1;
    }
    if(selectAY > mapH-1){
        selectAY = 0;
    }
    
    if(selectBX < 0){
        selectBX = mapW-1;
    }
    if(selectBX > mapW-1){
        selectBX = 0;
    }
    if(selectBY < 0){
        selectBY = mapH-1;
    }
    if(selectBY > mapH-1){
        selectBY = 0;
    }
    
    if(selectCX < 0){
        selectCX = mapW-1;
    }
    if(selectCX > mapW-1){
        selectCX = 0;
    }
    if(selectCY < 0){
        selectCY = mapH-1;
    }
    if(selectCY > mapH-1){
        selectCY = 0;
    }
}

void keyTyped(char c){
    bool dif = false;
    if(selectCX >= 0 && selectCY >= 0){
        if(c >= ' ' && c <= '~'){
            dif = displayBuffer[activeLayer][selectCX][selectCY] != c;
            displayBuffer[activeLayer][selectCX][selectCY] = c;
            selectCX++;
            if(selectCX >= mapW || (selectAX != selectBX && selectCX > selectBX)){
                selectCY++;
                if(selectCY >= mapH){
                    selectCY = mapH-1;
                    selectCX--;
                }else{
                    selectCX = selectAX;
                }
            }
            clampSelects();
        }else if(c == '\n'){
            selectBX = max(selectAX, selectCX-1);
            selectCX = selectAX;
            selectCY++;
            if(selectCY > mapH){
                selectCY = selectAY;
            }
            clampSelects();
        }else if(c == '\b'){
            if(selectCY >= selectAY && selectCX > selectAX){
                selectCX--;
            }else if(selectCX <= selectAX){
                selectCX = selectBX;
                selectCY--;
            }else{
                selectCX--;
            }
            clampSelects();
            dif = displayBuffer[activeLayer][selectCX][selectCY] != ' ';
            displayBuffer[activeLayer][selectCX][selectCY] = ' ';
        }
    }
    if(dif){
        saveUndoPoint();
    }
}

void moveSelect(int dx, int dy){
    if(selectBX == selectCX && selectBY == selectCY){
        if(selectAX == selectBX && selectAY == selectBY){
            selectAX += dx;
            selectAY += dy;
        }
        selectBX += dx;
        selectBY += dy;
    }
    selectCX += dx;
    selectCY += dy;
    
    clampSelects();
}

void scroll(int sx, int sy){
    editorScrollX += sx;
    editorScrollY -= sy;
}

void copySelect(){
    int minX = min(selectAX, selectBX);
    int minY = min(selectAY, selectBY);
    int maxX = max(selectAX, selectBX);
    int maxY = max(selectAY, selectBY);
    
    clipboard.clear();
    for(int x = minX; x <= maxX; x++){
        clipboard.push_back(vector<char>());
        for(int y = minY; y <= maxY; y++){
            clipboard[x-minX].push_back(displayBuffer[activeLayer][x][y]);
        }
    }
    
}

void deleteSelect(){
    int minX = min(selectAX, selectBX);
    int minY = min(selectAY, selectBY);
    int maxX = max(selectAX, selectBX);
    int maxY = max(selectAY, selectBY);
    
    for(int x = minX; x <= maxX; x++){
        for(int y = minY; y <= maxY; y++){
            displayBuffer[activeLayer][x][y] = ' ';
        }
    }
    saveUndoPoint();
}

void pasteSelectIgnore(char ignoreChar){
    int w = min(selectAX+(int)clipboard.size(), mapW);
    for(int x = selectAX; x < w; x++){
        int h = min(selectAY+(int)clipboard[x-selectAX].size(), mapH);
        for(int y = selectAY; y < h; y++){
            if(clipboard[x-selectAX][y-selectAY] != ignoreChar){
                displayBuffer[activeLayer][x][y] = clipboard[x-selectAX][y-selectAY];
            }
        }
    }
    saveUndoPoint();
}

void pasteSelect(){
    int w = min(selectAX+(int)clipboard.size(), mapW);
    for(int x = selectAX; x < w; x++){
        int h = min(selectAY+(int)clipboard[x-selectAX].size(), mapH);
        for(int y = selectAY; y < h; y++){
            displayBuffer[activeLayer][x][y] = clipboard[x-selectAX][y-selectAY];
        }
    }
    saveUndoPoint();
}

void promptIfNeverSavedAndSaveFile(){
    if(filename.size() == 0){
        promptAndSaveFile();
    }else{
        saveFile();
    }
}

void promptAndSaveFile(){
    string filename = "";
    printf("\n- Save As -\n");
    printf("Enter filename: ");
    while(filename.size() == 0){
        cin >> filename;
    }
    if(filename.find(".") == string::npos){
        filename += ".amap";
    }
    
    saveFile();
}

void saveFile(){
    SDL_SetWindowTitle(mainWindow, ("AsciiMap - "+filename).c_str());
    printf("- Saving %s -\n", filename.c_str());
    FILE* f = fopen(filename.c_str(), "wb");
    const char* amap = "amap";
    fwrite(amap, 4, 1, f);
    fwrite(&mapW, 4, 1, f);
    fwrite(&mapH, 4, 1, f);
    fwrite(&activeLayer, 4, 1, f);
    fwrite(&activeColorLayer, 4, 1, f);
    unsigned char layers = (unsigned char)displayBuffer.size();
    fwrite(&(layers), 1, 1, f);
    for(int i=0;i<visibleLayers.size();i++){
        char v = visibleLayers[i]?'V':'H';
        fwrite(&v, 1, 1, f);
    }
    fwrite(&metaDataSize, 4, 1, f);
    if(metaDataSize > 0){
        fwrite(metaData, metaDataSize, 1, f);
    }
    for(int i = 0; i < displayBuffer.size(); i++){
        for(int y = 0; y < displayBuffer[i][0].size(); y++){
            for(int x = 0; x < displayBuffer[i].size(); x++){
                fwrite(&displayBuffer[i][x][y], 1, 1, f);
            }
        }
    }
    fclose(f);
    printf("- Done -\n");
}

void promptAndLoadFile(){
    filename = "";
    printf("\n- Open File -\n");
    printf("Enter filename: ");
    while(filename.size() == 0){
        cin >> filename;
    }
    if(filename.find(".") == string::npos){
        filename += ".amap";
    }
    SDL_SetWindowTitle(mainWindow, ("AsciiMap - "+filename).c_str());
    
    loadFile();
}

void loadFile(){
    clearUndo();
    printf("\n- Loading %s -\n", filename.c_str());
    FILE* f = fopen(filename.c_str(), "rb");
    char* amap = (char*)malloc(4);
    fread(amap, 4, 1, f);
    if(amap[0] != 'a' || amap[1] != 'm' || amap[2] != 'a' || amap[3] != 'p'){
        printf("\n- Failed to load %s -\n", filename.c_str());
        return;
    }
    fread(&mapW, 4, 1, f);
    fread(&mapH, 4, 1, f);
    fread(&activeLayer, 4, 1, f);
    fread(&activeColorLayer, 4, 1, f);
    displayBuffer.clear();
    visibleLayers.clear();
    unsigned char layers;
    fread(&(layers), 1, 1, f);
    for(int i=0;i<layers;i++){
        makeNewLayer();
    }
    for(int i=0;i<layers;i++){
        char v;
        fread(&v, 1, 1, f);
        visibleLayers[i] = (v == 'V');
    }
    fread(&metaDataSize, 4, 1, f);
    if(metaDataSize > 0){
        metaData = (unsigned char*)malloc(metaDataSize);
        fread(metaData, metaDataSize, 1, f);
    }
    for(int i = 0; i < displayBuffer.size(); i++){
        for(int y = 0; y < displayBuffer[i][0].size(); y++){
            for(int x = 0; x < displayBuffer[i].size(); x++){
                fread(&displayBuffer[i][x][y], 1, 1, f);
            }
        }
    }
    fclose(f);
    updateLayers();
    printf("- Done -\n");
}





