#include "ofApp.h"

void ofApp::setup(){
    ofSetWindowShape(640, 480);
    ofFboSettings s;
    s.width = 640; s.height = 480;
    s.internalformat = GL_RGB;
    combinedInputFbo.allocate(s);
    analogFbo.allocate(s);
    scramblerFbo.allocate(s);
    
    gui.setup("Glitch Library");
    source.setup();
    
    analogGlitchGroup.setup("Analog Glitch");
    bEnableAnalogGlitch.setup("Enable Analog", false);
    bEnableHarshAnalog.setup("Harsh Mode", false);
    analogWetDry.setup("Wet/Dry", 0.0f, 0.0f, 1.0f);
    analogDistortionAmount.setup("Distortion", 0.0f, 0.0f, 1.0f);
    analogGlitchGroup.add(&bEnableAnalogGlitch);
    analogGlitchGroup.add(&bEnableHarshAnalog);
    analogGlitchGroup.add(&analogWetDry);
    analogGlitchGroup.add(&analogDistortionAmount);
    gui.add(&analogGlitchGroup);

    scramblerGroup.setup("Scrambler");
    bEnableScrambler.setup("Enable Scrambler", false);
    scrambleAmount.setup("Scramble Amount", 0.0f, 0.0f, 1.0f);
    scramblerGroup.add(&bEnableScrambler);
    scramblerGroup.add(&scrambleAmount);
    gui.add(&scramblerGroup);
// Setup Frame Glitch GUI
frameGlitchGroup.setup("Frame Glitch");
bEnableFrameGlitch.setup("Enable Time Jump", false);
glitchProbability.setup("Jump Probability", 0.05f, 0.0f, 1.0f);
maxFrameJump.setup("Max Frame History", 60, 1, bufferSize - 1);
jumpJitter.setup("Jump Jitter", 0.0f, 0.0f, 1.0f);
jumpFrequency.setup("Jump Frequency", 1.0f, 0.1f, 5.0f);
frameGlitchGroup.add(&bEnableFrameGlitch);
frameGlitchGroup.add(&glitchProbability);
frameGlitchGroup.add(&maxFrameJump);
frameGlitchGroup.add(&jumpJitter);
frameGlitchGroup.add(&jumpFrequency);
gui.add(&frameGlitchGroup);

    for(int i = 0; i < bufferSize; i++) {
        ofFbo fbo; fbo.allocate(640, 480, GL_RGB);
        frameBuffer.push_back(fbo);
    }

    string vert = "#version 120\n varying vec2 texCoordVarying; void main() { texCoordVarying = gl_MultiTexCoord0.xy; gl_Position = ftransform(); }\n";
    string analogFrag = "#version 120\n uniform sampler2D tex; uniform float time; uniform float distortion; varying vec2 texCoordVarying; void main() { vec2 uv = texCoordVarying; uv.x += sin(uv.y * 10.0 + time * 2.0) * distortion * 0.05; gl_FragColor = texture2D(tex, uv); }\n";
    analogShader.setupShaderFromSource(GL_VERTEX_SHADER, vert);
    analogShader.setupShaderFromSource(GL_FRAGMENT_SHADER, analogFrag);
    analogShader.linkProgram();
    
    string scramblerFrag = "#version 120\n uniform sampler2D tex; uniform float amount; varying vec2 texCoordVarying; void main() { vec2 uv = texCoordVarying; if(fract(uv.y * 10.0 + amount * 10.0) < amount) uv.x += amount * 0.5; gl_FragColor = texture2D(tex, uv); }\n";
    scramblerShader.setupShaderFromSource(GL_VERTEX_SHADER, vert);
    scramblerShader.setupShaderFromSource(GL_FRAGMENT_SHADER, scramblerFrag);
    scramblerShader.linkProgram();
}

void ofApp::update(){
    source.update();
    combinedInputFbo.begin();
    ofClear(0);
    source.getTexture().draw(0,0, 640, 480);
    combinedInputFbo.end();
    
    // 1. Scrambler -> Analog -> Buffer
    scramblerFbo.begin();
    if(bEnableScrambler) {
        scramblerShader.begin();
        scramblerShader.setUniformTexture("tex", combinedInputFbo.getTexture(), 0);
        scramblerShader.setUniform1f("amount", scrambleAmount);
        ofDrawRectangle(0,0,640,480);
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
        float d = bEnableHarshAnalog ? (ofRandomuf() > 0.9 ? ofRandom(0.1, 0.5) : 0.0) : analogDistortionAmount;
        analogShader.setUniform1f("distortion", d);
        ofDrawRectangle(0, 0, 640, 480);
        analogShader.end();
    } else {
        scramblerFbo.draw(0, 0);
    }
    analogFbo.end();

    frameBuffer[currentBufferIndex].begin();
    analogFbo.draw(0, 0);
    frameBuffer[currentBufferIndex].end();

    // Update Indices
    if (bEnableFrameGlitch) {
        // Use frequency to determine how often we check for jumps
        if (ofGetFrameNum() % (int)(10.0 / jumpFrequency) == 0 && ofRandom(1.0) < glitchProbability) {
            // Apply jitter to the jump distance
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
}

void ofApp::keyPressed(int key){
    if(key == 'l' || key == 'L') source.loadVideo();
}
