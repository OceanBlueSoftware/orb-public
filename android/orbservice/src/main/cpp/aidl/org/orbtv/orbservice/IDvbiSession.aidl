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
/* IDvbiSession.aidl - interface to DVB Integration from ORB C++ */

package org.orbtv.orbservice;

interface IDvbiSession {

    /**
     * Get the current capabilities of the terminal.
     *
     * @return JSON string of Capabilities (see ?? for JSON format)
     */
    byte[] getCapabilities();

    /**
     * Get a list of audio profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
     * the audio_profile element.
     *
     * @return JSON string of audio profiles supported by the terminal (see ?? for JSON format)
     */
    byte[] getAudioProfiles();

    /**
     * Get a list of video profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
     * the video_profile element.
     *
     * @return JSON string of video profiles supported by the terminal (see ?? for JSON format)
     */
    byte[] getVideoProfiles();

    /**
     * If the terminal supports UHD, get a list that describes the highest quality video format the
     * terminal supports, as defined by HBBTV 10.2.4.7 for the video_display_format element;
     * otherwise get an empty list.
     * <p>
     * Note: If the terminal changes its display format based on the content being played, multiple
     * elements may be included in the list when multiple frame rate families are usable or the
     * highest resolution does not support each highest quality parameter.
     *
     * @return JSON string that describes the highest quality video format (see ?? for JSON format)
     */
    byte[] getVideoDisplayFormats();

    /**
     * Get the current number of additional media streams containing SD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     * to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     * due to lack of resources for SD media.
     */
    int getExtraSDVideoDecodes();

    /**
     * Get the current number of additional media streams containing HD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     * to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     * due to lack of resources for HD media.
     */
    int getExtraHDVideoDecodes();

    /**
     * Get the current number of additional media streams containing UHD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     * to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     * due to lack of resources for UHD media.
     */
    int getExtraUHDVideoDecodes();

    /**
     * Get immutable system information.
     *
     * @return JSON string of SystemInformation (see ?? for JSON format)
     */
    byte[] getSystemInformation();

    /**
     * Gets a string containing languages to be used for audio playback, in order of preference.
     *
     * @return Comma separated string of languages (ISO 639-2 codes)
     */
    byte[] getPreferredAudioLanguage();

    /**
     * Gets a string containing languages to be used for subtitles, in order of preference.
     *
     * @return Comma separated string of languages (ISO 639-2 codes)
     */
    byte[] getPreferredSubtitleLanguage();

    /**
     * Gets a string containing languages to be used for the UI, in order of preference.
     *
     * @return Comma separated string of languages (ISO 639-2 codes)
     */
    byte[] getPreferredUILanguage();

    /**
     * Gets a string containing the three character country code identifying the country in which the
     * receiver is deployed.
     *
     * @return Country code (ISO 3166 alpha-3) string
     */
    byte[] getCountryId();

    /**
     * Gets whether subtitles are enabled in the TV context. So HbbTV knows to start subtitle
     * components on channel change, for example.
     *
     * @return true if enabled, false otherwise
     */
    boolean getSubtitlesEnabled();

    /**
     * Gets whether audio description is enabled in the TV context.
     *
     * @return true if enabled, false otherwise
     */
    boolean getAudioDescriptionEnabled();

    /**
     * Get DTT network IDs.
     *
     * @return Array of DTT network IDs.
     */
    int[] getDttNetworkIds();

    /**
     * Returns the actual volume level set.
     *
     * @return Integer value between 0 up to and including 100 to indicate volume level.
     */
    int getVolume();

    /**
     * Adjusts the volume of the currently playing media to the volume as indicated by volume.
     *
     * @param volume Integer value between 0 up to and including 100 to indicate volume level.
     * @return true if the volume has changed. false if the volume has not changed.
     */
    boolean setVolume(int volume);

    /**
     * Override the default component selection of the terminal for the specified type.
     * <p>
     * If id is empty, no component shall be selected for presentation (presentation is explicitly
     * disabled). Otherwise, the specified component shall be selected for presentation.
     * <p>
     * If playback has already started, the presented component shall be updated.
     * <p>
     * Default component selection shall be restored (revert back to the control of the terminal)
     * when: (1) the application terminates, (2) the channel is changed, (3) presentation has not
     * been explicitly disabled and the user selects another track in the terminal UI, or (4) the
     * restoreComponentSelection method is called.
     *
     * @param type Type of component selection to override (COMPONENT_TYPE_* code).
     * @param id   A platform-defined component id or an empty string to disable presentation.
     */
    void overrideComponentSelection(int type, in byte[] id);

    /**
     * Restore the default component selection of the terminal for the specified type.
     * <p>
     * If playback has already started, the presented component shall be updated.
     *
     * @param type Type of component selection override to clear (COMPONENT_TYPE_* code).
     */
    void restoreComponentSelection(int type);

    /**
     * Sets the presentation window of the DVB video. Values are in HbbTV 1280x720 coordinates.
     *
     * @param x      Rectangle definition
     * @param y      Rectangle definition
     * @param width  Rectangle definition
     * @param height Rectangle definition
     */
    void setDvbVideoRectangle(int x, int y, int width, int height);

    /**
     * Get the list of channels available.
     *
     * @return JSON string containing List of channels available (see ?? for JSON format)
     */
    byte[] getChannelList();

    /**
     * Returns the CCID of the current channel
     *
     * @return A CCID on success, an empty string otherwise
     */
   byte[] getCurrentCcid();

    /**
     * Find the channel with the given LCN and return its CCID.
     *
     * @param lcn LCN to find
     * @return A CCID on success, an empty string otherwise
     */
    //String findCcidWithLcn(String lcn);

    /**
     * Get the channel with the given CCID.
     *
     * @param ccid CCID for the required channel
     * @return JSON string of Channel (see ?? for JSON format)
     */
    byte[] getChannel(in byte[] ccid);

    /**
     * Tune to specified channel. The implementation relies on the 'idType' parameter to
     * determine the valid fields that describe the channel. Possible idTypes are:
     * ID_IPTV_SDS/ID_IPTV_URI - where 'ipBroadcastID' and 'sourceId' fields are valid
     * other ID_.. values - where 'onid', 'tsid' and 'sid' fields are valid
     * ID_DVB_SI_DIRECT - is supposed to be handled by setChannelByDsd()
     *
     * @param idType                     The type of channel
     * @param onid                       The original network ID for the required channel.
     * @param tsid                       The transport stream ID for the required channel.
     * @param sid                        The service ID for the required channel.
     * @param sourceID                   The ATSC source_ID of the channel.
     * @param ipBroadcastID              The DVB textual service identifier of the IP broadcast service.
     * @param trickplay                  Ignore unless PVR functionality is supported (does not affect return)
     * @param contentAccessDescriptorURL May be required by DRM-protected IPTV broadcasts
     * @param quiet                      Channel change operation
     *                                   0 - normal tune
     *                                   1 - normal tune and no UI displayed
     *                                   2 - quiet tune (user does not know)
     * @return negative value (e.g. BridgeTypes.CHANNEL_STATUS_CONNECTING) on success, or
     * zero/positive value (see BridgeTypes.CHANNEL_STATUS_.. error values) on failure
     */
    int setChannelByTriplet(int idType, int onid, int tsid, int sid, int sourceID,
                            in byte[] ipBroadcastID, boolean trickplay,
                            in byte[] contentAccessDescriptorURL, int quiet);

    /**
     * Tune to specified channel using DSD.
     *
     * @param dsd                        DSD for the required channel.
     * @param sid                        SID for the required channel.
     * @param trickplay                  Ignore unless PVR functionality is supported (does not affect return)
     * @param contentAccessDescriptorURL May be required by DRM-protected IPTV broadcasts
     * @param quiet                      Channel change operation
     *                                   0 - normal tune
     *                                   1 - normal tune and no UI displayed
     *                                   2 - quiet tune (user does not know)
     * @return negative value (e.g. BridgeTypes.CHANNEL_STATUS_CONNECTING) on success, or
     * zero/positive value (see BridgeTypes.CHANNEL_STATUS_.. error values) on failure
     */
    int setChannelByDsd(in byte[] dsd, int sid, boolean trickplay,
                        in byte[] contentAccessDescriptorURL, int quiet);

    /**
     * @param ccid The CCID of the channel to set.
     * @param ccid The CCID of the channel to set.
     * @param trickplay True if the application has optionally hinted trickplay resources are
     *    required; or false otherwise. Does not affect the success of this operation.
     * @param contentAccessDescriptorURL Optionally, additional information for DRM-protected IPTV
     *    broadcasts; or an empty string otherwise.
     * @param quiet Type of channel change: 0 for normal; 1 for normal, no UI; 2 for quiet (HbbTV
     *    A.2.4.3.2).
     * @return A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    int setChannelToCcid(in byte[] ccid, boolean trickplay,
        in byte[] contentAccessDescriptorURL, int quiet);
}
