#pragma once

#include "ofMain.h"
#include "beat_action.h"
#include "video_stream.h"
#include "monitor.h"
#include "stream_manager.h"
#include "monitor_manager.h"
#include "window_manager.h"
#include "etc_element.h"
#include "ofxAbletonLive.h"
#include "ofxMidi.h"
#include "ofxGui.h"
#include <functional>
#include <algorithm>

class ofApp : public ofBaseApp {
public:
    ofApp(std::shared_ptr<ofAppBaseWindow> mainWindow);
    ~ofApp();
    
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    void onVideoStreamAdded(ofxBenG::video_stream &stream);
    void onVideoStreamRemoved(ofxBenG::video_stream &stream);
    void onMonitorAdded(ofxBenG::monitor &monitor);
    void onMonitorRemoved(ofxBenG::monitor &monitor);
    void audioOut(float *output, int bufferSize, int nChannels);
    void testButtonPressed();
    void startPerformance();
    void testLights();
    ofxBenG::generic_action *muteAll();
    ofxBenG::generic_action *blackout();
    ofxBenG::generic_action *fadeLights(float durationBeats, int start, int end);
    ofxBenG::generic_action *executeTransition1();
    ofxBenG::generic_action *executeTransition2();
    ofxBenG::generic_action *executeTransition3();
    ofxBenG::generic_action *executeTransition4();
    ofxBenG::generic_action *playSample(ofxMaxiSample *sample);
    ofxBenG::generic_action *playClip(std::string track, std::string clip);
    ofxBenG::generic_action *playClip(ofxAbletonLiveClip *clip);
    ofxBenG::generic_action *stopClip(std::string track, std::string clip);
    ofxBenG::generic_action *setColor(ofxBenG::single_color_view *view, ofColor color);
    ofxBenG::generic_action *setSubmaster(int faderNumber, int percent);
    ofxBenG::generic_action *setChannel(int faderNumber, int percent);
    ofxBenG::lerp_action *fadeLight(int faderNumber, float durationBeats, int startLevel, int endLevel);
    ofxBenG::lerp_action *fadeVolume(ofxAbletonLiveTrack *track, float durationBeats, float startLevel, float endLevel);
    ofxBenG::lerp_action *fadeParameter(ofxAbletonLiveParameter *parameter, float durationBeats, float startLevel, float endLevel);

    // GUI
    ofxButton testButton;
    ofxButton goButton;
    ofxPanel gui;

    // Lights
    ofxBenG::etc_element_osc_proxy lightBoard;
    std::string lightBoardIp = "169.254.1.42";
    int lightBoardPort = 3000;
    std::vector<int> lights{5, 12, 13, 14, 16, 17, 25};
    
    // Video
    ofxBenG::video_stream *stream = nullptr;
    ofxPm::VideoBuffer *buffer = nullptr;
    ofxPm::VideoHeader *header = nullptr;
    ofxPm::BasicVideoRenderer *renderer = nullptr;
    ofTexture *holdFrame = nullptr;
    float recordingFps;

    // App
    std::shared_ptr<ofAppBaseWindow> myWindow;
    ofxBenG::stream_manager *streamManager;
    ofxBenG::monitor_manager *monitorManager;
    ofxBenG::window_manager *windowManager;
    ofxBenG::timeline *timeline;
    std::vector<ofxBenG::header_view*> cameraPreviews;
    int beatsInBar = 4;
    bool stopAll = false;
    bool isRunning = false;
    float const lightMin = 0;
    float const lightNormal = 75;
    float const lightMax = 100;
    float const volumeOff = 0.0;
    float const volumeMin = 0.5;
    float const volumeMax = 0.75;
    float const recordingBeats = 8;
    float const fadeInBeats = 3;
    float const fadeOutBeats = 2;
    int const frontMax = 81;
    int const sideMax = 25;
    int const backMax = 15;
    float const filterMin = 60;
    float const filterMax = 135;
    int const showLength = 60 * 5;
};

class blackout_view : public ofxBenG::window_view {
public:
    virtual void draw(ofPoint size) {
        ofPushStyle();
        ofSetColor(ofColor::black);
        ofDrawRectangle(0, 0, size[0], size[1]);
        ofPopStyle();
    }
};

