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

#include <jni.h>
#include <android/log.h>

#include <android/binder_ibinder_jni.h>
#include "generated/aidl/org/orbtv/orbservice/IOrbcSession.h"
#include "generated/aidl/org/orbtv/orbservice/IDvbiSession.h"

#include "DvbiSession.h"

#define TAG          "DvbiSession"
#define LOGI(x, ...) __android_log_print(ANDROID_LOG_INFO, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define LOGE(x, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

using aidl::org::orbtv::orbservice::IOrbcSession;
using aidl::org::orbtv::orbservice::IDvbiSession;
using aidl::org::orbtv::orbservice::DvbiSession;
using ndk::ScopedAStatus;

std::shared_ptr<IOrbcSession> g_orb_session;
static DvbiSession g_dvb_session;
static const char g_lang[4] = {"eng"};
static const char g_cnty[4] = {"gbr"};
static const char g_ccid[8] = {"ccid:01"};


extern "C" JNIEXPORT void JNICALL
Java_org_orbtv_mock203app_MainActivity_nativeServiceConnected(
        JNIEnv* env,
        jobject /* this */,
        jobject binder)
{
   AIBinder* pBinder = AIBinder_fromJavaBinder(env, binder);

   const ::ndk::SpAIBinder spBinder(pBinder);
   g_orb_session = IOrbcSession::fromBinder(spBinder);

   ::ndk::SpAIBinder dvbbinder = g_dvb_session.asBinder();
   std::shared_ptr<IDvbiSession> idvb = aidl::org::orbtv::orbservice::DvbiSession::fromBinder(dvbbinder);
   if (aidl::org::orbtv::orbservice::DvbiSession::setDefaultImpl(idvb))
   {
      LOGI("set default ok")
   }
   else
   {
      LOGE("set default failed")
   }

   LOGI("[cpp] onServiceConnected");
}

extern "C" JNIEXPORT void JNICALL
Java_org_orbtv_mock203app_MainActivity_nativeServiceDisconnected(
        JNIEnv* env,
        jobject /* this */)
{
   g_orb_session = nullptr;

   LOGI("[cpp] onServiceDisconnected");
}


extern "C" JNIEXPORT void JNICALL
Java_org_orbtv_mock203app_MainActivity_nativeTest(
        JNIEnv* env,
        jobject /* this */)
{
   ScopedAStatus status;
   vector<uint8_t> buffer(4);

   status = g_orb_session->onServiceListChanged();
   if (status.isOk())
   {
      LOGI("onServiceListChanged success")
   }
   else
   {
      LOGI("onServiceListChanged failed")
   }
   std::shared_ptr<IDvbiSession> idvb = DvbiSession::getDefaultImpl();
   if (idvb == nullptr)
   {
       LOGE("idvb is null")
   }
   status = g_orb_session->initialise(idvb);
   if (status.isOk())
   {
      LOGI("intialise() success")
   }
   else
   {
      LOGE("intialise() failed")
   }
    status = g_orb_session->processAIT(123, 9897, buffer);
    if (status.isOk())
    {
        LOGI("processAIT() success")
    }
    else
    {
        LOGI("processAIT() failed")
    }

    LOGI("");
}

DvbiSession::DvbiSession()
{
}

// ----------------------------------------------------------------------------------
//              Start of Interface functions required by ORB
// ----------------------------------------------------------------------------------

ScopedAStatus DvbiSession::getCapabilities(vector<uint8_t>* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getAudioProfiles(vector<uint8_t>* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getVideoProfiles(vector<uint8_t>* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getVideoDisplayFormats(vector<uint8_t>* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getExtraSDVideoDecodes(int32_t* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getExtraHDVideoDecodes(int32_t* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getExtraUHDVideoDecodes(int32_t* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getSystemInformation(std::vector<uint8_t>* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getPreferredAudioLanguage(std::vector<uint8_t>* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getPreferredSubtitleLanguage(std::vector<uint8_t>* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getPreferredUILanguage(vector<uint8_t>* lang)
{
    ScopedAStatus status;
    vector<uint8_t> lng(g_lang, g_lang + 3);

    LOGI("")
    *lang =  lng;

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getCountryId(vector<uint8_t>* country_id)
{
    ScopedAStatus status;
    vector<uint8_t> cid(g_cnty, g_cnty + 3);

    LOGI("")
    *country_id = cid;

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getSubtitlesEnabled(bool* enabled)
{
    ScopedAStatus status;

    LOGI("")
    *enabled = false;

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}


ScopedAStatus DvbiSession::getAudioDescriptionEnabled(bool* enabled)
{
   ScopedAStatus status;

   LOGI("")
   *enabled = false;

   status.set(AStatus_fromStatus(STATUS_OK));
   return status;
}

ScopedAStatus DvbiSession::getDttNetworkIds(vector<int32_t>* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getVolume(int32_t* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::setVolume(int32_t in_volume, bool* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::overrideComponentSelection(int32_t in_type, const vector<uint8_t>& in_id)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::restoreComponentSelection(int32_t in_type)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::setDvbVideoRectangle(int32_t in_x, int32_t in_y, int32_t in_width, int32_t in_height)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getChannelList(vector<uint8_t>* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getCurrentCcid(vector<uint8_t> *pccid)
{
    ScopedAStatus status;
    vector<uint8_t> ccid(g_ccid, g_ccid + 7);

    LOGI("")
    *pccid = ccid;

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::getChannel(const vector<uint8_t>& in_ccid, vector<uint8_t>* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::setChannelByTriplet(int32_t in_idType, int32_t in_onid, int32_t in_tsid, int32_t in_sid, int32_t in_sourceID,
    const vector<uint8_t>& in_ipBroadcastID, bool in_trickplay, const vector<uint8_t>& in_contentAccessDescriptorURL, int32_t in_quiet, int32_t* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::setChannelByDsd(const vector<uint8_t>& in_dsd, int32_t in_sid, bool in_trickplay,
    const vector<uint8_t>& in_contentAccessDescriptorURL, int32_t in_quiet, int32_t* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

ScopedAStatus DvbiSession::setChannelToCcid(const vector<uint8_t>& in_ccid, bool in_trickplay,
    const vector<uint8_t>& in_contentAccessDescriptorURL, int32_t in_quiet, int32_t* _aidl_return)
{
    ScopedAStatus status;

    LOGI("")

    status.set(AStatus_fromStatus(STATUS_OK));
    return status;
}

