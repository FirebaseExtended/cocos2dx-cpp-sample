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

#ifndef FIREBASE_COCOS_CLASSES_FIREBASE_DATABASE_SCENE_H_
#define FIREBASE_COCOS_CLASSES_FIREBASE_DATABASE_SCENE_H_

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "FirebaseCocos.h"
#include "FirebaseScene.h"
#include "firebase/auth.h"
#include "firebase/database.h"
#include "firebase/future.h"
#include "firebase/util.h"

class FirebaseDatabaseScene : public FirebaseScene {
 public:
  static cocos2d::Scene *createScene();

  bool init() override;

  void update(float delta) override;

  void menuCloseAppCallback(cocos2d::Ref *pSender) override;

  CREATE_FUNC(FirebaseDatabaseScene);

 private:
  /// An enum that tracks what the operations the app should be keeping track
  /// of. First the app initializes the library, then it logs in using Firebase
  /// Authentication, and then it listens for reads or writes to the database.
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

  /// Firebase Realtime Database, the entry point to all database operations.
  firebase::database::Database* database_;

  /// A future that completes when the DataSnapshot for a query is received.
  firebase::Future<firebase::database::DataSnapshot> query_future_;

  /// A future that completes when a databse write is complete.
  firebase::Future<void> set_future_;

  /// A text field where a database key string may be entered.
  cocos2d::ui::TextField* key_text_field_;

  /// A text field where a database value string may be entered.
  cocos2d::ui::TextField* value_text_field_;

  /// A button that sets a listener on the node given by the key text field.
  cocos2d::ui::Button* add_listener_button_;

  /// A button that queries the value given by the key text field.
  cocos2d::ui::Button* query_button_;

  /// A button that sets the key to the value, given by the text fields.
  cocos2d::ui::Button* set_button_;
};

#endif  // FIREBASE_COCOS_CLASSES_FIREBASE_DATABASE_SCENE_H_
