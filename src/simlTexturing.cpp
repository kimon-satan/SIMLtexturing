
#include "simlTexturing.h"


void SimlTexturingApp::prepareSettings(Settings * settings){
    
    settings->setWindowSize(mcWindowWidth, mcWindowHeight);
    settings->setFrameRate( 60.0f );
    settings->setBorderless();
}

void SimlTexturingApp::setup()
{

    mProp = 6.0/5.0;

    
    

    
    mVerticesX = NUM_SCREENS * 2;
    mVerticesY = 2;


    resizeScreens();
    renderSceneToFbo();
	

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
    
    
    
    mVboMesh->bufferIndices( indices );
    mVboMesh->bufferTexCoords2d( 0, texCoords );
    mVboMesh->bufferPositions(positions);
    mVboMesh->unbindBuffers();
    

    float fboProp = ((1 - 1/mProp) * 2)/tot + 1;
    
    mFboWidth = 128 * 6 * fboProp;
    mFboHeight = 72;
    
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
    gl::clear( Color( 0.5, 0.5f, 0.5f ) );
    
    // render an orange torus, with no textures


    
    Rand r;
    float pos = 0;
    
    for(int  i= 0; i < NUM_SCREENS; i++){
        gl::color( Color( r.nextFloat(), r.nextFloat(), r.nextFloat() ) );
        Rectf rect( pos/6 * mFbo.getWidth(), 0,
                   pos/6 * mFbo.getWidth() + mNormalizedProps[i]/6 * mFbo.getWidth(), mFbo.getHeight());
        gl::drawSolidRect( rect );
        pos += mNormalizedProps[i];
        
    }
    
    gl::color( Color( 1.0f, 1.0f, 1.0f ) );
    
    float incr = mFbo.getHeight() /2;
    int numBs = ceil(mFbo.getWidth()/incr);
    
    for(int i = 0; i < numBs; i++){
        gl::drawLine(Vec2f( i * incr,0), Vec2f( (i + 1) * incr, mFbo.getHeight()/2));
        gl::drawLine(Vec2f( i * incr,mFbo.getHeight()/2), Vec2f( (i + 1) * incr, mFbo.getHeight()));
        gl::drawLine(Vec2f( i * incr,0), Vec2f(i * incr, mFbo.getHeight()));
        gl::drawSolidCircle(Vec2f(i * incr,mFbo.getHeight()/2), 10);
    }
    
    gl::drawLine(Vec2f(0,mFbo.getHeight()/2), Vec2f(mFbo.getWidth(),mFbo.getHeight()/2));

   
    
    mFbo.unbindFramebuffer();
  
}


void SimlTexturingApp::update()
{



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
        mProp += 0.01;
        resizeScreens();
        renderSceneToFbo();
    }
    
    if(key.getCode() == KeyEvent::KEY_DOWN)
    {
        mProp -= 0.01;
        resizeScreens();
        renderSceneToFbo();
    }
    
    
    
}


CINDER_APP_BASIC( SimlTexturingApp, RendererGl )
