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

#include "FirebaseMessagingScene.h"

#include <stdarg.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <android/log.h>
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#endif

#include "FirebaseCocos.h"
#include "firebase/messaging.h"

USING_NS_CC;

/// Padding for the UI elements.
static const float kUIElementPadding = 10.0;

class FirebaseMessagingSceneListener : public firebase::messaging::Listener {
 public:
  FirebaseMessagingSceneListener(FirebaseMessagingScene* scene)
      : scene_(scene) {}

  void OnMessage(const firebase::messaging::Message& message) override {
    scene_->logMessage("Recieved a new message");
    if (!message.from.empty()) {
      scene_->logMessage("from: %s", message.from.c_str());
    }
    if (!message.error.empty()) {
      scene_->logMessage("error: %s", message.error.c_str());
    }
    if (!message.message_id.empty()) {
      scene_->logMessage("message_id: %s", message.message_id.c_str());
    }
    if (!message.data.empty()) {
      scene_->logMessage("data:");
      for (const auto& field : message.data) {
        scene_->logMessage(
            "  %s: %s", field.first.c_str(), field.second.c_str());
      }
    }
    if (message.notification) {
      scene_->logMessage("notification:");
      if (!message.notification->title.empty()) {
        scene_->logMessage("  title: %s", message.notification->title.c_str());
      }
      if (!message.notification->body.empty()) {
        scene_->logMessage("  body: %s", message.notification->body.c_str());
      }
      if (!message.notification->icon.empty()) {
        scene_->logMessage("  icon: %s", message.notification->icon.c_str());
      }
      if (!message.notification->tag.empty()) {
        scene_->logMessage("  tag: %s", message.notification->tag.c_str());
      }
      if (!message.notification->color.empty()) {
        scene_->logMessage("  color: %s", message.notification->color.c_str());
      }
      if (!message.notification->sound.empty()) {
        scene_->logMessage("  sound: %s", message.notification->sound.c_str());
      }
      if (!message.notification->click_action.empty()) {
        scene_->logMessage("  click_action: %s",
                           message.notification->click_action.c_str());
      }
    }
  }

  void OnTokenReceived(const char* token) override {
    // To send a message to a specific instance of your app a registration token
    // is required. These tokens are unique for each instance of the app. When
    // messaging::Initialize is called, a request is sent to the Firebase Cloud
    // Messaging server to generate a token. When that token is ready,
    // OnTokenReceived will be called. The token should be cached locally so
    // that a request doesn't need to be generated each time the app is started.
    //
    // Once a token is generated is should be sent to your app server, which can
    // then use it to send messages to users.
    scene_->logMessage("Recieved Registration Token: %s", token);
  }

 private:
  FirebaseMessagingScene* scene_;
};

/// Creates the Firebase scene.
Scene* CreateFirebaseScene() {
  return FirebaseMessagingScene::createScene();
}

/// Creates the FirebaseMessagingScene.
Scene* FirebaseMessagingScene::createScene() {
  // Create the scene.
  auto scene = Scene::create();

  // Create the layer.
  auto layer = FirebaseMessagingScene::create();

  // Add the layer to the scene.
  scene->addChild(layer);

  return scene;
}

/// Initializes the FirebaseScene.
bool FirebaseMessagingScene::init() {
  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  cocos2d::Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // Create the Firebase label.
  auto firebaseLabel =
      Label::createWithTTF("Firebase-Messaging", "fonts/Marker Felt.ttf", 20);
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

  // Intitialize Firebase Messaging. (This must happen after the log ui widget
  // is set up so that the listener has a place to send log messages to)
  CCLOG("Initializing the Messaging with Firebase API.");
  firebase::messaging::Initialize(*firebase::App::GetInstance(),
                                  new FirebaseMessagingSceneListener(this));

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

  // Schedule the update method for this scene.
  this->scheduleUpdate();

  return true;
}

// Called automatically every frame. The update is scheduled in `init()`.
void FirebaseMessagingScene::update(float /*delta*/) {}

/// Handles the user tapping on the close app menu item.
void FirebaseMessagingScene::menuCloseAppCallback(Ref* pSender) {
  CCLOG("Cleaning up Messaging C++ resources.");

  // Close the cocos2d-x game scene and quit the application.
  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}
