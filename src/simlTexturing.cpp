#include "Resources.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;

using std::vector;


/*** This sample demonstrates the Vbo class by creating a simple grid mesh with a texture mapped onto it.
 * The mesh has static indices and texture coordinates, but its vertex positions are dynamic.
 * It also creates a second mesh which shares static and index buffers, but has its own dynamic buffer ***/

class SimlTexturingApp : public AppBasic {
 public:
	void setup();
	void update();
	void draw();

	static const int VERTICES_X = 6, VERTICES_Y = 2;

	gl::VboMeshRef	mVboMesh;
	gl::TextureRef	mTexture;
    
    void prepareSettings(Settings * settings);
    
    private:
    

    

};

void SimlTexturingApp::setup()
{

    
	// setup the parameters of the Vbo
	int totalVertices = VERTICES_X * VERTICES_Y;
	int totalQuads = ( VERTICES_X - 1 ) * ( VERTICES_Y - 1 );
	gl::VboMesh::Layout layout;
	layout.setStaticIndices();
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	mVboMesh = gl::VboMesh::create( totalVertices, totalQuads * 4, layout, GL_QUADS );

	
	// buffer our static data - the texcoords and the indices
	vector<uint32_t> indices;
	vector<Vec2f> texCoords;
    
    float  x_coords[6] = {0.0, 0.25, 0.25, 0.75, 0.75, 1.0};
    float y_coords [12] = {
        0.0 ,0.5,
        0.0, 0.5,
        0.0, 1.0,
        0.0 ,1.0,
        0.5, 1.0,
        0.5, 1.0
    };
    
    float  x_tcoords[6] = {0.0, 0.33, 0.33, 0.66, 0.66, 1.0};
    float y_tcoords [12] = {
        0.0 ,1.0,
        0.0, 1.0,
        0.0, 1.0,
        0.0 ,1.0,
        0.0, 1.0,
        0.0, 1.0
    };
    
	for( int x = 0; x < VERTICES_X; ++x ) {
		for( int y = 0; y < VERTICES_Y; ++y ) {
			// create a quad for each vertex, except for along the bottom and right edges
			if( ( x + 1 < VERTICES_X ) && ( y + 1 < VERTICES_Y ) ) {
				indices.push_back( (x+0) * VERTICES_Y + (y+0) );
                
				indices.push_back( (x+1) * VERTICES_Y + (y+0) );
				indices.push_back( (x+1) * VERTICES_Y + (y+1) );
				indices.push_back( (x+0) * VERTICES_Y + (y+1) );
			}
			// the texture coordinates are mapped to [0,1.0)
			texCoords.push_back(Vec2f(x_tcoords[x], y_tcoords[y]));
		}
	}
	
	mVboMesh->bufferIndices( indices );
	mVboMesh->bufferTexCoords2d( 0, texCoords );
	
	mTexture = gl::Texture::create( loadImage( loadResource( RES_IMAGE ) ) );
    
    // dynmaically generate our new positions based on a simple sine wave
    
    
    vector<Vec3f> positions;
    positions.resize(totalVertices);
    

    //mVboMesh->
    for( int x = 0; x < VERTICES_X; ++x ) {
        for( int y = 0; y < VERTICES_Y; ++y ) {
            Vec3f p(  x_coords[x],  y_coords[y + x * 2], 0 );
            positions[x * VERTICES_Y + y] = p;
        }
    }
    
    mVboMesh->bufferPositions(positions);
    
    /*iter = mVboMesh->mapVertexBuffer();
    for( int i = 0; i < totalVertices; ++i ) {
            Vec3f p = *iter.getPositionPointer();
            iter.setPosition( Vec3f(p.x,  p.y,  i *   ) );
            ++iter;
    }*/
}

void SimlTexturingApp::prepareSettings(Settings * settings){
    settings->setWindowSize(1280, 300);
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


    glScalef(getWindowWidth(), getWindowHeight(), 1.0);
	mTexture->enableAndBind();
	gl::draw( mVboMesh );
	//gl::draw( mVboMesh2 );
}


CINDER_APP_BASIC( SimlTexturingApp, RendererGl )
