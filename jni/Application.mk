APP_PLATFORM := android-9
APP_OPTIM := release
APP_ABI := armeabi-v7a

# APP_STL := stlport_shared  --> does not seem to contain C++11 features
#APP_STL := gnustl_shared
APP_STL := c++_static
#NDK_TOOLCHAIN_VERSION=clang

APP_CPPFLAGS := -std=c++11
