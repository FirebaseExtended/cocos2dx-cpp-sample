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
