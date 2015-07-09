//
//  simlTexturing.h
//  simlTexturing
//
//  Created by Simon on 09/07/2015.
//
//

#ifndef simlTexturing_simlTexturing_h
#define simlTexturing_simlTexturing_h

#include "Resources.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "cinder/ImageIo.h"
#include <algorithm>

using namespace ci;
using namespace ci::app;

using std::vector;

class SimlTexturingApp : public AppBasic {
    
public:
    void setup();
    void update();
    void draw();
    void resizeScreens();
    void renderSceneToFbo();
    
    static const int NUM_SCREENS = 6;
    int mVerticesX, mVerticesY;
    
    gl::Fbo mFbo;
    gl::VboMeshRef	mVboMesh;
    gl::TextureRef	mTexture;
    
    static const int mcWindowWidth = 128 * 6 * 1.5;
    static const int mcWindowHeight =  72 * 1.5;
    int	mFboWidth, mFboHeight;
    
    float mProp; // endScreen/sideScreen
    float mNormalizedProps[NUM_SCREENS];
    float mScreenWidths [NUM_SCREENS];
    void prepareSettings(Settings * settings);
    
    void keyDown(KeyEvent key);
    
private:
    
    
    
    
};


#endif
