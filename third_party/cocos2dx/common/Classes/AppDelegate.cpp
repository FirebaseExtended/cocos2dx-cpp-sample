/****************************************************************************
 Copyright (c) 2013      cocos2d-x.org
 Copyright (c) 2013-2014 Chukong Technologies Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "AppDelegate.h"

#include "FirebaseScene.h"

#include "firebase/app.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#endif

USING_NS_CC;

static cocos2d::Size designResolutionSize = cocos2d::Size(480, 320);
static cocos2d::Size smallResolutionSize = cocos2d::Size(480, 320);
static cocos2d::Size mediumResolutionSize = cocos2d::Size(1024, 768);
static cocos2d::Size largeResolutionSize = cocos2d::Size(2048, 1536);

AppDelegate::AppDelegate() {}

AppDelegate::~AppDelegate() {}

// If you want a different context, modify the value of glContextAttrs.
// It will affect all platforms
void AppDelegate::initGLContextAttrs() {
  // Set OpenGL context attributes: red,green,blue,alpha,depth,stencil
  GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

  GLView::setGLContextAttrs(glContextAttrs);
}

// If you want to use the package manager to install more packages,
// Don't modify or remove this function
static int register_all_packages() {
  return 0;  // flag for packages manager
}

bool AppDelegate::applicationDidFinishLaunching() {
  // Initialize the director.
  auto director = Director::getInstance();
  auto glview = director->getOpenGLView();
  if (!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || \
    (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) ||   \
    (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
    glview = GLViewImpl::createWithRect(
        "FirebaseCocos", cocos2d::Rect(0, 0, designResolutionSize.width,
                                       designResolutionSize.height));
#else
    glview = GLViewImpl::create("FirebaseCocos");
#endif
    director->setOpenGLView(glview);
  }

  // Turn on display FPS.
  director->setDisplayStats(false);

  // Set FPS. The default value is 1.0 / 60 if this is not called.
  director->setAnimationInterval(1.0f / 60);

  // Set the design resolution.
  glview->setDesignResolutionSize(designResolutionSize.width,
                                  designResolutionSize.height,
                                  ResolutionPolicy::NO_BORDER);
  auto frameSize = glview->getFrameSize();
  // If the frame's height is larger than the height of medium size.
  if (frameSize.height > mediumResolutionSize.height) {
    director->setContentScaleFactor(
        MIN(largeResolutionSize.height / designResolutionSize.height,
            largeResolutionSize.width / designResolutionSize.width));
  }
  // If the frame's height is larger than the height of small size.
  else if (frameSize.height > smallResolutionSize.height) {
    director->setContentScaleFactor(
        MIN(mediumResolutionSize.height / designResolutionSize.height,
            mediumResolutionSize.width / designResolutionSize.width));
  }
  // If the frame's height is smaller than the height of medium size.
  else {
    director->setContentScaleFactor(
        MIN(smallResolutionSize.height / designResolutionSize.height,
            smallResolutionSize.width / designResolutionSize.width));
  }

  register_all_packages();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
  CCLOG("Initializing Firebase for Android.");
  firebase::App::Create(firebase::AppOptions(), JniHelper::getEnv(),
                        JniHelper::getActivity());
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  CCLOG("Initializing Firebase for iOS.");
  firebase::App::Create(firebase::AppOptions());
#else
  CCLOG("Initializing Firebase for Desktop.");
  CCLOG("Note: Functions in the Firebase C++ desktop API are stubs, and are "
        "provided for convenience only.");
  firebase::App::Create(firebase::AppOptions());
#endif

  // Create a scene. Scenes are reference counted and will auto-release on exit.
  // Currently doesn't compile on desktop. b/34988588
  auto scene = CreateFirebaseScene();

  // Run the scene.
  director->runWithScene(scene);

  return true;
}

// This function will be called when the app is inactive. Note, when receiving a
// phone call it is invoked.
void AppDelegate::applicationDidEnterBackground() {
  Director::getInstance()->stopAnimation();

  // If you use SimpleAudioEngine, it must be paused.
  // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
  Director::getInstance()->startAnimation();

  // If you use SimpleAudioEngine, it must resume here.
  // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
