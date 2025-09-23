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

#ifndef OPAPP_H
#define OPAPP_H

#include "base_app.h"
#include "utils.h"


namespace orb
{
class OpApp : public BaseApp
{
public:
    typedef enum
    {
        OPAPP_FOREGROUND_STATE,
        OPAPP_BACKGROUND_STATE,
        OPAPP_TRANSIENT_STATE,
        OPAPP_OVERLAID_FOREGROUND_STATE,
        OPAPP_OVERLAID_TRANSIENT_STATE
    } E_OPAPP_STATE;

    OpApp(ApplicationSessionCallback *sessionCallback);
    virtual ~OpApp() = default;

    OpApp(const BaseApp&) = delete;
    OpApp &operator=(const OpApp&) = delete;

    E_OPAPP_STATE GetState() const { return m_state; }

    /**
     * Set the application state.
     *
     * @param state The desired state to transition to.
     * @returns true if transitioned successfully to the desired state, false otherwise.
     */
    bool SetState(const E_OPAPP_STATE &state);

private:
    bool CanTransitionToState(const E_OPAPP_STATE &state);

    Utils::Timeout m_countdown;
    E_OPAPP_STATE m_state;
};

} // namespace orb

#endif // OPAPP_H
