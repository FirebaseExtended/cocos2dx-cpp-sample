// Copyright 2016 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "FirebaseInvitesScene.h"

#include <stdarg.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <android/log.h>
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#endif

#include "FirebaseCocos.h"
#include "firebase/future.h"
#include "firebase/invites.h"

USING_NS_CC;

/// Padding for the UI elements.
static const float kUIElementPadding = 10.0;

/// The title text for the Firebase buttons.
static const char* kInviteButtonText = "Invite";

/// Creates the Firebase scene.
Scene* CreateFirebaseScene() {
  return FirebaseInvitesScene::createScene();
}

/// Creates the FirebaseInvitesScene.
Scene* FirebaseInvitesScene::createScene() {
  // Create the scene.
  auto scene = Scene::create();

  // Create the layer.
  auto layer = FirebaseInvitesScene::create();

  // Add the layer to the scene.
  scene->addChild(layer);

  return scene;
}

/// Initializes the FirebaseScene.
bool FirebaseInvitesScene::init() {
  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  cocos2d::Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // Intitialize Firebase-Invites.
  CCLOG("Initializing the Invites with Firebase API.");
  firebase::invites::Initialize(*firebase::App::GetInstance());

  // Create the Firebase label.
  auto firebaseLabel =
      Label::createWithTTF("Firebase-Invites", "fonts/Marker Felt.ttf", 20);
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
          [=]() { this->createScrollView(scrollViewYPosition); });

  // Set up the invite button.
  invite_button_ = createButton(true, kInviteButtonText);
  invite_button_->addTouchEventListener(
      [this](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            firebase::invites::Invite invite;
            invite.title_text = "Invite Friends";
            invite.message_text = "Try out this super cool sample app!";
            invite.call_to_action_text = "Download now!";
            firebase::invites::SendInvite(invite);
            invite_sent_ = true;
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(invite_button_);

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
void FirebaseInvitesScene::update(float /*delta*/) {
  if (invite_sent_) {
    firebase::Future<firebase::invites::SendInviteResult> future =
        firebase::invites::SendInviteLastResult();
    if (future.Status() == firebase::kFutureStatusComplete) {
      if (future.Error() == 0) {
        const firebase::invites::SendInviteResult& result = *future.Result();
        if (result.invitation_ids.size() > 0) {
          // One or more invitations were sent. You can log the invitation IDs
          // here for analytics purposes, as they will be the same on the
          // receiving side.
          logMessage("Invite sent successfully!");
        }
        else {
          // Zero invitations were sent. This tells us that the user canceled
          // sending invitations.
          logMessage("Invite canceled.");
        }
      } else {
        // error() is nonzero, which means an error occurred. You can check
        // future_result.error_message() for more information.
        logMessage("Error sending the invite. (Error %i: \"%s\")",
            future.Error(), future.ErrorMessage());
      }
      invite_sent_ = false;
    } else {
      // The SendInvite() operation has not completed yet, which means the
      // Invites client UI is still on screen. Check the status() again soon.
    }
  }
}

/// Handles the user tapping on the close app menu item.
void FirebaseInvitesScene::menuCloseAppCallback(Ref* pSender) {
  CCLOG("Cleaning up Invites C++ resources.");

  // Close the cocos2d-x game scene and quit the application.
  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}
