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
 * ORB OrbInterface
 *
 */

#include "OrbInterface.h"
#include "log.h"

namespace orb
{

OrbInterface& OrbInterface::instance()
{
    static OrbInterface s_interface;
    return s_interface;
}

OrbInterface::OrbInterface()
{
}

OrbInterface::~OrbInterface()
{
}

Moderator* OrbInterface::getHbbTvModerator()
{
    return &mHbbTvModerator;
}

DvbBroker* OrbInterface::connectDvb(IDvbClient *dvb_client)
{
    mDvbBroker.setDvbClient(dvb_client);
    mHbbTvModerator.setDvbClient(dvb_client);
    return &mDvbBroker;
}

} // namespace orb
