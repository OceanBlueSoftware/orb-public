/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
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
 *
 * App model
 *
 * Note: This file is part of the platform-agnostic application manager library.
 */

#ifndef HBBTV_SERVICE_OPAPP_H
#define HBBTV_SERVICE_OPAPP_H

#include "app.h"

class OpApp : public App
{
public:
    class SessionCallback : public App::SessionCallback
    {
public:
        virtual void DispatchOperatorApplicationStateChange(uint16_t appId, const std::string &oldState, const std::string &newState) = 0;
        virtual void DispatchOperatorApplicationStateChangeCompleted(uint16_t appId, const std::string &oldState, const std::string &newState) = 0;
        virtual void DispatchOperatorApplicationContextChange(uint16_t appId, const std::string &startupLocation, const std::string &launchLocation = "") = 0;
        virtual void DispatchOpAppUpdate(uint16_t appId, const std::string &updateEvent) = 0;
        virtual ~SessionCallback() = default;
    };

    OpApp(const std::string &url, std::shared_ptr<SessionCallback> sessionCallback);

    OpApp(const Ait::S_AIT_APP_DESC &desc, bool isNetworkAvailable, std::shared_ptr<SessionCallback> sessionCallback);
    
    virtual ~OpApp() = default;

    OpApp(const App&) = delete;
    OpApp &operator=(const OpApp&) = delete;
    
    void SetState(const E_APP_STATE &state) override;
    
    E_APP_TYPE GetType() const override { return App::E_APP_TYPE::OPAPP_TYPE; }
    
    bool TransitionToBroadcastRelated() override;

private:
    bool canTransitionToState(const E_APP_STATE &state);

    Utils::Timeout m_countdown = Utils::Timeout([&] { 
        if (m_state == TRANSIENT_STATE || m_state == OVERLAID_TRANSIENT_STATE)
        {
            SetState(BACKGROUND_STATE);
        }
    });
};

#endif // HBBTV_SERVICE_OPAPP_H