
#include "simlTexturing.h"



void SimlTexturingApp::setup()
{

	setWindowSize(mcWindowWidth, mcWindowHeight);
	setFrameRate(60.0f);
	//getWindow()->setBorderless();

	setWindowPos(0, 0);

	g_Width = mcWindowHeight; // set global width and height to something
	g_Height = mcWindowHeight;
	mLastFrameCount = 0;
	mProp = 1.28;
	mTargetTime = 1.0;
	mCurrentShader = 0;

    
    mVerticesX = NUM_SCREENS * 2;
    mVerticesY = 2;

	mRenderMode = 0;


	auto plane = geom::Plane().subdivisions(ivec2(mVerticesX - 1, mVerticesY - 1));

	vector<gl::VboMesh::Layout> bufferLayout = {
		gl::VboMesh::Layout().usage(GL_DYNAMIC_DRAW).attrib(geom::Attrib::POSITION, 3),
		gl::VboMesh::Layout().usage(GL_DYNAMIC_DRAW).attrib(geom::Attrib::TEX_COORD_0, 2)
	};


	mVboMesh = gl::VboMesh::create(plane, bufferLayout);

	

	resizeScreens();
	renderSceneToFbo();

	mShaderProg.resize(3);

	//exception here might be becuase the frag shaders are for opengl 2.1
	mShaderProg[0] = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("passThruVert.glsl"))
		.fragment(loadAsset("seaFrag.glsl")));
	mShaderProg[1] = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("passThruVert.glsl"))
		.fragment(loadAsset("mountainFrag.glsl")));
	mShaderProg[2] = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("passThruVert.glsl"))
		.fragment(loadAsset("frag.glsl")));
	
	bInitialized = false;


}

void SimlTexturingApp::resizeScreens(){

 
    
    //if(mVboMesh)mVboMesh->reset();
    
    // setup the parameters of the Vbo
    int totalVertices = mVerticesX * mVerticesY;
    gl::VboMesh::Layout layout;
  
    vector<vec2> texCoords;
    
    vector<float> winXPoints(NUM_SCREENS * 2);
    vector<float> texXPoints(NUM_SCREENS * 2);
    vector<float> winYPoints(NUM_SCREENS * 4);
    vector<float> texYPoints(NUM_SCREENS * 4);
    
    //screen widths in meters
    
    
    float screenProps [NUM_SCREENS] = {mProp, 1.0, 1.0, mProp, 1.0, 1.0};
    float screenHeights [NUM_SCREENS];
    
    for(int i = 0; i < NUM_SCREENS; i ++)screenHeights[i] =  1.0/screenProps[i];
    
    
    
    //this method means that you lose some pixels from the bottom of the screen
    
    //normalise the sum of the array
    float tot = 0;
    for(int i = 0; i < NUM_SCREENS; i++)tot += screenProps[i];
    for(int i = 0; i < NUM_SCREENS; i++)mNormalizedProps [i] = screenProps[i] * (NUM_SCREENS/tot);
    
    float incr = 1.0/(float)NUM_SCREENS;
    
    float runningX = 0;
    
    for(int i = 0; i < texXPoints.size(); i++){
        
        //texX points are scaled according to physical screen width ... if the screen is wider
        texXPoints[i] = runningX;
        if(i%2 == 0)runningX += incr * mNormalizedProps[i/2];
        
        
        //winX points are all evenly spaced ... each projector is the same width
        if(i%2 == 0){
            winXPoints[i] = (i/2) * incr * mcWindowWidth;
        }else{
            winXPoints[i] = ((i+1)/2) * incr * mcWindowWidth;
        }
    }
    
    
    for(int i = 0; i < NUM_SCREENS; i++)
    {
        for(int j = 0; j < 4; j++){
            winYPoints[i * 4 + j] = (j%2 == 0)? 0.0 : screenHeights[i] * mcWindowHeight; // we shorten the screen heights for the larger screens
            texYPoints[i * 4 + j] = (j%2 == 0)? 0.0 : 1.0;
        }
        
        
    }
    
    vector<vec3> positions;
    positions.resize(totalVertices);
    
    for( int x = 0; x < mVerticesX; ++x ) {
        for( int y = 0; y < mVerticesY; ++y ) {
       
            // the texture coordinates are mapped to [0,1.0)
            texCoords.push_back(vec2(texXPoints[x], texYPoints[y]));
            vec3 p(  winXPoints[x],  winYPoints[y + x * 2], 0 );
            positions[x * mVerticesY + y] = p;
        }
    }
    



   

	mVboMesh->bufferAttrib(geom::Attrib::POSITION, sizeof(vec3) * mVerticesX * mVerticesY,
		&positions[0]);

	mVboMesh->bufferAttrib(geom::Attrib::TEX_COORD_0, sizeof(vec2) * mVerticesX * mVerticesY,
		&texCoords[0]);

    

    float fboProp = ((1 - 1/mProp) * 2)/tot + 1;

    
    mFboWidth = 128 * 6 * 2 *  fboProp;
    mFboHeight = 72 * 2;
    

    //this will need to be reset too
    gl::Fbo::Format format;
    //format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
   // if(mFbo)mFbo.reset();

    
    mFbo = gl::Fbo::create( mFboWidth, mFboHeight, format );
    
    
}

void SimlTexturingApp::renderSceneToFbo()
{

    

    
	

    
    // clear out the FBO with blue

    

	if (mRenderMode == 0)
	{
		renderTestImage();
	}
	else if(mRenderMode == 1){
		renderShaderImage();
	}

	else if (mRenderMode == 2){
		renderMovie();
	}
    

	else if (mRenderMode == 3){
		//renderSpout();
	}

   
  
}

void SimlTexturingApp::renderTestImage(){

	// bind the framebuffer - now everything we draw will go there
	gl::ScopedFramebuffer fbScp(mFbo);

	// setup the viewport to match the dimensions of the FBO
	gl::ScopedViewport scVp(mFbo->getSize());
	gl::setMatricesWindow(mFbo->getSize());
	gl::clear(Color(0.0f, 0.0f, 1.0f));
	gl::GlslProgRef solidShader = gl::getStockShader(gl::ShaderDef().color());
	Rand r;
	float pos = 0;

	for (int i = 0; i < NUM_SCREENS; i++){
		gl::color(Color(r.nextFloat(), r.nextFloat(), r.nextFloat()));
		Rectf rect(pos / 6 * mFbo->getWidth(), 0,
			pos / 6 * mFbo->getWidth() + mNormalizedProps[i] / 6 * mFbo->getWidth(), mFbo->getHeight());
		gl::drawSolidRect(rect);
		pos += mNormalizedProps[i];

	}

	gl::color(Color(1.0f, 1.0f, 1.0f));

	float incr = mFbo->getHeight() / 2;
	int numBs = ceil(mFbo->getWidth() / incr);

	for (int i = 0; i < numBs; i++){
		gl::drawLine(vec2(i * incr, 0), vec2((i + 1) * incr, mFbo->getHeight() / 2));
		gl::drawLine(vec2(i * incr, mFbo->getHeight() / 2), vec2((i + 1) * incr, mFbo->getHeight()));
		gl::drawLine(vec2(i * incr, 0), vec2(i * incr, mFbo->getHeight()));
		gl::drawSolidCircle(vec2(i * incr, mFbo->getHeight() / 2), 10);
	}

	gl::drawLine(vec2(0, mFbo->getHeight() / 2), vec2(mFbo->getWidth(), mFbo->getHeight() / 2));
}


void SimlTexturingApp::renderMovie(){
 	gl::ScopedFramebuffer fbScp(mFbo);

	// setup the viewport to match the dimensions of the FBO
	gl::ScopedViewport scVp(mFbo->getSize());
	gl::setMatricesWindow(mFbo->getSize());
	gl::clear(Color(0.0f, 0.0f, 1.0f));

	//gl::enableAlphaBlending();

	if (mMovie) {
		Rectf centeredRect = Rectf(mMovie->getTexture()->getBounds()).getCenteredFit(mFbo->getBounds(), true);
		gl::draw(mMovie->getTexture(), centeredRect);
	}
	

}

/*
void SimlTexturingApp::renderSpout(){

	unsigned int width, height;

	// -------- SPOUT -------------
	if (!bInitialized) {

		// This is a receiver, so the initialization is a little more complex than a sender
		// The receiver will attempt to connect to the name it is sent.
		// Alternatively set the optional bUseActive flag to attempt to connect to the active sender. 
		// If the sender name is not initialized it will attempt to find the active sender
		// If the receiver does not find any senders the initialization will fail
		// and "CreateReceiver" can be called repeatedly until a sender is found.
		// "CreateReceiver" will update the passed name, and dimensions.
		SenderName[0] = NULL; // the name will be filled when the receiver connects to a sender
		width = g_Width; // pass the initial width and height (they will be adjusted if necessary)
		height = g_Height;

		// Optionally set for DirectX 9 instead of default DirectX 11 functions
		//spoutreceiver.SetDX9(true);	

		// Initialize a receiver
		if (spoutreceiver.CreateReceiver(SenderName, width, height, true)) {
			console() << "Hello!!" << std::endl;

			// true to find the active sender
			// Optionally test for texture share compatibility
			// bMemoryMode informs us whether Spout initialized for texture share or memory share
			bMemoryMode = spoutreceiver.GetMemoryShareMode();

			// Is the size of the detected sender different from the current texture size ?
			// This is detected for both texture share and memoryshare
			//if (width != g_Width || height != g_Height) {
				// Reset the global width and height
				//g_Width = width;
				//g_Height = height;
				// Reset the local receiving texture size
				spoutTexture = gl::Texture(g_Width, g_Height);
				// reset render window
				//setWindowSize(g_Width, g_Height);
			//} 
			bInitialized = true;
			cout << "Spout Ready " << "\n";
		}
		else {
			// Receiver initialization will fail if no senders are running
			// Keep trying until one starts
		}
	} // endif not initialized
	// ----------------------------
	
	char txt[256];

	gl::setMatricesWindow(getWindowSize());
	gl::clear();
	gl::color(Color(1, 1, 1));

	// Save current global width and height - they will be changed
	// by receivetexture if the sender changes dimensions
	width = g_Width;
	height = g_Height;

	//
	// Try to receive the texture at the current size 
	//
	// NOTE : if ReceiveTexture is called with a framebuffer object bound, 
	// include the FBO id as an argument so that the binding is restored afterwards
	// because Spout uses an fbo for intermediate rendering
	if (bInitialized) {
		if (spoutreceiver.ReceiveTexture(SenderName, width, height, spoutTexture.getId(), spoutTexture.getTarget()),false,mFbo) {
			//	Width and height are changed for sender change so the local texture has to be resized.
			/*if (width != g_Width || height != g_Height) {
				// The sender dimensions have changed - update the global width and height
				g_Width = width;
				g_Height = height;
				// Update the local texture to receive the new dimensions
				spoutTexture = gl::Texture(g_Width, g_Height);
				// reset render window
				//setWindowSize(g_Width, g_Height);
				return; // quit for next round
			} */

			// Otherwise draw the texture and fill the screen
		/*	gl::draw(spoutTexture, getWindowBounds());

			// Show the user what it is receiving
			gl::enableAlphaBlending();
			sprintf_s(txt, "Receiving from [%s]", SenderName);
			gl::drawString(txt, vec2(toPixels(20), toPixels(20)), Color(1, 1, 1), Font("Verdana", toPixels(24)));
			sprintf_s(txt, "fps : %2.2d", (int)getAverageFps());
			gl::drawString(txt, vec2(getWindowWidth() - toPixels(100), toPixels(20)), Color(1, 1, 1), Font("Verdana", toPixels(24)));
			gl::drawString("RH click to select a sender", vec2(toPixels(20), getWindowHeight() - toPixels(40)), Color(1, 1, 1), Font("Verdana", toPixels(24)));
			gl::disableAlphaBlending();
			return; // received OK 
		}
	}

//	gl::enableAlphaBlending();
//	gl::drawString("No sender detected", vec2(toPixels(20), toPixels(20)), Color(1, 1, 1), Font("Verdana", toPixels(24)));
//	gl::disableAlphaBlending();
	// ----------------------------

}
*/


void SimlTexturingApp::renderShaderImage(){
    
    gl::ScopedFramebuffer fbScp(mFbo);
    
    // setup the viewport to match the dimensions of the FBO
    gl::ScopedViewport scVp(mFbo->getSize());
    gl::setMatricesWindow(mFbo->getSize());
    gl::clear(Color(0.0f, 0.0f, 1.0f));

    //gl::ScopedGlslProg shaderScp(mShaderProg[mCurrentShader ]);
	vec2 res = vec2(mFboWidth, mFboHeight);
	mShaderProg[mCurrentShader]->uniform("iResolution", res);
	mShaderProg[mCurrentShader]->uniform("iGlobalTime", (float)app::getElapsedSeconds());
	mShaderProg[mCurrentShader]->uniform("iMouse", vec2(0., 0));
    
    gl::BatchRef br = gl::Batch::create( geom::Rect(mFbo->getBounds()), mShaderProg[mCurrentShader] );
    
    br->draw();


}


void SimlTexturingApp::update()
{
    
    std::string s = std::to_string(getAverageFps());
    getWindow()->setTitle(s);

	renderSceneToFbo();

}


void SimlTexturingApp::draw()
{
	gl::clear(Color(0.0, 0.0, 0.0));
    gl::setMatricesWindow( getWindowSize() );
	gl::color(1.0, 1.0, 1.0);

	gl::ScopedGlslProg shaderScp(gl::getStockShader(gl::ShaderDef().texture()));
	mFbo->bindTexture();
	gl::draw(mVboMesh);
	mFbo->unbindTexture();
   

}

void SimlTexturingApp::keyDown(KeyEvent key){

    if(key.getCode() == KeyEvent::KEY_UP)
    {
        mProp += 0.005;
        resizeScreens();
        renderSceneToFbo();
    }
    
    if(key.getCode() == KeyEvent::KEY_DOWN)
    {
        mProp -= 0.005;
        resizeScreens();
        renderSceneToFbo();
    }

	if (key.getCode() == KeyEvent::KEY_LEFT){
		mCurrentShader = (mCurrentShader + 1)%mShaderProg.size();
	}
	 
	if (key.getCode() == KeyEvent::KEY_SPACE)
	{
		mRenderMode = (mRenderMode + 1) % 4;
	}

	if (key.getCode() == KeyEvent::KEY_o){
		fs::path moviePath = getOpenFilePath();
		if (!moviePath.empty())
			loadMovieFile(moviePath);
			
	}   

	if (key.getCode() == KeyEvent::KEY_ESCAPE){
		quit();
	}
	
    
    
    
}


void SimlTexturingApp::loadMovieFile(const fs::path &moviePath)
{
	try {
		// load up the movie, set it to loop, and begin playing
		mMovie = qtime::MovieGl::create(moviePath);
		mMovie->setLoop();
		mMovie->play();

		
	}
	catch (...) {
		console() << "Unable to load the movie." << std::endl;
		mMovie.reset();
		
	}

}

void SimlTexturingApp::shutdown()
{
	//spoutreceiver.ReleaseReceiver();
}

void SimlTexturingApp::mouseDown(MouseEvent event)
{
	/*if (mRenderMode == 3) {
		if (event.isRightDown()) { // Select a sender
			// SpoutPanel.exe must be in the executable path
			spoutreceiver.SelectSenderPanel(); // DirectX 11 by default
		}
	}*/
}



CINDER_APP( SimlTexturingApp, RendererGl )
