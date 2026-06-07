#pragma once

#include "ofMain.h"
#include "ofxGui.h"

// ==============================================================
// INPUT SOURCE
// ==============================================================
class InputSource {
public:
    inline void setup() {
        ofFboSettings s;
        s.width = 640; s.height = 480;
        s.internalformat = GL_RGB;
        s.textureTarget = GL_TEXTURE_2D; 
        fbo.allocate(s);
    }
    
    inline void loadVideo() {
        ofFileDialogResult res = ofSystemLoadDialog("Select Video", false);
        if(res.bSuccess) { 
            player.load(res.getPath()); 
            player.play(); 
            player.setLoopState(OF_LOOP_NORMAL);
        }
    }

    inline void update() {
        player.update();
        if(player.isLoaded() && player.isFrameNew()) { 
            fbo.begin(); 
            ofClear(0, 0, 0, 255); 
            player.draw(0,0, 640, 480); 
            fbo.end(); 
        }
    }

    inline ofTexture& getTexture() { return fbo.getTexture(); }
    
    inline void eject() { player.close(); }

private:
    ofVideoPlayer player;
    ofFbo fbo;
};

// ==============================================================
// OFAPP
// ==============================================================
class ofApp : public ofBaseApp{
public:
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override {} // Empty boilerplate inlined
    void keyPressed(int key) override;
    void keyReleased(int key) override {}
    void mouseMoved(int x, int y ) override {}
    void mouseDragged(int x, int y, int button) override {}
    void mousePressed(int x, int y, int button) override {}
    void mouseReleased(int x, int y, int button) override {} // Corrected 'y'
    void mouseScrolled(int x, int y, float scrollX, float scrollY) override {}
    void mouseEntered(int x, int y) override {}
    void mouseExited(int x, int y) override {} // Corrected 'y'
    void windowResized(int w, int h) override {}
    void dragEvent(ofDragInfo dragInfo) override {}
    void gotMessage(ofMessage msg) override {}
    
    InputSource source;
    ofxPanel gui;

    // Analog Glitch
    ofxGuiGroup analogGlitchGroup;
    ofxToggle bEnableAnalogGlitch;
    ofxToggle bEnableHarshAnalog;
    ofxFloatSlider analogWetDry;
    ofxFloatSlider analogDistortionAmount;
    ofxFloatSlider analogDistortionImageMixAmount;
    ofxFloatSlider analogVerticalSync;
    ofxFloatSlider analogHorizontalSync;
    ofxFloatSlider analogScanLineResolution;
    ofShader analogShader;
    // Frame Glitch
    ofxGuiGroup frameGlitchGroup;
    ofxToggle bEnableFrameGlitch;
    ofxFloatSlider glitchProbability;
    ofxIntSlider maxFrameJump;
    ofxFloatSlider jumpJitter; // NEW: Controls the randomness of the jump range
    ofxFloatSlider jumpFrequency; // NEW: Controls how often the jump is allowed
    vector<ofFbo> frameBuffer;
    int currentBufferIndex = 0;
    int displayBufferIndex = 0;
    const int bufferSize = 120;

    // Scrambler
    ofxGuiGroup scramblerGroup;
    ofxToggle bEnableScrambler;
    ofxFloatSlider scrambleAmount;
    ofShader scramblerShader;

    ofFbo combinedInputFbo;
    ofFbo analogFbo;
    ofFbo scramblerFbo;

    struct GlitchPreset {
        bool bEnableAnalogGlitch;
        bool bEnableHarshAnalog;
        float analogWetDry;
        float analogDistortionAmount;
        float analogDistortionImageMixAmount;
        float analogVerticalSync;
        float analogHorizontalSync;
        float analogScanLineResolution;
        bool bEnableScrambler;
        float scrambleAmount;
    };

    void savePreset(int index);
    void loadPreset(int index);
    vector<GlitchPreset> presets;
};
