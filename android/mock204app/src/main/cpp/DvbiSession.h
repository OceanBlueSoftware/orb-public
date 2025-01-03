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

using ndk::ScopedAStatus;

namespace aidl::org::orbtv::orbservice {

class DvbiSession : public BnDvbiSession {
public:
   DvbiSession();

public:
   ScopedAStatus getCapabilities(std::vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getAudioProfiles(std::vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getVideoProfiles(std::vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getVideoDisplayFormats(std::vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getExtraSDVideoDecodes(int32_t* _aidl_return) override;
   ScopedAStatus getExtraHDVideoDecodes(int32_t* _aidl_return) override;
   ScopedAStatus getExtraUHDVideoDecodes(int32_t* _aidl_return) override;

   ScopedAStatus getSystemInformation(std::vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getPreferredAudioLanguage(std::vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getPreferredSubtitleLanguage(std::vector<uint8_t>* _aidl_return) override;
   ScopedAStatus getPreferredUILanguage(std::string* _aidl_return) override;
   ScopedAStatus getCountryId(std::string* _aidl_return) override;
   ScopedAStatus getSubtitlesEnabled(bool* _aidl_return) override;
   ScopedAStatus getAudioDescriptionEnabled(bool* _aidl_return) override;
   ScopedAStatus getCurrentCcid(std::string* _aidl_return) override;

};

} // namespace aidl::org::orbtv::orbservice
