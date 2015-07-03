#include "Resources.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;

using std::vector;


class SimlTexturingApp : public AppBasic {
    
 public:
	void setup();
	void update();
	void draw();
    void renderSceneToFbo();

    static const int NUM_SCREENS = 6;
	int mVerticesX, mVerticesY;

    gl::Fbo mFbo;
	gl::VboMeshRef	mVboMesh;
	gl::TextureRef	mTexture;
    
    static const int	FBO_WIDTH = 1200, FBO_HEIGHT = 67;
    
    void prepareSettings(Settings * settings);
    
    private:
    

    

};

void SimlTexturingApp::prepareSettings(Settings * settings){
    settings->setWindowSize(128 * 6, 72);
    settings->setFrameRate( 60.0f );
}

void SimlTexturingApp::setup()
{


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
    
    float incr = 1.0/(float)NUM_SCREENS;

    for(int i = 0; i < winXPoints.size(); i++)
    {
        if(i%2 == 0){
          winXPoints[i] = (i/2) * incr;
          texXPoints[i] = (i/2) * incr;
        }else{
          winXPoints[i] = ((i+1)/2) * incr;
          texXPoints[i] = ((i+1)/2) * incr;
        }
    }
    
    for(int i = 0; i < winYPoints.size(); i++)
    {
        winYPoints[i] = (i%2 == 0)? 0.0 : 1.0;
        texYPoints[i] = (i%2 == 0)? 0.0 : 1.0;
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
    
	mTexture = gl::Texture::create( loadImage( loadResource( RES_IMAGE ) ) );
    
    renderSceneToFbo();

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
    gl::clear( Color( 0.25, 0.5f, 1.0f ) );
    
    // render an orange torus, with no textures

    gl::color( Color( 1.0f, 0.0f, 0.0f ) );
    
    float incr = mFbo.getWidth() /(float) NUM_SCREENS;
    
    for(int i = 0; i < NUM_SCREENS; i++){
        gl::drawLine(Vec2f( i * incr,0), Vec2f( (i + 1) * incr, mFbo.getHeight()));
    }
    
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
    gl::draw( mVboMesh );
    mFbo.getTexture().unbind();
    gl::popMatrices();


    


}


CINDER_APP_BASIC( SimlTexturingApp, RendererGl )