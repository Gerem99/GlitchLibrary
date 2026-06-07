#include "ofApp.h"

void ofApp::setup(){
    ofSetWindowShape(640, 480);
    
    ofFboSettings s;
    s.width = 640; s.height = 480;
    s.internalformat = GL_RGB;
    
    combinedInputFbo.allocate(s);
    analogFbo.allocate(s);
    scramblerFbo.allocate(s);
    
    ofDirectory dataDir(ofToDataPath(""));
    if(!dataDir.exists()) {
        dataDir.create(true);
    }

    ofDirectory shaderDir(ofToDataPath("Shaders"));
    if(!shaderDir.exists()) {
        ofLogNotice() << "Shaders missing, restoring from ofxPostGlitch addon...";
        string addonStr = ofFilePath::join(ofToDataPath(""), "../../../../../addons/ofxPostGlitch/example/bin/data/Shaders");
        ofDirectory addonDir(addonStr);
        if(addonDir.exists()) {
            addonDir.copyTo(ofToDataPath("Shaders"));
        }
    }
    
    postGlitch = std::make_unique<ofxPostGlitch>();
    postGlitch->setup(&combinedInputFbo);
    for (int i = 0; i < 17; i++) {
        postGlitch->setFx((ofxPostGlitchType)i, false);
    }
    
    vidGrabber.setup(640, 480);
    
    gui.setup("Glitch Library");
    
    mixerGroup.setName("Mixer & FX");
    fileToggle.set("File Layer", false);
    fileOp.set("File Opacity", 255.0f, 0.0f, 255.0f);
    camToggle.set("Cam Layer", true);
    camOp.set("Cam Opacity", 255.0f, 0.0f, 255.0f);
    glitchAmount.set("Glitch Amount", 0.0f, 0.0f, 1.0f);
    reactiveMode.set("Reactive Mode", false);
    motionThreshold.set("Motion Threshold", 0.01f, 0.0f, 0.1f);
    showDebug.set("Show Debug", false);
    invertColor.set("Invert Color", false);
    
    mixerGroup.add(fileToggle);
    mixerGroup.add(fileOp);
    mixerGroup.add(camToggle);
    mixerGroup.add(camOp);
    mixerGroup.add(glitchAmount);
    mixerGroup.add(reactiveMode);
    mixerGroup.add(motionThreshold);
    mixerGroup.add(showDebug);
    mixerGroup.add(invertColor);
    gui.add(mixerGroup);
    
    analogGlitchGroup.setName("Analog Glitch");
    bEnableAnalogGlitch.set("Enable Analog", false);
    bEnableHarshAnalog.set("Harsh Mode", false);
    analogWetDry.set("Wet/Dry", 0.0f, 0.0f, 1.0f);
    analogDistortionAmount.set("Distortion", 0.0f, 0.0f, 1.0f);
    analogGlitchGroup.add(bEnableAnalogGlitch);
    analogGlitchGroup.add(bEnableHarshAnalog);
    analogGlitchGroup.add(analogWetDry);
    analogGlitchGroup.add(analogDistortionAmount);
    gui.add(analogGlitchGroup);

    scramblerGroup.setName("Scrambler");
    bEnableScrambler.set("Enable Scrambler", false);
    scrambleAmount.set("Scramble Amount", 0.0f, 0.0f, 1.0f);
    scramblerGroup.add(bEnableScrambler);
    scramblerGroup.add(scrambleAmount);
    gui.add(scramblerGroup);

    frameGlitchGroup.setName("Frame Glitch");
    bEnableFrameGlitch.set("Enable Time Jump", false);
    glitchProbability.set("Jump Probability", 0.05f, 0.0f, 1.0f);
    maxFrameJump.set("Max Frame History", 60, 1, bufferSize - 1);
    jumpJitter.set("Jump Jitter", 0.0f, 0.0f, 1.0f);
    jumpFrequency.set("Jump Frequency", 1.0f, 0.1f, 5.0f);
    frameGlitchGroup.add(bEnableFrameGlitch);
    frameGlitchGroup.add(glitchProbability);
    frameGlitchGroup.add(maxFrameJump);
    frameGlitchGroup.add(jumpJitter);
    frameGlitchGroup.add(jumpFrequency);
    gui.add(frameGlitchGroup);

    for(int i = 0; i < bufferSize; i++) {
        ofFbo fbo; fbo.allocate(640, 480, GL_RGB);
        frameBuffer.push_back(fbo);
    }

    string vert = "#version 120\n varying vec2 texCoordVarying; void main() { texCoordVarying = gl_MultiTexCoord0.xy; gl_Position = ftransform(); }\n";
    string analogFrag = "#version 120\n uniform sampler2DRect tex; uniform float time; uniform float distortion; varying vec2 texCoordVarying; void main() { vec2 uv = texCoordVarying; uv.x += sin(uv.y * 0.02 + time * 2.0) * distortion * 30.0; gl_FragColor = texture2DRect(tex, uv); }\n";
    analogShader.setupShaderFromSource(GL_VERTEX_SHADER, vert);
    analogShader.setupShaderFromSource(GL_FRAGMENT_SHADER, analogFrag);
    analogShader.linkProgram();
    
    string scramblerFrag = "#version 120\n uniform sampler2DRect tex; uniform float amount; varying vec2 texCoordVarying; void main() { vec2 uv = texCoordVarying; if(fract(uv.y * 0.02 + amount * 10.0) < amount) uv.x += amount * 300.0; gl_FragColor = texture2DRect(tex, uv); }\n";
    scramblerShader.setupShaderFromSource(GL_VERTEX_SHADER, vert);
    scramblerShader.setupShaderFromSource(GL_FRAGMENT_SHADER, scramblerFrag);
    scramblerShader.linkProgram();
}

void ofApp::update(){
    if(camToggle) vidGrabber.update();
    if(fileToggle && vidPlayer.isLoaded()) {
        if (fileNeedsInit) {
            vidPlayer.setLoopState(OF_LOOP_NORMAL);
            vidPlayer.play();
            fileNeedsInit = false;
        }
        vidPlayer.update();
    }

    // Mix sources into combinedInputFbo
    combinedInputFbo.begin();
    ofClear(0, 0, 0, 255);
    ofEnableAlphaBlending();
    
    if (fileToggle && vidPlayer.isLoaded()) {
        ofSetColor(255, 255, 255, fileOp);
        vidPlayer.draw(0, 0, 640, 480);
    }
    if (camToggle && vidGrabber.isInitialized()) {
        ofSetColor(255, 255, 255, camOp);
        vidGrabber.draw(0, 0, 640, 480);
    }
    ofDisableAlphaBlending();
    combinedInputFbo.end();

    if (reactiveMode) {
        ofPixels fboPixels;
        combinedInputFbo.readToPixels(fboPixels);
        contourFinder.findContours(fboPixels);
        
        float totalArea = 0;
        for(int i = 0; i < contourFinder.size(); i++) {
            totalArea += contourFinder.getContourArea(i);
        }
        float normalizedMotion = ofMap(totalArea, 0, 640*480 * motionThreshold.get(), 0.0, 1.0, true);
        smoothedAmount = ofLerp(smoothedAmount, normalizedMotion, 0.05f);
        glitchAmount.set(smoothedAmount);
    }

    // Apply ofxPostGlitch from SimpleGlitchPrototype based on glitchAmount
    float amount = glitchAmount.get();
    bool newStates[6] = { amount > 0.15, amount > 0.40, amount > 0.65, false, false, false };
    ofxPostGlitchType fxTypes[6] = { OFXPOSTGLITCH_NOISE, OFXPOSTGLITCH_CONVERGENCE, OFXPOSTGLITCH_SHAKER, OFXPOSTGLITCH_TWIST, OFXPOSTGLITCH_CUTSLIDER, OFXPOSTGLITCH_SLITSCAN };

    for(int i=0; i<6; i++) {
        if(currentFxStates[i] != newStates[i]) {
            postGlitch->setFx(fxTypes[i], newStates[i]);
            currentFxStates[i] = newStates[i];
        }
    }
    postGlitch->setFx(OFXPOSTGLITCH_INVERT, invertColor.get());
    
    // Setup required values for ofxPostGlitch shaders
    postGlitch->setVal(0, 0.0f);
    postGlitch->setVal(1, ofMap(glitchAmount.get(), 0, 1, 10.0, 100.0, true));
    // Generate ofxPostGlitch into combinedInputFbo
    postGlitch->generateFx();

    // 1. Scrambler -> Analog -> Buffer
    scramblerFbo.begin();
    if(bEnableScrambler) {
        scramblerShader.begin();
        scramblerShader.setUniformTexture("tex", combinedInputFbo.getTexture(), 0);
        scramblerShader.setUniform1f("amount", scrambleAmount);
        combinedInputFbo.draw(0, 0);
        scramblerShader.end();
    } else {
        combinedInputFbo.draw(0,0);
    }
    scramblerFbo.end();

    analogFbo.begin();
    if(bEnableAnalogGlitch) {
        analogShader.begin();
        analogShader.setUniformTexture("tex", scramblerFbo.getTexture(), 0);
        analogShader.setUniform1f("time", ofGetElapsedTimef());
        float d = bEnableHarshAnalog ? (ofRandomuf() > 0.9f ? ofRandom(0.1f, 0.5f) : 0.0f) : analogDistortionAmount.get();
        analogShader.setUniform1f("distortion", d);
        scramblerFbo.draw(0, 0);
        analogShader.end();
    } else {
        scramblerFbo.draw(0, 0);
    }
    analogFbo.end();

    frameBuffer[currentBufferIndex].begin();
    analogFbo.draw(0, 0);
    frameBuffer[currentBufferIndex].end();

    // Update Indices for Time Jump
    if (bEnableFrameGlitch) {
        if (ofGetFrameNum() % (int)std::max(1.0f, 10.0f / jumpFrequency.get()) == 0 && ofRandom(1.0) < glitchProbability) {
            float jitterEffect = ofSignedNoise(ofGetElapsedTimef() * 10.0) * jumpJitter * maxFrameJump;
            int jump = (int)ofClamp(maxFrameJump + jitterEffect, 1, bufferSize - 1);
            displayBufferIndex = (currentBufferIndex - jump + bufferSize) % bufferSize;
        } else {
            displayBufferIndex = currentBufferIndex;
        }
    } else {
        displayBufferIndex = currentBufferIndex;
    }
    currentBufferIndex = (currentBufferIndex + 1) % bufferSize;
}

void ofApp::draw(){
    ofBackground(40);
    ofSetColor(255);
    
    frameBuffer[displayBufferIndex].draw(0,0, ofGetWidth(), ofGetHeight());
    gui.draw();
    
    if(showDebug) contourFinder.draw();
}

void ofApp::keyPressed(int key){
    if(key == 'l' || key == 'L') {
        ofFileDialogResult result = ofSystemLoadDialog("Select video file");
        if (result.bSuccess) {
            if (vidPlayer.isLoaded()) {
                vidPlayer.stop();
                vidPlayer.close();
            }
            if(vidPlayer.load(result.getPath())) {
                fileToggle = true;
                fileNeedsInit = true;
            }
        }
    }
    if(key == 't' || key == 'T') {
        if (vidPlayer.isLoaded()) {
            vidPlayer.stop();
            vidPlayer.close();
        }
        if(vidPlayer.load("test.mp4")) {
            fileToggle = true;
            fileNeedsInit = true;
        }
    }
}
