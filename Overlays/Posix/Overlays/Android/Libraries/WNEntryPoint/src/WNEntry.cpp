// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include "WNCore/inc/WNTypes.h"
#include "WNUtilities/inc/WNAndroidEventPump.h"
#include "WNUtilities/inc/WNAppData.h"
#include "WNUtilities/inc/WNLoggingData.h"
#include "WNUtilities/inc/WNCrashHandler.h"

#include <android/log.h>
#include <android_native_app_glue.h>
#include <sys/prctl.h>
#include <unistd.h>

extern int32_t wn_main(int32_t _argc, char* _argv[]);

void wn_dummy() {}

char* GetPackageName(struct android_app* state) {
  ANativeActivity* activity = state->activity;
  JNIEnv* env = 0;

  activity->vm->AttachCurrentThread(&env, 0);

  jclass cls = env->GetObjectClass(activity->clazz);
  jmethodID methodID =
      env->GetMethodID(cls, "getPackageName", "()Ljava/lang/String;");
  jobject result = env->CallObjectMethod(activity->clazz, methodID);

  char* tempstr;
  const char* str;
  jboolean isCopy;

  str = env->GetStringUTFChars((jstring)result, &isCopy);

  int newLen = strlen(str);

  tempstr = static_cast<char*>(malloc(sizeof(char) * newLen + 1));

  memcpy(tempstr, str, newLen);

  tempstr[newLen] = '\0';

  env->ReleaseStringUTFChars((jstring)result, str);
  activity->vm->DetachCurrentThread();

  return (tempstr);
}

void* main_proxy_thread(void* _package_name) {
  wn::utilities::initialize_crash_handler();

  char* package_name = static_cast<char*>(_package_name);
  int32_t retVal = wn_main(1, &package_name);

  __android_log_print(ANDROID_LOG_INFO, wn::utilities::gAndroidLogTag, "--FINISHED");
  __android_log_print(
      ANDROID_LOG_INFO, wn::utilities::gAndroidLogTag, "RETURN %d", retVal);

  wn::utilities::WNAndroidEventPump::GetInstance().KillMessagePump();

  return (NULL);
}

int main(int _argc, char* _argv[]) {
  wn::utilities::gAndroidLogTag = _argv[0];
  return wn_main(_argc, _argv);
}

void android_main(struct android_app* state) {
  char* packageName = GetPackageName(state);

  wn::utilities::gAndroidLogTag = packageName;
  wn::utilities::gAndroidApp = state;
  wn::utilities::gMainLooper = ALooper_forThread();

  app_dummy();

  __android_log_print(ANDROID_LOG_INFO, packageName, "--STARTED");

#if defined _WN_DEBUG
  if (access("/proc/sys/kernel/yama/ptrace_scope", F_OK) != -1) {
     prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);
  }

  FILE* debugFile = fopen("/sdcard/wait-for-debugger.txt", "r");

  if (debugFile) {
    __android_log_print(ANDROID_LOG_INFO, packageName, "--SLEEPING");

    sleep(10);  // sleep so that if we want to connect a debugger we can

    __android_log_print(ANDROID_LOG_INFO, packageName, "--WAKING UP");

    fclose(debugFile);
  } else {
    __android_log_print(ANDROID_LOG_INFO, packageName, "--NOT_DEBUGGING");
  }
#endif

  pthread_t mThread;

  pthread_create(&mThread, NULL, main_proxy_thread, packageName);

  wn::utilities::WNAndroidEventPump::GetInstance().PumpMessages(state);

  pthread_join(mThread, NULL);

  free(packageName);
  fclose(stdout);
}
