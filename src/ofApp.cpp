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
    ofxBenG::ableton()->setupLink(beatsPerMinute, 8.0);
    timeline = new ofxBenG::timeline();
    live.setup();
}

void ofApp::update() {
    live.update();
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
        timeline->scheduleNextWholeMeasure(new ofxBenG::generic_action([this]() {
            ofxBenG::ableton()->setClockToZero();
            scheduleNextMeasure();
            timeline->schedule(8, new ofxBenG::generic_action([this]() {
                live.getTrack("hammer")->getDevice("Fill In The Gaps")->setEnabled(true);
            }));
        }));
    }
}

ofxBenG::generic_action *ofApp::playSample(ofxMaxiSample *sample) {
    return new ofxBenG::generic_action([sample]() {
        ofxBenG::audio::getInstance()->playSample(sample);
    });
}

ofxBenG::generic_action *ofApp::playClip(std::string trackName, std::string clipName) {
    ofxAbletonLiveTrack *track = live.getTrack(trackName);
    ofxAbletonLiveClip *clip = track->getClip(clipName);
    return new ofxBenG::generic_action([clip]() { clip->play(); });
}

void ofApp::scheduleNextMeasure() {
    int const measure = ((int) floor(ofxBenG::ableton()->getBeat())) / 4;
    
    if (measure % 4 == 0) {
        auto stream = streamManager->getStream(0);
        float const blackoutLengthBeats = 0.0025;
        float const videoLengthBeats = 8;
        timeline->schedule(0, new ofxBenG::flicker(stream, blackoutLengthBeats, videoLengthBeats));
        timeline->schedule(8, playClip("flicker", "gas click"));
    }
    
    if (measure % 8 == 0) {
        //        timeline->schedule(0, playClip("bros", "forget-you"));
    }
    
    timeline->schedule(4, new ofxBenG::generic_action([this]() {
        scheduleNextMeasure();
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

