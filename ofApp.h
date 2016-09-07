#pragma once

#include "ofxOpenNI.h"
#include "ofMain.h"
#include "Car.hpp"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

        void userEvent(ofxOpenNIUserEvent & event);
    
    void send(string text);
    void updateMessage();
    void drawCar();
    void drawConnectMessage();
    void updateRotate();
    int getAngle(ofPoint l, ofPoint r);
        ofxOpenNI openNIDevice;
    
        ofTrueTypeFont verdana;
    ofPoint selectedPoint;
    
    ofTrueTypeFont		font; // for displaying messages on window
    Car myCar;
    string recvData;
    float  rotationValue = 1;
    float speed_level;
    bool is_start = false;
    ofSerial serial; //serial communication
    ofImage location, wheel;
    
    
    
};
