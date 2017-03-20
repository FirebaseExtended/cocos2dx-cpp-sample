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

#include "FirebaseStorageScene.h"

#include <stdarg.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <android/log.h>
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#endif

#include "FirebaseCocos.h"
#include "firebase/auth.h"
#include "firebase/storage.h"
#include "firebase/future.h"

USING_NS_CC;

/// Padding for the UI elements.
static const float kUIElementPadding = 10.0;

/// Placeholder labels for the text fields.
static const char* kKeyPlaceholderText = "Key";
static const char* kValuePlaceholderText = "Value";

static const char* kTestAppData = "test_app_data";

void StorageListener::OnPaused(firebase::storage::Controller*) {}

void StorageListener::OnProgress(firebase::storage::Controller* controller) {
  int transferred = controller->bytes_transferred();
  int total = controller->total_byte_count();
  if (total > 0) {
    int percent = 100 * transferred / total;
    scene_->logMessage("Transfer %i%% (%i/%i)", percent, transferred, total);
  }
}

/// Creates the Firebase scene.
Scene* CreateFirebaseScene() {
  return FirebaseStorageScene::createScene();
}

/// Creates the FirebaseStorageScene.
Scene* FirebaseStorageScene::createScene() {
  // Create the scene.
  auto scene = Scene::create();

  // Create the layer.
  auto layer = FirebaseStorageScene::create();

  // Add the layer to the scene.
  scene->addChild(layer);

  return scene;
}

/// Initializes the FirebaseScene.
bool FirebaseStorageScene::init() {
  if (!Layer::init()) {
    return false;
  }

  listener_.set_scene(this);

  auto visibleSize = Director::getInstance()->getVisibleSize();
  cocos2d::Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // Create the Firebase label.
  auto firebaseLabel =
      Label::createWithTTF("Firebase Storage", "fonts/Marker Felt.ttf", 20);
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

  // Use ModuleInitializer to initialize both Auth and Storage, ensuring no
  // dependencies are missing.
  void* initialize_targets[] = {&auth_, &storage_};

  CCLOG("Initializing the Storage with Firebase API.");
  const firebase::ModuleInitializer::InitializerFn initializers[] = {
      [](firebase::App* app, void* data) {
        void** targets = reinterpret_cast<void**>(data);
        firebase::InitResult result;
        *reinterpret_cast<firebase::auth::Auth**>(targets[0]) =
            firebase::auth::Auth::GetAuth(app, &result);
        return result;
      },
      [](firebase::App* app, void* data) {
        void** targets = reinterpret_cast<void**>(data);
        firebase::InitResult result;
        *reinterpret_cast<firebase::storage::Storage**>(targets[1]) =
            firebase::storage::Storage::GetInstance(app, &result);
        return result;
      }};

  // There are two ways to track long running operations: (1) retrieve the
  // future using a LastResult function or (2) Cache the future manually.
  //
  // Here we use method 1: the future is not cached but will be later retrieved
  // using InitializeLastResult. Which method is best for your app depends on
  // your use case.
  initializer_.Initialize(
      firebase::App::GetInstance(), initialize_targets, initializers,
      sizeof(initializers) / sizeof(initializers[0]));

  logMessage("Created the Storage %x class for the Firebase app.",
             static_cast<int>(reinterpret_cast<intptr_t>(storage_)));

  key_text_field_ = createTextField(kKeyPlaceholderText);
  this->addChild(key_text_field_);

  value_text_field_ = createTextField(kValuePlaceholderText);
  this->addChild(value_text_field_);

  get_bytes_button_ = createButton(false, "Query");
  get_bytes_button_->addTouchEventListener(
      [this](Ref* /*sender*/, cocos2d::ui::Widget::TouchEventType type) {
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            const char* key = key_text_field_->getString().c_str();
            firebase::storage::StorageReference reference =
                this->storage_->GetReference(kTestAppData).Child(key);
            this->logMessage("Querying key `%s`.", key);
            // There are two ways to track long running operations:
            // (1) retrieve the future using a LastResult function or (2) Cache
            // the future manually.
            //
            // Here (and below in the put_bytes_button_) we use method 2:
            // caching the future. Which method is best for your app depends on
            // your use case.
            get_bytes_future_ = reference.GetBytes(
                this->byte_buffer_, kBufferSize, &this->listener_);
            this->get_bytes_button_->setEnabled(false);
            this->put_bytes_button_->setEnabled(false);
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(get_bytes_button_);

  put_bytes_button_ = createButton(false, "Set");
  put_bytes_button_->addTouchEventListener(
      [this](Ref* /*sender*/, cocos2d::ui::Widget::TouchEventType type) {
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            const char* key = key_text_field_->getString().c_str();
            const char* value = value_text_field_->getString().c_str();
            size_t value_size = value_text_field_->getString().size();
            firebase::storage::StorageReference reference =
                this->storage_->GetReference(kTestAppData).Child(key);
            this->logMessage("Setting key `%s` to `%s`.", key, value);
            this->put_bytes_future_ = reference.PutBytes(value, value_size,
                                                         &this->listener_);
            this->get_bytes_button_->setEnabled(false);
            this->put_bytes_button_->setEnabled(false);
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(put_bytes_button_);

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

FirebaseStorageScene::State FirebaseStorageScene::updateInitialize() {
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
  logMessage("Successfully initialized Firebase Auth and Firebase Storage.");
  auth_->SignInAnonymously();
  return kStateLogin;
}

FirebaseStorageScene::State FirebaseStorageScene::updateLogin() {
  // Sign in using Auth before accessing the storage.
  //
  // The default Storage permissions allow anonymous user access. However,
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
  get_bytes_button_->setEnabled(true);
  put_bytes_button_->setEnabled(true);
  return kStateRun;
}

FirebaseStorageScene::State FirebaseStorageScene::updateRun() {
  if (get_bytes_future_.status() == firebase::kFutureStatusComplete) {
    if (get_bytes_future_.error() == firebase::storage::kErrorNone) {
      logMessage("GetBytes complete");
      const size_t* length = get_bytes_future_.result();
      logMessage("Got %i bytes: %s", static_cast<int>(*length), byte_buffer_);
    } else {
      logMessage("ERROR: Could not get bytes. Error %d: %s",
                 get_bytes_future_.error(), get_bytes_future_.error_message());
    }
    get_bytes_button_->setEnabled(true);
    put_bytes_button_->setEnabled(true);
    get_bytes_future_.Release();
  }
  if (put_bytes_future_.status() == firebase::kFutureStatusComplete) {
    if (put_bytes_future_.error() == firebase::storage::kErrorNone) {
      logMessage("PutBytes complete.");
      const firebase::storage::Metadata* metadata = put_bytes_future_.result();
      logMessage("Put %i bytes", static_cast<int>(metadata->size_bytes()));
    } else {
      logMessage("ERROR: Could not put bytes. Error %d: %s",
                 put_bytes_future_.error(), put_bytes_future_.error_message());
    }
    get_bytes_button_->setEnabled(true);
    put_bytes_button_->setEnabled(true);
    put_bytes_future_.Release();
  }
  return kStateRun;
}

// Called automatically every frame. The update is scheduled in `init()`.
void FirebaseStorageScene::update(float /*delta*/) {
  switch (state_) {
    case kStateInitialize: state_ = updateInitialize(); break;
    case kStateLogin: state_ = updateLogin(); break;
    case kStateRun: state_ = updateRun(); break;
    default: assert(0);
  }
}

/// Handles the user tapping on the close app menu item.
void FirebaseStorageScene::menuCloseAppCallback(Ref* pSender) {
  CCLOG("Cleaning up Storage C++ resources.");

  // Close the cocos2d-x game scene and quit the application.
  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}
