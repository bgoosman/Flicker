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

#define TO_SECONDS(x) ((float)(x) / (float)1e6)

#define PROMISE(action) \
new ofxBenG::generic_action([&]() {\
std::cout << #action << std::endl\
action;\
}

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
    void testButton2Pressed();
    void startPerformance();
    void onBlackoutButtonPressed();
    
    void muteAll() {
        ofxBenG::ableton()->getMasterTrack()->getDevice("Auto Filter")->getParameter("Frequency")->setValue(20);
    }
    
    void blackout() {
        for (int i = channelRange[0]; i < channelRange[1]; i++) {
            lightBoard.setChannel(i, 0);
        }
    }
    
    void fadeLights(float durationBeats, int start, int end) {
        for (int i = 0; i < allLightsUsed.size(); i++) {
            timeline->cue(fadeLight(allLightsUsed[i], seconds, start, end));
        }
    }
    
    void fadeLights(std::vector<int> channels, float durationBeats, int start, int end) {
        for (int i = 0; i < channels.size(); i++) {
            timeline->cue(fadeLight(channels[i], seconds, start, end));
        }
    }
    
    void executeTransition0() {
        timeline->cue(playClip("grand piano", "2"));
    }
    
    void executeTransition1() {
        auto frequency = ofxBenG::ableton()->getMasterTrack()->getDevice("Auto Filter")->getParameter("Frequency");
        timeline->cue(fadeParameter(frequency, 1.0, filterMax, filterMin));
        timeline->cue(PROMISE(playClip("guitar", "guitar 2")));
        timeline->cue(PROMISE(playClip("piano bass drone", "piano bass drone 1")));
        timeline->cue(PROMISE(playClip("grand piano", "3")));
        timeline->cueInSeconds(0.5, fadeLights(allLightsUsed, 0.5, lightNormal, lightMin));
        timeline->cueInSeconds(1.5, PROMISE(flickerLight(sideRight[1], 0.3)));
        timeline->cueInSeconds(4, fadeLights(allLightsUsed, 2, lightMin, lightNormal));
        timeline->cueInSeconds(4, fadeParameter(frequency, 2, filterMin, filterMax));
    }
    
    void executeTransition2() {
        auto frequency = ofxBenG::ableton()->getMasterTrack()->getDevice("Auto Filter")->getParameter("Frequency");
        timeline->cue(fadeParameter(frequency, 1.0, filterMax, filterMin));
        timeline->cue(playClip("grand piano", "4"));
        timeline->cueInSeconds(0.5, fadeLights(allLightsUsed, 0.5, lightNormal, lightMin));
        timeline->cueInSeconds(1.5, flickerLight(sideRight[1], 0.3));
        timeline->cueInSeconds(4, fadeLights(allLightsUsed, 2, lightMin, lightNormal));
        timeline->cueInSeconds(4, fadeParameter(frequency, 2, filterMin, filterMax));
    }
    
    void executeTransition3() {
        auto frequency = ofxBenG::ableton()->getMasterTrack()->getDevice("Auto Filter")->getParameter("Frequency");
        timeline->cue(fadeParameter(frequency, 1.0, filterMax, filterMin));
        timeline->cue(playClip("grand piano", "5"));
        timeline->cue(playClip("violin", "violin forward"));
        timeline->cueInSeconds(3.0, new ofxBenG::generic_action([this]() {
            ofxBenG::ableton()->getTrack("violin")->getDevice("Ping Pong Delay")->setEnabled(true);
            ofxBenG::ableton()->getTrack("violin")->getDevice("Ping Pong Delay")->getParameter("Freeze")->setValue(1);
        }));
        timeline->cueInSeconds(0.5, fadeLights(allLightsUsed, 0.5, lightNormal, lightMin));
        timeline->cueInSeconds(1.0, PROMISE(ofxBenG::ableton()->getTrack("guitar drones")->setMute(true)));
        timeline->cueInSeconds(1.5, flickerLight(sideRight[1], 0.3));
        timeline->cueInSeconds(4, fadeLights(allLightsUsed, 2, lightMin, lightNormal));
        timeline->cueInSeconds(4, fadeParameter(frequency, 2, filterMin, filterMax));
        timeline->cueInSeconds(6, fadeLights(allLightsUsedExceptChannelTwo, 42, lightNormal, lightMin));
        timeline->cueInSeconds(48, fadeLight(2, 12, lightNormal, lightMin));
        timeline->cueInSeconds(48, fadeParameter(frequency, 8, filterMax, 0));
    }
    
    void executeTransition4() {
        timeline->cueInSeconds(6, new ofxBenG::generic_action([this]() {
            ofxBenG::ableton()->stopAll();
            timeline->clearScheduledActions();
            timeline->cueInSeconds(4.0, fadeLights(houseLights, 2.0, lightMin, lightNormal));
        }));
    }

    void playSample(ofxMaxiSample *sample) {
        ofxBenG::audio::getInstance()->playSample(sample);
    }

    void playClip(std::string track, std::string clip) {
        ofxBenG::ableton()->playClip(track, clip);
    }

    void playClip(ofxAbletonLiveClip *clip) {
        clip->play();
    }
    
    void stopClip(std::string track, std::string clip) {
        ofxBenG::ableton()->stopClip(track, clip);
    }
    
    void setColor(ofxBenG::single_color_view *view, ofColor color) {
        view->setColor(color);
    }
    
    void setSubmaster(int faderNumber, int percent) {
        lightBoard.setSubmaster(faderNumber, percent);
    }
    
    void setChannel(int faderNumber, int percent) {
        lightBoard.setChannel(faderNumber, percent);
    }
    
    void flickerLight(int channel, float duration) {
        timeline->cue(PROMISE(setChannel(channel, lightMax)));
        timeline->cueInSeconds(duration, PROMISE(setChannel(channel, lightMin)));
    }
    
    void aroundTheWorld() {
        aroundTheWorldRunning = true;
        
        for (int i = 0; i < aroundTheWorldLights.size(); i++) {
            int const channel = aroundTheWorldLights[i];
            timeline->cueInSeconds(i * fadeInTime, fadeLight(channel, fadeInTime, lightMin, lightMax));
            timeline->cueInSeconds(i * fadeInTime + fadeInTime, fadeLight(channel, fadeOutTime, lightMax, lightMin));
        }
       
        timeline->cueInSeconds(fadeInTime * aroundTheWorldLights.size(), new ofxBenG::generic_action([&]() {
            if (aroundTheWorldRunning) timeline->cue(PROMISE(aroundTheWorld()));
        }));
    }

    ofxBenG::lerp_action *fadeLight(int faderNumber, float seconds, int startLevel, int endLevel) {
        return new ofxBenG::lerp_action(seconds, [=](float lerpValue, float min, float max) {
            int const faderLevel = (int)ofMap(lerpValue, min, max, (float)startLevel, (float)endLevel);
            lightBoard.setChannel(faderNumber, faderLevel);
        });
    }
    
    ofxBenG::lerp_action *fadeVolume(ofxAbletonLiveTrack *track, float seconds, float startLevel, float endLevel) {
        return new ofxBenG::lerp_action(seconds, [=](float lerpValue, float min, float max) {
            track->setVolume(ofMap(lerpValue, min, max, startLevel, endLevel));
        });
    }
    
    ofxBenG::lerp_action *fadeParameter(ofxAbletonLiveParameter *parameter, float seconds, float startLevel, float endLevel) {
        return new ofxBenG::lerp_action(seconds, [=](float lerpValue, float min, float max) {
            float newFrequency = ofMap(lerpValue, min, max, startLevel, endLevel);
            parameter->setValue(newFrequency);
        });
    }

    // GUI
    ofxButton testButton;
    ofxButton testButton2;
    ofxButton blackoutButton;
    ofxButton goButton;
    ofParameter<float> fadeOutTime;
    ofParameter<float> fadeInTime;
    ofParameter<float> fadeInPreTime;
    ofParameter<bool> aroundTheWorldRunning;
    ofParameter<float> lightMin;
    ofParameter<float> lightMax;
    ofParameterGroup parameters;
    ofxPanel gui;

    // Lights
    ofxBenG::etc_element_osc_proxy lightBoard;
    std::string lightBoardIp = "169.254.1.42";
    int lightBoardPort = 3000;
    std::vector<int> lights{5, 12, 13, 14, 16, 17, 25};
    ofPoint channelRange = {0, 120};
    std::vector<int> sideRight{11, 12, 13, 14};
    std::vector<int> sideLeft{15, 16, 17, 18};
    std::vector<int> backs{25};
    std::vector<int> fronts{1, 2, 3};
    std::vector<int> allLightsUsed{2, 11, 12, 15, 16, 25};
    std::vector<int> allLightsUsedExceptChannelTwo{11, 12, 15, 16, 25};
    std::vector<int> houseLights{1, 2, 3, 4, 5, 6, 11, 12, 13, 14, 15, 16, 17, 18, 22};
    std::vector<int> aroundTheWorldLights = {25, 12, 5, 16};
    float const lightNormal = 75;

    // Video
    ofxBenG::video_stream *stream = nullptr;
    ofxPm::VideoBuffer *buffer = nullptr;
    ofxPm::VideoHeader *header = nullptr;
    ofxPm::BasicVideoRenderer *renderer = nullptr;
    ofTexture *holdFrame = nullptr;
    float recordingFps;
    
    // Audio
    int const beatsInBar = 4;
    float const volumeOff = 0.0;
    float const volumeMin = 0.5;
    float const volumeMax = 0.75;
    float const filterMin = 60;
    float const filterMax = 135;

    // App
    std::shared_ptr<ofAppBaseWindow> myWindow;
    ofxBenG::stream_manager *streamManager;
    ofxBenG::monitor_manager *monitorManager;
    ofxBenG::window_manager *windowManager;
    ofxBenG::timeline *timeline;
    std::vector<ofxBenG::header_view*> cameraPreviews;
    bool isRunning = false;
    int const oneMinute = 60;
    int const showLength = oneMinute * 5;
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

