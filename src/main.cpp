#include "program.hpp"

using namespace std;

const int colors[] = {
    0x000000,//black
    0x800000,//dark red
    0x008000,//dark green
    0x808000,//dark yellow
    0x0055C1,//dark blue
    0x800080,//dark magenta
    0x008080,//dark cyan
    0xAAAAAA,//light grey
    0x555555,//dark grey
    0xFF0000,//light red
    0x00FF00,//light green
    0xFFFF00,//light yellow
    0x00A9FF,//light blue
    0xFF00FF,//light magenta
    0x00FFFF,//light cyan
    0xFFFFFF,//white
};

int colorCodeFromChar(char c){
    switch (c) {
        case 'K':
            return 0;
        case 'R':
            return 1;
        case 'G':
            return 2;
        case 'Y':
            return 3;
        case 'B':
            return 4;
        case 'M':
            return 5;
        case 'C':
            return 6;
        case 'w':
            return 7;
        case 'k':
            return 8;
        case 'r':
            return 9;
        case 'g':
            return 0xA;
        case 'y':
            return 0xB;
        case 'b':
            return 0xC;
        case 'm':
            return 0xD;
        case 'c':
            return 0xE;
        case 'W':
        default:
            return 0xF;
            return 0xF;
    }
}

string initalOpenPath;

SDL_Window* mainWindow;
SDL_Renderer* mainRenderer;

SDL_Texture* fontTexture;
SDL_Texture* uiTexture;

bool running = false;

const Uint8 *keyboardState;


//unsigned int beginTick = SDL_GetTicks();
//unsigned int endTick;

//double delta = 0;

string format(const char* fmt, ...) {
    string s = "";
    
    char buff[256];
    va_list args;
    va_start(args, fmt);
    vsprintf(buff, fmt, args);
    s += buff;
    va_end(args);
    
    return s;
}

void debug(const char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

bool initSDL(){
    
    editorW = (mapW * fontW) + 8;
    editorH = (mapH * fontH) + 8;
    
    debug("Init SDL\n");
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        debug("Failed to initalize SDL. Error: %s\n", SDL_GetError());
        return 1;
    }
    mainWindow = SDL_CreateWindow("ASCII Map", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, editorX+editorW, editorY+editorH, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if(mainWindow == NULL){
        debug("Failed to create window. Error: %s\n", SDL_GetError());
        return 1;
    }
    int imgFlags = IMG_INIT_PNG;
    if(!(IMG_Init(imgFlags) & imgFlags) ){
        debug("Failed to initalize SDL Image. Error: %s\n", IMG_GetError());
        return 1;
    }
    
    mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);
    if(!mainRenderer){
        debug("SDL_CreateRenderer: %s\n", SDL_GetError());
        return 1;
    }
    
    keyboardState = SDL_GetKeyboardState(NULL);
    
    fontTexture = IMG_LoadTexture(mainRenderer, "font.png");
    if(!fontTexture){
        debug("IMG_LoadTexture: %s\n", SDL_GetError());
        return 1;
    }
    
    uiTexture = IMG_LoadTexture(mainRenderer, "ui.png");
    if(!uiTexture){
        debug("IMG_LoadTexture: %s\n", SDL_GetError());
        return 1;
    }
    
    return 0;
}

void wasResized(int w, int h){
    editorW = w - editorX;
    editorH = h - editorY;
}

void refresh(){
    SDL_RenderClear(mainRenderer);
    
    SDL_SetRenderDrawColor(mainRenderer, 192, 192, 192, 255); 
    SDL_RenderFillRect(mainRenderer, NULL); 
    
    SDL_Rect selectSrc;
    selectSrc.x = 0;
    selectSrc.y = 0;
    selectSrc.w = fontW;
    selectSrc.h = fontH;
    
    SDL_Rect selectCSrc;
    selectCSrc.x = 0;
    selectCSrc.y = fontH*1;
    selectCSrc.w = fontW;
    selectCSrc.h = fontH;
    
    SDL_Rect selectHSrc;
    selectHSrc.x = 0;
    selectHSrc.y = fontH*2;
    selectHSrc.w = fontW;
    selectHSrc.h = fontH;
    
    SDL_Rect src;
    SDL_Rect dst;
    
    src.w = fontW;
    src.h = fontH;
    
    dst.w = fontW;
    dst.h = fontH;
    
    int w = max(0, min((editorX+editorScrollX+editorW)/fontW, mapW));
    int h = max(0, min((editorY+editorScrollY+editorH)/fontH, mapH));
    int sx = max(editorScrollX/fontW, 0);
    int sy = max(editorScrollY/fontH, 0);
    
    setUiColor({0, 0, 0});
    drawRect(sx*fontW + editorX - editorScrollX, sy*fontH + editorY - editorScrollY, w*fontW, h*fontH);
    setUiColor({255, 255, 255});
    
    for(int layer = (int)visibleLayers.size()-1; layer >= 0; layer--){
        if((layer == activeLayer || visibleLayers[layer]) && displayBuffer.size() > layer && (activeColorLayer < 0 || displayBuffer.size() > activeColorLayer)){
            
            for(int j=sy;j<h;j++){
                for(int i=sx;i<w;i++){
                    
                    char c = displayBuffer[layer][i][j];
                    if(c < ' '){
                        c = ' ';
                    }
                    unsigned char color = (activeColorLayer >= 0 && activeColorLayer != layer) ? colorCodeFromChar(displayBuffer[activeColorLayer][i][j]) : 0xF;
                    int ci = c-32;
                    
                    src.x = (ci%16)*fontW;
                    src.y = (ci/16)*fontH;
                    
                    dst.x = i*fontW + editorX - editorScrollX;
                    dst.y = j*fontH + editorY - editorScrollY;
                    
                    if(color > 0){
                        SDL_SetTextureColorMod(fontTexture, (colors[color] >> 16) & 0xFF, (colors[color] >> 8) & 0xFF, (colors[color] >> 0) & 0xFF);
                        Uint8 alpha;
                        if(visibleLayers[layer]){
                            if(layer == activeLayer || activeLayer == -1){
                                alpha = 0xFF;
                            }else{
                                alpha = 0x80;
                            }
                        }else{
                            alpha = 0x40;
                        }
                        SDL_SetTextureAlphaMod(fontTexture, alpha);
                        SDL_RenderCopy(mainRenderer, fontTexture, &src, &dst);
                    }
                    if(layer == activeLayer){
                        if((i == selectAX && j == selectAY) || (i == selectBX && j == selectBY)){
                            SDL_RenderCopy(mainRenderer, uiTexture, &selectSrc, &dst);
                        }else if(i == selectCX && j == selectCY){
                            SDL_RenderCopy(mainRenderer, uiTexture, &selectCSrc, &dst);
                        }else if(i == hoverX && j == hoverY){
                            SDL_RenderCopy(mainRenderer, uiTexture, &selectHSrc, &dst);
                        }
                    }
                }
            }
        }
    }
    SDL_SetTextureAlphaMod(fontTexture, 0xFF);
    
    
    draw();
    
    SDL_RenderPresent(mainRenderer);
    
    
    
    SDL_Event e;
    SDL_WaitEvent(&e);
    do{
        switch(e.type){
            case SDL_QUIT:{
                running = false;
                return;
            }
            case SDL_WINDOWEVENT:{
                switch (e.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:{
                        wasResized(e.window.data1, e.window.data2);
                        break;
                    }
                }
                break;
            }
            case SDL_MOUSEMOTION:{
                mouseButtonMove(e.motion.x, e.motion.y);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:{
                mouseButtonDown(e.button.button, e.button.x, e.button.y);
                break;
            }
            case SDL_MOUSEBUTTONUP:{
                mouseButtonUp(e.button.button, e.button.x, e.button.y);
                break;
            }
            case SDL_TEXTINPUT:{
                keyTyped(e.text.text[0]);
                break;
            }
            case SDL_KEYDOWN:{
                if(e.key.keysym.mod & KMOD_CTRL){
                    switch (e.key.keysym.scancode) {
                        case SDL_SCANCODE_C:
                            copySelect();
                            break;
                        case SDL_SCANCODE_X:
                            copySelect();
                            deleteSelect();
                            break;
                        case SDL_SCANCODE_V:
                            if(e.key.keysym.mod & KMOD_SHIFT){
                                pasteSelectIgnore(' ');
                            }else{
                                pasteSelect();
                            }
                            break;
                        case SDL_SCANCODE_S:
                            if(e.key.keysym.mod & KMOD_SHIFT){
                                promptAndSaveFile();
                            }else{
                                promptIfNeverSavedAndSaveFile();
                            }
                            break;
                        case SDL_SCANCODE_L:
                            promptAndLoadFile();
                            break;
                        case SDL_SCANCODE_A:
                            selectAX = 0;
                            selectAY = 0;
                            selectBX = mapW-1;
                            selectBY = mapH-1;
                            selectCX = 0;
                            selectCY = 0;
                            break;
                        case SDL_SCANCODE_Z:
                            if(e.key.keysym.mod & KMOD_SHIFT){
                                redo(); 
                            }else{
                                undo();
                            }
                            break;
                        default:
                            break;
                    }
                }else{
                    switch(e.key.keysym.scancode){
                        case SDL_SCANCODE_RETURN:{
                            keyTyped('\n');
                            break;
                        }
                        case SDL_SCANCODE_BACKSPACE:{
                            keyTyped('\b');
                            break;
                        }
                        case SDL_SCANCODE_LEFT:{
                            moveSelect(-1, 0);
                            break;
                        }
                        case SDL_SCANCODE_RIGHT:{
                            moveSelect(1, 0);
                            break;
                        }
                        case SDL_SCANCODE_UP:{
                            moveSelect(0, -1);
                            break;
                        }
                        case SDL_SCANCODE_DOWN:{
                            moveSelect(0, 1);
                            break;
                        }
                        case SDL_SCANCODE_DELETE:{
                            deleteSelect();
                            break;
                        }
                        default:
                            break;
                    }
                }
                break;
            }
            case SDL_MOUSEWHEEL:{
                scroll(e.wheel.x, e.wheel.y);
                break;
            }
                
                        
        }
    }while(SDL_PollEvent(&e));
    
    //endTick = SDL_GetTicks();
    //delta = double(endTick - beginTick) / 1000.0;
    //beginTick = SDL_GetTicks();
    
}


void setUiColor(Color c){
    SDL_SetTextureColorMod(uiTexture, c.r, c.g, c.b);
}

void setTextColor(Color c){
    SDL_SetTextureColorMod(fontTexture, c.r, c.g, c.b);
}

void drawChar(char c, int x, int y){
    SDL_Rect src;
    int ci = c - 32;
    src.x = (ci%16)*fontW;
    src.y = (ci/16)*fontH;
    src.w = fontW;
    src.h = fontH;
    
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = fontW;
    dst.h = fontH;
    
    SDL_SetTextureColorMod(fontTexture, 0xFF, 0xFF, 0xFF);
    SDL_RenderCopy(mainRenderer, fontTexture, &src, &dst);
}

void drawText(string s, int x, int y){
    for(int i = 0; i < s.length(); i++){
        drawChar(s[i], x, y);
        x += fontW;
    }
}

void drawRect(int x, int y, int w, int h){
    SDL_Rect src;
    src.x = 7;
    src.y = 0;
    src.w = 1;
    src.h = 1;
    
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = w;
    dst.h = h;
    
    SDL_RenderCopy(mainRenderer, uiTexture, &src, &dst);
}

void drawUi(int sx, int sy, int dx, int dy, int w, int h){
    SDL_Rect src;
    src.x = sx;
    src.y = sy;
    src.w = w;
    src.h = h;
    
    SDL_Rect dst;
    dst.x = dx;
    dst.y = dy;
    dst.w = w;
    dst.h = h;
    
    SDL_RenderCopy(mainRenderer, uiTexture, &src, &dst);
}

int main(int argc, char *argv[]){
    
    if(argc == 2){
        initalOpenPath = string(argv[1]);
    }
    
    if(initSDL()){
        return 1;
    }
    
    running = init();
    
    while(running){
        if(!update()){
            running = false;
        }
    }
    
    cleanup();
    
    SDL_DestroyWindow(mainWindow);
    SDL_Quit();
    
    return 0;
}
