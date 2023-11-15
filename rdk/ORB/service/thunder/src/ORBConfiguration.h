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
 */

#pragma once

#include "Module.h"

using namespace WPEFramework;

/**
 * JSON container for custom config options we want to use for
 * this plugin
 */
class ORBConfiguration : public Core::JSON::Container
{
public:

    ORBConfiguration()
        : Core::JSON::Container()
    {
        // Map a json object name to c++ object
        Add(_T("privateComRpc"), &PrivateComRpcServer);
    }

    ~ORBConfiguration() = default;

    ORBConfiguration(const ORBConfiguration&) = delete;
    ORBConfiguration& operator=(const ORBConfiguration&) = delete;

public:
    Core::JSON::Boolean PrivateComRpcServer;
}; // class ORBConfiguration