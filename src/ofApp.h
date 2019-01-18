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
    void scheduleNextMeasure();
    void armFaderPressed();
    void startPerformance();
    void onLfo1Value(float &value);
    void onLfo2Value(float &value);
    ofxBenG::generic_action *scheduleFlicker();
    ofxBenG::generic_action *playSample(ofxMaxiSample *sample);
    ofxBenG::generic_action *playClip(std::string track, std::string clip);
    ofxBenG::generic_action *stopClip(std::string track, std::string clip);
    ofxBenG::generic_action *onMeasure();
    ofxBenG::generic_action *setColor(ofxBenG::single_color_view *view, ofColor color);
    ofxBenG::generic_action *setSubmaster(int faderNumber, int percent);

    ofxFloatSlider faderLevel;
    ofxIntSlider faderNumber;
    ofxFloatSlider lfoFrequency;
    ofxButton armFader;
    ofxButton goButton;
    ofxPanel gui;

    std::shared_ptr<ofAppBaseWindow> myWindow;
    ofxBenG::etc_element lightBoard;
    ofxBenG::stream_manager *streamManager;
    ofxBenG::monitor_manager *monitorManager;
    ofxBenG::window_manager *windowManager;
    ofxBenG::timeline *timeline;
    ofxBenG::single_color_view *lightSquare;
    ofxBenG::single_color_view *cameraShade;
    ofxBenG::lfo_action *lfo1;
    ofxBenG::lfo_action *lfo2;
    ofColor transparentBlack = ofColor(0, 0, 0, 200);
    ofColor transparentWhite = ofColor(255, 255, 255, 0);
    std::vector<int> videoLengths;
    std::vector<ofxBenG::header_view*> cameraPreviews;
    int videoLengthIndex;
    int beatsInMeasure;
    bool stopAll = false;
    bool isRunning = false;
    float lightMin;
    float lightMax;
    float periodInBeats;
    float fadingInBeats;
    float fadingOutBeats;
    float recordingBeats;
    float blackoutBeats;
    ofxBenG::flicker *lastFlicker;
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

