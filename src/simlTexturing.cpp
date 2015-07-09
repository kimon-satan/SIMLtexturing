

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
	void keyDown(KeyEvent 	event);

    void renderSceneToFbo();

    static const int NUM_SCREENS = 6;
	int mVerticesX, mVerticesY;

    gl::Fbo mFbo;
	gl::VboMeshRef	mVboMesh;
	gl::Texture	 mTexture;
    
    static const int	FBO_WIDTH = 128 * 6 * (1.0f + 1/15.0f) * 10, FBO_HEIGHT = 72 * 10;
	float mScreenWidthsMeters[NUM_SCREENS]; 
    float mScreenWidths [NUM_SCREENS];
    void prepareSettings(Settings * settings);
    
    private:

	int mXpos;
    

};

void SimlTexturingApp::prepareSettings(Settings * settings){
    settings->setWindowSize(1280 * 6, 720);
    settings->setFrameRate( 60.0f );
	settings->setBorderless(true);
	settings->setWindowPos(1280, 0);
}

void SimlTexturingApp::setup()
{

	mXpos = 0;

	float screenWidths[] = { 6, 4.55, 4.55, 6, 4.55, 4.55 };

	for (int i = 0; i < NUM_SCREENS; i++)
	{
		mScreenWidthsMeters[i] = screenWidths[i];
	}


    gl::Fbo::Format format;
    //format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
    mFbo = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
    
    mVerticesX = NUM_SCREENS * 2;
    mVerticesY = 2;
	// setup the parameters of the Vbo
	int totalVertices = mVerticesX * mVerticesY;
	int totalQuads = ( mVerticesX - 1 ) * ( mVerticesY - 1 );
	gl::VboMesh::Layout layout;
	layout.setStaticIndices();
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	mVboMesh = gl::VboMesh::create( totalVertices, totalQuads * 4, layout, GL_QUADS );


	
	// buffer our static data - the texcoords and the indices
	vector<uint32_t> indices;
	vector<Vec2f> texCoords;
    
    vector<float> winXPoints(NUM_SCREENS * 2);
    vector<float> texXPoints(NUM_SCREENS * 2);
    vector<float> winYPoints(NUM_SCREENS * 4);
    vector<float> texYPoints(NUM_SCREENS * 4);
    
    //screen widths in meters

    
    
    float screenHeights [NUM_SCREENS];
    
    float smallestScr = 100;
    for(int i = 0; i < NUM_SCREENS; i ++)smallestScr = std::min(mScreenWidthsMeters[i], smallestScr);
    for(int i = 0; i < NUM_SCREENS; i ++)screenHeights[i] =  mScreenWidthsMeters[i]/smallestScr;
    
    
    
    //this method means that you lose some pixels from the bottom of the screen
    
    //normalise the sum of the array
    float tot = 0;
    for(int i = 0; i < NUM_SCREENS; i++)tot += mScreenWidthsMeters [i];
    for(int i = 0; i < NUM_SCREENS; i++)mScreenWidths [i] = mScreenWidthsMeters[i] * (NUM_SCREENS/tot);
    
    float incr = 1.0/(float)NUM_SCREENS;
    
    float runningX = 0;
    
    for(int i = 0; i < texXPoints.size(); i++){
        
        //texX points are scaled according to physical screen width ... if the screen is wider
        texXPoints[i] = runningX;
        if(i%2 == 0)runningX += incr * mScreenWidths[i/2];
        
        
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
            winYPoints[i * 4 + j] = (j%2 == 0)? 0.0 : 1.0/screenHeights[i]; // we shorten the screen heights for the larger screens
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
    
	//mTexture = gl::Texture::create( loadImage( loadResource( RES_IMAGE ) ) );
   


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
    

    
    Rand r;
    float pos = 0;
    
    for(int  i= 0; i < NUM_SCREENS; i++){
        gl::color( Color( r.nextFloat(), r.nextFloat(), r.nextFloat() ) );
        Rectf rect( pos/6 * mFbo.getWidth(), 0,
                   pos/6 * mFbo.getWidth() + mScreenWidths[i]/6 * mFbo.getWidth(), mFbo.getHeight());
        gl::drawSolidRect( rect );
        pos += mScreenWidths[i];
        
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

	if (mTexture){
		gl::pushMatrices();
		gl::translate(mXpos + mTexture.getWidth() / 2, mTexture.getHeight() / 2);
		//gl::scale(1, -1,1);
		gl::rotate(180);
		gl::draw(mTexture, Vec2f( - mTexture.getWidth()/2, - mTexture.getHeight()/2));
		//mXpos += 1;
		mXpos = mXpos%mFbo.getWidth();
		gl::popMatrices();
	}
    mFbo.unbindFramebuffer();
  
}


void SimlTexturingApp::keyDown(KeyEvent event){

	if (event.getChar() == 'o') {

		try {
			fs::path path = getOpenFilePath("", ImageIo::getLoadExtensions());
			if (!path.empty()) {
				mTexture = gl::Texture(loadImage(path));
			}
		}
		catch (...) {
			console() << "unable to load the texture file!" << std::endl;
		}

	}
}


void SimlTexturingApp::update()
{

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
    gl::draw( mVboMesh );
    mFbo.getTexture().unbind();
    gl::popMatrices();


    


}


CINDER_APP_BASIC( SimlTexturingApp, RendererGl )
