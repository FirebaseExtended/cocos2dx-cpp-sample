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

#ifndef FIREBASE_COCOS_CLASSES_FIREBASE_INVITES_SCENE_H_
#define FIREBASE_COCOS_CLASSES_FIREBASE_INVITES_SCENE_H_

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "FirebaseCocos.h"
#include "FirebaseScene.h"

class FirebaseInvitesScene : public FirebaseScene {
 public:
  FirebaseInvitesScene() : invite_button_(nullptr), invite_sent_(false) {}

  static cocos2d::Scene *createScene();

  bool init() override;

  void update(float delta) override;

  void menuCloseAppCallback(cocos2d::Ref *pSender) override;

  CREATE_FUNC(FirebaseInvitesScene);

 private:
  /// The button for loading an ad view.
  cocos2d::ui::Button* invite_button_;

  /// Keep track of whether an invite has been sent.
  bool invite_sent_;
};

#endif  // FIREBASE_COCOS_CLASSES_FIREBASE_INVITES_SCENE_H_
