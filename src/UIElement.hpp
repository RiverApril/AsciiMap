//
//  UIElement.hpp
//  AsciiMap
//
//  Created by Braeden Atlee on 7/27/17.
//  Copyright © 2017 Braeden Atlee. All rights reserved.
//

#ifndef UIElement_hpp
#define UIElement_hpp

#include <string>

#include "program.hpp"

using namespace std;

class UIElement{
public:
    int x, y, w, h;
    
    UIElement(int x, int y, int w, int h) : x(x), y(y), w(w), h(h){
        
    }
    virtual ~UIElement(){}
    
    virtual void draw(){}
    
    virtual void onClick(int button, int x, int y){}
    
};

class UIButton : public UIElement{
public:
    string s;
    Color c = {0x80, 0x80, 0x80};
    Color cHover = {0xA0, 0xA0, 0xA0};
    void (*action)(UIButton*, int);
    
    UIButton(string s, int x, int y, void (*action)(UIButton*, int)) : s(s), action(action), UIElement(x, y, (int)s.length()*fontW+4, fontH+4){
        
    }
    virtual ~UIButton(){}
    
    void draw(){
        if(mouseX >= x && mouseY >= y && mouseX <= x+w && mouseY <= y+h){
            setUiColor(cHover);
        }else{
            setUiColor(c);
        }
        setTextColor({255, 255, 255});
        drawRect(x, y, w, h);
        drawText(s, x+2, y+2);
    }
    
    void onClick(int button, int mx, int my){
        action(this, button);
    }
};

class UIList : public UIElement{
public:
    vector<string> list;
    Color c = {0x80, 0x80, 0x80};
    Color cSel = {0x40, 0x40, 0x40};
    int selected = -1;
    bool mustSelect = true;
    void (*action)(UIList*, int);
    
    UIList(vector<string> list, int x, int y, void (*action)(UIList*, int)) : list(list), action(action), UIElement(x, y, 0, 0){
        updateSize();
    }
    virtual ~UIList(){}
    
    void updateSize(){
        h = (int)list.size()*(fontH+4);
        w = 0;
        for(int i=0;i<list.size();i++){
            if(list[i].length()*fontW+4 > w){
                w = (int)list[i].length()*fontW+4;
            }
        }
    }
    
    void draw(){
        setTextColor({0xFF, 0xFF, 0xFF});
        for(int i=0;i<list.size();i++){
            if(i == selected){
                setUiColor(cSel);
                drawRect(x+4, y+(i*(fontH+4)), w, (fontH+4));
                drawText(list[i], x+2+4, y+(i*(fontH+4))+2);
            }else{
                setUiColor(c);
                drawRect(x, y+(i*(fontH+4)), w, (fontH+4));
                drawText(list[i], x+2, y+(i*(fontH+4))+2);
            }
        }
    }
    
    void onClick(int button, int x, int y){
        y -= this->y;
        y /= fontH+4;
        if(selected == y && !mustSelect){
            selected = -1;
        }else{
            selected = y;
        }
        action(this, button);
    }
};

class UILayers : public UIElement{
public:
    vector<bool>& layerVisible;
    int& editingLayer;
    int& colorLayer;
    Color inactiveC = {0x80, 0x80, 0x80};
    Color activeC = {0xFF, 0xFF, 0xFF};
    
    UILayers(vector<bool>& layerVisible, int& editingLayer, int& colorLayer, int x, int y) : layerVisible(layerVisible), editingLayer(editingLayer), colorLayer(colorLayer), UIElement(x, y, 0, 0){
        updateSize();
    }
    virtual ~UILayers(){}
    
    void updateSize(){
        h = (int)(layerVisible.size()+1)*(fontH+4);
        w = 4+fontW*6;
    }
    
    void draw(){
        for(int i=0;i<layerVisible.size();i++){
            setUiColor(layerVisible[i]?activeC:inactiveC);
            drawUi(14, 0, x+2, y+2+i*(fontH+4), fontW*2, fontH);
            setUiColor(i==editingLayer?activeC:inactiveC);
            drawUi(28, 0, x+2+(fontW*2), y+2+i*(fontH+4), fontW*2, fontH);
            setUiColor(i==colorLayer?activeC:inactiveC);
            drawUi(42, 0, x+2+(fontW*4), y+2+i*(fontH+4), fontW*2, fontH);
        }
    }
    
    void onClick(int button, int x, int y){
        y -= this->y;
        y /= fontH+4;
        x -= this->x;
        x -= 2;
        x /= fontW*2;
        if(x == 0){
            layerVisible[y] = !layerVisible[y];
        }else if(x == 1){
            if(editingLayer == y){
                editingLayer = -1;
            }else{
                editingLayer = y;
            }
        }else if(x == 2){
            if(colorLayer == y){
                colorLayer = -1;
            }else{
                colorLayer = y;
            }
        }
    }

};

#endif /* UIElement_hpp */


