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

class FirebaseScene : public cocos2d::Layer {
 public:
  static cocos2d::Scene *createScene();

  virtual bool init() = 0;

  virtual void update(float delta) = 0;

  virtual void menuCloseAppCallback(cocos2d::Ref* pSender) = 0;

  /// Updates the log text in the Firebase scene's TextWidget.
  void logMessage(std::string format, ...);

 protected:
  /// The Y position for a UI element.
  float nextYPosition;

  /// Creates a button for interacting with Firebase.
  cocos2d::ui::Button *createButton(bool btnEnabled, std::string btnTitleText);

  /// Creates the ScrollView that contains a TextWidget for displaying log text
  /// to the user.
  void createScrollView(float yPosition);
};

#endif  // FIREBASE_COCOS_CLASSES_FIREBASE_SCENE_H_
