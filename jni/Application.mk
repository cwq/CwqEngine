APP_PLATFORM := android-9
APP_OPTIM := release
APP_ABI := armeabi-v7a

#APP_STL := c++_static
#NDK_TOOLCHAIN_VERSION=clang

#APP_STL := gnustl_shared
APP_STL := gnustl_static
#at least 4.8 can use c++11
NDK_TOOLCHAIN_VERSION := 4.8

APP_CPPFLAGS := -std=c++11
