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

import org.orbtv.orbservice.IDvbiSession;

interface IOrbcSession {

    void initialise(in IDvbiSession dvb);

    /**
     * Requests the HbbTV engine to process the specified AIT. The HbbTV engine expects the relevant
     * AITs only (the first one after HBBTV_Start and when the version/PID changes). If more than one
     * stream is signalled in the PMT for a service with an application_signalling_descriptor, then
     * the application_signalling_descriptor for the stream containing the AIT for the HbbTV
     * application shall include the HbbTV application_type (0x0010).
     *
     * @param aitPid    PID of the AIT
     * @param serviceId Service ID the AIT refers to
     * @param data      The buffer containing the AIT row data
     */
    void processAIT(int aitPid, int serviceId, in byte[] data);

    /**
     * Called when the service list has changed.
     */
    void onServiceListChanged();

    /**
     * Called when the parental rating of the currently playing service has changed.
     *
     * @param blocked TRUE if the current service is blocked by the parental control system.
     */
    void onParentalRatingChanged(boolean blocked);

    /**
     * Called when there is a change in the set of components being presented.
     *
     * @param componentType Type of component whose presentation has changed.
     */
    void onSelectedComponentChanged(int componentType);

    /**
     * Called when there is a change in the set of components being presented.
     *
     * @param componentType If the presentation has changed for only one component type, this value
     * should be set to BridgeTypes.COMPONENT_TYPE_* for that specific type. If the presentation has
     * changed for more than one component type, this value should be set to
     * BridgeTypes.COMPONENT_TYPE_ANY.
     */
    void onComponentChanged(int componentType);

    /**
     * Called when there is a change in status of the service identified by the DVB triplet.
     *
     * @param onetId         Original Network ID
     * @param transId        Transport Stream ID
     * @param servId         Service ID
     * @param statusCode
     * @param permanentError
     */
    void onChannelStatusChanged(int onetId, int transId, int servId, int statusCode,
                                boolean permanentError);

    /**
     * Called when the present/following events have changed on the current service.
     */
    void onProgrammesChanged();

    /**
     * Called when the video aspect ratio has changed.
     *
     * @param aspectRatioCode Code as defined by TvBrowserTypes.ASPECT_RATIO_*
     */
    void onVideoAspectRatioChanged(int aspectRatioCode);

}
