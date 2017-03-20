// Copyright 2017 Google Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "FirebaseRemoteConfigScene.h"

#include <stdarg.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <android/log.h>
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#endif

#include "FirebaseCocos.h"
#include "firebase/remote_config.h"
#include "firebase/variant.h"

USING_NS_CC;

/// Padding for the UI elements.
static const float kUIElementPadding = 10.0;

/// Creates the Firebase scene.
Scene* CreateFirebaseScene() {
  return FirebaseRemoteConfigScene::createScene();
}

/// Creates the FirebaseRemoteConfigScene.
Scene* FirebaseRemoteConfigScene::createScene() {
  // Create the scene.
  auto scene = Scene::create();

  // Create the layer.
  auto layer = FirebaseRemoteConfigScene::create();

  // Add the layer to the scene.
  scene->addChild(layer);

  return scene;
}

/// Initializes the FirebaseScene.
bool FirebaseRemoteConfigScene::init() {
  namespace remote_config = ::firebase::remote_config;

  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  cocos2d::Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // Create the Firebase label.
  auto firebaseLabel = Label::createWithTTF(
      "Firebase Remote Config", "fonts/Marker Felt.ttf", 20);
  nextYPosition =
      origin.y + visibleSize.height - firebaseLabel->getContentSize().height;
  firebaseLabel->setPosition(
      cocos2d::Vec2(origin.x + visibleSize.width / 2, nextYPosition));
  this->addChild(firebaseLabel, 1);

  const float scrollViewYPosition = nextYPosition -
                                    firebaseLabel->getContentSize().height -
                                    kUIElementPadding * 2;
  // Create the ScrollView on the Cocos2d thread.
  cocos2d::Director::getInstance()
      ->getScheduler()
      ->performFunctionInCocosThread(
          [=]() { this->createScrollView(scrollViewYPosition, 0.0f); });

  // Intitialize Firebase RemoteConfig. (This must happen after the log ui 
  // widget is set up so that the listener has a place to send log messages to).
  CCLOG("Initializing the RemoteConfig with Firebase API.");
  remote_config::Initialize(*firebase::App::GetInstance());

  // Create the close app menu item.
  auto closeAppItem = MenuItemImage::create(
      "CloseNormal.png", "CloseSelected.png",
      CC_CALLBACK_1(FirebaseScene::menuCloseAppCallback, this));
  closeAppItem->setContentSize(cocos2d::Size(25, 25));
  // Position the close app menu item on the top-right corner of the screen.
  closeAppItem->setPosition(cocos2d::Vec2(
      origin.x + visibleSize.width - closeAppItem->getContentSize().width / 2,
      origin.y + visibleSize.height -
          closeAppItem->getContentSize().height / 2));

  // Create the Menu for touch handling.
  auto menu = Menu::create(closeAppItem, NULL);
  menu->setPosition(cocos2d::Vec2::ZERO);
  this->addChild(menu, 1);

  static const unsigned char kBinaryDefaults[] = {6, 0, 0, 6, 7, 3};

  static const remote_config::ConfigKeyValueVariant defaults[] = {
      {"TestBoolean", true},
      {"TestLong", 42},
      {"TestDouble", 3.14},
      {"TestString", "Hello World"},
      {"TestData", firebase::Variant::FromStaticBlob(kBinaryDefaults,
                                                     sizeof(kBinaryDefaults))}
  };
  size_t default_count = sizeof(defaults) / sizeof(defaults[0]);
  remote_config::SetDefaults(defaults, default_count);

  // The return values may not be the set defaults, if a fetch was previously
  // completed for the app that set them.
  {
    bool result = remote_config::GetBoolean("TestBoolean");
    logMessage("Get TestBoolean %d", result ? 1 : 0);
  }
  {
    int64_t result = remote_config::GetLong("TestLong");
    logMessage("Get TestLong %lld", result);
  }
  {
    double result = remote_config::GetDouble("TestDouble");
    logMessage("Get TestDouble %f", result);
  }
  {
    std::string result = remote_config::GetString("TestString");
    logMessage("Get TestString %s", result.c_str());
  }
  {
    std::vector<unsigned char> result = remote_config::GetData("TestData");
    for (size_t i = 0; i < result.size(); ++i) {
      const unsigned char value = result[i];
      logMessage("TestData[%d] = 0x%02x", i, value);
    }
  }

  // Enable developer mode and verified it's enabled.
  // NOTE: Developer mode should not be enabled in production applications.
  remote_config::SetConfigSetting(remote_config::kConfigSettingDeveloperMode,
                                  "1");
  if ((*remote_config::GetConfigSetting(
            remote_config::kConfigSettingDeveloperMode)
            .c_str()) != '1') {
    logMessage("Failed to enable developer mode");
  }

  future_ = remote_config::Fetch(0);

  // Schedule the update method for this scene.
  this->scheduleUpdate();

  return true;
}

// Called automatically every frame. The update is scheduled in `init()`.
void FirebaseRemoteConfigScene::update(float /*delta*/) {
  namespace remote_config = ::firebase::remote_config;

  if (future_.status() != firebase::kFutureStatusComplete) {
    return;
  }

  logMessage("Fetch Complete");
  bool activate_result = remote_config::ActivateFetched();
  logMessage("ActivateFetched %s", activate_result ? "succeeded" : "failed");

  const remote_config::ConfigInfo& info = remote_config::GetInfo();
  logMessage("Info last_fetch_time_ms=%d fetch_status=%d failure_reason=%d",
             static_cast<int>(info.fetch_time), info.last_fetch_status,
             info.last_fetch_failure_reason);

  // Print out the new values, which may be updated from the Fetch.
  {
    bool result = remote_config::GetBoolean("TestBoolean");
    logMessage("Updated TestBoolean %d", result ? 1 : 0);
  }
  {
    int64_t result = remote_config::GetLong("TestLong");
    logMessage("Updated TestLong %lld", result);
  }
  {
    double result = remote_config::GetDouble("TestDouble");
    logMessage("Updated TestDouble %f", result);
  }
  {
    std::string result = remote_config::GetString("TestString");
    logMessage("Updated TestString %s", result.c_str());
  }
  {
    std::vector<unsigned char> result = remote_config::GetData("TestData");
    for (size_t i = 0; i < result.size(); ++i) {
      const unsigned char value = result[i];
      logMessage("TestData[%d] = 0x%02x", i, value);
    }
  }

  // Print out the keys that are now tied to data
  std::vector<std::string> keys = remote_config::GetKeys();
  logMessage("GetKeys:");
  for (auto s = keys.begin(); s != keys.end(); ++s) {
    logMessage("  %s", s->c_str());
  }
  keys = remote_config::GetKeysByPrefix("TestD");
  logMessage("GetKeysByPrefix(\"TestD\"):");
  for (auto s = keys.begin(); s != keys.end(); ++s) {
    logMessage("  %s", s->c_str());
  }

  // Release a handle to the future so we can shutdown the Remote Config API
  // when exiting the app. Alternatively we could have placed future in a scope
  // different to our shutdown code below.
  future_.Release();
}

/// Handles the user tapping on the close app menu item.
void FirebaseRemoteConfigScene::menuCloseAppCallback(Ref* pSender) {
  CCLOG("Cleaning up Remote Config C++ resources.");
  firebase::remote_config::Terminate();

  // Close the cocos2d-x game scene and quit the application.
  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}
