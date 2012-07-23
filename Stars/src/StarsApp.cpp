#include "cinder/MayaCamUI.h"
#include "cinder/Utilities.h"
#include "cinder/Timer.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

#include "Background.h"
#include "Cam.h"
#include "Grid.h"
#include "Stars.h"
#include "UserInterface.h"

#include <irrKlang.h>

#pragma comment(lib, "irrKlang.lib")

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace irrklang;

class StarsApp : public AppBasic {
public:
	void	prepareSettings(Settings *settings);
	void	setup();
	void	shutdown();
	void	update();
	void	draw();

	void	mouseDown( MouseEvent event );	
	void	mouseDrag( MouseEvent event );	
	void	mouseUp( MouseEvent event );
	void	keyDown( KeyEvent event );
	void	resize( ResizeEvent event );
protected:
	void	forceHideCursor();
	void	forceShowCursor();
	void	constrainCursor( const Vec2i &pos );

	fs::path	getFirstFile( const fs::path &path, const fs::path &extension );	
	fs::path	getNextFile( const fs::path &current, const fs::path &extension );
protected:
	double			mTime;

	// cursor position
	Vec2i			mCursorPos;
	Vec2i			mCursorPrevious;

	// camera
	Cam				mCamera;

	// graphical elements
	Stars			mStars;
	Background		mBackground;
	Grid			mGrid;
	UserInterface	mUserInterface;

	// animation timer
	Timer			mTimer;

	// 
	bool			mIsGridVisible;
	bool			mIsCursorVisible;

	//
	ISoundEngine*	mSoundEngine;
	ISound*			mSound;
	ISound*			mMusic;
};

void StarsApp::prepareSettings(Settings *settings)
{
	settings->setFrameRate(200.0f);
	settings->setWindowSize(1280,720);

#if (defined WIN32 && defined NDEBUG)
	settings->setFullScreen(true);
#else
	// never start in full screen on MacOS or in debug mode
	settings->setFullScreen(false);
#endif
}

void StarsApp::setup()
{
	mTime = getElapsedSeconds();

	// create the spherical grid mesh
	mGrid.setup();

	// load the star database and create the VBO mesh
	mStars.load( loadAsset("hygxyz.csv") );
	//mStars.write( writeFile( getAssetPath("") / "hygxyz.dat" ) );	// TODO

	// load texture and shader
	mStars.setup();

	// create user interface
	mUserInterface.setup();

	// initialize background image
	mBackground.setup();

	// initialize camera
	mCamera.setup();

	//
	mIsGridVisible = false;

	//
	forceHideCursor();

	// initialize the IrrKlang Sound Engine
	mSoundEngine = createIrrKlangDevice();
	mSound = NULL;
	mMusic = NULL;

	if(mSoundEngine) {
		// play 3D Sun rumble
		std::string file = (getAssetPath("") / "sound/low_rumble_loop.mp3").string();
		mSound = mSoundEngine->play3D( file.c_str(), vec3df(0,0,0), true, true );
		if(mSound) {
			mSound->setMinDistance(2.5f);
			mSound->setMaxDistance(12.5f);
			mSound->setIsPaused(false);
		}

		// play background music (the first .mp3 file found in ./assets/music)
		fs::path path = getFirstFile( getAssetPath("") / "music", ".mp3" );
		if( !path.empty() ) mMusic = mSoundEngine->play2D( path.string().c_str(), false, true );
		if(mMusic) mMusic->setIsPaused(false);
	}

	//
	mTimer.start();
}

void StarsApp::shutdown()
{
	if(mSoundEngine) {
		mSoundEngine->stopAllSounds();

		if(mMusic) mMusic->drop();
		if(mSound) mSound->drop();

		mSoundEngine->drop();
	}
}

void StarsApp::update()
{	
	double elapsed = getElapsedSeconds() - mTime;
	mTime += elapsed;

	// animate camera
	double time = getElapsedSeconds() / 200.0;
	if(mMusic) time = mMusic->getPlayPosition() / (double) mMusic->getPlayLength();
	mCamera.setDistanceTime(time);
	mCamera.update(elapsed);

	// update background and user interface
	mBackground.setCameraDistance( mCamera.getCamera().getEyePoint().length() );
	mUserInterface.setCameraDistance( mCamera.getCamera().getEyePoint().length() );

	//
	if(mSoundEngine) {
		Vec3f pos = mCamera.getPosition();
		mSoundEngine->setListenerPosition( 
			vec3df(pos.x, pos.y, pos.z), 
			vec3df(-pos.x, -pos.y, -pos.z), 
			vec3df(0,0,0), 
			vec3df(0,1,0) );
	}
}

void StarsApp::draw()
{		
	gl::clear( Color::black() ); 

	gl::pushMatrices();
	gl::setMatrices( mCamera.getCamera() );
	{
		// draw background
		mBackground.draw();

		// draw grid
		if(mIsGridVisible) 
			mGrid.draw();

		// draw stars
		mStars.draw();
	}
	gl::popMatrices();

	// draw user interface
	mUserInterface.draw();

	// fade in at start of application
	gl::enableAlphaBlending();
	double t = math<double>::clamp( mTimer.getSeconds() / 3.0, 0.0, 1.0 );
	float a = ci::lerp<float>(1.0f, 0.0f, (float) t);

	if( a > 0.0f ) {
		gl::color( ColorA(0,0,0,a) );
		gl::drawSolidRect( getWindowBounds() );
	}
	gl::disableAlphaBlending();
}

void StarsApp::mouseDown( MouseEvent event )
{
	// allow user to control camera
	mCursorPos = mCursorPrevious = event.getPos();
	mCamera.mouseDown( mCursorPos );
}

void StarsApp::mouseDrag( MouseEvent event )
{
	mCursorPos += event.getPos() - mCursorPrevious;
	mCursorPrevious = event.getPos();

	constrainCursor( event.getPos() );

	// allow user to control camera
	mCamera.mouseDrag( mCursorPos, event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void StarsApp::mouseUp( MouseEvent event )
{
	// allow user to control camera
	mCursorPos = mCursorPrevious = event.getPos();
	mCamera.mouseUp( mCursorPos );
}

void StarsApp::keyDown( KeyEvent event )
{
	switch( event.getCode() )
	{
	case KeyEvent::KEY_f:
		// toggle full screen
		setFullScreen( !isFullScreen() );
		break;
	case KeyEvent::KEY_v:
		gl::enableVerticalSync( !gl::isVerticalSyncEnabled() );
		break;
	case KeyEvent::KEY_ESCAPE:
		// quit the application
		quit();
		break;
	case KeyEvent::KEY_SPACE:
		// enable animation
		mCamera.setup();
		break;
	case KeyEvent::KEY_g:
		// toggle grid
		mIsGridVisible = !mIsGridVisible;
		break;
	case KeyEvent::KEY_c:
		// toggle cursor
		if(mIsCursorVisible) 
			forceHideCursor();
		else 
			forceShowCursor();
		break;
		/*// 
		case KeyEvent::KEY_KP7:
		mBackground.rotateX(-0.05f);
		break;
		case KeyEvent::KEY_KP9:
		mBackground.rotateX(+0.05f);
		break;
		case KeyEvent::KEY_KP4:
		mBackground.rotateY(-0.05f);
		break;
		case KeyEvent::KEY_KP6:
		mBackground.rotateY(+0.05f);
		break;
		case KeyEvent::KEY_KP1:
		mBackground.rotateZ(-0.05f);
		break;
		case KeyEvent::KEY_KP3:
		mBackground.rotateZ(+0.05f);
		break;
		//*/
	}
}

void StarsApp::resize( ResizeEvent event )
{
	mCamera.resize( event );
}

void StarsApp::forceHideCursor()
{
	// forces the cursor to hide
#ifdef WIN32
	while( ::ShowCursor(false) >= 0 );
#else
	hideCursor();
#endif
	mIsCursorVisible = false;
}

void StarsApp::forceShowCursor()
{
	// forces the cursor to show
#ifdef WIN32
	while( ::ShowCursor(true) < 0 );
#else
	showCursor();
#endif
	mIsCursorVisible = true;
}

void StarsApp::constrainCursor( const Vec2i &pos )
{
	// keeps the cursor well within the window bounds,
	// so that we can continuously drag the mouse without
	// ever hitting the sides of the screen

	if( pos.x < 50 || pos.x > getWindowWidth() - 50 || pos.y < 50 || pos.y > getWindowHeight() - 50 )
	{
#ifdef WIN32
		POINT pt;
		mCursorPrevious.x = pt.x = getWindowWidth() / 2;
		mCursorPrevious.y = pt.y = getWindowHeight() / 2;

		HWND hWnd = getRenderer()->getHwnd();
		::ClientToScreen(hWnd, &pt);
		::SetCursorPos(pt.x,pt.y);
#else
		// if you want to implement this on MacOS, see: CGWarpMouseCursorPosition
#endif
	}
}	

fs::path	StarsApp::getFirstFile( const fs::path &path, const fs::path &extension )
{
	fs::directory_iterator end_itr;
	for( fs::directory_iterator i( path ); i != end_itr; ++i )
	{
		// skip if not a file
		if( !fs::is_regular_file( i->status() ) ) continue;

		// skip if extension does not match
		if( i->path().extension() != extension ) continue;

		// file matches, return it
		return i->path();
	}

	// failed, return empty path
	return fs::path();
}

fs::path	StarsApp::getNextFile( const fs::path &current, const fs::path &extension )
{
	// TODO
	return fs::path();
}

CINDER_APP_BASIC( StarsApp, RendererGl )