#pragma once

#include "ofMain.h"
#include "Car.hpp"
class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void send(string text);
		ofTrueTypeFont		font; // for displaying messages on window
        Car myCar;
		string recvData;
		float  rotationValue;
        bool is_start = false;
		ofSerial serial; //serial communication
    ofImage image;
};
