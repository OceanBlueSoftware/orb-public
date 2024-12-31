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

#include <android/binder_auto_utils.h>

#include "aidl/org/orbtv/orbservice/BnDvbiSession.h"

using namespace std;
using ndk::ScopedAStatus;

namespace aidl::org::orbtv::orbservice {

class DvbiSession : public BnDvbiSession {
public:
   DvbiSession();

public:
   ScopedAStatus getCapabilities(vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getAudioProfiles(vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getVideoProfiles(vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getVideoDisplayFormats(vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getExtraSDVideoDecodes(int32_t* _aidl_return) override;
   ScopedAStatus getExtraHDVideoDecodes(int32_t* _aidl_return) override;
   ScopedAStatus getExtraUHDVideoDecodes(int32_t* _aidl_return) override;
   ScopedAStatus getSystemInformation(std::vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getPreferredAudioLanguage(std::vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getPreferredSubtitleLanguage(std::vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getPreferredUILanguage(vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getCountryId(vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getSubtitlesEnabled(bool* _aidl_return) override;
   ScopedAStatus getAudioDescriptionEnabled(bool* _aidl_return) override;
   ScopedAStatus getDttNetworkIds(vector<int32_t>* _aidl_return) override;
   ScopedAStatus getVolume(int32_t* _aidl_return) override;
   ScopedAStatus setVolume(int32_t in_volume, bool* _aidl_return) override;
   ScopedAStatus overrideComponentSelection(int32_t in_type, const vector<uint8_t>& in_id) override;
   ScopedAStatus restoreComponentSelection(int32_t in_type) override;
   ScopedAStatus setDvbVideoRectangle(int32_t in_x, int32_t in_y, int32_t in_width, int32_t in_height) override;
   ScopedAStatus getChannelList(vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getCurrentCcid(vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getChannel(const vector<uint8_t>& in_ccid, vector<uint8_t>* _aidl_return) override;
   ScopedAStatus setChannelByTriplet(int32_t in_idType, int32_t in_onid, int32_t in_tsid, int32_t in_sid, int32_t in_sourceID, const vector<uint8_t>& in_ipBroadcastID, bool in_trickplay, const vector<uint8_t>& in_contentAccessDescriptorURL, int32_t in_quiet, int32_t* _aidl_return) override;
   ScopedAStatus setChannelByDsd(const vector<uint8_t>& in_dsd, int32_t in_sid, bool in_trickplay, const vector<uint8_t>& in_contentAccessDescriptorURL, int32_t in_quiet, int32_t* _aidl_return) override;
   ScopedAStatus setChannelToCcid(const vector<uint8_t>& in_ccid, bool in_trickplay, const vector<uint8_t>& in_contentAccessDescriptorURL, int32_t in_quiet, int32_t* _aidl_return) override;

};

} // namespace aidl::org::orbtv::orbservice
