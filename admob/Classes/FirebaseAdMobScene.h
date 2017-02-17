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
