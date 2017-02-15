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

#ifndef FIREBASE_COCOS_CLASSES_FIREBASE_ANALYTICS_SCENE_H_
#define FIREBASE_COCOS_CLASSES_FIREBASE_ANALYTICS_SCENE_H_

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "FirebaseCocos.h"
#include "FirebaseScene.h"

class FirebaseAnalyticsScene : public FirebaseScene {
 public:
  static cocos2d::Scene *createScene();

  FirebaseAnalyticsScene() :
      blue_button_click_count_(0),
      total_button_click_count_(0),
      previous_button_clicked_("None"),
      green_button_click_count_(0) {}

  bool init() override;

  void update(float delta) override;

  void menuCloseAppCallback(cocos2d::Ref *pSender) override;

  CREATE_FUNC(FirebaseAnalyticsScene);

 private:
  /// A blue button that records how many times it has been clicked.
  cocos2d::ui::Button* blue_button_;

  /// A red button that records how many times any button has been clicked.
  cocos2d::ui::Button* red_button_;

  /// A yellow button that records what button was clicked previously.
  cocos2d::ui::Button* yellow_button_;

  /// A green button that records what ratio of button clicks were green.
  cocos2d::ui::Button* green_button_;

  // The following are some arbitrary statistics to collect to demonstrate how
  // they can be reported to Firebase Analytics.

  /// The total number of times the blue button has been clicked.
  int blue_button_click_count_;

  /// The total number of times any button has been clicked.
  int total_button_click_count_;

  /// The button that was clicked previously.
  const char* previous_button_clicked_;

  /// What fraction of the clicks were made on the green button.
  int green_button_click_count_;
};

#endif  // FIREBASE_COCOS_CLASSES_FIREBASE_ANALYTICS_SCENE_H_
