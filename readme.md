Firebase Cocos2d-x Samples
==========================

iOS and Android cocos2d-x samples for the Firebase C++ SDK.

Introduction
------------

- [Read more about Firebase](https://firebase.google.com/docs)

Prerequisites
-------------

- The `setup_firebase_sample.py` script requires Python to be installed locally.
  Visit the [Python download page](https://www.python.org/downloads/)
  for more information.

- If you are running this script on a Mac, you will need CocoaPods installed.
  See the CocoaPods [Getting Started guide](https://guides.cocoapods.org/using/getting-started.html)
  for more details.

Getting Started
---------------

- Clone the Firebase Cocos2d-x Samples GitHub repo.
  ```
  git clone https://github.com/firebase/cocos2dx-cpp-sample.git
  ```
- Navigate to the directory that you just cloned and run the Firebase setup
  script, where FIREBASE_FEATURE is one of the following:
  AdMob, Analytics, Auth, Invites, Messaging, Remote_Config
  ```
  python setup_firebase_sample.py FIREBASE_FEATURE
  ```
- If you haven't done so already, you will need to add the cocos2d-x environment
  variables to the operating system's PATH variable. Navigate to the
  `sample_project/cocos2d` directory and run the cocos2d-x setup script:
  ```
  python setup.py
  ```

### iOS
- Follow the steps in
  [Set up your app in Firebase console](https://firebase.google.com/docs/cpp/setup#set_up_your_app_in_name_appmanager).
    - Bundle ID: org.cocos2dx.hellocpp
- Open the `sample_project/proj.ios_mac/HelloCpp.xcworkspace` with Xcode. From
  the Project Navigator, right click on `HelloCpp` and select “Add files to
  HelloCpp.” Find the `GoogleService-Info.plist` file that you downloaded in
  step 1 and click the Add button.
- In the top left corner, set the active scheme to HelloCpp-mobile and choose
  the device that you want to run the app on. Then, click the run button to
  build and run the app.

### Android
- Follow the steps in
  [Set up your app in Firebase console](https://firebase.google.com/docs/cpp/setup#set_up_your_app_in_name_appmanager_1).
    - Package Name: org.cocos2dx.hellocpp
- Add the google-services.json file that you downloaded in step 1 to the
  `sample_project/proj.android-studio/app` directory.
- In the terminal from the `sample_project` directory, compile the Cocos project
  for Android:
  ```
  cocos compile -p android --ap ## --android-studio --app-abi armeabi-v7a
  ```
  Where `##` is the Android platform used for building the apk,
  for example, `android-22`.
- Open the `sample_project/proj.android-studio` project in Android Studio and
  run the app. To install the APK on a device, navigate to
  `sample_project/bin/debug/android`.

Support
-------

[https://firebase.google.com/support/]

License
-------

Copyright 2017 Google Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
