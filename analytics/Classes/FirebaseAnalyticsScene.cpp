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

#include "FirebaseAnalyticsScene.h"

#include <stdarg.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <android/log.h>
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#endif

#include "FirebaseCocos.h"
#include "firebase/analytics.h"
#include "firebase/app.h"
#include "firebase/future.h"

USING_NS_CC;


/// Padding for the UI elements.
static const float kUIElementPadding = 10.0;

/// The title text for the Firebase buttons.
static const char* kBlueButtonText = "Fire Blue Event";
static const char* kRedButtonText = "Fire Red Event";
static const char* kYellowButtonText = "Fire Yellow Event";
static const char* kGreenButtonText = "Fire Green Event";

/// The colors for the buttons.
static const cocos2d::Color3B kBlueButtonColor(0x48, 0x85, 0xed);
static const cocos2d::Color3B kRedButtonColor(0xdb, 0x32, 0x36);
static const cocos2d::Color3B kYellowButtonColor(0xf4, 0xc2, 0x0d);
static const cocos2d::Color3B kGreenButtonColor(0x3c, 0xba, 0x54);

static const char* kButtonClickEvent = "button_clicked";
static const char* kButtonColor = "button_color";

/// Creates the Firebase scene.
Scene* CreateFirebaseScene() {
  return FirebaseAnalyticsScene::createScene();
}

/// Creates the FirebaseAnalyticsScene.
Scene* FirebaseAnalyticsScene::createScene() {
  // Create the scene.
  auto scene = Scene::create();

  // Create the layer.
  auto layer = FirebaseAnalyticsScene::create();

  // Add the layer to the scene.
  scene->addChild(layer);

  return scene;
}

/// Initializes the FirebaseScene.
bool FirebaseAnalyticsScene::init() {
  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  cocos2d::Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // Intitialize Firebase-Analytics.
  CCLOG("Initializing the Analytics with Firebase API.");
  firebase::analytics::Initialize(*firebase::App::GetInstance());

  // Create the Firebase label.
  auto firebaseLabel =
      Label::createWithTTF("Firebase-Analytics", "fonts/Marker Felt.ttf", 20);
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

  // Set up the blue button.
  blue_button_ = createButton(true, kBlueButtonText, kBlueButtonColor);
  blue_button_->addTouchEventListener(
      [this](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            // When the blue button is clicked it logs an event with parameters
            // that indicate the color of the button was blue, as well as how
            // many times the blue button was clicked.
            this->blue_button_click_count_++;
            this->total_button_click_count_++;
            logMessage("Clicked the Blue Button. Total blue button clicks: %i",
                       this->blue_button_click_count_);

            const firebase::analytics::Parameter kButtonClickParameters[] = {
                firebase::analytics::Parameter(kButtonColor, "blue"),
                firebase::analytics::Parameter("blue_button_click_count",
                                               this->blue_button_click_count_),
            };
            firebase::analytics::LogEvent(
                kButtonClickEvent, kButtonClickParameters,
                sizeof(kButtonClickParameters) /
                sizeof(kButtonClickParameters[0]));

            this->previous_button_clicked_ = "blue";
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(blue_button_);

  // Set up the red button.
  red_button_ = createButton(true, kRedButtonText, kRedButtonColor);
  red_button_->addTouchEventListener(
      [this](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            // When the red button is clicked it logs an event with parameters
            // that indicate the color of the button was red, as well as how
            // many times any button was clicked.
            this->total_button_click_count_++;
            logMessage("Clicked the Red Button. Total button clicks: %i",
                       this->total_button_click_count_);

            const firebase::analytics::Parameter kButtonClickParameters[] = {
                firebase::analytics::Parameter(kButtonColor, "red"),
                firebase::analytics::Parameter("total_button_click_count",
                                               this->total_button_click_count_),
            };
            firebase::analytics::LogEvent(
                kButtonClickEvent, kButtonClickParameters,
                sizeof(kButtonClickParameters) /
                sizeof(kButtonClickParameters[0]));

            this->previous_button_clicked_ = "red";
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(red_button_);

  // Set up the yellow button.
  yellow_button_ = createButton(true, kYellowButtonText, kYellowButtonColor);
  yellow_button_->addTouchEventListener(
      [this](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            // When the yellow button is clicked it logs an event with
            // parameters that indicate the color of the button was yellow, as
            // well as the color of the previous button that was clicked.
            this->total_button_click_count_++;
            logMessage("Clicked the Yellow Button. Previous button was: %s",
                       this->previous_button_clicked_);

            const firebase::analytics::Parameter kButtonClickParameters[] = {
                firebase::analytics::Parameter(kButtonColor, "yellow"),
                firebase::analytics::Parameter("previous_button",
                                               this->previous_button_clicked_),
            };
            firebase::analytics::LogEvent(
                kButtonClickEvent, kButtonClickParameters,
                sizeof(kButtonClickParameters) /
                sizeof(kButtonClickParameters[0]));

            this->previous_button_clicked_ = "yellow";
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(yellow_button_);

  // Set up the green button.
  green_button_ = createButton(true, kGreenButtonText, kGreenButtonColor);
  green_button_->addTouchEventListener(
      [this](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        cocos2d::ui::Button* button = static_cast<cocos2d::ui::Button*>(sender);
        switch (type) {
          case cocos2d::ui::Widget::TouchEventType::ENDED: {
            // When the green button is clicked it logs an event with
            // parameters that indicate the color of the button was green, as
            // well as what fraction of button clicks were made on the green
            // button.
            this->total_button_click_count_++;
            this->green_button_click_count_++;
            float ratio = static_cast<float>(this->green_button_click_count_) /
                this->total_button_click_count_;
            logMessage("Clicked the Green Button. Green button ratio: %f",
                       ratio);

            const firebase::analytics::Parameter kButtonClickParameters[] = {
                firebase::analytics::Parameter(kButtonColor, "green"),
                firebase::analytics::Parameter("green_click_ratio", ratio),
            };
            firebase::analytics::LogEvent(
                kButtonClickEvent, kButtonClickParameters,
                sizeof(kButtonClickParameters) /
                sizeof(kButtonClickParameters[0]));

            this->previous_button_clicked_ = "green";
            break;
          }
          default: {
            break;
          }
        }
      });
  this->addChild(green_button_);

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
void FirebaseAnalyticsScene::update(float /*delta*/) {}

/// Handles the user tapping on the close app menu item.
void FirebaseAnalyticsScene::menuCloseAppCallback(Ref* pSender) {
  CCLOG("Cleaning up Analytics C++ resources.");

  // Close the cocos2d-x game scene and quit the application.
  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}
