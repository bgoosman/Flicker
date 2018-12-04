#include "ofApp.h"

ofApp::ofApp(std::shared_ptr<ofAppBaseWindow> myWindow) :
myWindow(myWindow) {
    ofSetVerticalSync(true);
}

ofApp::~ofApp() {
    delete streamManager;
}

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetLogLevel(OF_LOG_WARNING);
    int const desiredWidth = 800;
    int const desiredHeight = 600;
    int const desiredFps = 30;
    int const defaultBufferSize = 1;
    double const beatsPerMinute = 60;
    streamManager = new ofxBenG::stream_manager(desiredWidth, desiredHeight, desiredFps, defaultBufferSize);
    ofAddListener(streamManager->onVideoStreamAdded, this, &ofApp::onVideoStreamAdded);
    ofAddListener(streamManager->onVideoStreamRemoved, this, &ofApp::onVideoStreamRemoved);
    monitorManager = new ofxBenG::monitor_manager();
    ofAddListener(monitorManager->onMonitorAdded, this, &ofApp::onMonitorAdded);
    ofAddListener(monitorManager->onMonitorRemoved, this, &ofApp::onMonitorRemoved);
    windowManager = new ofxBenG::window_manager();
    ofxBenG::ableton()->setup(beatsPerMinute, 8.0);
    timeline = new ofxBenG::timeline(4);
    ofxBenG::ableton()->stopAll();
}

void ofApp::update() {
    ofxBenG::ableton()->update();
    streamManager->update();
    monitorManager->update();
    timeline->update();
}

void ofApp::draw() {
    auto stream = streamManager->getStream(0);
    if (stream != nullptr)
        ofDrawBitmapString(ofToString(streamManager->getStream(0)->getFps()), ofPoint(5, 25));
    ofDrawBitmapString(ofToString(ofGetFrameRate()), ofPoint(5, 50));
}

void ofApp::audioOut(float* output, int bufferSize, int nChannels) {
    for (int i = 0; i < bufferSize; i++) {
        float mix = ofxBenG::audio::getInstance()->getMix();
        output[nChannels * i] = mix;
        output[nChannels * i + 1] = mix;
    }
}

void ofApp::onVideoStreamAdded(ofxBenG::video_stream &stream) {
    std::cout << "onVideoStreamAdded stream: " << stream.getDeviceName() << std::endl;
    auto window = windowManager->getWindowWithNoStream();
    if (window != nullptr) {
        window->setStream(&stream);
    }
}

void ofApp::onVideoStreamRemoved(ofxBenG::video_stream &stream) {
    std::cout << "onVideoStreamRemoved stream: " << stream.getDeviceName() << std::endl;
}

void ofApp::onMonitorAdded(ofxBenG::monitor &monitor) {
    std::cout << "Monitor added: " << monitor.toString() << std::endl;
    auto window = windowManager->makeWindow(myWindow);
    window->setStream(streamManager->getUnusedStream());
    window->setMonitor(&monitor);
}

void ofApp::onMonitorRemoved(ofxBenG::monitor &monitor) {
    std::cout << "Monitor removed: " << monitor.toString() << std::endl;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    if (key == 'f') {
        auto stream = streamManager->getStream(0);
        float const blackoutLengthBeats = 0.05;
        float const videoLengthBeats = ofxBenG::utilities::secondsToBeats(5, ofxBenG::ableton()->getTempo());
        timeline->scheduleNextWholeBeat(new ofxBenG::flicker(stream, blackoutLengthBeats, videoLengthBeats));
    }
    
    if (key == 'p') {
        streamManager->addVideoStream(ofxBenG::stream_manager::ps3eye);
    }
    
    if (key == 's') {
        int const beatsBetweenClips = 30;
        timeline->scheduleNextWholeBeat(new ofxBenG::generic_action([this]() {
            ofxBenG::ableton()->setClockToZero();
            timeline->schedule(0, new ofxBenG::generic_action([this]() { scheduleNextMeasure(); }));
            timeline->schedule(beatsBetweenClips, playClip("hammer", "hammer on metal 2"));
            timeline->schedule(beatsBetweenClips * 2, playClip("cat", "Modular UI - Source Recordings -Solo Beeps-028-50Sec"));
            timeline->schedule(beatsBetweenClips * 3, playClip("popcorn", "Modular UI - Confirm ToneFM-069"));
            timeline->schedule(beatsBetweenClips * 4, playClip("additive faceplant shimmer", "Additive Faceplant"));
            const int fiveMinutes = 60 * 5;
            timeline->schedule(fiveMinutes - beatsBetweenClips * 3, stopClip("popcorn", "Modular UI - Confirm ToneFM-069"));
            timeline->schedule(fiveMinutes - beatsBetweenClips * 2, stopClip("cat", "Modular UI - Source Recordings -Solo Beeps-028-50Sec"));
            timeline->schedule(fiveMinutes - beatsBetweenClips, stopClip("additive faceplant shimmer", "Additive Faceplant"));
            timeline->schedule(fiveMinutes, stopClip("hammer", "hammer on metal 2"));
            timeline->schedule(fiveMinutes - 1, new ofxBenG::generic_action([this]() {
                stopAll = true;
                for (auto window : windowManager->getWindows()) {
                    window->addView(new blackout_view());
                }
            }));
        }));
    }
}

ofxBenG::generic_action *ofApp::playSample(ofxMaxiSample *sample) {
    return new ofxBenG::generic_action([sample]() {
        ofxBenG::audio::getInstance()->playSample(sample);
    });
}

ofxBenG::generic_action *ofApp::playClip(std::string track, std::string clip) {
    return new ofxBenG::generic_action([track, clip]() {
        std::cout << ofxBenG::ableton()->getBeat() << " playClip " << track << " - " << clip << std::endl;
        ofxBenG::ableton()->playClip(track, clip);
    });
}

ofxBenG::generic_action *ofApp::stopClip(std::string track, std::string clip) {
    return new ofxBenG::generic_action([track, clip]() {
        std::cout << ofxBenG::ableton()->getBeat() << " stopClip " << track << " - " << clip << std::endl;
        ofxBenG::ableton()->stopClip(track, clip);
    });
}

void ofApp::scheduleNextMeasure() {
    int const measure = ((int) floor(ofxBenG::ableton()->getBeat())) / 4;
    
    if (measure % 4 == 0) {
        auto stream = streamManager->getStream(0);
        float const blackoutLengthBeats = 0.0025;
        float const videoLengthBeats = 8;
        float const beatsPerMove = 2;
        float const moves = 4;
        timeline->schedule(0, new ofxBenG::flicker(stream, blackoutLengthBeats, videoLengthBeats));
        timeline->schedule(0, playClip("flicker", "gas click 2"));
        timeline->schedule(beatsPerMove * moves, playClip("flicker", "gas click"));
    }

    timeline->schedule(4, new ofxBenG::generic_action([this]() {
        if (!stopAll) scheduleNextMeasure();
    }));
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
    
}

