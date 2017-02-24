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

  /// Creates a single line text entry field.
  cocos2d::ui::TextField *createTextField(const char* placeholder);

  /// Creates the ScrollView that contains a TextWidget for displaying log text
  /// to the user.
  ///
  /// By default the it will be half the width of the parent widget, but that
  /// can be adjusted by supplying a width argument.
  void createScrollView(float yPosition);
  void createScrollView(float yPosition, float widthFraction);
};

#endif  // FIREBASE_COCOS_CLASSES_FIREBASE_SCENE_H_
