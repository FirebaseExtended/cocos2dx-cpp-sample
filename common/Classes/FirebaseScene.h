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

#ifndef FIREBASE_COCOS_CLASSES_FIREBASE_SCENE_H_
#define FIREBASE_COCOS_CLASSES_FIREBASE_SCENE_H_

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "FirebaseCocos.h"

/// This factory method creates a FirebaseScene. The implementation is provided
/// by each sample project so that it will always return the FirebaseScene
/// associated with that sample.
cocos2d::Scene* CreateFirebaseScene();

class FirebaseScene : public cocos2d::Layer {
 public:
  /// Initialize the scene. This must be called before the scene can be
  /// interacted with or updated.
  ///
  /// @return True if the scene was initialized successfully.
  virtual bool init() = 0;

  /// Update the scene. This is run once per frame.
  ///
  /// @param delta The number of seconds since the last update.
  virtual void update(float delta) = 0;

  /// Called just before the app closes. This is where cleanup and shutdown 
  /// logic should go.
  virtual void menuCloseAppCallback(cocos2d::Ref* pSender) = 0;

  /// Updates the log text in the Firebase scene's TextWidget.
  void logMessage(std::string format, ...);

 protected:
  /// The Y position for a UI element.
  float nextYPosition;

  /// Creates a button for interacting with Firebase.
  cocos2d::ui::Button *createButton(
      bool buttonEnabled, const std::string& buttonTitleText,
      const cocos2d::Color3B& buttonColor);

  /// Creates a button for interacting with Firebase.
  cocos2d::ui::Button *createButton(
      bool buttonEnabled, const std::string& buttonTitleText);

  /// Creates the ScrollView that contains a TextWidget for displaying log text
  /// to the user.
  void createScrollView(float yPosition);
};

#endif  // FIREBASE_COCOS_CLASSES_FIREBASE_SCENE_H_
