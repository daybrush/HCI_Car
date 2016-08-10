//
//  Car.cpp
//  myCar
//
//  Created by younkue's air on 2016. 8. 7..
//
//

#include "Car.hpp"


int cycle = 0;
int avgerr = 10;
void Car::setup() {
    position.clear();
}
void Car::init(int angle) {
    this->angle = this->prevAngle = this->initAngle = angle;
    this->is_init = true;
}

void Car::setAngle(int angle) {
    if(angle - this->angle <= avgerr || ABS(angle) >= 170 && ABS(this->angle) >= 170) {
        this->prevAngle = this->angle;
        this->angle = angle;
    }
}

void Car::add() {
    if((++cycle )% 20 != 0)
        return;
    
    if(this->speed == 0 || this->is_stop)
        return;
    
    int ang = this->angle - this->initAngle;
    
    double c = cos(-angle * PI / 180);
    double  s = sin(-angle * PI / 180);
    
    double x = -s * this->speed / 200;
    double y = c * this->speed / 200;
    if(this->is_reverse)
        y *= -1;
    position.push_back(ofPoint(x, y, 0));
}

void Car::drawPosition() {
    int x = 0, y  = 0;
    ofPushView();
    ofPushStyle();
    ofSetColor(200, 60, 60);
    for(int i = position.size() -1; (i >= (int)position.size() - 60) && i >= 0 ; --i) {
        ofDrawCircle(x, y, 4);
        ofDrawLine(x, y, x - position[i].x, y + position[i].y);
        x -= position[i].x;
        y += position[i].y;
    }
    ofPopStyle();
    ofPopView();
}