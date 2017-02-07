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

#include "FirebaseScene.h"

#include <stdarg.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <android/log.h>
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#endif

#include "FirebaseCocos.h"

USING_NS_CC;

// The images for the Firebase buttons.
static const std::string kNormalButtonImage = "NormalButtonImage.png";
static const std::string kSelectedButtonImage = "SelectedButtonImage.png";
static const std::string kDisabledButtonImage = "SelectedButtonImage.png";

/// Padding for the UI elements.
static const float kUIElementPadding = 10.0;

/// The title text size for the Firebase buttons.
static const float kButtonTitleFontSize = 12.0;

/// The content size for the Firebase buttons.
static const cocos2d::Size kButtonContentSize = cocos2d::Size(150, 20);

/// The factor used to determine when to resize the ScrollView's inner container
/// height.
static const float kScrollViewContainerHeightFactor = 0.85;

/// The logging ScrollView.
cocos2d::ui::ScrollView* scrollView;

/// The log text for the Text Widget.
cocos2d::ui::Text* logTextWidget;

/// The log text.
std::string logTextString;

cocos2d::ui::Button* FirebaseScene::createButton(
    bool buttonEnabled, const std::string& buttonTitleText,
    const cocos2d::Color3B& buttonColor) {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  cocos2d::Vec2 origin = Director::getInstance()->getVisibleOrigin();
  cocos2d::ui::Button* button = cocos2d::ui::Button::create(
      kNormalButtonImage, kSelectedButtonImage, kDisabledButtonImage);
  button->setEnabled(buttonEnabled);
  button->setColor(buttonColor);
  button->setTitleText(buttonTitleText);
  button->setTitleFontSize(kButtonTitleFontSize);
  button->ignoreContentAdaptWithSize(false);
  button->setContentSize(kButtonContentSize);
  nextYPosition -= button->getContentSize().height + kUIElementPadding;
  button->setPosition(
      cocos2d::Vec2(origin.x + visibleSize.width / 4, nextYPosition));

  return button;
}

cocos2d::ui::Button* FirebaseScene::createButton(
    bool buttonEnabled, const std::string& buttonTitleText) {
  return createButton(buttonEnabled, buttonTitleText, cocos2d::Color3B::WHITE);
}

void FirebaseScene::createScrollView(float yPosition) {
  cocos2d::Size visibleSize = Director::getInstance()->getVisibleSize();
  cocos2d::Vec2 origin = Director::getInstance()->getVisibleOrigin();

  scrollView = cocos2d::ui::ScrollView::create();
  auto scrollViewFrameSize = Size(visibleSize.width / 2, yPosition);
  scrollView->setContentSize(scrollViewFrameSize);
  scrollView->setPosition(
      cocos2d::Point(origin.x + visibleSize.width / 2, origin.y));
  cocos2d::Size scrollViewContainerSize =
      cocos2d::Size(scrollViewFrameSize.width, scrollViewFrameSize.height);
  scrollView->setInnerContainerSize(scrollViewContainerSize);

  logTextWidget = cocos2d::ui::Text::create("", "fonts/arial.ttf", 12);
  logTextWidget->setTextAreaSize(
      cocos2d::Size(scrollViewContainerSize.width, 0));
  scrollView->addChild(logTextWidget);

  this->addChild(scrollView);
}

/// Adds text to the log TextWidget.
void FirebaseScene::logMessage(std::string format, ...) {
  static const int kLineBufferSize = 100;
  char buffer[kLineBufferSize + 2];

  va_list list;
  va_start(list, format);
  int stringLength = vsnprintf(buffer, kLineBufferSize, format.c_str(), list);
  stringLength =
      stringLength < kLineBufferSize ? stringLength : kLineBufferSize;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
  __android_log_vprint(ANDROID_LOG_INFO, "FIREBASE-COCOS", format.c_str(),
                       list);
#endif
  va_end(list);
  logTextString.append("\n");
  logTextString.append(buffer);

  // Update the log TextWidget and the ScrollView's inner container size on the
  // Cocos2d thread.
  cocos2d::Director::getInstance()
      ->getScheduler()
      ->performFunctionInCocosThread([=]() {
        logTextWidget->setString(logTextString);
        cocos2d::Size scrollViewContainerSize =
            scrollView->getInnerContainerSize();
        // Check to see if the ScrollView's inner container needs to be resized.
        if (logTextWidget->getContentSize().height /
                scrollViewContainerSize.height >=
            kScrollViewContainerHeightFactor) {
          cocos2d::Size newScrollViewContainerSize =
              cocos2d::Size(scrollViewContainerSize.width,
                            scrollViewContainerSize.height * 2);
          scrollView->setInnerContainerSize(newScrollViewContainerSize);
          scrollViewContainerSize = scrollView->getInnerContainerSize();
        }
        logTextWidget->setPosition(
            cocos2d::Point(scrollViewContainerSize.width -
                               logTextWidget->getContentSize().width / 2,
                           scrollViewContainerSize.height + kUIElementPadding -
                               logTextWidget->getContentSize().height / 2));
        float scrollPercent = logTextWidget->getContentSize().height /
                              scrollViewContainerSize.height;
        scrollView->scrollToPercentVertical(scrollPercent * 100, 1.0, false);
      });
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
// Returns the iOS RootViewController's main view (i.e. the EAGLView).
WindowContext getWindowContext() {
  return (id)Director::getInstance()->getOpenGLView()->getEAGLView();
}
#endif
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
// Returns the Android Activity.
WindowContext getWindowContext() { return JniHelper::getActivity(); }
#endif
