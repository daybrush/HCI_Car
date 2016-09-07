#include "ofMain.h"

class Car {
public:
    void setup();
    void init(int angle);
    void setAngle(int angle);
    void drawPosition();
    void add();
    
    int speed;
    int initAngle;
    int prevAngle;
    int angle;
    vector<ofPoint> position;
    bool is_init;
    bool is_stop = true;
};
