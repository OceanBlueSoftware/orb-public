
/**
 * ORB Software. Copyright (c) 2025 Ocean Blue Software Limited
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

#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <json/json.h>
#include "JsonRpcService.h"

/**
* ORB JsonRpcService Utility static Functions
*/
namespace orb
{
namespace networkServices { 
// Constants
#define F_SUBTITLES "subtitles"
#define F_DIALOGUE_ENHANCEMENT "dialogueEnhancement"
#define F_UI_MAGNIFIER "uiMagnifier"
#define F_HIGH_CONTRAST_UI "highContrastUI"
#define F_SCREEN_READER "screenReader"
#define F_RESPONSE_TO_USER_ACTION "responseToUserAction"
#define F_AUDIO_DESCRIPTION "audioDescription"
#define F_IN_VISION_SIGNING "inVisionSigning"

#define PC_SUBTITLES "subtitlesPrefChange"
#define PC_DIALOGUE_ENHANCEMENT "dialogueEnhancementPrefChange"
#define PC_UI_MAGNIFIER "uiMagnifierPrefChange"
#define PC_HIGH_CONTRAST_UI "highContrastUIPrefChange"
#define PC_SCREEN_READER "screenReaderPrefChange"
#define PC_RESPONSE_TO_USER_ACTION "responseToUserActionPrefChange"
#define PC_AUDIO_DESCRIPTION "audioDescriptionPrefChange"
#define PC_IN_VISION_SIGNING "inVisionSigningPrefChange"

#define MD_NEGOTIATE_METHODS "org.hbbtv.negotiateMethods"
#define MD_SUBSCRIBE "org.hbbtv.subscribe"
#define MD_UNSUBSCRIBE "org.hbbtv.unsubscribe"
#define MD_NOTIFY "org.hbbtv.notify"

#define MD_AF_FEATURE_SUPPORT_INFO "org.hbbtv.af.featureSupportInfo"
#define MD_AF_FEATURE_SETTINGS_QUERY "org.hbbtv.af.featureSettingsQuery"
#define MD_AF_FEATURE_SUPPRESS "org.hbbtv.af.featureSuppress"

#define MD_AF_DIALOGUE_ENHANCEMENT_OVERRIDE "org.hbbtv.af.dialogueEnhancementOverride"
#define MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION "org.hbbtv.af.triggerResponseToUserAction"

#define MD_VOICE_READY "org.hbbtv.app.voice.ready"
#define MD_STATE_MEDIA "org.hbbtv.app.state.media"

#define MD_INTENT_MEDIA_PAUSE "org.hbbtv.app.intent.media.pause"
#define MD_INTENT_MEDIA_PLAY "org.hbbtv.app.intent.media.play"
#define MD_INTENT_MEDIA_FAST_FORWARD "org.hbbtv.app.intent.media.fast-forward"
#define MD_INTENT_MEDIA_FAST_REVERSE "org.hbbtv.app.intent.media.fast-reverse"
#define MD_INTENT_MEDIA_STOP "org.hbbtv.app.intent.media.stop"
#define MD_INTENT_MEDIA_SEEK_CONTENT "org.hbbtv.app.intent.media.seek-content"
#define MD_INTENT_MEDIA_SEEK_RELATIVE "org.hbbtv.app.intent.media.seek-relative"
#define MD_INTENT_MEDIA_SEEK_LIVE "org.hbbtv.app.intent.media.seek-live"
#define MD_INTENT_MEDIA_SEEK_WALLCLOCK "org.hbbtv.app.intent.media.seek-wallclock"
#define MD_INTENT_SEARCH "org.hbbtv.app.intent.search"
#define MD_INTENT_DISPLAY "org.hbbtv.app.intent.display"
#define MD_INTENT_PLAYBACK "org.hbbtv.app.intent.playback"

// OPAPP Video Window ==> TERMINAL
#define MD_IPPLAYBACK_STATUS_UPDATE "org.hbbtv.ipplayback.statusUpdate"
#define MD_IPPLAYBACK_MEDIA_POSITION_UPDATE "org.hbbtv.ipplayback.mediaPositionUpdate"
#define MD_IPPLAYBACK_SET_COMPONENTS "org.hbbtv.ipplayback.setComponents"
#define MD_IPPLAYBACK_SET_TIMELINE_MAPPING "org.hbbtv.ipplayback.setTimelineMapping"
#define MD_IPPLAYBACK_SET_PRESENT_FOLLOWING "org.hbbtv.ipplayback.setPresentFollowing"

// TERMINAL ==> OPAPP Video Window
#define MD_IPPLAYER_SELECT_CHANNEL "org.hbbtv.ipplayer.selectChannel"
#define MD_IPPLAYER_STOP "org.hbbtv.ipplayer.stop"
#define MD_IPPLAYER_PLAY "org.hbbtv.ipplayer.play"
#define MD_IPPLAYER_SET_VIDEO_WINDOW "org.hbbtv.ipplayer.setVideoWindow"
#define MD_IPPLAYER_SET_RELATIVE_VOLUME "org.hbbtv.ipplayer.setRelativeVolume"
#define MD_IPPLAYER_PAUSE "org.hbbtv.ipplayer.pause"
#define MD_IPPLAYER_RESUME "org.hbbtv.ipplayer.resume"
#define MD_IPPLAYER_SEEK "org.hbbtv.ipplayer.seek"
#define MD_IPPLAYER_SELECT_COMPONENTS "org.hbbtv.ipplayer.selectComponents"
#define MD_IPPLAYER_RESOLVE_TIMELINE "org.hbbtv.ipplayer.resolveTimeline"

#define OPTIONAL_INT_NOT_SET -999999
#define OPTIONAL_STR_NOT_SET ""

    const static std::map<std::string, int> ACCESSIBILITY_FEATURE_IDS = {
        {F_SUBTITLES, 0},
        {F_DIALOGUE_ENHANCEMENT, 1},
        {F_UI_MAGNIFIER, 2},
        {F_HIGH_CONTRAST_UI, 3},
        {F_SCREEN_READER, 4},
        {F_RESPONSE_TO_USER_ACTION, 5},
        {F_AUDIO_DESCRIPTION, 6},
        {F_IN_VISION_SIGNING, 7},
    };

    const static std::map<int, std::string> ACCESSIBILITY_FEATURE_NAMES = {
        {0, F_SUBTITLES},
        {1, F_DIALOGUE_ENHANCEMENT},
        {2, F_UI_MAGNIFIER},
        {3, F_HIGH_CONTRAST_UI},
        {4, F_SCREEN_READER},
        {5, F_RESPONSE_TO_USER_ACTION},
        {6, F_AUDIO_DESCRIPTION},
        {7, F_IN_VISION_SIGNING},
    }; 
   class JsonRpcServiceUtil {
    public: 
        static Json::Value QuerySettingsSubtitles(bool enabled, int size, const std::string &fontFamily,
            const std::string &textColour, int textOpacity, const std::string &edgeType, const std::string
            &edgeColour, const std::string &backgroundColour, int backgroundOpacity, const std::string
            &windowColour, int windowOpacity, const std::string &language);

        static Json::Value QuerySettingsDialogueEnhancement(int dialogueEnhancementGainPreference,
            int dialogueEnhancementGain, int dialogueEnhancementLimitMin, int dialogueEnhancementLimitMax);

        static Json::Value QuerySettingsUIMagnifier(bool enabled, const std::string &magType);

        static Json::Value QuerySettingsHighContrastUI(bool enabled, const std::string &hcType);

        static Json::Value QuerySettingsScreenReader(bool enabled, int speed, const std::string &voice,
            const std::string &language);

        static Json::Value QuerySettingsResponseToUserAction(bool enabled, const std::string &type);

        static Json::Value QuerySettingsAudioDescription(bool enabled, int gainPreference, int
            panAzimuthPreference);

        static Json::Value GetMethodsInJsonArray(const std::unordered_set<std::string>& set);

        static bool IsMethodInSet(const std::unordered_set<std::string> &set, const std::string& method);

        static bool IsMethodInJsonArray(const Json::Value& array, const std::string& method);

        static bool HasParam(const Json::Value &json, const std::string &param, const
            Json::ValueType& type);

        static bool HasJsonParam(const Json::Value &json, const std::string &param);

        static std::string EncodeJsonId(const Json::Value& id);

        static Json::Value DecodeJsonId(const std::string& id);

        static Json::Value CreateFeatureSettingsQuery(const std::string& feature, const Json::Value& value);

        static Json::Value CreateNotifyRequest(const Json::Value& params);

        static Json::Value CreateClientRequest(const std::string& id, const std::string& method, const
            Json::Value& params);

        static Json::Value CreateJsonResponse(const std::string& id, const Json::Value& result);

        static Json::Value CreateJsonErrorResponse(const std::string& id, const Json::Value& error);

        static std::string GetErrorMessage(JsonRpcService::JsonRpcStatus status);

        static std::string GetAccessibilityFeatureName(int id);

        static int GetAccessibilityFeatureId(const std::string& name);

        static std::time_t ConvertISO8601ToSecond(const std::string& input);

        static std::string ConvertSecondToISO8601(const int sec); 

        static std::string GetId(const Json::Value& obj); 

        static int GetAccessibilityFeatureId(const Json::Value& obj);

        static void AddArrayToJson(Json::Value &json, const std::string &key, const std::vector<int> &array);

        static std::string GetStringValueFromJson(const Json::Value &json, const std::string &key);

        static int GetIntValueFromJson(const Json::Value &json, const std::string &key);

        static bool GetBoolValueFromJson(const Json::Value &json, const std::string &key);
        
    };
} // namespace networkServices
} // namespace orb
