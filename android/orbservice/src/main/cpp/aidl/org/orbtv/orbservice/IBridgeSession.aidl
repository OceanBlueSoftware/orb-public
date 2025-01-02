/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
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
/* IOrbcSession.aidl - interface to ORB C++ from DVB integration */

package org.orbtv.orbservice;

import org.orbtv.orbservice.IBrowserSession;

interface IBridgeSession {

    void initialise(in IBrowserSession browser);

    byte[] executeRequest(in byte[] jsonstr);

}
