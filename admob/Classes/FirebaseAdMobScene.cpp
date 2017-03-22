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

#include "FirebaseAdMobScene.h"

#include <stdarg.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <android/log.h>
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#endif

#include "FirebaseCocos.h"
#include "firebase/admob.h"

USING_NS_CC;

namespace rewarded_video = firebase::admob::rewarded_video;

bool interstitialAdShown = false;
bool rewardedVideoAdShown = false;

/// Padding for the UI elements.
static const float kUIElementPadding = 10.0;

// The title text for the Firebase buttons.
static const std::string kLoadAdViewText = "Load Banner";
static const std::string kShowAdViewText = "Show Banner";
static const std::string kHideAdViewText = "Hide Banner";
static const std::string kMoveAdViewText = "Move Banner";
static const std::string kLoadInterstitialText = "Load Interstitial";
static const std::string kShowInterstitialText = "Show Interstitial";
static const std::string kLoadRewardedVideoText = "Load Rewarded Video";
static const std::string kShowRewardedVideoText = "Show Rewarded Video";

/// The possible screen positions for the ad view.
const int adViewPositions[6] = {
    firebase::admob::BannerView::kPositionTop,
    firebase::admob::BannerView::kPositionBottom,
    firebase::admob::BannerView::kPositionTopLeft,
    firebase::admob::BannerView::kPositionTopRight,
    firebase::admob::BannerView::kPositionBottomLeft,
    firebase::admob::BannerView::kPositionBottomRight};

/// The ad view screen positions mapped to std::strings.
const std::string adViewPositionStrings[6] = {"top center",  "bottom center",
                                              "top left",    "top right",
                                              "bottom left", "bottom right"};

/// The size of the adViewPositions array.
static const int adViewPositionsCount =
    sizeof(adViewPositions) / sizeof(adViewPositions[0]);

/// The ad view screen position index.
static int adViewPositionIndex = 0;

// The AdMob app IDs.
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
const char* kAdMobAppID = "ca-app-pub-3940256099942544~3347511713";
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
const char* kAdMobAppID = "ca-app-pub-3940256099942544~1458002511";
#else
const char* kAdMobAppID = "";
#endif

// These ad units are configured to always serve test ads.
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
const char* kAdViewAdUnit = "ca-app-pub-3940256099942544/6300978111";
const char* kInterstitialAdUnit = "ca-app-pub-3940256099942544/1033173712";
const char* kRewardedVideoAdUnit = "ca-app-pub-3940256099942544/2888167318";
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
const char* kAdViewAdUnit = "ca-app-pub-3940256099942544/2934735716";
const char* kInterstitialAdUnit = "ca-app-pub-3940256099942544/4411468910";
const char* kRewardedVideoAdUnit = "ca-app-pub-3940256099942544/6386090517";
#else
const char* kAdViewAdUnit = "";
const char* kInterstitialAdUnit = "";
const char* kRewardedVideoAdUnit = "";
#endif

// The ad view's ad size.
static const int kAdViewWidth = 320;
static const int kAdViewHeight = 50;

// Sample keywords to use in making the request.
static const char* kKeywords[] = {"AdMob", "C++", "Fun"};

// Sample test device IDs to use in making the request.
static const char* kTestDeviceIDs[] = {"2077ef9a63d2b398840261c8221a0c9b",
                                       "098fe087d987c9a878965454a65654d7"};

// Sample birthday value to use in making the request.
static const int kBirthdayDay = 10;
static const int kBirthdayMonth = 11;
static const int kBirthdayYear = 1976;

/// A simple listener that logs changes to an ad view.
class LoggingAdViewListener : public firebase::admob::BannerView::Listener {
 public:
  LoggingAdViewListener(FirebaseScene* scene) : scene(scene) {}
  void OnPresentationStateChanged(
      firebase::admob::BannerView* adView,
      firebase::admob::BannerView::PresentationState state) override {
    scene->logMessage("The ad view's PresentationState has changed to %d.",
                      state);
  }
  void OnBoundingBoxChanged(firebase::admob::BannerView* adView,
                            firebase::admob::BoundingBox box) override {
    scene->logMessage(
        "The ad view's BoundingBox has changed to (x: %d, y: %d, width: %d, "
        "height %d).",
        box.x, box.y, box.width, box.height);
  }

 private:
  FirebaseScene* scene;
};

/// A simple listener that logs changes to an InterstitialAd.
class LoggingInterstitialAdListener
    : public firebase::admob::InterstitialAd::Listener {
 public:
  LoggingInterstitialAdListener(FirebaseScene* scene) : scene(scene) {}
  void OnPresentationStateChanged(
      firebase::admob::InterstitialAd* interstitialAd,
      firebase::admob::InterstitialAd::PresentationState state) override {
    scene->logMessage("InterstitialAd PresentationState has changed to %d.",
                      state);
  }

 private:
  FirebaseScene* scene;
};

/// A simple listener that logs changes to rewarded video state.
class LoggingRewardedVideoListener : public rewarded_video::Listener {
 public:
  LoggingRewardedVideoListener(FirebaseScene* scene) : scene(scene) {}
  void OnRewarded(rewarded_video::RewardItem reward) override {
    scene->logMessage("Rewarding user with %f %s.", reward.amount,
                      reward.reward_type.c_str());
  }
  void OnPresentationStateChanged(
      rewarded_video::PresentationState state) override {
    scene->logMessage("Rewarded video PresentationState has changed to %d.",
                      state);
  }

 private:
  FirebaseScene* scene;
};

/// This function is called when the Future for the last call to the ad view's
/// Initialize() method completes.
static void onAdViewInitializeCompletionCallback(
    const firebase::Future<void>& future, void* userData) {
  FirebaseScene* scene = static_cast<FirebaseScene*>(userData);
  if (future.error() == firebase::admob::kAdMobErrorNone) {
    scene->logMessage("Initializing the ad view completed successfully.");
  } else {
    scene->logMessage("Initializing the ad view failed.");
    scene->logMessage(
        "ERROR: Action failed with error code %d and message \"%s\".",
        future.error(), future.error_message());
  }
}

/// This function is called when the Future for the last call to the ad view's
/// LoadAd() method completes.
static void onAdViewLoadAdCompletionCallback(
    const firebase::Future<void>& future, void* userData) {
  FirebaseScene* scene = static_cast<FirebaseScene*>(userData);
  if (future.error() == firebase::admob::kAdMobErrorNone) {
    scene->logMessage("Loading the ad view completed successfully.");
  } else {
    scene->logMessage("Loading the ad view failed.");
    scene->logMessage(
        "ERROR: Action failed with error code %d and message \"%s\".",
        future.error(), future.error_message());
  }
}

/// This function is called when the Future for the last call to the
/// InterstitialAds's Initialize() method completes.
static void onInterstitialAdInitializeCompletionCallback(
    const firebase::Future<void>& future, void* userData) {
  FirebaseScene* scene = static_cast<FirebaseScene*>(userData);
  if (future.error() == firebase::admob::kAdMobErrorNone) {
    scene->logMessage(
        "Initializing the interstitial ad completed successfully.");
  } else {
    scene->logMessage("Initializing the interstitial ad failed.");
    scene->logMessage(
        "ERROR: Action failed with error code %d and message \"%s\".",
        future.error(), future.error_message());
  }
}

/// This function is called when the Future for the last call to the
/// InterstitialAds's LoadAd() method completes.
static void onInterstitialAdLoadAdCompletionCallback(
    const firebase::Future<void>& future, void* userData) {
  FirebaseScene* scene = static_cast<FirebaseScene*>(userData);
  if (future.error() == firebase::admob::kAdMobErrorNone) {
    scene->logMessage("Loading the interstitial ad completed successfully.");
  } else {
    scene->logMessage("Loading the interstitial ad failed.");
    scene->logMessage(
        "ERROR: Action failed with error code %d and message \"%s\".",
        future.error(), future.error_message());
  }
}

/// This function is called when the Future for the last call to
/// rewarded_video::Initialize() method completes.
static void onRewardedVideoInitializeCompletionCallback(
    const firebase::Future<void>& future, void* userData) {
  FirebaseScene* scene = static_cast<FirebaseScene*>(userData);
  if (future.error() == firebase::admob::kAdMobErrorNone) {
    scene->logMessage("Initializing rewarded video completed successfully.");
  } else {
    scene->logMessage("Initializing rewarded video failed.");
    scene->logMessage(
        "ERROR: Action failed with error code %d and message \"%s\".",
        future.error(), future.error_message());
  }
}

/// This function is called when the Future for the last call to
/// rewarded_video::LoadAd() method completes.
static void onRewardedVideoLoadAdCompletionCallback(
    const firebase::Future<void>& future, void* userData) {
  FirebaseScene* scene = static_cast<FirebaseScene*>(userData);
  if (future.error() == firebase::admob::kAdMobErrorNone) {
    scene->logMessage("Loading rewarded video completed successfully.");
  } else {
    scene->logMessage("Loading rewarded video failed.");
    scene->logMessage(
        "ERROR: Action failed with error code %d and message \"%s\".",
        future.error(), future.error_message());
    // Rewarded Video returned an error. This might be because the developer did
    // not put their Rewarded Video ad unit into kRewardedVideoAdUnit above.
    scene->logMessage("WARNING: Is your Rewarded Video ad unit ID correct?");
    scene->logMessage(
        "Ensure kRewardedVideoAdUnit is set to your own Rewarded Video ad unit "
        "ID.");
  }
}

/// Creates the Firebase scene.
Scene* CreateFirebaseScene() {
  return FirebaseAdMobScene::createScene();
}

/// Creates the FirebaseAdMobScene.
Scene* FirebaseAdMobScene::createScene() {
  // Create the scene.
  auto scene = Scene::create();

  // Create the layer.
  auto layer = FirebaseAdMobScene::create();

  // Add the layer to the scene.
  scene->addChild(layer);

  return scene;
}

/// Initializes the FirebaseScene.
bool FirebaseAdMobScene::init() {
  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  cocos2d::Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // Intitialize Firebase-AdMob.
  CCLOG("Initializing the AdMob with Firebase API.");
  firebase::admob::Initialize(*firebase::App::GetInstance(), kAdMobAppID);

  // Create the AdMob ad listener classes.
  adViewListener = new LoggingAdViewListener(this);
  interstitialAdListener = new LoggingInterstitialAdListener(this);
  rewardedVideoListener = new LoggingRewardedVideoListener(this);

  // Create the Firebase label.
  auto firebaseLabel =
      Label::createWithTTF("Firebase-AdMob", "fonts/Marker Felt.ttf", 20);
  nextYPosition =
      origin.y + visibleSize.height - firebaseLabel->getContentSize().height;
  firebaseLabel->setPosition(
      cocos2d::Vec2(origin.x + visibleSize.width / 2, nextYPosition));
  this->addChild(firebaseLabel, 1);

  const float scrollViewYPosition = nextYPosition -
                                    firebaseLabel->getContentSize().height -
                                    kUIElementPadding * 2;
  // Create the ScrollView on the Cocos2d thread.
  cocos2d::Director::getInstance()
      ->getScheduler()
      ->performFunctionInCocosThread(
          [=]() { this->createScrollView(scrollViewYPosition); });

  // Create the AdMob ad view object.
  adView = new firebase::admob::BannerView();
  // Create an ad size for the ad view.
  firebase::admob::AdSize adSize;
  adSize.ad_size_type = firebase::admob::kAdSizeStandard;
  adSize.width = kAdViewWidth;
  adSize.height = kAdViewHeight;
  logMessage("Initializing the ad view.");
  adView->Initialize(getWindowContext(), kAdViewAdUnit, adSize);
  adView->InitializeLastResult().OnCompletion(
      onAdViewInitializeCompletionCallback, this);

  // Set up the load ad view button.
  loadAdViewBtn = createButton(true, kLoadAdViewText);
  loadAdViewBtn->addTouchEventListener(
      [&](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED:
            // This simple example only allows loading the ad view once. Once
            // the load ad view button is pressed, we disable it for the
            // remainder of time that the application is running. For your app,
            // you can initialize and load as many ad views as you would like.
            // Just remember that only one ad view can be shown on the screen at
            // a time.
            button->setEnabled(false);
            logMessage("Setting the ad view listener.");
            adView->SetListener(adViewListener);
            logMessage("Loading the ad view.");
            adView->LoadAd(createAdRequest());
            adView->LoadAdLastResult().OnCompletion(
                onAdViewLoadAdCompletionCallback, this);
            break;
          default:
            break;
        }
      });
  this->addChild(loadAdViewBtn);

  // Set up the show/hide ad view button.
  showHideAdViewBtn = createButton(false, kShowAdViewText);
  showHideAdViewBtn->addTouchEventListener(
      [&](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        std::string titleText = button->getTitleText();
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED:
            if (titleText.compare(kShowAdViewText) == 0) {
              logMessage("Showing the ad view.");
              adView->Show();
              button->setTitleText(kHideAdViewText);
            } else {
              logMessage("Hiding the ad view.");
              adView->Hide();
              button->setTitleText(kShowAdViewText);
            }
            break;
          default:
            break;
        }
      });
  this->addChild(showHideAdViewBtn);

  // Set up the move ad view button.
  moveAdViewBtn = createButton(false, kMoveAdViewText);
  moveAdViewBtn->addTouchEventListener(
      [&](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        firebase::admob::BannerView::Position adViewPosition =
            static_cast<firebase::admob::BannerView::Position>(
                adViewPositions[adViewPositionIndex]);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED:
            logMessage("Moving the ad view to %s.",
                       adViewPositionStrings[adViewPositionIndex].c_str());
            adView->MoveTo(adViewPosition);
            ++adViewPositionIndex;
            adViewPositionIndex = (adViewPositionIndex == adViewPositionsCount)
                                      ? 0
                                      : adViewPositionIndex;
            break;
          default:
            break;
        }
      });
  this->addChild(moveAdViewBtn);

  // Create the AdMob IntersitialAd object.
  interstitialAd = new firebase::admob::InterstitialAd();
  logMessage("Initializing the interstitial ad.");
  interstitialAd->Initialize(getWindowContext(), kInterstitialAdUnit);
  interstitialAd->InitializeLastResult().OnCompletion(
      onInterstitialAdInitializeCompletionCallback, this);

  // Set up the load interstitial ad button.
  loadInterstitialAdBtn = createButton(true, kLoadInterstitialText);
  loadInterstitialAdBtn->addTouchEventListener(
      [&](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED:
            // Once the load interstitial button has been pressed, we disable it
            // until the interstitial ad has been displayed to and dismissed
            // by the user. The load interstitial button is reenabled in the
            // update() method.
            button->setEnabled(false);
            logMessage("Setting the InterstitialAd listener.");
            interstitialAd->SetListener(interstitialAdListener);
            logMessage("Loading the interstitial ad.");
            interstitialAd->LoadAd(createAdRequest());
            interstitialAd->LoadAdLastResult().OnCompletion(
                onInterstitialAdLoadAdCompletionCallback, this);
            break;
          default:
            break;
        }
      });
  this->addChild(loadInterstitialAdBtn);

  // Set up the show interstitial ad button.
  showInterstitialAdBtn = createButton(false, kShowInterstitialText);
  showInterstitialAdBtn->addTouchEventListener(
      [&](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED:
            // The show intersitial button is enabled in the update() method
            // when the interstial ad has successfully loaded. Here the show
            // interstitial button has been pressed by the user, so we disable
            // the button and display the interstitial ad to the user.
            button->setEnabled(false);
            logMessage("Showing the interstitial ad.");
            interstitialAd->Show();
            // Invalidate all Futures and enable loadInterstitialAdBtn.
            interstitialAdShown = true;
            break;
          default:
            break;
        }
      });
  this->addChild(showInterstitialAdBtn);

  logMessage("Initializing rewarded video.");
  rewarded_video::Initialize();
  rewarded_video::InitializeLastResult().OnCompletion(
      onRewardedVideoInitializeCompletionCallback, this);

  // Set up the load rewarded video button.
  loadRewardedVideoBtn = createButton(true, kLoadRewardedVideoText);
  loadRewardedVideoBtn->addTouchEventListener(
      [&](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED:
            // Once the load rewarded video button has been pressed, we disable
            // it until the rewarded video has been displayed to and dismissed
            // by the user. The load rewarded video button is reenabled in the
            // update() method.
            button->setEnabled(false);
            logMessage("Setting the rewarded video listener.");
            rewarded_video::SetListener(rewardedVideoListener);
            logMessage("Loading the rewarded video.");
            rewarded_video::LoadAd(kRewardedVideoAdUnit, createAdRequest());
            rewarded_video::LoadAdLastResult().OnCompletion(
                onRewardedVideoLoadAdCompletionCallback, this);
            break;
          default:
            break;
        }
      });
  this->addChild(loadRewardedVideoBtn);

  // Set up the show rewarded video button.
  showRewardedVideoBtn = createButton(false, kShowRewardedVideoText);
  showRewardedVideoBtn->addTouchEventListener(
      [&](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED:
            // The show rewarded video button is enabled in the update() method
            // when the rewarded video has successfully loaded. Here the show
            // rewarded video button has been pressed by the user, so we disable
            // the button and display the rewarded video to the user.
            button->setEnabled(false);
            logMessage("Showing the rewarded video ad.");
            rewarded_video::Show(getWindowContext());
            // Invalidate all Futures and enable loadRewardedVideoBtn.
            rewardedVideoAdShown = true;
            break;
          default:
            break;
        }
      });
  this->addChild(showRewardedVideoBtn);

  // Create the close app menu item.
  auto closeAppItem = MenuItemImage::create(
      "CloseNormal.png", "CloseSelected.png",
      CC_CALLBACK_1(FirebaseScene::menuCloseAppCallback, this));
  closeAppItem->setContentSize(cocos2d::Size(25, 25));
  // Position the close app menu item on the top-right corner of the screen.
  closeAppItem->setPosition(cocos2d::Vec2(
      origin.x + visibleSize.width - closeAppItem->getContentSize().width / 2,
      origin.y + visibleSize.height -
          closeAppItem->getContentSize().height / 2));

  // Create the Menu for touch handling.
  auto menu = Menu::create(closeAppItem, NULL);
  menu->setPosition(cocos2d::Vec2::ZERO);
  this->addChild(menu, 1);

  // Schedule the update method for this scene.
  this->scheduleUpdate();

  return true;
}

// Called automatically every frame. The update is scheduled in `init()`.
void FirebaseAdMobScene::update(float delta) {
  // If the ad view's Initialize() future has completed successfully, enable the
  // ad view buttons.
  if (adView->InitializeLastResult().status() ==
          firebase::kFutureStatusComplete &&
      adView->InitializeLastResult().error() ==
          firebase::admob::kAdMobErrorNone) {
    // If ad view's LoadAd() has not been called yet, enable the load ad view
    // button.
    if (adView->LoadAdLastResult().status() == firebase::kFutureStatusInvalid) {
      loadAdViewBtn->setEnabled(true);
    }
    showHideAdViewBtn->setEnabled(true);
    moveAdViewBtn->setEnabled(true);
  }

  // Once the InterstitialAd::Intitialize() future has completed successfully,
  // enable the interstitial ad buttons.
  if (interstitialAd->InitializeLastResult().status() ==
          firebase::kFutureStatusComplete &&
      interstitialAd->InitializeLastResult().error() ==
          firebase::admob::kAdMobErrorNone) {
    // If InterstitialAd::LoadAd() method has not been called yet, enable the
    // load interstitial ad button.
    if (interstitialAd->LoadAdLastResult().status() ==
        firebase::kFutureStatusInvalid) {
      loadInterstitialAdBtn->setEnabled(true);
    }
    // Once the InterstitialAd::LoadAd() future has completed successfully,
    // enable the show interstitial ad button.
    if (interstitialAd->LoadAdLastResult().status() ==
            firebase::kFutureStatusComplete &&
        interstitialAd->LoadAdLastResult().error() ==
            firebase::admob::kAdMobErrorNone &&
        !interstitialAdShown) {
      showInterstitialAdBtn->setEnabled(true);
    }
    // Once the InterstitialAd::Show() future has completed and the interstitial
    // ad has been displayed and dismissed by the user, clean up the existing
    // interstitial ad object and create a new one. Note: InterstitialAd is a
    // single-use object that can load and show a single AdMob interstitial ad.
    if (interstitialAd->ShowLastResult().status() ==
            firebase::kFutureStatusComplete &&
        interstitialAd->presentation_state() ==
            firebase::admob::InterstitialAd::kPresentationStateHidden) {
      // Invalidate all Futures and enable loadInterstitialAdBtn.
      interstitialAdShown = false;
      delete interstitialAd;
      interstitialAd = new firebase::admob::InterstitialAd();
      logMessage("Initializing the interstitial ad.");
      interstitialAd->Initialize(getWindowContext(), kInterstitialAdUnit);
      interstitialAd->InitializeLastResult().OnCompletion(
          onInterstitialAdInitializeCompletionCallback, this);
    }
    // If the InterstitialAd::LoadAd() future completed but there was an error,
    // then clean up the existing interstitial ad object and create a new one.
    if (interstitialAd->InitializeLastResult().status() ==
            firebase::kFutureStatusComplete &&
        interstitialAd->LoadAdLastResult().status() ==
            firebase::kFutureStatusComplete &&
        interstitialAd->LoadAdLastResult().error() !=
            firebase::admob::kAdMobErrorNone) {
      // Invalidate all Futures and enable loadInterstitialAdBtn.
      interstitialAdShown = false;
      delete interstitialAd;
      interstitialAd = new firebase::admob::InterstitialAd();
      logMessage("Reinitializing the interstitial ad.");
      interstitialAd->Initialize(getWindowContext(), kInterstitialAdUnit);
      interstitialAd->InitializeLastResult().OnCompletion(
          onInterstitialAdInitializeCompletionCallback, this);
    }
  }

  // Once the rewarded_video::Intitialize() future has completed successfully,
  // enable the rewarded video buttons.
  if (rewarded_video::InitializeLastResult().status() ==
          firebase::kFutureStatusComplete &&
      rewarded_video::InitializeLastResult().error() ==
          firebase::admob::kAdMobErrorNone) {
    // If rewarded_video::LoadAd() method has not been called yet, enable the
    // load rewarded video button.
    if (rewarded_video::LoadAdLastResult().status() ==
        firebase::kFutureStatusInvalid) {
      loadRewardedVideoBtn->setEnabled(true);
    }
    // Once the rewarded_video::LoadAd() future has completed successfully,
    // enable the show rewarded video button.
    if (rewarded_video::LoadAdLastResult().status() ==
            firebase::kFutureStatusComplete &&
        rewarded_video::LoadAdLastResult().error() ==
            firebase::admob::kAdMobErrorNone &&
        !rewardedVideoAdShown) {
      showRewardedVideoBtn->setEnabled(true);
    }
    // Once the rewarded_video::Show() future has completed and the rewarded
    // video has been displayed and dismissed by the user, invalidate all
    // existing futures for rewarded_video methods and enable the load rewarded
    // video button.
    if (rewarded_video::ShowLastResult().status() ==
            firebase::kFutureStatusComplete &&
        rewarded_video::presentation_state() ==
            firebase::admob::rewarded_video::kPresentationStateHidden) {
      // Invalidate all Futures and enable loadRewardedVideoBtn.
      rewardedVideoAdShown = false;
      rewarded_video::Destroy();
      logMessage("Initializing rewarded video.");
      rewarded_video::Initialize();
      rewarded_video::InitializeLastResult().OnCompletion(
          onRewardedVideoInitializeCompletionCallback, this);
    }
    // If the rewarded_video::LoadAd() future completed but there was an error,
    // then clean up the existing rewarded_video namespace and reinitialize.
    if (rewarded_video::InitializeLastResult().status() ==
            firebase::kFutureStatusComplete &&
        rewarded_video::LoadAdLastResult().status() ==
            firebase::kFutureStatusComplete &&
        rewarded_video::LoadAdLastResult().error() !=
            firebase::admob::kAdMobErrorNone) {
      // Invalidate all Futures and enable loadRewardedVideoBtn.
      rewardedVideoAdShown = false;
      rewarded_video::Destroy();
      logMessage("Reinitializing rewarded video.");
      rewarded_video::Initialize();
      rewarded_video::InitializeLastResult().OnCompletion(
          onRewardedVideoInitializeCompletionCallback, this);
    }
  }
}

firebase::admob::AdRequest FirebaseAdMobScene::createAdRequest() {
  firebase::admob::AdRequest request;
  // If the app is aware of the user's gender, it can be added to the targeting
  // information. Otherwise, "unknown" should be used.
  request.gender = firebase::admob::kGenderUnknown;

  // This value allows publishers to specify whether they would like the request
  // to be treated as child-directed for purposes of the Children’s Online
  // Privacy Protection Act (COPPA).
  // See http://business.ftc.gov/privacy-and-security/childrens-privacy.
  request.tagged_for_child_directed_treatment =
      firebase::admob::kChildDirectedTreatmentStateTagged;

  // The user's birthday, if known. Note that months are indexed from one.
  request.birthday_day = kBirthdayDay;
  request.birthday_month = kBirthdayMonth;
  request.birthday_year = kBirthdayYear;

  // Additional keywords to be used in targeting.
  request.keyword_count = sizeof(kKeywords) / sizeof(kKeywords[0]);
  request.keywords = kKeywords;

  // "Extra" key value pairs can be added to the request as well. Typically
  // these are used when testing new features.
  static const firebase::admob::KeyValuePair kRequestExtras[] = {
      {"the_name_of_an_extra", "the_value_for_that_extra"}};
  request.extras_count = sizeof(kRequestExtras) / sizeof(kRequestExtras[0]);
  request.extras = kRequestExtras;

  // This example uses ad units that are specially configured to return test ads
  // for every request. When using your own ad unit IDs, however, it's important
  // to register the device IDs associated with any devices that will be used to
  // test the app. This ensures that regardless of the ad unit ID, those
  // devices will always receive test ads in compliance with AdMob policy.
  //
  // Device IDs can be obtained by checking the logcat or the Xcode log while
  // debugging. They appear as a long string of hex characters.
  request.test_device_id_count =
      sizeof(kTestDeviceIDs) / sizeof(kTestDeviceIDs[0]);
  request.test_device_ids = kTestDeviceIDs;

  return request;
}

/// Handles the user tapping on the close app menu item.
void FirebaseAdMobScene::menuCloseAppCallback(Ref* pSender) {
  CCLOG("Cleaning up AdMob C++ resources.");
  delete adView;
  delete interstitialAd;
  rewarded_video::Destroy();
  delete adViewListener;
  delete interstitialAdListener;
  delete rewardedVideoListener;
  firebase::admob::Terminate();

  // Close the cocos2d-x game scene and quit the application.
  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}
