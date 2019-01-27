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
    void testButtonPressed();
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
    ofxBenG::lerp_action *fadeLight(int faderNumber, float durationBeats, float startLevel, float endLevel);
    ofxBenG::lerp_action *fadeVolume(ofxAbletonLiveTrack *track, float durationBeats, float startLevel, float endLevel);
    ofxBenG::lerp_action *fadeParameter(ofxAbletonLiveParameter *parameter, float durationBeats, float startLevel, float endLevel);

    ofxFloatSlider faderLevel;
    ofxIntSlider faderNumber;
    ofxFloatSlider lfoFrequency;
    ofxButton armFader;
    ofxButton testButton;
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
    int videoLengthIndex = 0;
    int beatsInMeasure = 8;
    bool stopAll = false;
    bool isRunning = false;
    float lightMin = 0;
    float lightMax = 100;
    float volumeOff = 0.0;
    float volumeMin = 0.5;
    float volumeMax = 0.75;
    float recordingBeats = 8;
    float fadeInBeats = 3;
    float fadeOutBeats = 2;
    int const frontFaderNumber = 37;
    int const houseFaderNumber = 38;
    int const sideFaderNumber = 39;
    int const backFaderNumber = 40;
    int frontMax = 81;
    int sideMax = 25;
    int backMax = 15;
    float const filterMin = 30;
    float const filterMax = 80;

    ofxBenG::video_stream *stream = nullptr;
    ofxPm::VideoBuffer *buffer = nullptr;
    ofxPm::VideoHeader *header = nullptr;
    ofxPm::BasicVideoRenderer *renderer = nullptr;
    ofTexture *holdFrame = nullptr;
    float recordingFps;
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

