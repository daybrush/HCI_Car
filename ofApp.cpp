#include "ofApp.h"

//--------------------------------------------------------------
int numUsers;
bool is_ready;
void ofApp::setup(){
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    openNIDevice.setup();
    openNIDevice.addImageGenerator();
    openNIDevice.addDepthGenerator();
    openNIDevice.setRegister(true);
    openNIDevice.setMirror(true);
    
    openNIDevice.addUserGenerator();
    openNIDevice.setMaxNumUsers(2);
    openNIDevice.start();
    
    // set properties for all user masks and point clouds
    //openNIDevice.setUseMaskPixelsAllUsers(true); // if you just want pixels, use this set to true
    openNIDevice.setUseMaskTextureAllUsers(true); // this turns on mask pixels internally AND creates mask textures efficiently
    openNIDevice.setUsePointCloudsAllUsers(true);
    openNIDevice.setPointCloudDrawSizeAllUsers(2); // size of each 'point' in the point cloud
    openNIDevice.setPointCloudResolutionAllUsers(2); // resolution of the mesh created for the point cloud eg., this will use every second depth pixel
    openNIDevice.setMirror(true);
    verdana.loadFont(ofToDataPath("verdana.ttf"), 24);
    
    selectedPoint = ofPoint(200,200);
    
    
    font.loadFont("DIN.otf", 14);
    serial.listDevices();
    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    int baud = 9600;
    serial.setup("/dev/cu.HC-06-DevB", baud);
//    serial.setup("/dev/cu.usbmodem1421", baud);

    recvData.clear();
    myCar.setup();
    
    
    location.loadImage("location.png");
    location.resize(50, 50);
    
    wheel.loadImage("wheel.png");
    wheel.resize(150, 150);
}

//--------------------------------------------------------------

int sendSpeed = 0;
bool is_str_start = 0;

void ofApp::updateMessage() {
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
void ofApp::updateRotate() {
    int depth_min, depth_max;
    numUsers = openNIDevice.getNumTrackedUsers();
    
    
    ofPoint leftHandPos, rightHandPos;
    for (int i = 0; i < numUsers; i++){
        
        
        ofxOpenNIUser & user = openNIDevice.getTrackedUser(i);
        
        // and point clouds:
        
        //        ofPushMatrix();
        //        ofTranslate(320, 240, 10);
        //        user.drawPointCloud();
        //        ofPopMatrix();
        
        ofxOpenNIJoint leftHand = user.getJoint(JOINT_LEFT_HAND);
        ofxOpenNIJoint rightHand = user.getJoint(JOINT_RIGHT_HAND);
        ofxOpenNIJoint head = user.getJoint(JOINT_HEAD);
        
        leftHandPos = leftHand.getProjectivePosition();
        rightHandPos = rightHand.getProjectivePosition();
        ofPoint headPos = head.getProjectivePosition();
        int depth = rightHandPos.z - leftHandPos.z;
        depth = depth / 10;
        
        int speed = myCar.speed / 100;
        if(headPos.z - rightHandPos.z <  0) {
            myCar.speed = 0;
        }
        if(abs(depth)> 9) {
            
            myCar.speed += depth > 0 ? -10  : 10;
            
            if(myCar.speed >= 2400)
                myCar.speed = 2400;
            
            if(myCar.speed <= -2400)
                myCar.speed = -2400;
        }
        
        int angle = getAngle(leftHandPos, rightHandPos);
        angle = angle / 10;
        
        int angle_value = (angle < 0 ? -1 : 1) * (ABS(angle) + 1);
        
        
        if(leftHandPos.x == 0)
            continue;
        
        if(speed != int(myCar.speed / 100)) {
            send(ofToString(myCar.speed) + "@");
            myCar.is_stop = false;
        }
        if(angle_value != rotationValue) {
            send(ofToString(angle_value) + "%");
            rotationValue = angle_value;
            myCar.is_stop = false;
        }
        
    }
    
    

}
void ofApp::update(){
    openNIDevice.update();
    numUsers = openNIDevice.getNumTrackedUsers();
    is_ready = serial.isInitialized() && numUsers>0;
    //is_ready = serial.isInitialized();
    
//    is_ready = true;
    updateMessage();
    updateRotate();
    
    int angle_value = (rotationValue < 0 ? -1 : 1) * (ABS(rotationValue) - 1);
    //myCar.setAngle(myCar.angle + angle_value * 4);
}

//--------------------------------------------------------------

void ofApp::drawConnectMessage() {
    ofSetColor(255, 255, 255);
    ofPushMatrix();
    openNIDevice.drawDebug();
    ofPopMatrix();
    
    for (int i = 0; i < numUsers; i++){
        
        
        ofxOpenNIUser & user = openNIDevice.getTrackedUser(i);
        
        // and point clouds:ㄱ
        
        ofPushMatrix();
        ofTranslate(320, 240, 10);
        user.drawPointCloud();
        ofPopMatrix();
    }
    
    
    
    if(!serial.isInitialized()) {
        ofPushMatrix();
        ofPushView();
         ofTranslate(ofGetWindowWidth() / 2 - 100, ofGetWindowHeight() / 2);
        verdana.drawString("Connect Bluetooth Car", 0, 0);
        ofPopView();
        ofPopMatrix();
    }
    if(numUsers == 0) {
        ofPushMatrix();
        ofPushView();
        ofTranslate(ofGetWindowWidth() / 2 - 100, ofGetWindowHeight() / 2 + 100);
        verdana.drawString("Connect Kinnect", 0, 0);
        ofPopView();
        ofPopMatrix();
    }
//    if(!is_start) {
//        ofPushMatrix();
//        ofPushView();
//        ofTranslate(ofGetWindowWidth() / 2 - 100, ofGetWindowHeight() / 2 + 200);
//        verdana.drawString("Init Car", 0, 0);
//        ofPopView();
//        ofPopMatrix();
//    }

}
void ofApp::drawCar() {

   
    
    ofBackground(20,20,20);
    ofSetColor(220);
    int radius = 120;
    int margin = 20;
    int border = 10;
    int angle = 0;
    
    
    ofPushMatrix();
    ofTranslate(ofGetWindowWidth() / 2, ofGetWindowHeight() / 2);
    myCar.drawPosition();
    ofPushMatrix();
    ofRotate(myCar.angle - myCar.initAngle);
    location.draw(-25, -25);
    ofPopMatrix();
    ofPopMatrix();
    
    ofPushView();
    ofTranslate(ofGetWindowWidth() / 2, ofGetWindowHeight() - 100);
    ofPushView();
    int angle_value = (rotationValue < 0 ? -1 : 1) * (ABS(rotationValue) - 1);
    ofRotate(angle_value * 10);
    wheel.draw(-75, -75);
    ofPopView();
    ofPopView();
    
    ofPushView();
    ofTranslate(ofGetWindowWidth() / 2, ofGetWindowHeight() - 50);
    ofPushView();
    verdana.drawString(ofToString(myCar.speed/ 100), 0, 0);
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
    ofRotate(60 + abs(myCar.speed) / 10);
    ofDrawTriangle(-4, 0, 0, radius * 0.7, 4, 0);
    ofPopStyle();
    ofPopView();
    
    ofPopView();
}
void ofApp::draw(){
    if(!is_ready) {
        drawConnectMessage();
        return;
        
    }
    
    
    drawCar();
    
    
   
    
    
    
    

}


int ofApp::getAngle(ofPoint l, ofPoint r) {
    double dx = r.x - l.x;
    double dy = r.y - l.y;
    double radius = sqrt(dx * dx + dy * dy);
    
    if(radius == 0)
        return 0;
    
    double sin = dy / radius;
    double cos = dx / radius;
    
    
    double rad = asin(sin);
    rad = rad * 180 / PI;
    if(cos < 0) {
        if(rad > 0)
            rad = 90;
        else
            rad = -90;
    }
    
    

    
    return rad;
}

//--------------------------------------------------------------
void ofApp::userEvent(ofxOpenNIUserEvent & event){
    // show user event messages in the console
    ofLogNotice() << getUserStatusAsString(event.userStatus) << "for user" << event.id << "from device" << event.deviceID;
}

//--------------------------------------------------------------
void ofApp::exit(){
    openNIDevice.stop();
}

//--------------------------------------------------------------
void ofApp::send(string text) {
    unsigned char cstr[10];
    //strncpy((char *)cstr,text.c_str(), text.size());
    serial.writeBytes((unsigned char *)text.c_str(), text.size());
    
    cout << "send : " << text << endl;
}
void ofApp::keyPressed(int key){
    if(!is_ready) {
        cout << "NOT CONNECT"  << endl;
        
        if(!is_start && key == 32) {
            cout << "press init"<<endl;
            
            send("init#");
        }
        
        
        return;
    }
    
    if(key == 's' || key == 'S') {
        send("0@");
        myCar.speed = 0;
        myCar.is_stop = true;
    } else if(key == 32) {
        myCar.speed += 100;
        if(myCar.speed >= 2400)
            myCar.speed = 2400;
        
        myCar.is_stop = false;
        send(ofToString(myCar.speed) + "@");
        
    } else if(key == 'b' || key == 'B') {
        myCar.speed -= 100;
        if(myCar.speed <= -2400)
            myCar.speed = -2400;
        
        myCar.is_stop = false;
        send(ofToString(myCar.speed) + "@");
    } else if (key =='i' || key == 'I') {
        send("init#");
    } else if(key == 358){
        //right
        rotationValue += 1;
        if(rotationValue == 0)
            rotationValue = 1;
        
        
        send(ofToString(rotationValue) + "%");
        cout << "r" <<rotationValue << endl;
    } else if(key == 356) {
        rotationValue -= 1;
        if(rotationValue == 0)
            rotationValue = -1;
        
        send(ofToString(rotationValue) + "%");
        cout << "l" <<rotationValue << endl;
    
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    selectedPoint.x = x;
    selectedPoint.y = y;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
