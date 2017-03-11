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

#include "FirebaseAuthScene.h"

#include <stdarg.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <android/log.h>
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#endif

#include "FirebaseCocos.h"
#include "firebase/auth.h"

USING_NS_CC;


/// Padding for the UI elements.
static const float kUIElementPadding = 10.0;

/// Placeholder labels for the text fields.
static const char* kEmailPlaceholderText = "Email";
static const char* kPasswordPlaceholderText = "Password";

/// Creates the Firebase scene.
Scene* CreateFirebaseScene() {
  return FirebaseAuthScene::createScene();
}

/// Creates the FirebaseAuthScene.
Scene* FirebaseAuthScene::createScene() {
  // Create the scene.
  auto scene = Scene::create();

  // Create the layer.
  auto layer = FirebaseAuthScene::create();

  // Add the layer to the scene.
  scene->addChild(layer);

  return scene;
}

/// Initializes the FirebaseScene.
bool FirebaseAuthScene::init() {
  using firebase::auth::Auth;
  using firebase::auth::Credential;
  using firebase::auth::EmailAuthProvider;

  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  cocos2d::Vec2 origin = Director::getInstance()->getVisibleOrigin();

  CCLOG("Initializing the Auth with Firebase API.");
  Auth* auth = Auth::GetAuth(firebase::App::GetInstance());

  // Create the Firebase label.
  auto firebaseLabel =
      Label::createWithTTF("Firebase-Auth", "fonts/Marker Felt.ttf", 20);
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

  logMessage("Created the Auth %x class for the Firebase app.",
             static_cast<int>(reinterpret_cast<intptr_t>(auth)));

  // It's possible for current_user() to be non-null if the previous run
  // left us in a signed-in state.
  if (auth->current_user() == nullptr) {
    logMessage("No user signed in at creation time.");
  } else {
    logMessage("Current user %s already signed in, so signing them out.",
               auth->current_user()->display_name().c_str());
  }

  email_text_field_ = createTextField(kEmailPlaceholderText);
  this->addChild(email_text_field_);

  password_text_field_ = createTextField(kPasswordPlaceholderText);
  password_text_field_->setPasswordEnabled(true);
  this->addChild(password_text_field_);

  register_user_button_ = createButton(true, "Register user");
  register_user_button_->addTouchEventListener(
      [this, auth](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            this->logMessage("Registering user...");
            const char* email = email_text_field_->getString().c_str();
            const char* password = password_text_field_->getString().c_str();
            this->create_user_future_ =
                auth->CreateUserWithEmailAndPassword(email, password);
            this->register_user_button_->setEnabled(false);
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(register_user_button_);

  credentialed_sign_in_button_ = createButton(true, "Sign in");
  credentialed_sign_in_button_->addTouchEventListener(
      [this, auth](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            this->logMessage("Signing in...");
            const char* email = email_text_field_->getString().c_str();
            const char* password = password_text_field_->getString().c_str();
            Credential email_cred =
                EmailAuthProvider::GetCredential(email, password);
            this->sign_in_future_ = auth->SignInWithCredential(email_cred);
            this->credentialed_sign_in_button_->setEnabled(false);
            this->anonymous_sign_in_button_->setEnabled(false);
            this->sign_out_button_->setEnabled(true);
            this->anonymous_sign_in_ = false;
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(credentialed_sign_in_button_);

  anonymous_sign_in_button_ = createButton(true, "Sign in anonymously");
  anonymous_sign_in_button_->addTouchEventListener(
      [this, auth](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            this->logMessage("Signing in anonymously...");
            // Anonymous sign in must be enabled in the Firebase Console.
            this->sign_in_future_ = auth->SignInAnonymously();
            this->credentialed_sign_in_button_->setEnabled(false);
            this->anonymous_sign_in_button_->setEnabled(false);
            this->sign_out_button_->setEnabled(true);
            this->anonymous_sign_in_ = true;
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(anonymous_sign_in_button_);

  sign_out_button_ = createButton(false, "Sign out");
  sign_out_button_->addTouchEventListener(
      [this, auth](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            this->logMessage("Signed out");
            auth->SignOut();
            this->credentialed_sign_in_button_->setEnabled(true);
            this->anonymous_sign_in_button_->setEnabled(true);
            this->sign_out_button_->setEnabled(false);
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(sign_out_button_);

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
void FirebaseAuthScene::update(float /*delta*/) {
  using firebase::auth::AuthError;
  if (create_user_future_.status() == firebase::kFutureStatusComplete) {
    const AuthError error = static_cast<AuthError>(create_user_future_.error());
    if (error == firebase::auth::kAuthErrorNone) {
      logMessage("Created new user successfully.");
    } else {
      logMessage("ERROR: User creation failed: %d, `%s`", error,
                 sign_in_future_.error_message());
    }
    this->register_user_button_->setEnabled(true);
    create_user_future_.Release();
  }
  if (sign_in_future_.status() == firebase::kFutureStatusComplete) {
    const AuthError error = static_cast<AuthError>(sign_in_future_.error());
    if (error == firebase::auth::kAuthErrorNone) {
      logMessage("Signed in successfully.");
    } else {
      logMessage("ERROR: Sign in failed: %d, `%s`", error,
                 sign_in_future_.error_message());
      if (this->anonymous_sign_in_) {
        logMessage("You may need to enable anonymous login in the Firebase "
                   "Console.");
        logMessage("(In the console, navigate to Authentication > "
                   "Sign-in Method > Anonymous and click Enable)");
      }
      this->credentialed_sign_in_button_->setEnabled(true);
      this->anonymous_sign_in_button_->setEnabled(true);
      this->sign_out_button_->setEnabled(false);
    }
    sign_in_future_.Release();
  }
}

/// Handles the user tapping on the close app menu item.
void FirebaseAuthScene::menuCloseAppCallback(Ref* pSender) {
  CCLOG("Cleaning up Auth C++ resources.");

  // Close the cocos2d-x game scene and quit the application.
  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}
