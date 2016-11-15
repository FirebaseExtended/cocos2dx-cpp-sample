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

#ifndef FIREBASE_COCOS_CLASSES_FIREBASE_ADMOB_SCENE_H_
#define FIREBASE_COCOS_CLASSES_FIREBASE_ADMOB_SCENE_H_

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "FirebaseCocos.h"
#include "FirebaseScene.h"

#include "firebase/admob.h"
#include "firebase/admob/banner_view.h"
#include "firebase/admob/interstitial_ad.h"
#include "firebase/admob/rewarded_video.h"
#include "firebase/admob/types.h"
#include "firebase/app.h"
#include "firebase/future.h"

class LoggingAdViewListener;
class LoggingInterstitialAdListener;
class LoggingRewardedVideoListener;

class FirebaseAdMobScene : public FirebaseScene {
 public:
  static cocos2d::Scene *createScene();

  bool init() override;

  void update(float delta) override;

  void menuCloseAppCallback(cocos2d::Ref *pSender) override;

  CREATE_FUNC(FirebaseAdMobScene);

 private:
  /// The AdMob ad view C++ object.
  firebase::admob::BannerView* adView;

  /// The AdMob InterstitialAd C++ object.
  firebase::admob::InterstitialAd* interstitialAd;

  /// The button for loading an ad view.
  cocos2d::ui::Button* loadAdViewBtn;

  /// The button for showing and hiding the ad view.
  cocos2d::ui::Button* showHideAdViewBtn;

  /// The button for moving the ad view.
  cocos2d::ui::Button* moveAdViewBtn;

  /// The button for loading an InterstitialAd.
  cocos2d::ui::Button* loadInterstitialAdBtn;

  /// The button for showing the InterstitialAd.
  cocos2d::ui::Button* showInterstitialAdBtn;

  /// The button for loading a rewarded video.
  cocos2d::ui::Button* loadRewardedVideoBtn;

  /// The button for showing the rewarded video.
  cocos2d::ui::Button* showRewardedVideoBtn;

  /// The ad view listener.
  LoggingAdViewListener* adViewListener;

  /// The admob::InterstitialAd listener.
  LoggingInterstitialAdListener* interstitialAdListener;

  /// The admob::rewarded_video listener.
  LoggingRewardedVideoListener* rewardedVideoListener;

  /// Returns an AdMob AdRequest.
  firebase::admob::AdRequest createAdRequest();
};

#endif  // FIREBASE_COCOS_CLASSES_FIREBASE_ADMOB_SCENE_H_
