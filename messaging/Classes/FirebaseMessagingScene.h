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

#ifndef FIREBASE_COCOS_CLASSES_FIREBASE_MESSAGING_SCENE_H_
#define FIREBASE_COCOS_CLASSES_FIREBASE_MESSAGING_SCENE_H_

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "FirebaseCocos.h"
#include "FirebaseScene.h"

class FirebaseMessagingScene : public FirebaseScene {
 public:
  static cocos2d::Scene *createScene();

  bool init() override;

  void update(float delta) override;

  void menuCloseAppCallback(cocos2d::Ref *pSender) override;

  CREATE_FUNC(FirebaseMessagingScene);
};

#endif  // FIREBASE_COCOS_CLASSES_FIREBASE_MESSAGING_SCENE_H_
