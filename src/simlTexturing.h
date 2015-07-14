//
//  simlTexturing.h
//  simlTexturing
//
//  Created by Simon on 09/07/2015.
//
//

#ifndef simlTexturing_simlTexturing_h
#define simlTexturing_simlTexturing_h


#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/GeomIO.h"
#include "cinder/ImageIo.h"
#include "cinder/CameraUi.h"
#include "cinder/Rand.h"
#include "cinder/qtime/QuickTimeGl.h"
/*
#include "cinder/gl/GlslProg.h"

#include "Spout.h"
#include <algorithm>
*/



#include "../include/Resources.h"



using namespace ci;
using namespace ci::app;

using std::vector;

class SimlTexturingApp : public App {
    
public:
    void setup();
    void update();
    void draw();
    void resizeScreens();
    void renderSceneToFbo();

	void renderMovie();
	//void renderSpout();
	void loadMovieFile(const fs::path &moviePath);
	void mouseDown(MouseEvent event);
	void renderTestImage();
	//void renderShaderImage();
    
    static const int NUM_SCREENS = 6;
    int mVerticesX, mVerticesY;
	int mLastFrameCount;
    
    gl::FboRef mFbo;
    gl::VboMeshRef	mVboMesh;

	vector<gl::GlslProgRef>	mShaderProg;
	qtime::MovieGlRef		mMovie;


    static const int mcWindowWidth = 128 * 6 * 10;
    static const int mcWindowHeight =  720;
    int	mFboWidth, mFboHeight;
	int mCurrentShader, mRenderMode;
    
	bool mIsTestImage;

    float mProp; // endScreen/sideScreen
	float mTargetTime;
    float mNormalizedProps[NUM_SCREENS];
    float mScreenWidths [NUM_SCREENS];

    
    void keyDown(KeyEvent key);

	void shutdown();


	//SpoutReceiver spoutreceiver;				// Create a Spout receiver object
	//gl::Texture spoutTexture;					// Local Cinder texture used for sharing

	bool bInitialized;							// true if a sender initializes OK
	bool bDoneOnce;								// only try to initialize once
	bool bMemoryMode;							// tells us if texture share compatible
	unsigned int g_Width, g_Height;				// size of the texture being sent out
	char SenderName[256];						// sender name 


    
private:
    
    
    
    
};


#endif
