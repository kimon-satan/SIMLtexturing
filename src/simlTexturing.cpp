
#include "simlTexturing.h"


void SimlTexturingApp::prepareSettings(Settings * settings){
    
    settings->setWindowSize(mcWindowWidth, mcWindowHeight);
    settings->setFrameRate( 60.0f );
    settings->setBorderless();

	settings->setWindowPos(1280, 0);

	g_Width = mcWindowHeight; // set global width and height to something
	g_Height = mcWindowHeight;

}

void SimlTexturingApp::setup()
{


	mLastFrameCount = 0;
	mProp = 1.28;
	mTargetTime = 1.0;
	mCurrentShader = 0;

    
    mVerticesX = NUM_SCREENS * 2;
    mVerticesY = 2;

	mRenderMode = 0;
    resizeScreens();
    renderSceneToFbo();


	mShaderProg.resize(3);
		
	mShaderProg[0] = gl::GlslProg(loadResource(RES_PT_VERT_GLSL, "GLSL"), loadResource(RES_SEA_FRAG_GLSL, "GLSL"));
	mShaderProg[1] = gl::GlslProg(loadResource(RES_PT_VERT_GLSL, "GLSL"), loadResource(RES_MOUNTAIN_FRAG_GLSL, "GLSL"));
	mShaderProg[2] = gl::GlslProg(loadResource(RES_PT_VERT_GLSL, "GLSL"), loadResource(RES_FRAG_GLSL, "GLSL"));

	bInitialized = false;


}

void SimlTexturingApp::resizeScreens(){

 
    
    if(mVboMesh)mVboMesh->reset();
    
    // setup the parameters of the Vbo
    int totalVertices = mVerticesX * mVerticesY;
    int totalQuads = ( mVerticesX - 1 ) * ( mVerticesY - 1 );
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    mVboMesh = gl::VboMesh::create( totalVertices, totalQuads * 4, layout, GL_QUADS );
    
    vector<uint32_t> indices;
    vector<Vec2f> texCoords;
    
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
            winXPoints[i] = (i/2) * incr;
        }else{
            winXPoints[i] = ((i+1)/2) * incr;
        }
    }
    
    
    for(int i = 0; i < NUM_SCREENS; i++)
    {
        for(int j = 0; j < 4; j++){
            winYPoints[i * 4 + j] = (j%2 == 0)? 0.0 : screenHeights[i]; // we shorten the screen heights for the larger screens
            texYPoints[i * 4 + j] = (j%2 == 0)? 0.0 : 1.0;
        }
        
        
    }
    
    vector<Vec3f> positions;
    positions.resize(totalVertices);
    
    for( int x = 0; x < mVerticesX; ++x ) {
        for( int y = 0; y < mVerticesY; ++y ) {
            // create a quad for each vertex, except for along the bottom and right edges
            if( ( x + 1 < mVerticesX ) && ( y + 1 < mVerticesY ) ) {
                
                indices.push_back( (x+0) * mVerticesY + (y+0) );
                indices.push_back( (x+1) * mVerticesY + (y+0) );
                indices.push_back( (x+1) * mVerticesY + (y+1) );
                indices.push_back( (x+0) * mVerticesY + (y+1) );
            }
            // the texture coordinates are mapped to [0,1.0)
            texCoords.push_back(Vec2f(texXPoints[x], texYPoints[y]));
            Vec3f p(  winXPoints[x],  winYPoints[y + x * 2], 0 );
            positions[x * mVerticesY + y] = p;
        }
    }
    

	//console() << "texture x: " << std::endl;

	//for (int i = 0; i < texXPoints.size(); i++)
	//{
	//	console()  << texXPoints[i] * 8192 << ",";
	//}

	//console() << std::endl;


	console() << "Hello!!" << std::endl;

   

    mVboMesh->bufferIndices( indices );
    mVboMesh->bufferTexCoords2d( 0, texCoords );
    mVboMesh->bufferPositions(positions);
    mVboMesh->unbindBuffers();
    

    float fboProp = ((1 - 1/mProp) * 2)/tot + 1;

    
    mFboWidth = 128 * 6 * 10 *  fboProp;
    mFboHeight = 720;
    

    //this will need to be reset too
    gl::Fbo::Format format;
    //format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
    if(mFbo){
        mFbo.unbindTexture();
        mFbo.reset();

    }
    
    mFbo = gl::Fbo( mFboWidth, mFboHeight, format );
    
    
}

void SimlTexturingApp::renderSceneToFbo()
{
    // this will restore the old framebuffer binding when we leave this function
    // on non-OpenGL ES platforms, you can just call mFbo.unbindFramebuffer() at the end of the function
    // but this will restore the "screen" FBO on OpenGL ES, and does the right thing on both platforms
    gl::SaveFramebufferBinding bindingSaver;
    
    // bind the framebuffer - now everything we draw will go there
    mFbo.bindFramebuffer();
    
    // setup the viewport to match the dimensions of the FBO
    gl::setViewport( mFbo.getBounds() );
    gl::setMatricesWindow( mFbo.getSize() );
    
    // set the modelview matrix to reflect our current rotation
    
    // clear out the FBO with blue
    gl::clear( Color( 0.0f, 0.0f, 0.0f ) );
    

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
		renderSpout();
	}

   
    mFbo.unbindFramebuffer();
  
}

void SimlTexturingApp::renderTestImage(){
	Rand r;
	float pos = 0;

	for (int i = 0; i < NUM_SCREENS; i++){
		gl::color(Color(r.nextFloat(), r.nextFloat(), r.nextFloat()));
		Rectf rect(pos / 6 * mFbo.getWidth(), 0,
			pos / 6 * mFbo.getWidth() + mNormalizedProps[i] / 6 * mFbo.getWidth(), mFbo.getHeight());
		gl::drawSolidRect(rect);
		pos += mNormalizedProps[i];

	}

	gl::color(Color(1.0f, 1.0f, 1.0f));

	float incr = mFbo.getHeight() / 2;
	int numBs = ceil(mFbo.getWidth() / incr);

	for (int i = 0; i < numBs; i++){
		gl::drawLine(Vec2f(i * incr, 0), Vec2f((i + 1) * incr, mFbo.getHeight() / 2));
		gl::drawLine(Vec2f(i * incr, mFbo.getHeight() / 2), Vec2f((i + 1) * incr, mFbo.getHeight()));
		gl::drawLine(Vec2f(i * incr, 0), Vec2f(i * incr, mFbo.getHeight()));
		gl::drawSolidCircle(Vec2f(i * incr, mFbo.getHeight() / 2), 10);
	}

	gl::drawLine(Vec2f(0, mFbo.getHeight() / 2), Vec2f(mFbo.getWidth(), mFbo.getHeight() / 2));
}

void SimlTexturingApp::renderMovie(){

	gl::clear(Color(1.0, 1.0, 1.0));
	//gl::enableAlphaBlending();

	if (mMovie) {
		Rectf centeredRect = Rectf(mMovie->getTexture().getBounds()).getCenteredFit(mFbo.getBounds(), true);
		gl::draw(mMovie->getTexture(), centeredRect);
	}
	

}

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
			gl::draw(spoutTexture, getWindowBounds());

			// Show the user what it is receiving
			gl::enableAlphaBlending();
			sprintf_s(txt, "Receiving from [%s]", SenderName);
			gl::drawString(txt, Vec2f(toPixels(20), toPixels(20)), Color(1, 1, 1), Font("Verdana", toPixels(24)));
			sprintf_s(txt, "fps : %2.2d", (int)getAverageFps());
			gl::drawString(txt, Vec2f(getWindowWidth() - toPixels(100), toPixels(20)), Color(1, 1, 1), Font("Verdana", toPixels(24)));
			gl::drawString("RH click to select a sender", Vec2f(toPixels(20), getWindowHeight() - toPixels(40)), Color(1, 1, 1), Font("Verdana", toPixels(24)));
			gl::disableAlphaBlending();
			return; // received OK 
		}
	}

//	gl::enableAlphaBlending();
//	gl::drawString("No sender detected", Vec2f(toPixels(20), toPixels(20)), Color(1, 1, 1), Font("Verdana", toPixels(24)));
//	gl::disableAlphaBlending();
	// ----------------------------

}

void SimlTexturingApp::renderShaderImage(){

	mShaderProg[mCurrentShader ].bind();
	Vec2f res = Vec2f(mFboWidth, mFboHeight);
	mShaderProg[mCurrentShader].uniform("iResolution", res);
	mShaderProg[mCurrentShader].uniform("iGlobalTime", (float)app::getElapsedSeconds());
	mShaderProg[mCurrentShader].uniform("iMouse", Vec2f(0., 0));
	gl::drawSolidRect(mFbo.getBounds());
	mShaderProg[mCurrentShader].unbind();

}

void SimlTexturingApp::update()
{
	if (app::getElapsedSeconds() >= mTargetTime){
		int fps = app::getElapsedFrames() - mLastFrameCount;
		//console() <<  fps << std::endl;
		mLastFrameCount = app::getElapsedFrames();
		mTargetTime += 1;
	}
	renderSceneToFbo();

}


void SimlTexturingApp::draw()
{
    
    gl::setViewport( getWindowBounds() );
    gl::setMatricesWindow( getWindowSize() );
	// this pair of lines is the standard way to clear the screen in OpenGL
	gl::clear( Color( 0.15f, 0.15f, 0.15f ) );
    
    gl::color( Color( 1.0f, 1.0f, 1.0f ) );
    
    gl::pushMatrices();
    glScalef(getWindowWidth(), getWindowHeight(), 1.0);

    mFbo.getTexture().enableAndBind();
    //gl::draw(  Rectf( 0, 0, 128 * 6, 72 ) );
    //gl::draw(mFbo.getTexture());
    gl::draw( mVboMesh );
    mFbo.getTexture().unbind();
    gl::popMatrices();


    


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
		mMovie->reset();
		
	}

}

void SimlTexturingApp::shutdown()
{
	spoutreceiver.ReleaseReceiver();
}

void SimlTexturingApp::mouseDown(MouseEvent event)
{
	if (mRenderMode == 3) {
		if (event.isRightDown()) { // Select a sender
			// SpoutPanel.exe must be in the executable path
			spoutreceiver.SelectSenderPanel(); // DirectX 11 by default
		}
	}
}



CINDER_APP_BASIC( SimlTexturingApp, RendererGl )
