//
//  simlTexturing.h
//  simlTexturing
//
//  Created by Simon on 09/07/2015.
//
//

#ifndef simlTexturing_simlTexturing_h
#define simlTexturing_simlTexturing_h

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/GlslProg.h"
#include <algorithm>

#include "../include/Resources.h"


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

	void renderTestImage();
	void renderShaderImage();
    
    static const int NUM_SCREENS = 6;
    int mVerticesX, mVerticesY;
    
    gl::Fbo mFbo;
    gl::VboMeshRef	mVboMesh;

	vector<gl::GlslProg>	mShaderProg;


    static const int mcWindowWidth = 128 * 6 * 10;
    static const int mcWindowHeight =  720;
    int	mFboWidth, mFboHeight;
	int mCurrentShader;
    
	bool mIsTestImage;

    float mProp; // endScreen/sideScreen
	float mTargetTime;
    float mNormalizedProps[NUM_SCREENS];
    float mScreenWidths [NUM_SCREENS];
    void prepareSettings(Settings * settings);
    
    void keyDown(KeyEvent key);
    
private:
    
    
    
    
};


#endif
