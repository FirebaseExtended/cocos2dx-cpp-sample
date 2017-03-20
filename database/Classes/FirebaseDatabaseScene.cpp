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

#include "FirebaseDatabaseScene.h"

#include <stdarg.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <android/log.h>
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#endif

#include "FirebaseCocos.h"
#include "firebase/auth.h"
#include "firebase/database.h"
#include "firebase/future.h"

USING_NS_CC;

/// Padding for the UI elements.
static const float kUIElementPadding = 10.0;

/// Placeholder labels for the text fields.
static const char* kKeyPlaceholderText = "Key";
static const char* kValuePlaceholderText = "Value";

static const char* kTestAppData = "test_app_data";

static void logDataSnapshot(FirebaseDatabaseScene* scene,
                            const firebase::database::DataSnapshot& snapshot) {
  const firebase::Variant& value_variant = snapshot.value();
  const char* key = snapshot.key();
  switch (value_variant.type()) {
    case firebase::Variant::kTypeNull: {
      scene->logMessage("key: `%s`, value: null", key);
      break;
    }
    case firebase::Variant::kTypeInt64: {
      int64_t value = value_variant.int64_value();
      scene->logMessage("key: \"%s\", value: %lli", key, value);
      break;
    }
    case firebase::Variant::kTypeDouble: {
      double value = value_variant.double_value();
      scene->logMessage("key: \"%s\", value: %f", key, value);
      break;
    }
    case firebase::Variant::kTypeBool: {
      bool value = value_variant.bool_value();
      scene->logMessage("key: \"%s\", value: %s", key,
                         value ? "true" : "false");
      break;
    }
    case firebase::Variant::kTypeMutableString:
    case firebase::Variant::kTypeStaticString: {
      const char* value = value_variant.string_value();
      scene->logMessage("key: \"%s\", value: \"%s\"", key,
                         value ? value : "<null>");
      break;
    }
    default: {
      scene->logMessage("ERROR: This sample only supports scalar variants.");
    }
  }
}

// An example of a ValueListener object. This specific version will
// simply log every value it sees, and store them in a list so we can
// confirm that all values were received.
class SampleValueListener : public firebase::database::ValueListener {
 public:
  SampleValueListener(FirebaseDatabaseScene* scene) : scene_(scene) {};

  void OnValueChanged(
      const firebase::database::DataSnapshot& snapshot) override {
    scene_->logMessage("ValueListener::OnValueChanged");
    logDataSnapshot(scene_, snapshot);
  }

  void OnCancelled(const firebase::database::Error& error_code,
                   const char* error_message) override {
    scene_->logMessage("ERROR: SampleValueListener canceled: %d: %s",
                       error_code, error_message);
  }

 private:
  FirebaseDatabaseScene* scene_;
};

/// Creates the Firebase scene.
Scene* CreateFirebaseScene() {
  return FirebaseDatabaseScene::createScene();
}

/// Creates the FirebaseDatabaseScene.
Scene* FirebaseDatabaseScene::createScene() {
  // Create the scene.
  auto scene = Scene::create();

  // Create the layer.
  auto layer = FirebaseDatabaseScene::create();

  // Add the layer to the scene.
  scene->addChild(layer);

  return scene;
}

/// Initializes the FirebaseScene.
bool FirebaseDatabaseScene::init() {
  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  cocos2d::Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // Create the Firebase label.
  auto firebaseLabel =
      Label::createWithTTF("Firebase Database", "fonts/Marker Felt.ttf", 20);
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

  // Use ModuleInitializer to initialize both Auth and Database, ensuring no
  // dependencies are missing.
  void* initialize_targets[] = {&auth_, &database_};

  CCLOG("Initializing the Database with Firebase API.");
  const firebase::ModuleInitializer::InitializerFn initializers[] = {
      [](firebase::App* app, void* data) {
        // this->logMessage("Attempt to initialize Firebase Auth.");
        void** targets = reinterpret_cast<void**>(data);
        firebase::InitResult result;
        *reinterpret_cast<firebase::auth::Auth**>(targets[0]) =
            firebase::auth::Auth::GetAuth(app, &result);
        return result;
      },
      [](firebase::App* app, void* data) {
        // this->logMessage("Attempt to initialize Firebase Database.");
        void** targets = reinterpret_cast<void**>(data);
        firebase::InitResult result;
        *reinterpret_cast<firebase::database::Database**>(targets[1]) =
            firebase::database::Database::GetInstance(app, &result);
        return result;
      }};

  // There are two ways to track long running operations: (1) retrieve the
  // future using a LastResult function or (2) Cache the future manually.
  //
  // Here we use method 1: the future is not cached but will be later retrieved
  // using SetValueLastResult. Which method is best for your app depends on
  // your use case.
  initializer_.Initialize(
      firebase::App::GetInstance(), initialize_targets, initializers,
      sizeof(initializers) / sizeof(initializers[0]));

  logMessage("Created the Database %x class for the Firebase app.",
             static_cast<int>(reinterpret_cast<intptr_t>(database_)));

  key_text_field_ = createTextField(kKeyPlaceholderText);
  this->addChild(key_text_field_);

  value_text_field_ = createTextField(kValuePlaceholderText);
  this->addChild(value_text_field_);

  add_listener_button_ = createButton(false, "Add Listener");
  add_listener_button_->addTouchEventListener(
      [this](Ref* /*sender*/, cocos2d::ui::Widget::TouchEventType type) {
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            const char* key = key_text_field_->getString().c_str();
            firebase::database::DatabaseReference reference =
                this->database_->GetReference(kTestAppData).Child(key);
            this->logMessage("Adding ValueListener to key `%s`.", key);
            // The SampleValueListener will respond to changes in this entry's
            // value. Changes can be made by other instances of this sample app
            // or in the Firebase Console.
            reference.AddValueListener(new SampleValueListener(this));
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(add_listener_button_);

  query_button_ = createButton(false, "Query");
  query_button_->addTouchEventListener(
      [this](Ref* /*sender*/, cocos2d::ui::Widget::TouchEventType type) {
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            const char* key = key_text_field_->getString().c_str();
            firebase::database::DatabaseReference reference =
                this->database_->GetReference(kTestAppData).Child(key);
            this->logMessage("Querying key `%s`.", key);
            // There are two ways to track long running operations:
            // (1) retrieve the future using a LastResult function or (2) Cache
            // the future manually.
            //
            // Here (and below in the set_button_) we use method 2: caching the
            // future. Which method is best for your app depends on your use
            // case.
            query_future_ = reference.GetValue();
            this->query_button_->setEnabled(false);
            this->set_button_->setEnabled(false);
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(query_button_);

  set_button_ = createButton(false, "Set");
  set_button_->addTouchEventListener(
      [this](Ref* /*sender*/, cocos2d::ui::Widget::TouchEventType type) {
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            const char* key = key_text_field_->getString().c_str();
            const char* value = value_text_field_->getString().c_str();
            firebase::database::DatabaseReference reference =
                this->database_->GetReference(kTestAppData).Child(key);
            this->logMessage("Setting key `%s` to `%s`.", key, value);
            this->set_future_ = reference.SetValue(value);
            this->query_button_->setEnabled(false);
            this->set_button_->setEnabled(false);
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(set_button_);

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

  state_ = kStateInitialize;

  // Schedule the update method for this scene.
  this->scheduleUpdate();

  return true;
}

FirebaseDatabaseScene::State FirebaseDatabaseScene::updateInitialize() {
  firebase::Future<void> initialize_future =
      initializer_.InitializeLastResult();
  if (initialize_future.status() != firebase::kFutureStatusComplete) {
    return kStateInitialize;
  }
  if (initialize_future.error() != 0) {
    logMessage("Failed to initialize Firebase libraries: %s",
               initialize_future.error_message());
    return kStateRun;
  }
  logMessage("Successfully initialized Firebase Auth and Firebase Database.");
  auth_->SignInAnonymously();
  return kStateLogin;
}

FirebaseDatabaseScene::State FirebaseDatabaseScene::updateLogin() {
  // Sign in using Auth before accessing the database.
  //
  // The default Database permissions allow anonymous user access. However,
  // Firebase Auth does not allow anonymous user login by default. This setting
  // can be changed in the Auth settings page for your project in the Firebase
  // Console under the "Sign-In Method" tab.
  firebase::Future<firebase::auth::User*> sign_in_future =
      auth_->SignInAnonymouslyLastResult();
  if (sign_in_future.status() != firebase::kFutureStatusComplete) {
    return kStateLogin;
  }
  if (sign_in_future.error() != firebase::auth::kAuthErrorNone) {
    logMessage("ERROR: Could not sign in anonymously. Error %d: %s",
               sign_in_future.error(), sign_in_future.error_message());
    logMessage(
        "Ensure your application has the Anonymous sign-in provider enabled in "
        "the Firebase Console.");
    return kStateRun;
  }
  logMessage("Auth: Signed in anonymously.");
  add_listener_button_->setEnabled(true);
  query_button_->setEnabled(true);
  set_button_->setEnabled(true);
  return kStateRun;
}

FirebaseDatabaseScene::State FirebaseDatabaseScene::updateRun() {
  if (query_future_.status() == firebase::kFutureStatusComplete) {
    if (query_future_.error() == firebase::database::kErrorNone) {
      logMessage("Query complete");
      const firebase::database::DataSnapshot* snapshot = query_future_.result();
      logDataSnapshot(this, *snapshot);
    } else {
      logMessage("ERROR: Could not query value. Error %d: %s",
                 query_future_.error(), query_future_.error_message());
    }
    query_button_->setEnabled(true);
    set_button_->setEnabled(true);
    query_future_.Release();
  }
  if (set_future_.status() == firebase::kFutureStatusComplete) {
    if (set_future_.error() == firebase::database::kErrorNone) {
      logMessage("Database updated.");
    } else {
      logMessage("ERROR: Could not set value. Error %d: %s",
                 set_future_.error(), set_future_.error_message());
    }
    query_button_->setEnabled(true);
    set_button_->setEnabled(true);
    set_future_.Release();
  }
  return kStateRun;
}

// Called automatically every frame. The update is scheduled in `init()`.
void FirebaseDatabaseScene::update(float /*delta*/) {
  switch (state_) {
    case kStateInitialize: state_ = updateInitialize(); break;
    case kStateLogin: state_ = updateLogin(); break;
    case kStateRun: state_ = updateRun(); break;
    default: assert(0);
  }
}

/// Handles the user tapping on the close app menu item.
void FirebaseDatabaseScene::menuCloseAppCallback(Ref* pSender) {
  CCLOG("Cleaning up Database C++ resources.");

  // Close the cocos2d-x game scene and quit the application.
  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}
