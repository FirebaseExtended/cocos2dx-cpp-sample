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

#ifndef FIREBASE_COCOS_CLASSES_FIREBASE_STORAGE_SCENE_H_
#define FIREBASE_COCOS_CLASSES_FIREBASE_STORAGE_SCENE_H_

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "FirebaseCocos.h"
#include "FirebaseScene.h"
#include "firebase/auth.h"
#include "firebase/storage.h"
#include "firebase/future.h"
#include "firebase/util.h"

// The size of the byte buffer that data will be written to.
static const size_t kBufferSize = 1024;

class StorageListener : public firebase::storage::Listener {
 public:
  virtual ~StorageListener() {}

  /// Called whenever a transferred is paused.
  void OnPaused(firebase::storage::Controller* controller) override;

  /// Called repeatedly as a transfer is in progress.
  void OnProgress(firebase::storage::Controller* controller) override;

  void set_scene(FirebaseScene* scene) { scene_ = scene; }

 private:
  FirebaseScene* scene_;
};

class FirebaseStorageScene : public FirebaseScene {
 public:
  static cocos2d::Scene *createScene();

  bool init() override;

  void update(float delta) override;

  void menuCloseAppCallback(cocos2d::Ref *pSender) override;

  CREATE_FUNC(FirebaseStorageScene);

 private:
  /// An enum that tracks what the operations the app should be keeping track
  /// of. First the app initializes the library, then it logs in using Firebase
  /// Authentication, and then it listens for reads or writes to the storage.
  enum State {
    kStateInitialize,
    kStateLogin,
    kStateRun,
  };

  /// The update loop to run while initializing the app.
  State updateInitialize();

  /// The update loop to run while logging in.
  State updateLogin();

  /// The update loop to run once all setup is complete.
  State updateRun();

  /// Tracks the current state of the app through its setup and main loop.
  State state_;

  /// The ModuleInitializer is a utility class to make initializing multiple
  /// Firebase libraries easier.
  firebase::ModuleInitializer initializer_;

  /// Firebase Auth, used for logging into Firebase.
  firebase::auth::Auth* auth_;

  /// Firebase Storage, the entry point to all storage operations.
  firebase::storage::Storage* storage_;

  /// A byte buffer for Firebase Storage to write to.
  char byte_buffer_[kBufferSize];

  /// A listener that responds to PutBytes and GetBytes progress.
  StorageListener listener_;

  /// A future that completes when the DataSnapshot for a query is received.
  firebase::Future<size_t> get_bytes_future_;

  /// A future that completes when a databse write is complete.
  firebase::Future<firebase::storage::Metadata> put_bytes_future_;

  /// A text field where a storage key string may be entered.
  cocos2d::ui::TextField* key_text_field_;

  /// A text field where a storage value string may be entered.
  cocos2d::ui::TextField* value_text_field_;

  /// A button that queries the value given by the key text field.
  cocos2d::ui::Button* get_bytes_button_;

  /// A button that sets the key to the value, given by the text fields.
  cocos2d::ui::Button* put_bytes_button_;
};

#endif  // FIREBASE_COCOS_CLASSES_FIREBASE_STORAGE_SCENE_H_
