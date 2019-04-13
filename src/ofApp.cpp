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
    double const beatsPerMinute = 20;
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
    parameters.setName("settings");
    parameters.add(fadeInTime.set("fadeInTime", 1.0, 0.0, 10.0));
    parameters.add(fadeOutTime.set("fadeOutTime", 1.5, 0.0, 10.0));
    parameters.add(fadeInPreTime.set("fadeInPreTime", 0.5, 0.0, 10.0));
    parameters.add(lightMin.set("lightMin", 40.0, 0.0, 100.0));
    parameters.add(lightMax.set("lightMax", 75.0, 0.0, 100.0));
    parameters.add(aroundTheWorldRunning.set("aroundTheWorldRunning", false));
    gui.setup(parameters);
    gui.add(testButton.setup("test 1", 150, 25));
    gui.add(testButton2.setup("test 2", 150, 25));
    gui.add(blackoutButton.setup("blackout", 150, 25));
    gui.add(goButton.setup("Go", 150, 150));
    goButton.addListener(this, &ofApp::startPerformance);
    testButton.addListener(this, &ofApp::testButtonPressed);
    testButton2.addListener(this, &ofApp::testButton2Pressed);
    blackoutButton.addListener(this, &ofApp::onBlackoutButtonPressed);
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

    ofDrawBitmapString("bpm: " + ofToString(ofxBenG::ableton()->getTempo()), ofPoint(x, y));
    y += topPadding;

    ofDrawBitmapString("beat: " + ofToString(ofxBenG::ableton()->getBeat()), ofPoint(x, y));
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
        CUE(stopClip("muffle", "fumbling around"));
        CUE(stopClip("piano treble drone", "piano treble drone 1"));
        CUE(stopClip("guitar", "guitar 2"));
        CUE(stopClip("grand piano", "1"));
        CUE(stopClip("grand piano", "2"));
        CUE(stopClip("grand piano", "3"));
        CUE(stopClip("grand piano", "4"));
        CUE(stopClip("piano bass drone", "piano bass drone 1"));
        CUE(stopClip("guitar drone", "guitar drone uplifting"));
        CUE(stopClip("violin", "violin forward"));
        CUE(ofxBenG::ableton()->setClockToZero());
        CUE(ofxBenG::ableton()->play());
        CUE(ofxBenG::ableton()->getMasterTrack()->getDevice("Auto Filter")->getParameter("Frequency")->setValue(135));
        CUE(ofxBenG::ableton()->getTrack("violin")->getDevice("Simple Delay")->setEnabled(false));
        CUE(ofxBenG::ableton()->getTrack("violin")->getDevice("Ping Pong Delay")->setEnabled(false));
        CUE(ofxBenG::ableton()->getTrack("violin")->getDevice("Ping Pong Delay")->getParameter("Freeze")->setValue(0));
        CUE(ofxBenG::ableton()->getTrack("guitar drones")->setVolume(0));
        CUE(ofxBenG::ableton()->getTrack("guitar drones")->setMute(false));
        CUE(blackout());
        CUE(playClip("muffle", "fumbling around"));
        CUE(playClip("grand piano", "1"));
        CUE_IN_SECONDS(1, fadeLights(60.0, lightMin, lightMax));
        CUE_IN_SECONDS(1, playClip("guitar drones", "guitar drone uplifting"));
        CUE_IN_SECONDS(1, fadeVolume(ofxBenG::ableton()->getTrack("guitar drones"), 60, 0, 0.7));
        CUE_IN_SECONDS(oneMinute / 2, playClip("piano treble drone", "piano treble drone 1"));
        CUE_IN_SECONDS(1 * oneMinute, executeTransition0());
        CUE_IN_SECONDS(2 * oneMinute, executeTransition1());
        CUE_IN_SECONDS(3 * oneMinute, executeTransition2());
        CUE_IN_SECONDS(4 * oneMinute, executeTransition3());
        CUE_IN_SECONDS(showLength, executeTransition4());
    }));
}

void ofApp::testButtonPressed() {
    timeline->cue(fadeLights(5.0, lightMin, lightNormal));
    CUE(playClip("test", "1"));
}

void ofApp::testButton2Pressed() {
    timeline->cue(aroundTheWorld());
    CUE(playClip("steve", "airplane scrub short"));
    CUE_IN_SECONDS(1.0, stopClip("steve", "airplane scrub short"));
    CUE_IN_SECONDS(3.0, playClip("steve", "airplane scrub short"));
    timeline->cue(fadeVolume(ofxBenG::ableton()->getTrack("guitar drones"), 1.0, volumeMin, volumeMax));
}

void ofApp::onBlackoutButtonPressed() {
    timeline->cue(blackout());
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
