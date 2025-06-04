## Android

The Orb project has two Android reference integrations:

### A. The mock reference (mockorbapp)

In the mock reference the orblibrary is integrated with the mockorbapp. The implementation mocks any calls that would normally use other components such as a broadcast stack.

#### Building

Open this repo as your project in Android studio. The mock reference can be built as a regular app in Android studio and run on an emulator or device.

### B. The DTVKit reference

In the DTVKit reference the orblibrary is integrated with the DTVKit Tv Input Source. The implementation calls the DTVKit broadcast stack which itself is integrated with the Android Tuner framework.

#### Building

For build instructions, please refer to README.md:
https://github.com/OpenRedButtonProject/orb-reference-android
or
https://github.com/OpenRedButtonProject/orb-204-reference-android

Notes for later Android versions:
Note 1: 
jsoncpp_ORB is only needed for Android 11. In later versions, standard jsoncpp will work.

Note 2: 
For Android 13 and above
a) build environement needs "RELAX_USES_LIBRARY_CHECK=true" to avoid dependency checks on prebuilt Chromium APKs
b) Where ORB is included in vendor space, libwebsockets also needs to be build in vendor space

