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

#ifndef FIREBASE_COCOS_CLASSES_FIREBASE_COCOS_H_
#define FIREBASE_COCOS_CLASSES_FIREBASE_COCOS_H_

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <jni.h>
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
extern "C" {
#include <objc/objc.h>
}  // extern "C"
#endif

// WindowContext represents the handle to the parent window.  It's type
// (and usage) vary based on the OS.
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
typedef jobject WindowContext;  // A jobject to the Java Activity.
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
typedef id WindowContext;  // A pointer to an iOS UIView.
#else
typedef void* WindowContext;  // A void* for any other environments.
#endif

// Returns a variable that describes the window context for the app. On Android
// this will be a jobject pointing to the Activity. On iOS, it's an id pointing
// to the root view of the view controller.
WindowContext getWindowContext();

#endif  // FIREBASE_COCOS_CLASSES_FIREBASE_COCOS_H_
