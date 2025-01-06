/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android/log.h>

#include "BridgeSession.h"

#define TAG                "BridgeSession"
#define LOGI(x, ...)    __android_log_print(ANDROID_LOG_INFO, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define LOGE(x, ...)    __android_log_print(ANDROID_LOG_ERROR, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

#ifdef NDK_AIDL
namespace aidl {
#endif

using org::orbtv::orbservice::BridgeSession;
using org::orbtv::orbservice::IBrowserSession;

static SH_PTR<IBrowserSession> g_browser_session;

BridgeSession* BridgeSession::s_instance = nullptr;
mutex BridgeSession::s_mtx;

BridgeSession* BridgeSession::getInstance()
{
    if (s_instance == nullptr) {
        lock_guard<mutex> lock(s_mtx);
        if (s_instance == nullptr) {
          #ifdef NDK_AIDL
            static BridgeSession bridge_session;
            s_instance = &bridge_session;
          #else
            s_instance = new BridgeSession();
          #endif
        }
    }
    return s_instance;
}

STATUS BridgeSession::initialise(const SH_PTR<IBrowserSession>& browser)
{
   STATUS status;
   bool enabled;

   if (browser == nullptr)
   {
      LOGE("dvb is null")
   }
   else
   {
      g_browser_session = browser;
      LOGI("")
      //status = dvb->getSubtitlesEnabled(&enabled);
      //LOGI("en=%d ok%d", enabled, status.isOk())
   }

   return STATUS::ok();
}

STATUS BridgeSession::executeRequest(const vector<uint8_t>& jsonstr, vector<uint8_t>* result)
{
#ifdef NDK_AIDL
    // this cannot be implemented in NDK
#else
    std::string json_request(jsonstr.begin(), jsonstr.end());
    LOGI("json_request=%s", json_request.c_str());
#endif

    return STATUS::ok();
}

STATUS BridgeSession::getTvKeyCodeForApp(int32_t a_code, int32_t appId, int32_t* tv_code)
{
    LOGI("")
    *tv_code = 0;
    return STATUS::ok();
}

STATUS BridgeSession::notifyLoadApplicationFailed(int32_t appId)
{
    LOGI("")
    return STATUS::ok();
}

STATUS BridgeSession::notifyApplicationPageChanged(int32_t appId, const vector<uint8_t>& url)
{
    LOGI("")
    return STATUS::ok();
}

STATUS BridgeSession::LoadDsmccDvbUrl(const vector<uint8_t>& dvb_url, int32_t requestId)
{
    LOGI("")
    return STATUS::ok();
}

#ifdef NDK_AIDL
} // namespace aidl
#endif
