#include "ofApp.h"

ofApp::ofApp(std::shared_ptr<ofAppBaseWindow> myWindow) :
myWindow(myWindow) {
    ofSetVerticalSync(true);
}

ofApp::~ofApp() {
    ofxBenG::ableton()->stopAll();
    delete streamManager;
}

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetLogLevel(OF_LOG_WARNING);
//    int const desiredWidth = 720;
//    int const desiredHeight = 486;
//    float const desiredFps = 59.94;
//    int const desiredWidth = 1920;
//    int const desiredHeight = 1080;
//    float const desiredFps = 59.94;
    int const desiredWidth = 1280;
    int const desiredHeight = 720;
    float const desiredFps = 60;
    int const defaultBufferSize = 60;
    double const beatsPerMinute = 60;
    streamManager = new ofxBenG::stream_manager(desiredWidth, desiredHeight, desiredFps, defaultBufferSize);
    streamManager->excludeDevice("FaceTime HD Camera");
    ofAddListener(streamManager->onVideoStreamAdded, this, &ofApp::onVideoStreamAdded);
    ofAddListener(streamManager->onVideoStreamRemoved, this, &ofApp::onVideoStreamRemoved);
    monitorManager = new ofxBenG::monitor_manager();
    monitorManager->excludeMonitor("HP Pavilion32");
    monitorManager->excludeMonitor("Color LCD"); 
    monitorManager->excludeMonitor("BenQ GW2765");
    ofAddListener(monitorManager->onMonitorAdded, this, &ofApp::onMonitorAdded);
    ofAddListener(monitorManager->onMonitorRemoved, this, &ofApp::onMonitorRemoved);
    windowManager = new ofxBenG::window_manager();
    windowManager->setFullscreen(true);
    timeline = new ofxBenG::timeline(4);
    lightBoard.setup("169.254.1.42", 3000);
    gui.setup();
    gui.add(testButton.setup("Test", 150, 25));
    gui.add(goButton.setup("Go", 150, 150));
    goButton.addListener(this, &ofApp::startPerformance);
    testButton.addListener(this, &ofApp::testButtonPressed);
    renderer = new ofxPm::BasicVideoRenderer;
    ofxBenG::ableton()->setup(beatsPerMinute, 8.0);
    ofxBenG::ableton()->update();
    ofxBenG::ableton()->stopAll();
}

void ofApp::update() {
    ofxBenG::ableton()->update();
    streamManager->update();
    monitorManager->update();
    timeline->update();
}

void ofApp::draw() {
    if (isRunning) {
        ofBackground(ofColor::darkMagenta);
    }

    std::stringstream strm;
    strm << "fps: " << ofGetFrameRate();
    ofSetWindowTitle(strm.str());

    float topPadding = 25;

    gui.draw();

    float x = gui.getPosition()[0];
    float y = gui.getHeight() + gui.getPosition()[1] + topPadding;

    auto stream = streamManager->getStream(0);
    if (stream != nullptr) {
        ofDrawBitmapString(ofToString(stream->getFps()), ofPoint(x, y));
        y += topPadding;
    }

    ofDrawBitmapString("Beat: " + ofToString(ofxBenG::ableton()->getBeat()), ofPoint(x, y));
    y += topPadding;

    float const previewWidth = 160;
    float const previewHeight = 90;

    for (auto monitor : monitorManager->getMonitors()) {
        ofDrawBitmapString("Monitor: " + monitor->getName(), ofPoint(x, y));
        y += topPadding;
    }

    for (auto headerView : cameraPreviews) {
        headerView->draw(x, y, previewWidth, previewHeight);
        y += previewHeight + 5;
    }
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

    auto headerView = new ofxBenG::header_view(stream.makeHeader(0));
    cameraPreviews.push_back(headerView);

    auto window = windowManager->getWindowWithNoStream();
    if (window != nullptr && window->getMonitorName() != "Optoma WXGA") {
        window->setStream(&stream);
    }
}

void ofApp::onVideoStreamRemoved(ofxBenG::video_stream &stream) {
    std::cout << "onVideoStreamRemoved stream: " << stream.getDeviceName() << std::endl;
}

void ofApp::onMonitorAdded(ofxBenG::monitor &monitor) {
    std::cout << "Monitor added: " << monitor.toString() << std::endl;
    auto window = windowManager->makeWindow(myWindow);
    window->setMonitor(&monitor);
    window->setStream(streamManager->getUnusedStream());
}

void ofApp::onMonitorRemoved(ofxBenG::monitor &monitor) {
    std::cout << "Monitor removed: " << monitor.toString() << std::endl;
}

void ofApp::startPerformance() {
    isRunning = true;
    timeline->scheduleNextWholeBeat(new ofxBenG::generic_action([this]() {
        ofxBenG::ableton()->setClockToZero();
        ofxBenG::ableton()->play();
        ofxBenG::ableton()->getMasterTrack()->getDevice("Auto Filter")->getParameter("Frequency")->setValue(135);
        ofxBenG::ableton()->getTrack("violin")->getDevice("Simple Delay")->setEnabled(false);
        ofxBenG::ableton()->getTrack("violin")->getDevice("Ping Pong Delay")->setEnabled(false);
        ofxBenG::ableton()->getTrack("violin")->getDevice("Ping Pong Delay")->getParameter("Freeze")->setValue(0);
        ofxBenG::ableton()->getTrack("guitar drones")->setVolume(0);
        ofxBenG::ableton()->getTrack("guitar drones")->setMute(false);
        timeline->executeAction(playClip("muffle", "fumbling around"));
        timeline->executeAction(playClip("piano bass drone", "piano bass drone 1"));
        timeline->executeAction(playClip("guitar", "guitar 2"));
        timeline->executeAction(fadeLights(0.5, lightNormal, lightMin));
        timeline->schedule(1.0, fadeLights(60.0, lightMin, lightMax));
        timeline->scheduleOnNthBeatFromNow(30, playClip("piano treble drone", "piano treble drone 1"));
        timeline->scheduleOnNthBeatFromNow(61, playClip("guitar drones", "guitar drone uplifting"));
        timeline->scheduleOnNthBeatFromNow(61, fadeVolume(ofxBenG::ableton()->getTrack("guitar drones"), 8, 0, 0.7));
        timeline->scheduleOnNthBeatFromNow(2 * 60, executeTransition1());
        timeline->scheduleOnNthBeatFromNow(3 * 60, executeTransition2());
        timeline->scheduleOnNthBeatFromNow(4 * 60, executeTransition3());
        timeline->scheduleOnNthBeatFromNow(showLength, executeTransition4());
    }));
}

ofxBenG::generic_action *ofApp::muteAll() {
    return new ofxBenG::generic_action([this]() {
        ofxBenG::ableton()->getMasterTrack()->getDevice("Auto Filter")->getParameter("Frequency")->setValue(20);
    });
}

ofxBenG::generic_action *ofApp::blackout() {
    return new ofxBenG::generic_action([this]() {
        for (int i = 0; i < lights.size(); i++) {
            lightBoard.setChannel(lights[i], 0);
        }
    });
}

ofxBenG::generic_action *ofApp::executeTransition1() {
    return new ofxBenG::generic_action([this]() {
        int const sideRightClose = 12;
        int const sideRightFar = 13;
        int const sideRightBack = 14;
        int const sideLeftClose = 16;
        int const sideLeftFar = 17;
        int const back = 25;
        auto frequency = ofxBenG::ableton()->getMasterTrack()->getDevice("Auto Filter")->getParameter("Frequency");
        timeline->executeAction(fadeParameter(frequency, 1.0, filterMax, filterMin));
        ofxBenG::ableton()->getTrack("violin")->getDevice("Simple Delay")->setEnabled(true);
        timeline->executeAction(playClip("violin", "violin forward"));
        timeline->schedule(0.5, fadeLight(sideRightClose, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(sideRightFar, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(sideLeftClose, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(sideLeftFar, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(back, 0.5, 75, 0));
        timeline->schedule(1.5, setChannel(sideRightFar, 100));
        timeline->schedule(1.8, setChannel(sideRightFar, 0));
        timeline->scheduleOnNthBeatFromNow(4, fadeParameter(frequency, 2, filterMin, filterMax));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(sideRightClose, 2, 0, 75));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(sideRightFar, 2, 0, 75));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(sideLeftClose, 2, 0, 75));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(sideLeftFar, 2, 0, 75));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(back, 2, 0, 75));
    });
}

ofxBenG::generic_action *ofApp::executeTransition2() {
    return new ofxBenG::generic_action([this]() {
        int const sideRightClose = 12;
        int const sideRightFar = 13;
        int const sideRightBack = 14;
        int const sideLeftClose = 16;
        int const sideLeftFar = 17;
        int const back = 25;
        auto frequency = ofxBenG::ableton()->getMasterTrack()->getDevice("Auto Filter")->getParameter("Frequency");
        timeline->executeAction(fadeParameter(frequency, 1.0, filterMax, filterMin));
        ofxBenG::ableton()->getTrack("violin")->getDevice("Ping Pong Delay")->setEnabled(true);
        timeline->executeAction(playClip("grand piano", "1"));
        timeline->schedule(0.5, fadeLight(sideRightClose, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(sideRightFar, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(sideLeftClose, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(sideLeftFar, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(back, 0.5, 75, 0));
        timeline->schedule(1.5, setChannel(sideRightFar, 100));
        timeline->schedule(1.8, setChannel(sideRightFar, 0));
        timeline->scheduleOnNthBeatFromNow(4, fadeParameter(frequency, 2, filterMin, filterMax));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(sideRightClose, 2, 0, 75));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(sideRightFar, 2, 0, 75));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(sideLeftClose, 2, 0, 75));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(sideLeftFar, 2, 0, 75));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(back, 2, 0, 75));
    });
}

ofxBenG::generic_action *ofApp::executeTransition3() {
    return new ofxBenG::generic_action([this]() {
        int const sideRightClose = 12;
        int const sideRightFar = 13;
        int const sideRightBack = 14;
        int const sideLeftClose = 16;
        int const sideLeftFar = 17;
        int const back = 25;
        auto frequency = ofxBenG::ableton()->getMasterTrack()->getDevice("Auto Filter")->getParameter("Frequency");
        timeline->executeAction(fadeParameter(frequency, 1.0, filterMax, filterMin));
        timeline->schedule(1.0, new ofxBenG::generic_action([this] {ofxBenG::ableton()->getTrack("guitar drones")->setMute(true);}));
        ofxBenG::ableton()->getTrack("violin")->getDevice("Ping Pong Delay")->getParameter("Freeze")->setValue(1);
        timeline->schedule(0.5, fadeLight(sideRightClose, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(sideRightFar, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(sideLeftClose, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(sideLeftFar, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(back, 0.5, 75, 0));
        timeline->schedule(1.5, setChannel(sideRightFar, 100));
        timeline->schedule(1.8, setChannel(sideRightFar, 0));
        timeline->scheduleOnNthBeatFromNow(4, fadeParameter(frequency, 2, filterMin, filterMax));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(sideRightClose, 2, 0, 75));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(sideRightFar, 2, 0, 75));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(sideLeftClose, 2, 0, 75));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(sideLeftFar, 2, 0, 75));
        timeline->scheduleOnNthBeatFromNow(4, fadeLight(back, 2, 0, 75));
    });
}

ofxBenG::generic_action *ofApp::executeTransition4() {
    return new ofxBenG::generic_action([this]() {
        int const sideRightClose = 12;
        int const sideRightFar = 13;
        int const sideRightBack = 14;
        int const sideLeftClose = 16;
        int const sideLeftFar = 17;
        int const back = 25;
        auto frequency = ofxBenG::ableton()->getMasterTrack()->getDevice("Auto Filter")->getParameter("Frequency");
        timeline->executeAction(fadeParameter(frequency, 1.0, filterMax, filterMin));
        timeline->schedule(0.5, fadeLight(sideRightClose, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(sideRightFar, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(sideLeftClose, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(sideLeftFar, 0.5, 75, 0));
        timeline->schedule(0.5, fadeLight(back, 0.5, 75, 0));
        timeline->schedule(1.5, setChannel(sideRightFar, 100));
        timeline->schedule(3.0, setChannel(sideRightFar, 0));
        timeline->scheduleOnNthBeatFromNow(6, new ofxBenG::generic_action([this]() {
            stopAll = true;
            ofxBenG::ableton()->stopAll();
            timeline->clearScheduledActions();
            auto frequency = ofxBenG::ableton()->getMasterTrack()->getDevice("Auto Filter")->getParameter("Frequency");
            timeline->executeAction(fadeParameter(frequency, 1.0, filterMin, 0));
            timeline->schedule(4.0, fadeLights(2.0, 0, 75));
        }));
    });
}

void ofApp::testButtonPressed() {
    testLights();
}

void ofApp::testLights() {
    timeline->executeAction(blackout());
    timeline->executeAction(fadeLights(3.0, 0, 75));
}

ofxBenG::generic_action *ofApp::fadeLights(float durationBeats, int start, int end) {
    return new ofxBenG::generic_action([this, durationBeats, start, end]() {
        for (int i = 0; i < lights.size(); i++) {
            timeline->executeAction(fadeLight(lights[i], durationBeats, start, end));
        }
    });
}

ofxBenG::generic_action *ofApp::playClip(ofxAbletonLiveClip *clip) {
    return new ofxBenG::generic_action([clip]() {
        clip->play();
    });
}

ofxBenG::generic_action *ofApp::playSample(ofxMaxiSample *sample) {
    return new ofxBenG::generic_action([sample]() {
        ofxBenG::audio::getInstance()->playSample(sample);
    });
}

ofxBenG::generic_action *ofApp::playClip(std::string track, std::string clip) {
    return new ofxBenG::generic_action([track, clip]() {
        //        std::cout << ofxBenG::ableton()->getBeat() << " playClip " << track << " - " << clip << std::endl;
        ofxBenG::ableton()->playClip(track, clip);
    });
}

ofxBenG::generic_action *ofApp::stopClip(std::string track, std::string clip) {
    return new ofxBenG::generic_action([track, clip]() {
        //        std::cout << ofxBenG::ableton()->getBeat() << " stopClip " << track << " - " << clip << std::endl;
        ofxBenG::ableton()->stopClip(track, clip);
    });
}

ofxBenG::generic_action *ofApp::setColor(ofxBenG::single_color_view *view, ofColor color) {
    return new ofxBenG::generic_action([view, color]() {
        //         std::cout << ofxBenG::ableton()->getBeat() << " setColor " << color << std::endl;
        view->setColor(color);
    });
}

ofxBenG::generic_action *ofApp::setSubmaster(int faderNumber, int percent) {
    return new ofxBenG::generic_action([this, faderNumber, percent]() {
        std::cout << ofxBenG::ableton()->getBeat() << " setSubmaster " << faderNumber << " @ " << percent << "%" << std::endl;
        lightBoard.setSubmaster(faderNumber, percent);
    });
}

ofxBenG::generic_action *ofApp::setChannel(int faderNumber, int percent) {
    return new ofxBenG::generic_action([this, faderNumber, percent]() {
        std::cout << ofxBenG::ableton()->getBeat() << " setChannel " << faderNumber << " @ " << percent << "%" << std::endl;
        lightBoard.setChannel(faderNumber, percent);
    });
}

ofxBenG::lerp_action *ofApp::fadeLight(int faderNumber, float durationBeats, int startLevel, int endLevel) {
    return new ofxBenG::lerp_action(durationBeats, [this, faderNumber, startLevel, endLevel](float lerpValue, float min, float max) {
        int const faderLevel = (int)ofMap(lerpValue, min, max, (float)startLevel, (float)endLevel);
        this->lightBoard.setChannel(faderNumber, faderLevel);
    });
}

ofxBenG::lerp_action *ofApp::fadeVolume(ofxAbletonLiveTrack *track, float durationBeats, float startLevel, float endLevel) {
    return new ofxBenG::lerp_action(durationBeats, [this, track, startLevel, endLevel](float lerpValue, float min, float max) {
        //        std::cout << ofxBenG::ableton()->getBeat() << " " << lerpValue << " " << ofMap(lerpValue, min, max, startLevel, endLevel) << std::endl;
        track->setVolume(ofMap(lerpValue, min, max, startLevel, endLevel));
    });
}

ofxBenG::lerp_action *ofApp::fadeParameter(ofxAbletonLiveParameter *parameter, float durationBeats, float startLevel, float endLevel) {
    return new ofxBenG::lerp_action(durationBeats, [this, parameter, startLevel, endLevel](float value, float min, float max) {
        float newFrequency = ofMap(value, min, max, startLevel, endLevel);
        //std::cout << newFrequency << std::endl;
        parameter->setValue(newFrequency);
    });
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    if (key == 's') {
        startPerformance();
    }
    
    if (key == 'q') {
        ofExit(0);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
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
