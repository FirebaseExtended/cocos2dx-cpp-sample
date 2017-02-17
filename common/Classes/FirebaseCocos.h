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
