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
    int const desiredWidth = 1280;
    int const desiredHeight = 720;
    int const desiredFps = 60;
    int const defaultBufferSize = 1;
    double const beatsPerMinute = 60;
    lightSquare = new ofxBenG::single_color_view(ofColor::black);
    cameraShade = new ofxBenG::single_color_view(transparentWhite);
    streamManager = new ofxBenG::stream_manager(desiredWidth, desiredHeight, desiredFps, defaultBufferSize);
    streamManager->excludeDevice("C922 Pro Stream Webcam");
//    streamManager->excludeDevice("Blackmagic");
    streamManager->excludeDevice("FaceTime HD Camera");
    ofAddListener(streamManager->onVideoStreamAdded, this, &ofApp::onVideoStreamAdded);
    ofAddListener(streamManager->onVideoStreamRemoved, this, &ofApp::onVideoStreamRemoved);
    monitorManager = new ofxBenG::monitor_manager();
    monitorManager->excludeMonitor("Color LCD");
    monitorManager->excludeMonitor("BenQ GW2765");
    ofAddListener(monitorManager->onMonitorAdded, this, &ofApp::onMonitorAdded);
    ofAddListener(monitorManager->onMonitorRemoved, this, &ofApp::onMonitorRemoved);
    windowManager = new ofxBenG::window_manager();
    windowManager->setFullscreen(true);
    ofxBenG::ableton()->setup(beatsPerMinute, 8.0);
    timeline = new ofxBenG::timeline(4);
    ofxBenG::ableton()->stopAll();
    videoLengths.push_back(8);
    videoLengths.push_back(4);
    videoLengths.push_back(2);
    videoLengths.push_back(2);
    videoLengths.push_back(1);
    videoLengths.push_back(1);
    videoLengths.push_back(2);
    videoLengths.push_back(2);
    videoLengths.push_back(4);
    lightBoard.setup("mio");
    gui.setup();
    gui.add(faderNumber.setup("faderNumber", 39, 1, 127));
    gui.add(faderLevel.setup("faderLevel", 0, 0, 100));
    gui.add(lfoFrequency.setup("LFO frequency", 0.1, 0.1, 7));
    gui.add(armFader.setup("Arm Fader", 150, 25));
    gui.add(testButton.setup("Test", 150, 25));
    gui.add(goButton.setup("Go", 150, 150));
    armFader.addListener(this, &ofApp::armFaderPressed);
    goButton.addListener(this, &ofApp::startPerformance);
    testButton.addListener(this, &ofApp::testButtonPressed);
    renderer = new ofxPm::BasicVideoRenderer;
}

void ofApp::armFaderPressed() {
    lightBoard.setSubmaster(faderNumber, faderLevel);
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

    float topPadding = 25;

    gui.draw();

    float x = gui.getPosition()[0];
    float y = gui.getHeight() + gui.getPosition()[1] + topPadding;

    auto stream = streamManager->getStream(0);
    if (stream != nullptr) {
        ofDrawBitmapString(ofToString(stream->getFps()), ofPoint(x, y));
        y += topPadding;
    }
    ofDrawBitmapString(ofToString(ofGetFrameRate()), ofPoint(x, y));
    y += topPadding;

//    bool isAbletonGreen = true;
//    isAbletonGreen &= ofxBenG::ableton()->isClipPresent("bros", "drier egg timer");
//    isAbletonGreen &= ofxBenG::ableton()->isClipPresent("wistful elegiac drone", "volca loop 6");
//    isAbletonGreen &= ofxBenG::ableton()->isClipPresent("hammer", "hammer on metal 2");
//    isAbletonGreen &= ofxBenG::ableton()->isClipPresent("flicker", "gas click 2");
//    isAbletonGreen &= ofxBenG::ableton()->isClipPresent("flicker", "gas click");
//    ofDrawBitmapString(isAbletonGreen ? "All clips present" : "Some clips missing", ofPoint(leftPadding, topPadding + 50));
//    std::string numPeers = "Number of Link peers: " + ofToString(ofxBenG::ableton()->getNumPeers());
//    ofDrawBitmapString(numPeers, ofPoint(leftPadding, topPadding + 75));

    float const previewWidth = 160;
    float const previewHeight = 90;

    for (auto monitor : monitorManager->getMonitors()) {
        ofDrawBitmapString(monitor->getName(), ofPoint(x, y));
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

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
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

ofxBenG::generic_action *ofApp::onMeasure() {
     return new ofxBenG::generic_action([this]() {
         scheduleNextMeasure();
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

ofxBenG::lerp_action *ofApp::fadeLight(int faderNumber, float durationBeats, float startLevel, float endLevel) {
    return new ofxBenG::lerp_action(durationBeats, [this, faderNumber, startLevel, endLevel](float lerpValue, float min, float max) {
        float const faderLevel = ofMap(lerpValue, min, max, startLevel, endLevel);
        this->lightBoard.setSubmaster(faderNumber, faderLevel);
    });
}

ofxBenG::lerp_action *ofApp::fadeVolume(ofxAbletonLiveTrack *track, float durationBeats, float startLevel, float endLevel) {
    return new ofxBenG::lerp_action(durationBeats, [this, track, startLevel, endLevel](float lerpValue, float min, float max) {
//        std::cout << ofxBenG::ableton()->getBeat() << " " << lerpValue << " " << ofMap(lerpValue, min, max, startLevel, endLevel) << std::endl;
        track->setVolume(ofMap(lerpValue, min, max, startLevel, endLevel));
    });
}

ofxBenG::lerp_action *ofApp::fadeParameter(ofxAbletonLiveParameter *parameter, float durationBeats, float startLevel, float endLevel) {
    return new ofxBenG::lerp_action(2.0, [this, parameter, startLevel, endLevel](float value, float min, float max) {
        float newFrequency = ofMap(value, min, max, startLevel, endLevel);
        parameter->setValue(newFrequency);
    });
}

void ofApp::onLfo1Value(float &value) {
//    std::cout << "lfo1( " << value << ")" << std::endl;
//    float const trackVolume = lfo1->map(value, 0.30, 0.82);
//    auto track = ofxBenG::ableton()->getTrack("wind");
//    std::cout << "setVolume( " << trackVolume << ")" << std::endl;
//    track->setVolume(trackVolume);

//    float const level = lfo1->map(value, 10.0, 50.0);
//    std::cout << "setSubmaster(37, " << level << ")" << std::endl;
//    lightBoard.setSubmaster(faderNumber, (int)level);
}

void ofApp::onLfo2Value(float &value) {
//    float frequency = lfo2->map(value, 0.2, 7.0);
//    std::cout << value << std::endl;
//    lfo1->setFrequency(frequency);
}

void ofApp::startPerformance() {
    isRunning = true;

//        windowManager->getWindowForMonitor("Optoma WXGA")->addView(lightSquare);
//        windowManager->getWindowForMonitor("Color LCD")->addView(lightSquare);

//    windowManager->getWindowForMonitor("Optoma 1080P")->addView(cameraShade);
//        windowManager->getWindowForMonitor("BenQ GW2765")->addView(cameraShade);

    timeline->scheduleNextWholeBeat(new ofxBenG::generic_action([this]() {
        ofxBenG::ableton()->setClockToZero();
        ofxBenG::ableton()->play();

        timeline->schedule(0.0, setSubmaster(houseFaderNumber, lightMin));
        timeline->schedule(0.25, setSubmaster(frontFaderNumber, lightMin));
        timeline->schedule(0.5, setSubmaster(sideFaderNumber, lightMin));
        timeline->schedule(0.75, setSubmaster(backFaderNumber, lightMin));

        auto stream = streamManager->getStream(0);
        stream->getWindow()->addView(new ofxBenG::generic_view([this](ofPoint windowSize) {
            if (holdFrame != nullptr) {
                holdFrame->draw(0, 0, windowSize[0], windowSize[1]);
            } else if (renderer->isSetup()) {
                renderer->draw(0, 0, windowSize[0], windowSize[1]);
            }
        }));

        ofxBenG::ableton()->getTrack("wind")->setVolume(volumeOff);
        ofxBenG::ableton()->getTrack("arp")->setVolume(volumeOff);
        ofxBenG::ableton()->getTrack("bass")->setVolume(volumeOff);
        ofxBenG::ableton()->getTrack("drone")->setVolume(volumeOff);
        ofxBenG::ableton()->getTrack("birds")->setVolume(volumeOff);
        ofxBenG::ableton()->getTrack("arp")->getDevice("Auto Filter")->getParameter("Frequency")->setValue(filterMin);


        int const beatsInFiveMinutes = ofxBenG::ableton()->getTempo() * 5;
        auto halfWay = beatsInFiveMinutes / 2;
        auto oneThirdWay = beatsInFiveMinutes / 3;
        auto quarterWay = beatsInFiveMinutes / 4;
        auto threeFourthsWay = quarterWay * 3;
        timeline->executeAction(fadeVolume(ofxBenG::ableton()->getTrack("wind"), fadeInBeats, volumeOff, volumeMin));
        timeline->executeAction(fadeVolume(ofxBenG::ableton()->getTrack("arp"), threeFourthsWay, volumeOff, volumeMax));
        timeline->executeAction(fadeVolume(ofxBenG::ableton()->getTrack("bass"), fadeInBeats, volumeOff, volumeMin));
        timeline->executeAction(fadeVolume(ofxBenG::ableton()->getTrack("drone"), fadeInBeats, volumeOff, volumeMin));
        timeline->executeAction(fadeVolume(ofxBenG::ableton()->getTrack("birds"), fadeInBeats, volumeOff, volumeMin));
//        timeline->schedule(halfWay, playClip("birds", "American Coots"));
        timeline->scheduleOnNthBeatFromNow(beatsInMeasure, playClip("wind", "Wind on Peale Island"));
        timeline->schedule(quarterWay, playClip("bass", "Black Sand Pool"));
        timeline->schedule(0.0, playClip("arp", "laverne"));
//        timeline->schedule(halfWay, playClip("drone", "drone 1"));

        timeline->scheduleOnNthBeatFromNow(beatsInMeasure, scheduleFlicker());
        timeline->scheduleOnNthBeatFromNow(beatsInMeasure, onMeasure());

        timeline->scheduleOnNthBeatFromNow(beatsInFiveMinutes - beatsInMeasure, stopClip("arp", "laverne"));
        timeline->scheduleOnNthBeatFromNow(beatsInFiveMinutes - beatsInMeasure, stopClip("birds", "American Coots"));
        timeline->scheduleOnNthBeatFromNow(beatsInFiveMinutes - 1, stopClip("wind", "Wind on Peale Island"));
        timeline->scheduleOnNthBeatFromNow(beatsInFiveMinutes - 1, new ofxBenG::generic_action([this]() {
            stopAll = true;
            ofxBenG::ableton()->stopAll();
            timeline->clearScheduledActions();
            timeline->schedule(0.0, setSubmaster(frontFaderNumber, lightMin));
            timeline->schedule(0.5, setSubmaster(sideFaderNumber, lightMin));
            timeline->schedule(1.0, setSubmaster(backFaderNumber, lightMin));
            timeline->schedule(4.0, setSubmaster(houseFaderNumber, lightMax));
            for (auto window : windowManager->getWindows()) {
                window->addView(new blackout_view());
            }
        }));
    }));
}

ofxBenG::generic_action *ofApp::scheduleFlicker() {
    return new ofxBenG::generic_action([this]() {
        if (!stopAll) {
            ofxBenG::ableton()->log("Fade lights in");
            timeline->executeAction(fadeLight(sideFaderNumber, 1.0, lightMin, sideMax));
            timeline->schedule(1.0, fadeLight(backFaderNumber, 1.0, lightMin, backMax));
            timeline->schedule(2.0, fadeLight(frontFaderNumber, 1.0, lightMin, frontMax));
            timeline->executeAction(fadeVolume(ofxBenG::ableton()->getTrack("wind"), fadeInBeats, volumeMin, volumeMax));
            timeline->executeAction(fadeVolume(ofxBenG::ableton()->getTrack("drone"), fadeInBeats, volumeMax, volumeMin));
            timeline->executeAction(fadeParameter(ofxBenG::ableton()->getTrack("arp")->getDevice("Auto Filter")->getParameter("Frequency"), fadeInBeats, filterMin, filterMax));

            float acc = fadeInBeats;
            timeline->scheduleOnNthBeatFromNow(acc, [this]() {
                if (buffer != nullptr) {
                    ofxBenG::ableton()->log("Play last buffer forwards");
                    timeline->executeAction(new ofxBenG::pan_video(header, recordingBeats, recordingBeats, ofxBenG::ableton()->getTempo(), recordingFps, ofxBenG::pan_video::PLAY_FORWARDS));
                }

                ofxBenG::ableton()->log("Record video");
                auto stream = streamManager->getStream(0);
                recordingFps = stream->getFps();
                int const size = ofxBenG::utilities::beatsToSeconds(recordingBeats, ofxBenG::ableton()->getTempo()) * recordingFps;
                buffer = stream->makeBuffer(size);
                buffer->resume();

                ofxBenG::ableton()->log("Play melody");
                timeline->executeAction(playClip("recording", "printer"));
            });
            acc += recordingBeats;

            timeline->scheduleOnNthBeatFromNow(acc, [this]() {
                ofxBenG::ableton()->log("Fade lights out");
                if (renderer->isSetup()) {
                    holdFrame = renderer->getLastTexture();
                }
                buffer->stop();
                timeline->executeAction(playClip("recording", "printer reversed"));
                timeline->executeAction(fadeLight(sideFaderNumber, 1.0, sideMax, lightMin));
                timeline->schedule(1.0, fadeLight(backFaderNumber, 0.5, backMax, lightMin));
                timeline->schedule(1.5, fadeLight(frontFaderNumber, 0.5, frontMax, lightMin));
                timeline->executeAction(fadeVolume(ofxBenG::ableton()->getTrack("wind"), fadeOutBeats, volumeMax, volumeMin));
                timeline->executeAction(fadeVolume(ofxBenG::ableton()->getTrack("drone"), fadeOutBeats, volumeMin, volumeMax));
                timeline->executeAction(fadeParameter(ofxBenG::ableton()->getTrack("arp")->getDevice("Auto Filter")->getParameter("Frequency"), fadeOutBeats, filterMax, filterMin));
            });
            acc += fadeOutBeats;

            timeline->scheduleOnNthBeatFromNow(acc, [this]() {
                ofxBenG::ableton()->log("Play recording backwards");
                header = new ofxPm::VideoHeader;
                header->setup(*buffer);
                renderer->setup(*header);
                timeline->executeAction(new ofxBenG::pan_video(header, recordingBeats, recordingBeats, ofxBenG::ableton()->getTempo(), recordingFps, ofxBenG::pan_video::PLAY_BACKWARDS));
                timeline->schedule(recordingBeats - 5, fadeLight(backFaderNumber, 1.0, lightMin, 30));
                timeline->schedule(recordingBeats - 1, fadeLight(backFaderNumber, 1.0, 30, lightMin));
                if (holdFrame != nullptr) {
                    holdFrame = nullptr;
                }
            });
            acc += recordingBeats;

            timeline->scheduleOnNthBeatFromNow(acc, scheduleFlicker());
        }
    });
}

void ofApp::scheduleNextMeasure() {
    int const measure = ((int) floor(ofxBenG::ableton()->getBeat())) / 4;

    timeline->schedule(beatsInMeasure, new ofxBenG::generic_action([this]() {
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

void ofApp::testButtonPressed() {
    timeline->executeAction(new ofxBenG::lerp_action(2.0, [this](float value, float min, float max) {
        auto freqMin = 30;
        auto freqMax = 80;
        float newFrequency = ofMap(value, min, max, freqMin, freqMax);
        ofxBenG::ableton()->log(ofToString(newFrequency));
        ofxBenG::ableton()->getTrack("arp")->getDevice("Auto Filter")->getParameter("Frequency")->setValue(newFrequency);
    }));
}

