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

#include "OrbcSession.h"

#define TAG                "OrbcSession"
#define LOGI(x, ...)    __android_log_print(ANDROID_LOG_INFO, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define LOGE(x, ...)    __android_log_print(ANDROID_LOG_ERROR, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

#ifdef NDK_AIDL
namespace aidl {
#endif

using org::orbtv::orbservice::OrbcSession;
using org::orbtv::orbservice::IDvbiSession;

static SH_PTR<IDvbiSession> g_dvb_session;

OrbcSession* OrbcSession::s_instance = nullptr;
mutex OrbcSession::s_mtx;

OrbcSession* OrbcSession::getInstance()
{
    if (s_instance == nullptr) {
        lock_guard<mutex> lock(s_mtx);
        if (s_instance == nullptr) {
          #ifdef NDK_AIDL
            static OrbcSession orb_session;
            s_instance = &orb_session;
          #else
            s_instance = new OrbcSession();
          #endif
        }
    }
    return s_instance;
}

STATUS OrbcSession::initialise(const SH_PTR<IDvbiSession>& dvb)
{
   STATUS status;
   bool enabled;

   if (dvb == nullptr)
   {
      LOGE("dvb is null")
   }
   else
   {
      g_dvb_session = dvb;
      LOGI("")
      //status = dvb->getSubtitlesEnabled(&enabled);
      //LOGI("en=%d ok%d", enabled, status.isOk())
   }

   return STATUS::ok();
}

STATUS OrbcSession::processAIT(int32_t aitPid, int32_t serviceId, const vector<uint8_t>& in_data)
{
   STATUS status;
   string ccid;
   vector<uint8_t> vccid;

   LOGI("(%d, %d)", aitPid, serviceId)

   if (g_dvb_session == nullptr)
   {
      LOGE("g_dvb_session is null")
   }
   else
   {
      status = g_dvb_session->getCurrentCcid(&vccid);
      if (status.isOk())
      {
          std::string ccid( vccid.begin(), vccid.end() );
          LOGI("ccid ok %s", ccid.c_str())
      }
   }

   return STATUS::ok();
}

STATUS OrbcSession::onServiceListChanged()
{
   LOGI("")

   return STATUS::ok();
}

STATUS OrbcSession::onParentalRatingChanged(bool blocked)
{
   LOGI("blk?=%d", blocked)

   return STATUS::ok();
}

STATUS OrbcSession::onSelectedComponentChanged(int32_t componentType)
{
   LOGI("ctyp=%d", componentType)

   return STATUS::ok();
}

STATUS OrbcSession::onComponentChanged(int32_t componentType)
{
   LOGI("ctyp=%d", componentType)

   return STATUS::ok();
}

STATUS OrbcSession::onChannelStatusChanged(int32_t onetId, int32_t transId, int32_t servId, int32_t statusCode, bool permanentError)
{
   LOGI("onid=%d tid=%d sid=%d, status=%d, perr?=%d", onetId, transId, servId, statusCode, permanentError)

   return STATUS::ok();
}

STATUS OrbcSession::onProgrammesChanged()
{
   LOGI("")

   return STATUS::ok();
}

STATUS OrbcSession::onVideoAspectRatioChanged(int32_t aspectRatio)
{
   LOGI("aspectRatio?=%d", aspectRatio)

   return STATUS::ok();
}



#ifdef NDK_AIDL
} // namespace aidl
#endif
