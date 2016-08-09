#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofSetVerticalSync(true);
	
	ofBackground(255);	
	font.loadFont("DIN.otf", 14);
	
	serial.listDevices();
	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
	
	// this should be set to whatever com port your serial device is connected to.
	// (ie, COM4 on a pc, /dev/tty.... on linux, /dev/tty... on a mac)
	// arduino users check in arduino app....
	int baud = 9600;
//	serial.setup("/dev/cu.HC-06-DevB", baud); //open the first device
    serial.setup("/dev/cu.usbmodem1411", baud);
    
	//serial.setup("COM4", baud); // windows example
	//serial.setup("/dev/tty.usbserial-A4001JEC", baud); // mac osx example
	//serial.setup("/dev/ttyUSB0", baud); //linux example

    recvData.clear();
    myCar.setup();
    
    
    image.loadImage("location.png");
    image.resize(50, 50);
}

//--------------------------------------------------------------

int sendSpeed = 0;
bool is_str_start = 0;
void testApp::update(){
	char ch;
    

    recvData.clear();
    is_str_start = false;
	if (serial.available()) {
		while((ch = serial.readByte())>0) {
            // angle #
            //   speed @ angle/ = 255/100/
            //init
            if(ch == '$') {
                is_str_start = true;
                recvData.clear();
                continue;
            } else if(is_str_start) {
                if (ch == '#') {
                    cout << "initAngle";
                    myCar.init(ofToFloat(recvData));
                    
                    is_start = true;
                }
                else if(ch == '/') {
                    cout << "setAngle";
                    if(myCar.is_init) {
                        myCar.setAngle(ofToFloat(recvData));
                    }
                }
                else if (ch == '@') {
                    cout << "setSpeed";
                    //myCar.speed = ofToFloat(recvData);
                } else if(ch == '*') {
                    cout << "print";
                } 
                else {
                    recvData += ch;
                    continue;
                }
                cout << recvData << endl;
                
                recvData.clear();
            }
           
		}
	}
    
    myCar.add();
}

//--------------------------------------------------------------
void testApp::draw(){
    if(!serial.isInitialized())
        return;
    
	ofBackground(20,20,20);
	ofSetColor(220);
    int radius = 120;
    int margin = 20;
    int border = 10;
    int angle = 0;
    
    
    ofPushView();
        ofTranslate(ofGetWindowWidth() / 2, ofGetWindowHeight() / 2);
        myCar.drawPosition();
        ofPushView();
            ofRotate(myCar.angle - myCar.initAngle);
            image.draw(-25, -25);
        ofPopView();
    ofPopView();
    
    
    
    
    ofPushView();
        ofTranslate(radius + margin, ofGetWindowHeight() -(radius + margin) );
        ofPushStyle();
            ofSetColor(255, 255, 255);
            ofDrawCircle(0, 0, radius);
        ofPopStyle();
        ofPushStyle();
            ofSetColor(0, 0, 0);
            ofDrawCircle(0, 0, radius - border);
        ofPopStyle();
    
    
    
        ofPushStyle();
            ofSetColor(255, 255, 255);
            ofDrawCircle(0,0, 7);
        ofPopStyle();
        for(int i = 0; i < 30; ++i) {
            ofPushView();
                ofRotate(-90 + 60 + i * 240 / 30);
                ofTranslate(-radius +border, 0);
            ofDrawRectangle(0, 0, i%5==0? 15 : 10, i%5==0? 4 : 3);
            ofPopView();
        }
        ofPushView();
            ofSetColor(255, 255, 255);
            ofTranslate(-6, 5);
            int tr =(radius - margin - 20);
            for(int i = 0; i <= 6; ++i) {
                double ang = (150 + i * (240 / 6)) * PI / 180;
                
                font.drawString(ofToString(i), cos(ang) * tr, sin(ang) * tr);
            }
        ofPopView();
        ofPushView();
        ofPushStyle();
            ofSetColor(255, 255, 255);
            ofTranslate(0 , 0);
            ofRotate(60 + myCar.speed / 10);
            ofDrawTriangle(-4, 0, 0, radius * 0.7, 4, 0);
        ofPopStyle();
        ofPopView();
    
    ofPopView();


}

//--------------------------------------------------------------
void testApp::send(string text) {
    unsigned char cstr[10];
    //strncpy((char *)cstr,text.c_str(), text.size());
    serial.writeBytes((unsigned char *)text.c_str(), text.size());
}
void testApp::keyPressed(int key){
    if(!is_start) {
        if(key == 32) {
            cout << "press init"<<endl;
            
            send("init#");
        }
        return;
    }
    if(key == 357) {
        //forward
        if(myCar.direction != 0 || myCar.is_reverse || myCar.is_stop) {
            myCar.is_reverse = false;
            myCar.direction = 0;
            myCar.is_stop = false;
            send("2/");
        }
        
    } else if(key == 356) {
        if(myCar.direction != -1 || myCar.is_stop) {
            send("4/");
            myCar.direction = -1;
            myCar.is_stop = false;
        }
    } else if(key == 358) {
        if(myCar.direction != 1 || myCar.is_stop) {
            //right
            send("6/");
            myCar.direction = 1;
            myCar.is_stop =false;
        }
    } else if(key == 359) {
        if(myCar.direction != 0 || !myCar.is_reverse || myCar.is_stop) {
        //back
            send("8/");
            myCar.direction = 0;
            myCar.is_stop =false;
            myCar.is_reverse = true;
        }
    } else if(key == 's' || key == 'S') {
        if(!myCar.is_stop) {
          //stop
            send("5/");
            myCar.direction = 0;
            myCar.speed = 0;
            myCar.is_stop = true;
        }
    } else if(key == 32) {
        myCar.speed += 100;
        if(myCar.speed >= 2400)
            myCar.speed = 2400;
        
        
        send(ofToString(myCar.speed) + "@");
        
    } else if(key == 'b' || key == 'B') {
        myCar.speed -= 100;
        if(myCar.speed <= 0)
            myCar.speed = 00;
        
        
        send(ofToString(myCar.speed) + "@");
    } else if (key =='i' || key == 'I') {
        send("init#");
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
