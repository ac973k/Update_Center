QT       += core gui network androidextras testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    updatecenter.cpp

HEADERS += \
    updatecenter.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

ANDROID_ABIS = armeabi-v7a
android: include(C:/Users/Danilka/AppData/Local/Android/Sdk/android_openssl/openssl.pri)

ANDROID_EXTRA_LIBS = C:/Users/Danilka/AppData/Local/Android/Sdk/android_openssl/latest/arm/libcrypto_1_1.so C:/Users/Danilka/AppData/Local/Android/Sdk/android_openssl/latest/arm/libssl_1_1.so

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
