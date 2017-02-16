#!/usr/bin/python
# coding=utf-8
#
# Sets up the Firebase cocos2d-x sample project.
#
# For help:
#
#   python setup_firebase_sample.py --help
#
# Run the script:
#
#   python setup_firebase_sample.py FIREBASE_FEATURE
#
# Where FIREBASE_FEATURE is one of the following:
#
#   AdMob, Analytics, Auth, Invites, Messaging, or Remote_Config
#

import argparse
import fileinput
import logging
import os
import platform
import shutil
import sys
import urllib
import zipfile

# The setup_firebase_sample.py script directory.
ROOT_DIRECTORY = os.path.dirname(os.path.abspath(__file__))
# The Firebase SDK download URL.
FIREBASE_SDK_URL = "https://dl.google.com/firebase/sdk/cpp/firebase_cpp_sdk_2.1.2.zip"
# The cocos2d-x GitHub release download URL.
COCOS2DX_GITHUB_URL = "https://github.com/cocos2d/cocos2d-x/archive/cocos2d-x-3.13.1.zip"
# The iOS project directory.
IOS_PROJECT_DIR = os.path.join(ROOT_DIRECTORY, "sample_project/proj.ios_mac")
# The iOS project.pbxproj file.
IOS_PROJECT_FILE = os.path.join(IOS_PROJECT_DIR,
                                "HelloCpp.xcodeproj/project.pbxproj")
# The Android project directory.
ANDROID_PROJECT_DIR = os.path.join(ROOT_DIRECTORY,
                                   "sample_project/proj.android-studio")
# The Android.mk file.
ANDROID_MAKEFILE = os.path.join(ANDROID_PROJECT_DIR, "app/jni/Android.mk")
# The Libs directory for the cocos2d-x sample project. This is where the
# Firebase SDK is located.
LIBS_DIR = os.path.join(ROOT_DIRECTORY, "sample_project/Libs")
# The Firebase features passed by the caller of this script.
FEATURE_ARGS_ARRAY = []
# The Firebase features supported by this script.
FIREBASE_FEATURES_ARRAY = ["AdMob", "Analytics", "Invites", "Messaging"]


def add_cocos2dx_library():
  """Adds the cocos2d-x library to the sample project.

  Creates the sample project directory, adds the cocos2d-x library to the
  sample project directory, runs the cocos2d/download-deps.py script to pull
  in the third-party dependencies, and creates the .cocos-project.json file
  which is required for compiling the Android app.

  Raises:
    IOError: An error occurred retrieving the zip file.
    zipfile.BadZipfile: An error occurred because the zip file is not valid.
  """
  logging.info("Adding the cocos2d-x library (this may take a bit)...")
  sample_project_dir = os.path.join(ROOT_DIRECTORY, "sample_project")
  os.makedirs(sample_project_dir)
  os.chdir(sample_project_dir)
  try:
    zip_name = "cocos2dx.zip"
    filename, headers = urllib.urlretrieve(COCOS2DX_GITHUB_URL, zip_name)
    zip_file = zipfile.ZipFile(filename)
    zip_file.extractall(sample_project_dir)
    zip_file.close()
    for file in os.listdir("."):
      # The extracted folder name is prefixed with "cocos2d-x-cocos2d-x-##."
      if file.startswith("cocos2d-x-cocos2d-x-"):
        os.rename(file, "cocos2d")
        break
    os.remove(os.path.join(sample_project_dir, zip_name))
  except IOError as e:
    logging.exception("IOError: [Errno %d] %s: in %s", e.errno, e.strerror,
                      sys._getframe().f_code.co_name)
    exit()
  except zipfile.BadZipfile, e:
    logging.exception("%s is not a valid zip file in %s.", COCOS2DX_GITHUB_URL,
                      sys._getframe().f_code.co_name)
    exit()
  # Download the third-party dependencies into the cocos2d library.
  os.system("python cocos2d/download-deps.py -r yes")
  os.chdir(ROOT_DIRECTORY)
  # Create the .cocos-project.json file. This is a hidden file that declares the
  # project type and is required for building and running the Android app. For
  # this sample project, the project type is "cpp."
  file = open("sample_project/.cocos-project.json", "w")
  file.write('{"project_type": "cpp"}')
  file.close()
  logging.info("Finished adding the cocos2d-x library to the sample project.")


def add_firebase_sdk():
  """Adds the latest version of the Firebase SDK to the sample project.

  Raises:
    IOError: An error occurred retrieving the zip file.
    zipfile.BadZipfile: An error occurred because the zip file is not valid.
  """
  logging.info("Adding the Firebase SDK (this may take a bit)...")
  try:
    zip_name = "firebase_cpp_sdk.zip"
    filename, headers = urllib.urlretrieve(FIREBASE_SDK_URL, zip_name)
    zip_file = zipfile.ZipFile(filename)
    zip_file.extractall(LIBS_DIR)
    zip_file.close()
    os.remove(os.path.join(ROOT_DIRECTORY, zip_name))
  except IOError as e:
    logging.exception("IOError: [Errno %d] %s: in %s", e.errno, e.strerror,
                      sys._getframe().f_code.co_name)
    exit()
  except zipfile.BadZipfile, e:
    logging.exception("%s is not a valid zip file in %s.", FIREBASE_SDK_URL,
                      sys._getframe().f_code.co_name)
    exit()
  logging.info(
      "Finished adding the Firebase SDK to the sample project's Libs directory")


def add_cpp_default_template():
  """Adds the C++ default project template to the sample project directory.

  Raises:
    IOError: An error occurred copying the C++ default project template to the
             sample project directory.
  """
  cpp_default_template_dir = os.path.join(
      ROOT_DIRECTORY, "sample_project/cocos2d/templates/cpp-template-default")
  dst_dir = os.path.join(ROOT_DIRECTORY, "sample_project")
  try:
    for item in os.listdir(cpp_default_template_dir):
      src = os.path.join(cpp_default_template_dir, item)
      dst = os.path.join(dst_dir, item)
      if os.path.isdir(src):
        shutil.copytree(src, dst)
      else:
        shutil.copy(src, dst)
  except IOError as e:
    logging.exception("IOError: [Errno %d] %s: in %s", e.errno, e.strerror,
                      sys._getframe().f_code.co_name)
    exit()
  logging.info(
      "Added the C++ default project template to the sample project directory.")


def add_project_template_files():
  """Adds the project template files to the sample project.

  Raises:
    IOError: An error occurred copying the project template files.
  """
  ios_project_file = os.path.join(
      ROOT_DIRECTORY, "common/project_template_files/project.pbxproj")
  android_makefile = os.path.join(ROOT_DIRECTORY,
                                  "common/project_template_files/Android.mk")
  ios_dst_dir = os.path.join(IOS_PROJECT_DIR, "HelloCpp.xcodeproj")
  android_dst_dir = os.path.join(ANDROID_PROJECT_DIR, "app/jni")
  try:
    shutil.copy(ios_project_file, ios_dst_dir)
    shutil.copy(android_makefile, android_dst_dir)
  except IOError as e:
    logging.exception("IOError: [Errno %d] %s: in %s", e.errno, e.strerror,
                      sys._getframe().f_code.co_name)
    exit()
  logging.info("Added the project template files to the sample project.")


def update_ios_project_file():
  """Updates the iOS project.pbxproj file.

  Replaces "{FIREBASE_FEATURE}" placeholders in the project file with the
  Firebase feature passed to this script. Specifying the feature ensures that
  the proper frameworks and implementation files are referenced in the iOS
  project file.
  """
  firebase_feature = FEATURE_ARGS_ARRAY[0].lower()
  for line in fileinput.input(IOS_PROJECT_FILE, inplace=True):
    if "{FIREBASE_FEATURE}" in line:
      print line.replace("{FIREBASE_FEATURE}", firebase_feature).replace("\n",
                                                                         "")
    elif "{FIREBASE_FEATURE_CAMELCASE}" in line:
      print line.replace("{FIREBASE_FEATURE_CAMELCASE}",
                         FEATURE_ARGS_ARRAY[0]).replace("\n", "")
    else:
      print line.replace("\n", "")
  logging.info("Updated the iOS project.pbxproj file.")


def update_android_makefile():
  """Updates the Android.mk file.

  Replaces "{FIREBASE_FEATURE}" placeholders in the Android.mk file with the
  Firebase feature passed to this script. Specifying the feature ensures that
  the proper libraries and implementation files are referenced in the Android
  project.
  """
  firebase_feature = FEATURE_ARGS_ARRAY[0].lower()
  for line in fileinput.input(ANDROID_MAKEFILE, inplace=True):
    if "{FIREBASE_FEATURE}" in line:
      print line.replace("{FIREBASE_FEATURE}", firebase_feature).replace("\n",
                                                                         "")
    elif "{FIREBASE_FEATURE_CAMELCASE}" in line:
      print line.replace("{FIREBASE_FEATURE_CAMELCASE}",
                         FEATURE_ARGS_ARRAY[0]).replace("\n", "")
    else:
      print line.replace("\n", "")
  logging.info("Updated the Android.mk file.")


def add_resource_files():
  """Adds the resource files to the sample project's Resources directory.

  Raises:
    IOError: An error occurred copying the resource files.
  """
  common_resources_dir = os.path.join(ROOT_DIRECTORY, "common/Resources")
  dst_dir = os.path.join(ROOT_DIRECTORY, "sample_project/Resources")
  try:
    common_resources_files = os.listdir(common_resources_dir)
    for file_name in common_resources_files:
      full_file_name = os.path.join(common_resources_dir, file_name)
      if os.path.isfile(full_file_name):
        shutil.copy(full_file_name, dst_dir)
  except IOError as e:
    logging.exception("IOError: [Errno %d] %s: in %s", e.errno, e.strerror,
                      sys._getframe().f_code.co_name)
    exit()
  logging.info("Added files to the sample project's Resources directory.")


def add_class_files():
  """Adds the C++ class files to the sample project's Classes directory.

  Raises:
    IOError: An error occurred copying the class files.
  """
  firebase_feature = FEATURE_ARGS_ARRAY[0].lower()
  common_classes_dir = os.path.join(ROOT_DIRECTORY, "common/Classes")
  feature_classes_dir = os.path.join(ROOT_DIRECTORY, firebase_feature,
                                     "Classes")
  dst_dir = os.path.join(ROOT_DIRECTORY, "sample_project/Classes")
  try:
    shutil.rmtree(dst_dir)
    shutil.copytree(common_classes_dir, dst_dir)
    feature_classes_files = os.listdir(feature_classes_dir)
    for file_name in feature_classes_files:
      full_file_name = os.path.join(feature_classes_dir, file_name)
      if os.path.isfile(full_file_name):
        shutil.copy(full_file_name, dst_dir)
  except IOError as e:
    logging.exception("IOError: [Errno %d] %s: in %s", e.errno, e.strerror,
                      sys._getframe().f_code.co_name)
    exit()
  logging.info("Added files to the sample project's Classes directory.")


def add_android_files():
  """Adds the Android gradle files to the sample project.

  Raises:
    IOError: An error occurred copying the Android files.
  """
  firebase_feature = FEATURE_ARGS_ARRAY[0].lower()
  project_gradle_file = os.path.join(ROOT_DIRECTORY, firebase_feature,
                                     "build.gradle")
  app_gradle_file = os.path.join(ROOT_DIRECTORY, firebase_feature,
                                 "app/build.gradle")
  try:
    shutil.copy(project_gradle_file, ANDROID_PROJECT_DIR)
    shutil.copy(app_gradle_file, os.path.join(ANDROID_PROJECT_DIR, "app"))
  except IOError as e:
    logging.exception("IOError: [Errno %d] %s: in %s", e.errno, e.strerror,
                      sys._getframe().f_code.co_name)
    exit()
  logging.info("Added the Android gradle files to the sample project.")


def run_pod_install():
  """Installs the Firebase pods in the iOS project.

  This function is only run on Mac (Darwin).

  Raises:
    IOError: An error occurred copying the Podfile.
  """
  logging.info("Running pod install...")
  firebase_feature = FEATURE_ARGS_ARRAY[0].lower()
  podfile = os.path.join(ROOT_DIRECTORY, firebase_feature, "Podfile")
  try:
    shutil.copy(podfile, IOS_PROJECT_DIR)
  except IOError as e:
    logging.exception("IOError: [Errno %d] %s: in %s", e.errno, e.strerror,
                      sys._getframe().f_code.co_name)
    exit()
  os.chdir(IOS_PROJECT_DIR)
  os.system("pod install")
  os.chdir(ROOT_DIRECTORY)
  logging.info("Finished running pod install.")


def check_valid_arguments():
  """Checks if the user passed valid arguments to the script."""
  parser = argparse.ArgumentParser(
      description="This script sets up the cocos2d-x sample project for "
      "Firebase C++.")
  parser.add_argument(
      "feature",
      metavar="FIREBASE_FEATURE",
      nargs=1,
      help="The Firebase feature must be one of the following: "
      "AdMob, Analytics, Auth, Invites, Messaging, or Remote_Config")
  args = parser.parse_args()
  for feature in args.feature:
    feature_str = str(feature)
    if not feature_str in FIREBASE_FEATURES_ARRAY:
      return False
    FEATURE_ARGS_ARRAY.append(feature_str)
  return True


def main():
  """The main function."""
  # The root logger of the hierarchy.
  logger = logging.getLogger()
  logger.setLevel(logging.DEBUG)

  # Add a StreamHandler to log to stdout.
  stream_handler = logging.StreamHandler(sys.stdout)
  stream_handler.setLevel(logging.DEBUG)
  formatter = logging.Formatter(
      "%(asctime)s - %(name)s - %(levelname)s - %(message)s")
  stream_handler.setFormatter(formatter)
  logger.addHandler(stream_handler)

  if not check_valid_arguments():
    logging.info(
        "This script takes one argument, the name of the Firebase feature you "
        "want to use for the cocos2d-x sample project. Valid values are:")
    logging.info(FIREBASE_FEATURES_ARRAY)
    exit()

  add_cocos2dx_library()
  add_firebase_sdk()
  add_cpp_default_template()
  add_project_template_files()
  update_ios_project_file()
  update_android_makefile()
  add_resource_files()
  add_class_files()
  add_android_files()
  if platform.system() == "Darwin":
    run_pod_install()
  logging.info("Finished setting up %s.", FEATURE_ARGS_ARRAY)


# Check to see if this script is being called directly.
if __name__ == "__main__":
  exit(main())
