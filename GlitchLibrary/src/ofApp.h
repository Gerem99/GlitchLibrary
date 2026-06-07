#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxCv.h"
#include "ofxPostGlitch.h"

class ofApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;

    // FBOs
    ofFbo combinedInputFbo;

    // Sources
    ofVideoGrabber vidGrabber;
    ofVideoPlayer vidPlayer;

    // OpenCV for Reactive Mode
    ofxCv::ContourFinder contourFinder;
    
    // PostGlitch
    std::unique_ptr<ofxPostGlitch> postGlitch;

    // GUI
    ofxPanel gui;

    // Mixer & FX GUI
    ofParameterGroup mixerGroup;
    ofParameter<bool> fileToggle;
    ofParameter<float> fileOp;
    ofParameter<bool> camToggle;
    ofParameter<float> camOp;
    ofParameter<float> glitchAmount;
    ofParameter<bool> reactiveMode;
    ofParameter<bool> invertColor;

    // Reactive parameters
    ofParameterGroup reactiveGroup;
    ofParameter<float> motionThreshold;
    ofParameter<bool> showDebug;
    ofParameter<float> lowThreshold;
    ofParameter<float> medThreshold;
    ofParameter<float> highThreshold;


    // Master Chaos
    ofParameterGroup masterChaosGroup;
    ofParameter<float> masterChaos;

    float smoothedAmount = 0.0f;
    bool fileNeedsInit = false;
    bool currentFxStates[6] = {false, false, false, false, false, false};
};
