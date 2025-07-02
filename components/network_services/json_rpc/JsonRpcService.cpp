/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "JsonRpcServiceUtil.h"
#include "log.h"
#include "JsonRpcService.h"

#include <iostream>
#include <sstream>

namespace orb {
namespace networkServices {

const int sizeOfAccessibilityFeature = ACCESSIBILITY_FEATURE_IDS.size();

JsonRpcService::JsonRpcService(
    int port,
    const std::string &endpoint,
    std::unique_ptr<ISessionCallback> sessionCallback) :
    WebSocketService("JsonRpcService", port, false, "lo"),
    m_endpoint(endpoint),
    m_sessionCallback(std::move(sessionCallback))
{
    LOGI("JsonRpcService::JsonRpcService: endpoint=" << m_endpoint << ", port=" << port);    
    RegisterJsonRPCMethods();
    RegisterSupportedMethods();
}

bool JsonRpcService::OnConnection(WebSocketConnection *connection)
{
    if (connection->Uri() != m_endpoint)
    {
        LOGE("Unknown endpoint received. Got: " + connection->Uri() +
            ", expected: " + m_endpoint);
        return false;
    }
    LOGI("Connected: connectionId=" << connection->Id());
    InitialConnectionData(connection->Id());
    return true;
}

void JsonRpcService::OnMessageReceived(WebSocketConnection *connection, const std::string &text)
{
    int connectionId = connection->Id();
    LOGI("Message received: connection=" << connectionId << ", text=" << text);
    // Parse request
    Json::Value obj;
    Json::Reader reader;
    JsonRpcStatus status = JsonRpcStatus::UNKNOWN;
 
    if (!reader.parse(text, obj))
    {
        status = JsonRpcStatus::PARSE_ERROR;
        return handleError(connectionId, status, obj);
    }

    if (!JsonRpcServiceUtil::HasParam(obj, "jsonrpc", Json::stringValue) || obj["jsonrpc"] != "2.0")
    {
        status = JsonRpcStatus::INVALID_REQUEST;
        return handleError(connectionId, status, obj);
    }

    //case of error message
    if (JsonRpcServiceUtil::HasJsonParam(obj, "error"))
    {
        status = JsonRpcService::ReceiveError(connectionId, obj);
        if (status != JsonRpcStatus::SUCCESS)
        {
            return handleError(connectionId, status, obj);
        }
    }

    //case with method
    std::string method;
    if (JsonRpcServiceUtil::HasParam(obj, "method", Json::stringValue))
    {
        method = obj["method"].asString();
        Json::Value negotiateMethods = GetConnectionData(connectionId,
            ConnectionDataType::NegotiateMethodsAppToTerminal);
        if (!negotiateMethods.isArray())
        {
            LOGE("Warning, connection data lost, parameter has wrong type.");
        }
        if (!JsonRpcServiceUtil::IsMethodInJsonArray(negotiateMethods, method))
        {
            status = JsonRpcStatus::METHOD_NOT_FOUND;
            return handleError(connectionId, status, obj);
        }
        else if (JsonRpcServiceUtil::HasJsonParam(obj, "result") &&
                 JsonRpcServiceUtil::HasParam(obj["result"], "method", Json::stringValue))
        {
            //case with method in result
            method = obj["result"]["method"].asString();
        }
        else
        {
            //cannot find parameter of "method"
            status = JsonRpcStatus::INVALID_REQUEST;
            return handleError(connectionId, status, obj);
        }
    }

    auto it = m_json_rpc_methods.find(method);
    if (it != m_json_rpc_methods.end())
    {
        status = it->second(connectionId, obj);
    }
    else
    {
        status = JsonRpcStatus::METHOD_NOT_FOUND;
    }

    if (status == JsonRpcStatus::NOTIFICATION_ERROR)
    {
        LOGE("Error in notification message");
    } 
    else if (status != JsonRpcStatus::SUCCESS)
    {
        handleError(connectionId, status, obj);
    }
}

void JsonRpcService::handleError(int connectionId, JsonRpcStatus status, const Json::Value& obj) 
{
    int code = static_cast<int>(status);
    std::string message = JsonRpcServiceUtil::GetErrorMessage(status);
    std::string id = OPTIONAL_STR_NOT_SET;
    if (status != JsonRpcStatus::PARSE_ERROR &&
        status != JsonRpcStatus::INVALID_REQUEST)
     {
        id = JsonRpcServiceUtil::GetId(obj);
     }
     RespondError(connectionId, id, code, message);
}

void JsonRpcService::OnDisconnected(WebSocketConnection *connection)
{
    std::lock_guard<std::recursive_mutex> lock(mConnectionsMutex);
    m_connectionData.erase(connection->Id());
}

void JsonRpcService::OnServiceStopped()
{
}

void JsonRpcService::RegisterSupportedMethods()
{
    m_supported_methods_app_to_terminal.insert(MD_NEGOTIATE_METHODS);
    m_supported_methods_app_to_terminal.insert(MD_SUBSCRIBE);
    m_supported_methods_app_to_terminal.insert(MD_UNSUBSCRIBE);
    m_supported_methods_app_to_terminal.insert(MD_AF_FEATURE_SUPPORT_INFO);
    m_supported_methods_app_to_terminal.insert(MD_AF_FEATURE_SETTINGS_QUERY);
    m_supported_methods_app_to_terminal.insert(MD_AF_FEATURE_SUPPRESS);
    m_supported_methods_app_to_terminal.insert(MD_AF_DIALOGUE_ENHANCEMENT_OVERRIDE);
    m_supported_methods_app_to_terminal.insert(MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION);
    m_supported_methods_app_to_terminal.insert(MD_VOICE_READY);
    m_supported_methods_app_to_terminal.insert(MD_STATE_MEDIA);

    m_supported_methods_terminal_to_app.insert(MD_NOTIFY);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_PAUSE);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_PLAY);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_FAST_FORWARD);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_FAST_REVERSE);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_STOP);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_SEEK_CONTENT);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_SEEK_RELATIVE);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_SEEK_LIVE);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_SEEK_WALLCLOCK);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_SEARCH);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_DISPLAY);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_PLAYBACK);

    // OPAPP ==> TERMINAL
    m_supported_methods_opapp_to_terminal.insert(MD_IPPLAYBACK_STATUS_UPDATE);
    m_supported_methods_opapp_to_terminal.insert(MD_IPPLAYBACK_MEDIA_POSITION_UPDATE);
    m_supported_methods_opapp_to_terminal.insert(MD_IPPLAYBACK_SET_COMPONENTS);
    m_supported_methods_opapp_to_terminal.insert(MD_IPPLAYBACK_SET_TIMELINE_MAPPING);
    m_supported_methods_opapp_to_terminal.insert(MD_IPPLAYBACK_SET_PRESENT_FOLLOWING);
    // TERMINAL ==> OPAPP
    m_supported_methods_terminal_to_opapp.insert(MD_IPPLAYER_SELECT_CHANNEL);
    m_supported_methods_terminal_to_opapp.insert(MD_IPPLAYER_STOP);
    m_supported_methods_terminal_to_opapp.insert(MD_IPPLAYER_PLAY);
    m_supported_methods_terminal_to_opapp.insert(MD_IPPLAYER_SET_VIDEO_WINDOW);
    m_supported_methods_terminal_to_opapp.insert(MD_IPPLAYER_SET_RELATIVE_VOLUME);
    m_supported_methods_terminal_to_opapp.insert(MD_IPPLAYER_PAUSE);
    m_supported_methods_terminal_to_opapp.insert(MD_IPPLAYER_RESUME);
    m_supported_methods_terminal_to_opapp.insert(MD_IPPLAYER_SEEK);
    m_supported_methods_terminal_to_opapp.insert(MD_IPPLAYER_SELECT_COMPONENTS);
    m_supported_methods_terminal_to_opapp.insert(MD_IPPLAYER_RESOLVE_TIMELINE);
}

void JsonRpcService::RegisterJsonRPCMethods() 
{
    // Register methods to handle JSON-RPC requests and responses from applications
    RegisterMethod(MD_NEGOTIATE_METHODS, &JsonRpcService::RequestNegotiateMethods);
    RegisterMethod(MD_SUBSCRIBE, &JsonRpcService::RequestSubscribe);
    RegisterMethod(MD_UNSUBSCRIBE, &JsonRpcService::RequestUnsubscribe);

    RegisterMethod(MD_AF_FEATURE_SUPPORT_INFO, &JsonRpcService::RequestFeatureSupportInfo);
    RegisterMethod(MD_AF_FEATURE_SETTINGS_QUERY, &JsonRpcService::RequestFeatureSettingsQuery);
    RegisterMethod(MD_AF_FEATURE_SUPPRESS, &JsonRpcService::RequestFeatureSuppress);

    RegisterMethod(MD_AF_DIALOGUE_ENHANCEMENT_OVERRIDE,
        &JsonRpcService::RequestDialogueEnhancementOverride);
    RegisterMethod(MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION,
        &JsonRpcService::RequestTriggerResponseToUserAction);

    RegisterMethod(MD_VOICE_READY, &JsonRpcService::NotifyVoiceReady);
    RegisterMethod(MD_STATE_MEDIA, &JsonRpcService::NotifyStateMedia);
    RegisterMethod(MD_INTENT_MEDIA_PAUSE, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_INTENT_MEDIA_PLAY, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_INTENT_MEDIA_FAST_FORWARD, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_INTENT_MEDIA_FAST_REVERSE, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_INTENT_MEDIA_STOP, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_INTENT_MEDIA_SEEK_CONTENT, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_INTENT_MEDIA_SEEK_RELATIVE, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_INTENT_MEDIA_SEEK_LIVE, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_INTENT_MEDIA_SEEK_WALLCLOCK, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_INTENT_SEARCH, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_INTENT_DISPLAY, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_INTENT_PLAYBACK, &JsonRpcService::ReceiveConfirm);

    // Register methods to handle JSON-RPC requests from OpApp
    RegisterMethod(MD_IPPLAYBACK_STATUS_UPDATE, &JsonRpcService::RequestIPPlaybackStatusUpdate);
    RegisterMethod(MD_IPPLAYBACK_MEDIA_POSITION_UPDATE, &JsonRpcService::RequestIPPlaybackMediaPositionUpdate);
    RegisterMethod(MD_IPPLAYBACK_SET_COMPONENTS, &JsonRpcService::RequestIPPlaybackSetComponents);
    RegisterMethod(MD_IPPLAYBACK_SET_TIMELINE_MAPPING, &JsonRpcService::RequestIPPlaybackSetTimelineMapping);
    RegisterMethod(MD_IPPLAYBACK_SET_PRESENT_FOLLOWING, &JsonRpcService::RequestIPPlaybackSetPresentFollowing);

    // Register methods to handle JSON-RPC response from OpApp
    RegisterMethod(MD_IPPLAYER_SELECT_CHANNEL, &JsonRpcService::ReceiveConfirmForSelectChannel);
    RegisterMethod(MD_IPPLAYER_STOP, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_IPPLAYER_PLAY, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_IPPLAYER_SET_VIDEO_WINDOW, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_IPPLAYER_SET_RELATIVE_VOLUME, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_IPPLAYER_PAUSE, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_IPPLAYER_RESUME, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_IPPLAYER_SEEK, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_IPPLAYER_SELECT_COMPONENTS, &JsonRpcService::ReceiveConfirm);
    RegisterMethod(MD_IPPLAYER_RESOLVE_TIMELINE, &JsonRpcService::ReceiveConfirm);
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestNegotiateMethods(int connectionId, const
    Json::Value &obj)
{
    std::string id = JsonRpcServiceUtil::GetId(obj);
    if(id.empty())
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }

    if (!JsonRpcServiceUtil::HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!JsonRpcServiceUtil::HasParam(obj["params"], "terminalToApp", Json::arrayValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!JsonRpcServiceUtil::HasParam(obj["params"], "appToTerminal", Json::arrayValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    Json::Value terminalToApp = obj["params"]["terminalToApp"];
    Json::Value appToTerminal = obj["params"]["appToTerminal"];

    //add method to set
    Json::Value filteredTerminalToApp = FilterMethods(connectionId, terminalToApp,
        false);
    Json::Value filteredAppToTerminal = FilterMethods(connectionId, appToTerminal,
        true);

    m_sessionCallback->RequestNegotiateMethods();
    RespondNegotiateMethods(connectionId, id,
        filteredTerminalToApp,
        filteredAppToTerminal);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestSubscribe(int connectionId, const
    Json::Value &obj)
{    
    std::string id = JsonRpcServiceUtil::GetId(obj);
    if(id.empty())
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }

    if (!JsonRpcServiceUtil::HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!JsonRpcServiceUtil::HasParam(obj["params"], "msgType", Json::arrayValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    Json::Value msgType = obj["params"]["msgType"];

    SubscribeOptions options;
    for (const auto& msg: msgType)
    {
        int length = msg.asString().length();
        std::string feature = msg.asString().substr(0, length - 10);     // Remove PrefChange
        int featureId = JsonRpcServiceUtil::GetAccessibilityFeatureId(feature);
        if (featureId > -1 && featureId < sizeOfAccessibilityFeature)
        {
            SetSubscribeOptions(options, featureId);
            SetConnectionData(connectionId,
                ConnectionDataType::SubscribedMethods,
                msg.asString());
        }
        else
        {
            return JsonRpcStatus::INVALID_PARAMS;
        }
    }
    m_sessionCallback->RequestSubscribe(options);
    RespondSubscribe(connectionId, id, msgType);
    return JsonRpcStatus::SUCCESS;
}

void JsonRpcService::SetSubscribeOptions(SubscribeOptions &options, int featureId)
{
    std::string featureName = JsonRpcServiceUtil::GetAccessibilityFeatureName(featureId);
    if (featureName == F_SUBTITLES)
    {
        options.subtitles = true;
    }
    else if (featureName == F_DIALOGUE_ENHANCEMENT)
    {
        options.dialogueEnhancement = true;     
    }
    else if (featureName == F_UI_MAGNIFIER)
    {
        options.uiMagnifier = true;
    }
    else if (featureName == F_HIGH_CONTRAST_UI)
    {
        options.highContrastUI = true;
    }
    else if (featureName == F_SCREEN_READER)
    {
        options.screenReader = true;
    }
    else if (featureName == F_RESPONSE_TO_USER_ACTION)
    {
        options.responseToUserAction = true;
    }
    else if (featureName == F_AUDIO_DESCRIPTION)
    {
        options.audioDescription = true;
    }
    else if (featureName == F_IN_VISION_SIGNING)
    {
        options.inVisionSigning = true;
    } else
    {
        LOGE("Unknown feature ID: " << featureId);
    }
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestUnsubscribe(int connectionId, const
    Json::Value &obj)
{
    std::string id = JsonRpcServiceUtil::GetId(obj);
    if(id.empty())
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }

    if (!JsonRpcServiceUtil::HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!JsonRpcServiceUtil::HasParam(obj["params"], "msgType", Json::arrayValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    Json::Value msgType = obj["params"]["msgType"];

    SubscribeOptions options;
    for (const auto& msg: msgType)
    {
        int length = msg.asString().length();
        std::string feature = msg.asString().substr(0, length - 10);     // Remove PrefChange
        int featureId = JsonRpcServiceUtil::GetAccessibilityFeatureId(feature);
        if (featureId > -1 && featureId < sizeOfAccessibilityFeature)
        {
            SetSubscribeOptions(options, featureId);
            SetConnectionData(connectionId, ConnectionDataType::UnsubscribedMethods,
                msg.asString());
        }
        else
        {
            return JsonRpcStatus::INVALID_PARAMS;
        }
    }
    m_sessionCallback->RequestUnsubscribe(options);
    RespondUnsubscribe(connectionId, id, msgType);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestFeatureSupportInfo(int connectionId, const
    Json::Value &obj)
{
    return HandleFeatureRequest(connectionId, obj, MD_AF_FEATURE_SUPPORT_INFO);
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestFeatureSettingsQuery(int connectionId, const
    Json::Value &obj)
{

    return HandleFeatureRequest(connectionId, obj, MD_AF_FEATURE_SETTINGS_QUERY);
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestFeatureSuppress(int connectionId, const
    Json::Value &obj)
{
    return HandleFeatureRequest(connectionId, obj, MD_AF_FEATURE_SUPPRESS);
}

 JsonRpcService::JsonRpcStatus JsonRpcService::HandleFeatureRequest(int connectionId, const Json::Value &obj, 
        const std::string &feature) 
{
    std::string id = JsonRpcServiceUtil::GetId(obj);
    if(id.empty())
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    int featureId = JsonRpcServiceUtil::GetAccessibilityFeatureId(obj);
    
    if (featureId == -1)
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }

    if (feature == MD_AF_FEATURE_SUPPORT_INFO) 
    {
        m_sessionCallback->RequestFeatureSupportInfo(connectionId, id, featureId);
    } 
    else if (feature == MD_AF_FEATURE_SETTINGS_QUERY) 
    {
        m_sessionCallback->RequestFeatureSettingsQuery(connectionId, id, featureId);
    } 
    else if (feature == MD_AF_FEATURE_SUPPRESS) 
    {
        m_sessionCallback->RequestFeatureSuppress(connectionId, id, featureId);
    } 
    else 
    {
        return JsonRpcStatus::INVALID_REQUEST;
    }
    return JsonRpcStatus::SUCCESS;
}


JsonRpcService::JsonRpcStatus JsonRpcService::RequestDialogueEnhancementOverride(int connectionId,
    const Json::Value &obj)
{
    std::string id = JsonRpcServiceUtil::GetId(obj);
    if(id.empty())
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }

    int dialogueEnhancementGain = OPTIONAL_INT_NOT_SET;
    if (JsonRpcServiceUtil::HasJsonParam(obj, "params"))
    {
        Json::Value params = obj["params"];
        if (JsonRpcServiceUtil::HasParam(params, "dialogueEnhancementGain", Json::intValue))
        {
            dialogueEnhancementGain = params["dialogueEnhancementGain"].asInt();
        }
        else
        {
            RespondError(connectionId, id, -24, "Dialogue Enhancement override failed");
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
    }
    m_sessionCallback->RequestDialogueEnhancementOverride(connectionId, id,
        dialogueEnhancementGain);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestTriggerResponseToUserAction(
    int connectionId, const Json::Value &obj)
{
    std::string id = JsonRpcServiceUtil::GetId(obj);
    if(id.empty())
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!JsonRpcServiceUtil::HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!JsonRpcServiceUtil::HasParam(obj["params"], "magnitude", Json::stringValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string magnitude = obj["params"]["magnitude"].asString();
    if (magnitude != "triggerPrimary" && magnitude != "triggerSecondary" && magnitude !=
        "triggerException")
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    m_sessionCallback->RequestTriggerResponseToUserAction(connectionId, id, magnitude);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::NotifyVoiceReady(int connectionId, const
    Json::Value &obj)
{
    if (!JsonRpcServiceUtil::HasJsonParam(obj, "params") ||
        !JsonRpcServiceUtil::HasParam(obj["params"], "ready", Json::booleanValue))
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    bool voiceReady = obj["params"]["ready"].asBool();
    SetConnectionData(connectionId, ConnectionDataType::VoiceReady, voiceReady);
    m_sessionCallback->NotifyVoiceReady(voiceReady);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::NotifyStateMedia(int connectionId, const
    Json::Value &obj)
{
    Json::Value params;
    if (!JsonRpcServiceUtil::HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    params = obj["params"];

    if (!JsonRpcServiceUtil::HasParam(params, "state", Json::stringValue))
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    std::string state = params["state"].asString();
    if (state != "no-media" &&
        state != "error" &&
        state != "buffering" &&
        state != "paused" &&
        state != "playing" &&
        state != "stopped")
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    std::unique_ptr<ConnectionData> mediaData(new ConnectionData);
    mediaData->state = state;
    if (!JsonRpcServiceUtil::HasJsonParam(params, "availableActions"))
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    
    Json::Value actions = params["availableActions"];
    mediaData->actionPause =  GetActionValue(actions, "pause"); //HasParam(actions, "pause", Json::booleanValue) && actions["pause"];
    mediaData->actionPlay = GetActionValue(actions, "play");
    mediaData->actionFastForward = GetActionValue(actions, "fast-forward");
    mediaData->actionFastReverse = GetActionValue(actions, "fast-reverse");
    mediaData->actionStop = GetActionValue(actions, "stop");
    mediaData->actionSeekContent = GetActionValue(actions, "seek-content");
    mediaData->actionSeekRelative = GetActionValue(actions, "seek-relative");
    mediaData->actionSeekLive = GetActionValue(actions, "seek-live");
    mediaData->actionSeekWallclock = GetActionValue(actions, "seek-wallclock");

    if (state == "no-media" || state == "error")
    {
        SetStateMediaToConnectionData(connectionId, *mediaData);
        m_sessionCallback->NotifyStateMedia(state);
        return JsonRpcStatus::SUCCESS;
    }

    // state is "buffering", "paused", "playing" or "stopped"
    std::string kind = JsonRpcServiceUtil::GetStringValueFromJson(params, "kind");
    std::string type = JsonRpcServiceUtil::GetStringValueFromJson(params, "type");
    if (kind.empty() || (kind != "audio" && kind != "audio-video") ||
        type.empty() || (type != "live" && type != "on-demand"))
    {
        LOGE("Invalid settings for either type or kind in notification message");
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }

    if (!JsonRpcServiceUtil::HasJsonParam(params, "metadata"))
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    Json::Value metadata = params["metadata"];
    mediaData->mediaId = JsonRpcServiceUtil::GetStringValueFromJson(metadata, "mediaId");
    mediaData->title = JsonRpcServiceUtil::GetStringValueFromJson(metadata, "title");
    mediaData->secondTitle = JsonRpcServiceUtil::GetStringValueFromJson(metadata, "secondaryTitle");
    mediaData->synopsis = JsonRpcServiceUtil::GetStringValueFromJson(metadata, "synopsis");
    if (mediaData->title.empty())
    {
        LOGE("Invalid value for metadata title in notification message");
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }

    if (state == "stopped")
    {
        SetStateMediaToConnectionData(connectionId, *mediaData);
        m_sessionCallback->NotifyStateMedia(state);
        return JsonRpcStatus::SUCCESS;
    }

    // state is "buffering", "paused" or "playing"
    Json::Value current = params["currentTime"];
    Json::Value range = params["range"];
    if (!params.isMember("currentTime") || !JsonRpcServiceUtil::HasJsonParam(params, "range") ||
        !range.isMember("start") || !range.isMember("end"))
    {
        LOGE("A required item is missing in notification message");
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    Json::Value start = range["start"];
    Json::Value end = range["end"];
    int biasTime = 0;
    int systemTime = std::time(nullptr);
    int currentTime = -1;
    int startTime = -1;
    int endTime = -1;
    bool isCurrentNum = current.type() == Json::intValue || current.type() == Json::realValue;
    bool isStartNum = start.type() == Json::intValue || start.type() == Json::realValue;
    bool isEndNum = end.type() == Json::intValue || end.type() == Json::realValue;
    if (isCurrentNum && isStartNum && isEndNum)
    {
        double curr = params["currentTime"].asDouble();
        double st = systemTime + (start.asDouble() - curr);
        double e = systemTime + (end.asDouble() - curr);
        if (st < INT_MIN || st > INT_MAX || e < INT_MIN || e > INT_MAX)
        {
            LOGE("Invalid time and range of media in notification message");
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
        currentTime = systemTime;
        startTime = (int) st;
        endTime = (int) e;
    }
    else if (current.type() == Json::stringValue && current.type() == Json::stringValue &&
             current.type() == Json::stringValue)
    {
        currentTime = JsonRpcServiceUtil::ConvertISO8601ToSecond(current.asString());
        biasTime = currentTime - systemTime;
        startTime = JsonRpcServiceUtil::ConvertISO8601ToSecond(start.asString());
        endTime = JsonRpcServiceUtil::ConvertISO8601ToSecond(end.asString());
    }
    if (systemTime < 0 || currentTime < 0 || startTime < 0 || endTime < 0 ||
        !(currentTime >= startTime && currentTime <= endTime))
    {
        LOGE("Invalid time and range of media in notification message");
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    mediaData->biasToSystemTime = biasTime;
    mediaData->startTime = startTime;
    mediaData->endTime = endTime;

    Json::Value subtitles;
    Json::Value audioDescription;
    Json::Value signLanguage;
    bool subtitlesEnabled = false;
    bool subtitlesAvailable = false;
    bool audioDescripEnabled = false;
    bool audioDescripAvailable = false;
    bool signLangEnabled = false;
    bool signLangAvailable = false;

    if (JsonRpcServiceUtil::HasJsonParam(params, "accessibility"))
    {
        if (JsonRpcServiceUtil::HasJsonParam(params["accessibility"], "subtitles"))
        {
            subtitles = params["accessibility"]["subtitles"];
            subtitlesEnabled = JsonRpcServiceUtil::GetBoolValueFromJson(subtitles, "enabled");
            subtitlesAvailable = JsonRpcServiceUtil::GetBoolValueFromJson(subtitles, "available");
        }

        if (JsonRpcServiceUtil::HasJsonParam(params["accessibility"], "audioDescription"))
        {
            audioDescription = params["accessibility"]["audioDescription"];
            audioDescripEnabled = JsonRpcServiceUtil::GetBoolValueFromJson(audioDescription, "enabled");
            audioDescripAvailable = JsonRpcServiceUtil::GetBoolValueFromJson(audioDescription, "available");
        }

        if (JsonRpcServiceUtil::HasJsonParam(params["accessibility"], "signLanguage"))
        {
            signLanguage = params["accessibility"]["signLanguage"];
            signLangEnabled = JsonRpcServiceUtil::GetBoolValueFromJson(signLanguage, "enabled");
            signLangAvailable = JsonRpcServiceUtil::GetBoolValueFromJson(signLanguage, "available");
        }
    }

    LOGI("NotifyStateMedia: connectionId=" << connectionId
        << ", state=" << state
        << ", mediaId=" << mediaData->mediaId
        << ", title=" << mediaData->title
        << ", secondTitle=" << mediaData->secondTitle
        << ", synopsis=" << mediaData->synopsis
        << ", currentTime=" << currentTime
        << ", startTime=" << startTime
        << ", endTime=" << endTime
        << ", subtitlesEnabled=" << subtitlesEnabled
        << ", subtitlesAvailable=" << subtitlesAvailable
        << ", audioDescripEnabled=" << audioDescripEnabled
        << ", audioDescripAvailable=" << audioDescripAvailable
        << ", signLangEnabled=" << signLangEnabled
        << ", signLangAvailable=" << signLangAvailable);

    if (state == "buffering" || state == "paused" || state == "playing")
    {
        if (!JsonRpcServiceUtil::HasParam(subtitles, "enabled", Json::booleanValue) ||
            !JsonRpcServiceUtil::HasParam(subtitles, "available", Json::booleanValue) ||
            !JsonRpcServiceUtil::HasParam(audioDescription, "enabled", Json::booleanValue) ||
            !JsonRpcServiceUtil::HasParam(audioDescription, "available", Json::booleanValue) ||
            !JsonRpcServiceUtil::HasParam(signLanguage, "enabled", Json::booleanValue) ||
            !JsonRpcServiceUtil::HasParam(signLanguage, "available", Json::booleanValue))
        {
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
    }
    SetStateMediaToConnectionData(connectionId, *mediaData);
    m_sessionCallback->NotifyStateMedia(state);
    return JsonRpcStatus::SUCCESS;
}

bool JsonRpcService::GetActionValue(const Json::Value &actions, const std::string &actionName)
{
    return JsonRpcServiceUtil::HasParam(actions, actionName, Json::booleanValue) && actions[actionName].asBool();
}

JsonRpcService::JsonRpcStatus JsonRpcService::ReceiveConfirm(int connectionId, const
    Json::Value &obj)
{
    std::string id = JsonRpcServiceUtil::GetId(obj);
    if(id.empty())
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    Json::Value result = obj["result"];
    if (!JsonRpcServiceUtil::HasParam(result, "method", Json::stringValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }    
    std::string method = result["method"].asString();
    m_sessionCallback->ReceiveConfirm(connectionId, id, method);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::ReceiveConfirmForSelectChannel(int connectionId, const Json::Value &obj) 
{
    std::string id = JsonRpcServiceUtil::GetId(obj);
    Json::Value result = obj["result"];
    if(id.empty() 
    || !JsonRpcServiceUtil::HasParam(result, "method", Json::stringValue) 
    || !JsonRpcServiceUtil::HasParam(result, "sessionID", Json::intValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string method = result["method"].asString();
    int sessionId = result["sessionID"].asInt();
    m_sessionCallback->ReceiveConfirmForSelectChannel(connectionId, id, method, sessionId);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::ReceiveError(int connectionId,
    const Json::Value &obj)
{
    Json::Value error = obj["error"];
    std::string id = JsonRpcServiceUtil::GetId(obj);
    if(id.empty())
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }

    int code = error["code"].asInt();
    std::string message = JsonRpcServiceUtil::GetStringValueFromJson(error, "message");
    std::string method = JsonRpcServiceUtil::GetStringValueFromJson(error, "method");
    std::string data = JsonRpcServiceUtil::GetStringValueFromJson(error, "data");
    if (method != OPTIONAL_STR_NOT_SET || data != OPTIONAL_STR_NOT_SET)
    {
        m_sessionCallback->ReceiveError(code, message, method, data);
    }
    else
    {
        m_sessionCallback->ReceiveError(code, message);
    }
    return JsonRpcStatus::SUCCESS;
}

void JsonRpcService::RespondFeatureSupportInfo(int connectionId, const std::string& id,
    int featureId,
    const std::string& value)
{
    Json::Value result;

    result["method"] = MD_AF_FEATURE_SUPPORT_INFO;
    if (featureId < 0 || featureId >= sizeOfAccessibilityFeature)
    {
        return;
    }
    result["feature"] = JsonRpcServiceUtil::GetAccessibilityFeatureName(featureId);
    result["value"] = value;
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsSubtitles(int connectionId, const std::string &id,
    bool enabled,
    int size, const std::string &fontFamily,
    const std::string &textColour,
    int textOpacity,
    const std::string &edgeType,
    const std::string &edgeColour,
    const std::string &backgroundColour,
    int backgroundOpacity,
    const std::string &windowColour,
    int windowOpacity,
    const std::string &language)
{
    Json::Value value = JsonRpcServiceUtil::QuerySettingsSubtitles(enabled, size, fontFamily, textColour, textOpacity,
        edgeType, edgeColour, backgroundColour, backgroundOpacity, windowColour, windowOpacity,
        language);
    Json::Value result = JsonRpcServiceUtil::CreateFeatureSettingsQuery(F_SUBTITLES, value);
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsDialogueEnhancement(int connectionId,
    const std::string &id,
    int dialogueEnhancementGainPreference,
    int dialogueEnhancementGain,
    int dialogueEnhancementLimitMin,
    int dialogueEnhancementLimitMax)
{
    Json::Value value = JsonRpcServiceUtil::QuerySettingsDialogueEnhancement(dialogueEnhancementGainPreference,
        dialogueEnhancementGain, dialogueEnhancementLimitMin, dialogueEnhancementLimitMax);
    Json::Value result = JsonRpcServiceUtil::CreateFeatureSettingsQuery(F_DIALOGUE_ENHANCEMENT, value);
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsUIMagnifier(int connectionId, const std::string &id,
    bool enabled,
    const std::string &magType)
{
    Json::Value value = JsonRpcServiceUtil::QuerySettingsUIMagnifier(enabled, magType);
    Json::Value result = JsonRpcServiceUtil::CreateFeatureSettingsQuery(F_UI_MAGNIFIER, value);
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsHighContrastUI(int connectionId,
    const std::string &id,
    bool enabled,
    const std::string &hcType)
{
    Json::Value value = JsonRpcServiceUtil::QuerySettingsHighContrastUI(enabled, hcType);
    Json::Value result = JsonRpcServiceUtil::CreateFeatureSettingsQuery(F_HIGH_CONTRAST_UI, value);
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsScreenReader(int connectionId,
    const std::string &id,
    bool enabled,
    int speed,
    const std::string &voice,
    const std::string &language)
{
    Json::Value value = JsonRpcServiceUtil::QuerySettingsScreenReader(enabled, speed, voice, language);
    Json::Value result = JsonRpcServiceUtil::CreateFeatureSettingsQuery(F_SCREEN_READER, value);
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsResponseToUserAction(int connectionId,
    const std::string &id,
    bool enabled,
    const std::string &type)
{
    Json::Value value = JsonRpcServiceUtil::QuerySettingsResponseToUserAction(enabled, type);
    Json::Value result = JsonRpcServiceUtil::CreateFeatureSettingsQuery(F_RESPONSE_TO_USER_ACTION, value);
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsAudioDescription(int connectionId, const std::string &id,
    bool enabled,
    int gainPreference,
    int panAzimuthPreference)
{
    Json::Value value = JsonRpcServiceUtil::QuerySettingsAudioDescription(enabled, gainPreference,
        panAzimuthPreference);
    Json::Value result = JsonRpcServiceUtil::CreateFeatureSettingsQuery(F_AUDIO_DESCRIPTION, value);
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsInVisionSigning(int connectionId, const std::string &id,
    bool enabled)
{
    Json::Value value;
    value["enabled"] = enabled;
    Json::Value result = JsonRpcServiceUtil::CreateFeatureSettingsQuery(F_IN_VISION_SIGNING, value);
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSuppress(int connectionId, const std::string &id,
    int featureId,
    const std::string &value)
{
    Json::Value result;
    result["method"] = MD_AF_FEATURE_SUPPRESS;
    if (featureId < 0 || featureId >= sizeOfAccessibilityFeature)
    {
        return;
    }
    result["feature"] = JsonRpcServiceUtil::GetAccessibilityFeatureName(featureId);
    result["value"] = value;
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondSubscribe(int connectionId, const std::string &id,
    const Json::Value &msgTypeList)
{
    Json::Value result;
    result["msgType"] = msgTypeList;
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondUnsubscribe(int connectionId, const std::string &id,
    const Json::Value &msgTypeList)
{
    Json::Value result;
    result["msgType"] = msgTypeList;
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondNegotiateMethods(int connectionId, const std::string &id,
    const Json::Value& terminalToApp,
    const Json::Value& appToTerminal)
{
    Json::Value result;
    result["method"] = MD_NEGOTIATE_METHODS;
    result["terminalToApp"] = terminalToApp;
    result["appToTerminal"] = appToTerminal;
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondError(int connectionId, const std::string &id,
    int code, const std::string &message)
{
    Json::Value error;
    error["code"] = code;
    error["message"] = message;
    Json::Value response = JsonRpcServiceUtil::CreateJsonErrorResponse(id, error);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondError(int connectionId, const std::string &id,
    int code, const std::string &message, const std::string &data)
{
    Json::Value error;
    error["code"] = code;
    error["message"] = message;
    if (data != OPTIONAL_STR_NOT_SET)
    {
        error["data"] = data;
    }
    Json::Value response = JsonRpcServiceUtil::CreateJsonErrorResponse(id, error);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::VoiceRequestDescription()
{
    std::vector<int> connectionIds = GetAllConnectionIds();
    for (int connectionId : connectionIds)
    {
        Json::Value title = GetConnectionData(connectionId, ConnectionDataType::Title);
        Json::Value secTitle = GetConnectionData(connectionId, ConnectionDataType::SecondTitle);
        Json::Value synopsis = GetConnectionData(connectionId, ConnectionDataType::Synopsis);
        if (!title.isString() || title.asString().empty())
        {
            LOGE("Warning, connection data lost, parameter has the wrong type.");
            m_sessionCallback->RespondMessage("No media is playing");
            return;
        }
        std::stringstream message;
        message << "You're watching " << title.asString();
        if (secTitle.isString() && !secTitle.asString().empty())
        {
            message << " {Secondary title: " << secTitle.asString() << "}";
        }
        if (synopsis.isString() && !synopsis.asString().empty())
        {
            message << " \nSynopsis: " << synopsis.asString();
        }
        m_sessionCallback->RespondMessage(message.str());
    }
}

void JsonRpcService::SendIntentMediaPause()
{
    Json::Value params;
    params["origin"] = "voice";
    SendIntentMessage(MD_INTENT_MEDIA_PAUSE, params);
}

void JsonRpcService::SendIntentMediaPlay()
{
    Json::Value params;
    params["origin"] = "voice";
    SendIntentMessage(MD_INTENT_MEDIA_PLAY, params);
}

void JsonRpcService::SendIntentMediaFastForward()
{
    Json::Value params;
    params["origin"] = "voice";
    SendIntentMessage(MD_INTENT_MEDIA_FAST_FORWARD, params);
}

void JsonRpcService::SendIntentMediaFastReverse()
{
    Json::Value params;
    params["origin"] = "voice";
    SendIntentMessage(MD_INTENT_MEDIA_FAST_REVERSE, params);
}

void JsonRpcService::SendIntentMediaStop()
{
    Json::Value params;
    params["origin"] = "voice";
    SendIntentMessage(MD_INTENT_MEDIA_STOP, params);
}

void JsonRpcService::SendIntentMediaSeekContent(const std::string &anchor, int offset)
{
    Json::Value params;
    params["origin"] = "voice";
    params["anchor"] = anchor;
    params["offset"] = offset;
    SendIntentMessage(MD_INTENT_MEDIA_SEEK_CONTENT, params);
}

void JsonRpcService::SendIntentMediaSeekRelative(int offset)
{
    Json::Value params;
    params["origin"] = "voice";
    params["offset"] = offset;
    SendIntentMessage(MD_INTENT_MEDIA_SEEK_RELATIVE, params);
}

void JsonRpcService::SendIntentMediaSeekLive(int offset)
{
    Json::Value params;
    params["origin"] = "voice";
    params["offset"] = offset;
    SendIntentMessage(MD_INTENT_MEDIA_SEEK_LIVE, params);
}

void JsonRpcService::SendIntentMediaSeekWallclock(const std::string &dateTime)
{
    Json::Value params;
    params["origin"] = "voice";
    params["date-time"] = dateTime;
    SendIntentMessage(MD_INTENT_MEDIA_SEEK_WALLCLOCK, params);
}

void JsonRpcService::SendIntentSearch(const std::string &query)
{
    Json::Value params;
    params["origin"] = "voice";
    params["query"] = query;
    SendIntentMessage(MD_INTENT_SEARCH, params);
}

void JsonRpcService::SendIntentDisplay(const std::string &mediaId)
{
    Json::Value params;
    params["origin"] = "voice";
    params["mediaId"] = mediaId;
    SendIntentMessage(MD_INTENT_DISPLAY, params);
}

void JsonRpcService::SendIntentPlayback(const std::string &mediaId,
    const std::string &anchor,
    int offset)
{
    Json::Value params;
    params["origin"] = "voice";
    params["mediaId"] = mediaId;
    if (anchor != OPTIONAL_STR_NOT_SET)
    {
        params["anchor"] = anchor;
    }
    if (offset != OPTIONAL_INT_NOT_SET)
    {
        params["offset"] = offset;
    }
    SendIntentMessage(MD_INTENT_PLAYBACK, params);
}

void JsonRpcService::NotifySubtitles(bool enabled,
    int size, const std::string &fontFamily,
    const std::string &textColour, int textOpacity,
    const std::string &edgeType, const std::string &edgeColour,
    const std::string &backgroundColour, int backgroundOpacity,
    const std::string &windowColour, int windowOpacity,
    const std::string &language)
{
    Json::Value params;
    params["msgType"] = PC_SUBTITLES;
    params["value"] = JsonRpcServiceUtil::QuerySettingsSubtitles(enabled, size, fontFamily, textColour, textOpacity,
        edgeType, edgeColour, backgroundColour, backgroundOpacity, windowColour, windowOpacity,
        language);
    SendNotifyMessage(JsonRpcServiceUtil::GetAccessibilityFeatureId(std::string(F_SUBTITLES)), params);
}

void JsonRpcService::NotifyDialogueEnhancement(
    int dialogueEnhancementGainPreference,
    int dialogueEnhancementGain,
    int dialogueEnhancementLimitMin,
    int dialogueEnhancementLimitMax)
{
    Json::Value params;
    params["msgType"] = PC_DIALOGUE_ENHANCEMENT;
    params["value"] = JsonRpcServiceUtil::QuerySettingsDialogueEnhancement(dialogueEnhancementGainPreference,
        dialogueEnhancementGain, dialogueEnhancementLimitMin, dialogueEnhancementLimitMax);
    SendNotifyMessage(JsonRpcServiceUtil::GetAccessibilityFeatureId(std::string(F_DIALOGUE_ENHANCEMENT)), params);
}

void JsonRpcService::NotifyUIMagnifier(bool enabled, const std::string &magType)
{
    Json::Value params;
    params["msgType"] = PC_UI_MAGNIFIER;
    params["value"] = JsonRpcServiceUtil::QuerySettingsUIMagnifier(enabled, magType);
    SendNotifyMessage(JsonRpcServiceUtil::GetAccessibilityFeatureId(std::string(F_UI_MAGNIFIER)), params);
}

void JsonRpcService::NotifyHighContrastUI(bool enabled, const std::string &hcType)
{
    Json::Value params;
    params["msgType"] = PC_HIGH_CONTRAST_UI;
    params["value"] = JsonRpcServiceUtil::QuerySettingsUIMagnifier(enabled, hcType);
    SendNotifyMessage(JsonRpcServiceUtil::GetAccessibilityFeatureId(std::string(F_HIGH_CONTRAST_UI)), params);
}

void JsonRpcService::NotifyScreenReader(bool enabled, int speed, const std::string &voice,
    const std::string &language)
{
    Json::Value params;
    params["msgType"] = PC_SCREEN_READER;
    params["value"] = JsonRpcServiceUtil::QuerySettingsScreenReader(enabled, speed, voice, language);
    SendNotifyMessage(JsonRpcServiceUtil::GetAccessibilityFeatureId(std::string(F_SCREEN_READER)), params);
}

void JsonRpcService::NotifyResponseToUserAction(bool enabled, const std::string &type)
{
    Json::Value params;
    params["msgType"] = PC_RESPONSE_TO_USER_ACTION;
    params["value"] = JsonRpcServiceUtil::QuerySettingsResponseToUserAction(enabled, type);
    SendNotifyMessage(JsonRpcServiceUtil::GetAccessibilityFeatureId(std::string(F_RESPONSE_TO_USER_ACTION)), params);
}

void JsonRpcService::NotifyAudioDescription(bool enabled, int gainPreference, int
    panAzimuthPreference)
{
    Json::Value params;
    params["msgType"] = PC_AUDIO_DESCRIPTION;
    params["value"] = JsonRpcServiceUtil::QuerySettingsAudioDescription(enabled, gainPreference, panAzimuthPreference);
    SendNotifyMessage(JsonRpcServiceUtil::GetAccessibilityFeatureId(std::string(F_AUDIO_DESCRIPTION)), params);
}

void JsonRpcService::NotifyInVisionSigning(bool enabled)
{
    Json::Value params;
    params["msgType"] = PC_IN_VISION_SIGNING;
    Json::Value value;
    value["enabled"] = enabled;
    params["value"] = value;
    SendNotifyMessage(JsonRpcServiceUtil::GetAccessibilityFeatureId(std::string(F_IN_VISION_SIGNING)), params);
}

void JsonRpcService::RespondDialogueEnhancementOverride(int connectionId,
    const std::string &id,
    int dialogueEnhancementGain)
{
    Json::Value result;
    result["method"] = MD_AF_DIALOGUE_ENHANCEMENT_OVERRIDE;
    if (dialogueEnhancementGain != OPTIONAL_INT_NOT_SET)
    {
        result["dialogueEnhancementGain"] = dialogueEnhancementGain;
    }
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondTriggerResponseToUserAction(int connectionId,
    const std::string &id,
    bool actioned)
{
    Json::Value result;
    result["method"] = MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION;
    result["actioned"] = actioned;
    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

// Helper functions
/**
 * Generate a unique ID for an intent associated with a given connection.
 *
 * This method increments the intent ID count for the certain connection,
 * generates a unique ID based on the incremented count.
 * Start from "IntentId1", "IntentId2", ...
 *
 * @param connectionId The ID of the connection for which the intent ID is generated.
 *
 * @return A unique JSON-RPC intent ID as a string.
 */
std::string JsonRpcService::GenerateId(int connectionId)
{
    Json::Value data = GetConnectionData(connectionId, ConnectionDataType::IntentIdCount);
    if (data.isInt())
    {
        int intentId = data.asInt();
        intentId++;    //Generate the unique 'id' for intent media method
        std::string id = JsonRpcServiceUtil::EncodeJsonId("IntentId" + std::to_string(intentId));
        SetConnectionData(connectionId,
            ConnectionDataType::IntentIdCount,
            intentId);
        return id;
    }
    LOGE("Warning, connection data lost, parameter has wrong type.");
    return OPTIONAL_STR_NOT_SET;
}

/**
 * This method retrieves the connection IDs who have subscribed and have
 * negotiated support for the 'MD_NOTIFY' method (used for notifications).
 *
 * @param result A vector to store the connection IDs of satisfied clients.
 * @param msgTypeIndex The index of the notification message type in
 *                     ACCESSIBILITY_FEATURE_NAMES predefined list.
 */
void JsonRpcService::GetNotifyConnectionIds(std::vector<int> &result,
    const int msgTypeIndex)
{
    std::string suffix = "PrefChange";
    std::string msgType = ACCESSIBILITY_FEATURE_NAMES.at(msgTypeIndex) + suffix;
    std::vector<int> connectionIds = GetAllConnectionIds();
    for (int i : connectionIds)
    {
        Json::Value subscribedMethods = GetConnectionData(i,
            ConnectionDataType::SubscribedMethods);
        Json::Value negotiateMethods = GetConnectionData(i,
            ConnectionDataType::NegotiateMethodsTerminalToApp);
        if (!subscribedMethods.isArray() || !negotiateMethods.isArray())
        {
            LOGE("Warning, connection data lost, parameter has wrong type.");
            continue;
        }

        if (JsonRpcServiceUtil::IsMethodInJsonArray(subscribedMethods, msgType) &&
            JsonRpcServiceUtil::IsMethodInJsonArray(negotiateMethods, MD_NOTIFY))
        {
            result.push_back(i);
        }
    }
}

/**
 * This method allows the registration of a JSON-RPC method by associating it with
 * its name. The registered method can later be invoked using the provided name
 * in JSON-RPC requests.
 *
 * @param name The name of the method to register.
 * @param method A callable pointer representing the JSON-RPC method to be registered.
 */
void JsonRpcService::RegisterMethod(const std::string &name, JsonRpcMethod method)
{
    m_json_rpc_methods[name] = std::bind(method, this, std::placeholders::_1,
        std::placeholders::_2);
}

/**
 * This method takes a connection ID and a JSON response, sends the formatted
 * response message to the client with the specified connection ID.
 * This method is the exit point of class JsonRpcService.
 *
 * @param connectionId The unique identifier of the client connection.
 * @param jsonResponse The JSON data to be sent as a response to the client.
 */
void JsonRpcService::SendJsonMessageToClient(int connectionId,
    const Json::Value &jsonResponse)
{
    Json::FastWriter writer;
    std::string message = writer.write(jsonResponse);
    std::lock_guard<std::recursive_mutex> lock(mConnectionsMutex);
    WebSocketConnection *connection = GetConnection(connectionId);
    if (connection != nullptr)
    {
        std::ostringstream oss;
        oss << message;
        connection->SendMessage(oss.str());
    }
}

/**
 * Send an intent message to available clients.
 *
 * @param method The name of the intent method to send.
 * @param params The JSON parameters associated with the intent.
 */
void JsonRpcService::SendIntentMessage(const std::string& method, Json::Value &params)
{
    std::vector<int> connectionIds;
    CheckIntentMethod(connectionIds, method);
    for (int connectionId : connectionIds)
    {
        std::string id = GenerateId(connectionId);
        AdjustTimeRange(connectionId, method, params);
        Json::Value response = JsonRpcServiceUtil::CreateClientRequest(id, method, params);
        SendJsonMessageToClient(connectionId, response);
    }
}

/**
 * Send an intent message to available clients.
 *
 * @param method The name of the intent method to send.
 * @param params The JSON parameters associated with the intent.
 */
void JsonRpcService::AdjustTimeRange(int id, const std::string& method, Json::Value &params)
{
    Json::Value bias = GetConnectionData(id, ConnectionDataType::BiasToSystemTime);
    Json::Value start = GetConnectionData(id, ConnectionDataType::StartTime);
    Json::Value end = GetConnectionData(id, ConnectionDataType::EndTime);
    if (!bias.isInt() || !start.isInt() || !end.isInt())
    {
        LOGE("Warning, connection data lost, parameter has wrong type.");
        return;
    }
    int biasTime = bias.asInt();
    int startTime = start.asInt();
    int endTime = end.asInt();
    if (startTime == -1 || endTime == -1)
    {
        LOGE("Warning, connection data lost, parameter is invalid.");
        return;
    }
    int setTime;
    if (method == MD_INTENT_MEDIA_SEEK_CONTENT || method == MD_INTENT_MEDIA_SEEK_RELATIVE ||
        method == MD_INTENT_MEDIA_SEEK_LIVE || (method == MD_INTENT_PLAYBACK && params.isMember(
            "offset")))
    {
        int offset = 0, anchorTime = 0;
        if (params.isMember("anchor") && params["anchor"].asString() == "start")
        {
            anchorTime = startTime;
        }
        else if (params.isMember("anchor") && params["anchor"].asString() == "end")
        {
            anchorTime = endTime;
        }
        else
        {
            anchorTime = std::time(nullptr) + biasTime;
        }
        offset = params["offset"].asInt();
        setTime = anchorTime + offset;
        setTime = std::max(setTime, startTime);
        setTime = std::min(setTime, endTime);
        params["offset"] = (int) (setTime - anchorTime);
    }
    else if (method == MD_INTENT_MEDIA_SEEK_WALLCLOCK)
    {
        setTime = JsonRpcServiceUtil::ConvertISO8601ToSecond(params["date-time"].asString());
        setTime = std::max(setTime, startTime);
        setTime = std::min(setTime, endTime);
        std::string setWallclock = JsonRpcServiceUtil::ConvertSecondToISO8601(setTime);
        params["date-time"] = setWallclock;
    }
}

/**
 * Send a notification message to available clients.
 *
 * @param msgTypeIndex The index of the notification message type in a predefined list.
 * @param params The JSON parameters associated with the notification.
 */
void JsonRpcService::SendNotifyMessage(int msgTypeIndex, const Json::Value &params)
{
    Json::Value response = JsonRpcServiceUtil::CreateNotifyRequest(params);
    std::vector<int> connectionIds;
    GetNotifyConnectionIds(connectionIds, msgTypeIndex);
    for (int connectionId : connectionIds)
    {
        SendJsonMessageToClient(connectionId, response);
    }
}

/**
 * Remove the method not in table.
 *
 * @param connectionId The connection ID.
 * @param methodsList The Json::arrayValue of methods.
 * @param isAppToTerminal The bool shows appToTerminal or terminalToApp.
 *
 * @return The Json::arrayValue of filtered methods
 */
Json::Value JsonRpcService::FilterMethods(int connectionId,
    const Json::Value& methodsList,
    bool isAppToTerminal)
{
    Json::Value newMethodsList;

    bool isOpApp = IsOpApp(connectionId);

    for (const auto& method : methodsList)
    {
        std::string methodName = method.asString();
        if (isAppToTerminal && 
            (JsonRpcServiceUtil::IsMethodInSet(m_supported_methods_app_to_terminal, methodName) 
            || (isOpApp && JsonRpcServiceUtil::IsMethodInSet(m_supported_methods_opapp_to_terminal, methodName))))
        {
            SetConnectionData(connectionId,
                ConnectionDataType::NegotiateMethodsAppToTerminal,
                methodName);
            newMethodsList.append(method);
        }
        else if (!isAppToTerminal && 
            (JsonRpcServiceUtil::IsMethodInSet(m_supported_methods_terminal_to_app, methodName) 
            || (isOpApp && JsonRpcServiceUtil::IsMethodInSet(m_supported_methods_terminal_to_opapp, methodName))))
        {
            SetConnectionData(connectionId,
                ConnectionDataType::NegotiateMethodsTerminalToApp,
                methodName);
            newMethodsList.append(method);
        }
    }
    return newMethodsList;
}

/**
 * This method retrieves connection IDs that are VoiceReady, negotiated,
 * and have been available actions from state media.
 *
 * @param result A vector to store the connection IDs of satisfied clients.
 * @param msgTypeIndex The index of the notification message type in
 *                     ACCESSIBILITY_FEATURE_NAMES predefined list.
 */
void JsonRpcService::CheckIntentMethod(std::vector<int> &result, const std::string& method)
{
    std::vector<int> connectionIds = GetAllConnectionIds();
    for (int i : connectionIds)
    {
        Json::Value voiceReadyJson = GetConnectionData(i, ConnectionDataType::VoiceReady);
        Json::Value negotiateMethodsJson = GetConnectionData(i,
            ConnectionDataType::NegotiateMethodsTerminalToApp);

        if (!voiceReadyJson.isBool() || !negotiateMethodsJson.isArray())
        {
            LOGE("Warning, connection data lost, parameter has wrong type.");
            continue;
        }
        if (!voiceReadyJson.asBool() || !JsonRpcServiceUtil::IsMethodInJsonArray(negotiateMethodsJson, method))
        {
            continue;
        }

        Json::Value stateJson = GetConnectionData(i, ConnectionDataType::State);
        Json::Value actionPauseJson = GetConnectionData(i, ConnectionDataType::ActionPause);
        Json::Value actionPlayJson = GetConnectionData(i, ConnectionDataType::ActionPlay);
        Json::Value actionFastForwardJson = GetConnectionData(i,
            ConnectionDataType::ActionFastForward);
        Json::Value actionFastReverseJson = GetConnectionData(i,
            ConnectionDataType::ActionFastReverse);
        Json::Value actionStopJson = GetConnectionData(i, ConnectionDataType::ActionStop);
        Json::Value actionSeekContentJson = GetConnectionData(i,
            ConnectionDataType::ActionSeekContent);
        Json::Value actionSeekRelativeJson = GetConnectionData(i,
            ConnectionDataType::ActionSeekRelative);
        Json::Value actionSeekLiveJson = GetConnectionData(i, ConnectionDataType::ActionSeekLive);
        Json::Value actionSeekWallclockJson = GetConnectionData(i,
            ConnectionDataType::ActionSeekWallclock);

        if (!stateJson.isString() || !actionPauseJson.isBool() ||
            !actionPlayJson.isBool() || !actionFastForwardJson.isBool() ||
            !actionFastReverseJson.isBool() || !actionStopJson.isBool() ||
            !actionSeekContentJson.isBool() || !actionSeekRelativeJson.isBool() ||
            !actionSeekLiveJson.isBool() || !actionSeekWallclockJson.isBool())
        {
            LOGE("Warning, connection data lost, parameter has wrong type.");
            continue;
        }

        bool shouldAddConnection = false;
        if (method == MD_INTENT_SEARCH || method == MD_INTENT_DISPLAY || method ==
            MD_INTENT_PLAYBACK)
        {
            shouldAddConnection = true;
        }
        else if (stateJson.asString().empty() || stateJson.asString() == "no-media" ||
                 stateJson.asString() == "error")
        {
            continue;
        }
        else if (stateJson.asString() == "stopped")
        {
            shouldAddConnection = (method == MD_INTENT_MEDIA_PLAY && actionPlayJson.asBool());
        }
        else if (method == MD_INTENT_MEDIA_PAUSE && actionPauseJson.asBool())
        {
            shouldAddConnection = stateJson.asString() != "paused";
        }
        else if ((method == MD_INTENT_MEDIA_PLAY && actionPlayJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_FAST_FORWARD && actionFastForwardJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_FAST_REVERSE && actionFastReverseJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_STOP && actionStopJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_SEEK_CONTENT && actionSeekContentJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_SEEK_RELATIVE && actionSeekRelativeJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_SEEK_LIVE && actionSeekLiveJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_SEEK_WALLCLOCK && actionSeekWallclockJson.asBool()))
        {
            shouldAddConnection = true;
        }

        if (shouldAddConnection)
        {
            result.push_back(i);
        }
    }
}

/**
 * Getter Setter function for connection data map. These methods locks the
 * mConnectionsMutex to ensure thread safety while updating the connection data.
 */


/**
 * This method get all register connection IDs.
 *
 * @return A list of connection ids.
 */
std::vector<int> JsonRpcService::GetAllConnectionIds()
{
    std::lock_guard<std::recursive_mutex> lock(mConnectionsMutex);
    std::vector<int> connectionIds;
    for (const auto& entry : m_connectionData)
    {
        connectionIds.push_back(entry.first);
    }
    return connectionIds;
}

/**
 * This method get all register connection IDs.
 *
 * @return A list of connection ids.
 */
void JsonRpcService::InitialConnectionData(int connectionId)
{
    std::lock_guard<std::recursive_mutex> lock(mConnectionsMutex);
    m_connectionData[connectionId].intentIdCount = 0;
    m_connectionData[connectionId].negotiateMethodsAppToTerminal.insert(MD_NEGOTIATE_METHODS);
}

/**
 * This method updates connection data based on the specified type and value.
 *
 * @param connectionId The unique identifier of the connection to update.
 * @param type The type of connection data to update.
 * @param value The new value for the specified connection data.
 */
void JsonRpcService::SetConnectionData(int connectionId, ConnectionDataType type, const
    Json::Value& value)
{
    std::lock_guard<std::recursive_mutex> lock(mConnectionsMutex);
    if (m_connectionData.find(connectionId) == m_connectionData.end())
    {
        return;
    }
    ConnectionData& connectionData = m_connectionData[connectionId];

    switch (type)
    {
        case ConnectionDataType::NegotiateMethodsAppToTerminal:
            connectionData.negotiateMethodsAppToTerminal.insert(value.asString());
            break;
        case ConnectionDataType::NegotiateMethodsTerminalToApp:
            connectionData.negotiateMethodsTerminalToApp.insert(value.asString());
            break;
        case ConnectionDataType::SubscribedMethods:
            connectionData.subscribedMethods.insert(value.asString());
            break;
        case ConnectionDataType::UnsubscribedMethods:
            connectionData.subscribedMethods.erase(value.asString());
            break;
        case ConnectionDataType::IntentIdCount:
            connectionData.intentIdCount = value.asInt();
            break;
        case ConnectionDataType::VoiceReady:
            connectionData.voiceReady = value.asBool();
            break;
        default:
            break;
    }
}

/**
 * This method updates connection data based on the specified type and value.
 *
 * @param connectionId The unique identifier of the connection to update.
 * @param mediaData The data for state media
 */
void JsonRpcService::SetStateMediaToConnectionData(int connectionId, const
    ConnectionData& mediaData)
{
    std::lock_guard<std::recursive_mutex> lock(mConnectionsMutex);
    if (m_connectionData.find(connectionId) == m_connectionData.end())
    {
        return;
    }
    ConnectionData& connectionData = m_connectionData[connectionId];
    connectionData.state = mediaData.state;
    connectionData.actionPause = mediaData.actionPause;
    connectionData.actionPlay = mediaData.actionPlay;
    connectionData.actionFastForward = mediaData.actionFastForward;
    connectionData.actionFastReverse = mediaData.actionFastReverse;
    connectionData.actionStop = mediaData.actionStop;
    connectionData.actionSeekContent = mediaData.actionSeekContent;
    connectionData.actionSeekRelative = mediaData.actionSeekRelative;
    connectionData.actionSeekLive = mediaData.actionSeekLive;
    connectionData.actionSeekWallclock = mediaData.actionSeekWallclock;
    connectionData.biasToSystemTime = mediaData.biasToSystemTime;
    connectionData.startTime = mediaData.startTime;
    connectionData.endTime = mediaData.endTime;
    connectionData.title = mediaData.title;
    connectionData.mediaId = mediaData.mediaId;
    connectionData.secondTitle = mediaData.secondTitle;
    connectionData.synopsis = mediaData.synopsis;
}

/**
 * This method retrieves connection data of the specified type for a given connection ID.
 *
 * @param connectionId The unique identifier of the connection to retrieve data for.
 * @param type The type of connection data to retrieve.
 *
 * @return A Json::Value containing the requested connection data. The structure and content
 * of the Json::Value depend on the specified 'type':
 *   - For NegotiateMethodsAppToTerminal, NegotiateMethodsTerminalToApp, and SubscribedMethods,
 *     the Json::Value is an array of strings.
 *   - For IntentIdCount, the Json::Value is an integer value.
 *   - For State, the Json::Value is a single string value.
 *   - For VoiceReady, ActionPause, ActionPlay, ActionFastForward, ActionFastReverse, ActionStop,
 *     ActionSeekContent, ActionSeekRelative, ActionSeekLive, and ActionSeekWallclock, the Json::Value
 *     is a boolean value.
 *   - For biasToSystemTime, StartTime, EndTime, the value is an int.
 *   - For UnsubscribedMethods, an empty Json::Value is returned (indicating no data).
 */
Json::Value JsonRpcService::GetConnectionData(int connectionId, ConnectionDataType type)
{
    std::lock_guard<std::recursive_mutex> lock(mConnectionsMutex);
    Json::Value value;

    if (m_connectionData.find(connectionId) != m_connectionData.end())
    {
        const ConnectionData& connectionData = m_connectionData[connectionId];

        switch (type)
        {
            case ConnectionDataType::NegotiateMethodsAppToTerminal:
            {
                value = JsonRpcServiceUtil::GetMethodsInJsonArray(connectionData.negotiateMethodsAppToTerminal);
                break;
            }
            case ConnectionDataType::NegotiateMethodsTerminalToApp:
            {
                value = JsonRpcServiceUtil::GetMethodsInJsonArray(connectionData.negotiateMethodsTerminalToApp);
                break;
            }
            case ConnectionDataType::SubscribedMethods:
            {
                value = JsonRpcServiceUtil::GetMethodsInJsonArray(connectionData.subscribedMethods);
                break;
            }
            case ConnectionDataType::UnsubscribedMethods:
                break;
            case ConnectionDataType::IntentIdCount:
                value = connectionData.intentIdCount;
                break;
            case ConnectionDataType::VoiceReady:
                value = connectionData.voiceReady;
                break;
            case ConnectionDataType::State:
                value = connectionData.state;
                break;
            case ConnectionDataType::ActionPause:
                value = connectionData.actionPause;
                break;
            case ConnectionDataType::ActionPlay:
                value = connectionData.actionPlay;
                break;
            case ConnectionDataType::ActionFastForward:
                value = connectionData.actionFastForward;
                break;
            case ConnectionDataType::ActionFastReverse:
                value = connectionData.actionFastReverse;
                break;
            case ConnectionDataType::ActionStop:
                value = connectionData.actionStop;
                break;
            case ConnectionDataType::ActionSeekContent:
                value = connectionData.actionSeekContent;
                break;
            case ConnectionDataType::ActionSeekRelative:
                value = connectionData.actionSeekRelative;
                break;
            case ConnectionDataType::ActionSeekLive:
                value = connectionData.actionSeekLive;
                break;
            case ConnectionDataType::ActionSeekWallclock:
                value = connectionData.actionSeekWallclock;
                break;
            case ConnectionDataType::MediaId:
                value = connectionData.mediaId;
                break;
            case ConnectionDataType::Title:
                value = connectionData.title;
                break;
            case ConnectionDataType::SecondTitle:
                value = connectionData.secondTitle;
                break;
            case ConnectionDataType::Synopsis:
                value = connectionData.synopsis;
                break;
            case ConnectionDataType::BiasToSystemTime:
                value = static_cast<int>(connectionData.biasToSystemTime);
                break;
            case ConnectionDataType::StartTime:
                value = static_cast<int>(connectionData.startTime);
                break;
            case ConnectionDataType::EndTime:
                value = static_cast<int>(connectionData.endTime);
                break;
            default:
                break;
        }
    }
    return value;
}

bool JsonRpcService::IsOpApp(int connectionId) {
    std::lock_guard<std::recursive_mutex> lock(mConnectionsMutex);
    if (m_connectionData.find(connectionId) != m_connectionData.end())
    {
        return m_connectionData[connectionId].opAppEnabled;
    }
    return false;
}

JsonRpcService::JsonRpcStatus JsonRpcService::ResponseIPPlaybackRequest(int connectionId, const std::string &id, const std::string &method)
{
    Json::Value result;
    result["method"] = method;

    Json::Value response = JsonRpcServiceUtil::CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestIPPlaybackStatusUpdate(int connectionId, const Json::Value &obj) 
{
    return HandleIPPlaybackRequest(connectionId, obj, MD_IPPLAYBACK_STATUS_UPDATE);
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestIPPlaybackMediaPositionUpdate(int connectionId, const Json::Value &obj)
{
    return HandleIPPlaybackRequest(connectionId, obj, MD_IPPLAYBACK_MEDIA_POSITION_UPDATE);
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestIPPlaybackSetComponents(int connectionId, const Json::Value &obj)
{
    return HandleIPPlaybackRequest(connectionId, obj, MD_IPPLAYBACK_SET_COMPONENTS);
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestIPPlaybackSetTimelineMapping(int connectionId, const Json::Value &obj)
{
    return HandleIPPlaybackRequest(connectionId, obj, MD_IPPLAYBACK_SET_TIMELINE_MAPPING);
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestIPPlaybackSetPresentFollowing(int connectionId, const Json::Value &obj)
{
    return HandleIPPlaybackRequest(connectionId, obj, MD_IPPLAYBACK_SET_PRESENT_FOLLOWING);
}

JsonRpcService::JsonRpcStatus JsonRpcService::HandleIPPlaybackRequest(int connectionId, const Json::Value &obj, const std::string &method) 
{
    std::string id = JsonRpcServiceUtil::GetId(obj);
    if (id.empty()) {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!JsonRpcServiceUtil::HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }

    Json::Value params = obj["params"];
    if (method == MD_IPPLAYBACK_STATUS_UPDATE) {
        // Handle the IP playback status update request
        m_sessionCallback->RequestIPPlaybackStatusUpdate(params);
    } else if (method == MD_IPPLAYBACK_MEDIA_POSITION_UPDATE) {
       // Handle the IP playback media position update request
        m_sessionCallback->RequestIPPlaybackMediaPositionUpdate(params);
    } else if (method == MD_IPPLAYBACK_SET_COMPONENTS) {
        // Handle the IP playback set components request
        m_sessionCallback->RequestIPPlaybackSetComponents(params);
    } else if (method == MD_IPPLAYBACK_SET_TIMELINE_MAPPING) {
        // Handle the IP playback set timeline mapping request
        m_sessionCallback->RequestIPPlaybackSetTimelineMapping(params);
    } else if (method == MD_IPPLAYBACK_SET_PRESENT_FOLLOWING) {
        // Handle the IP playback set present following request
        m_sessionCallback->RequestIPPlaybackSetPresentFollowing(params);
    }
    return ResponseIPPlaybackRequest(connectionId, id, method);
}

void JsonRpcService::SendIPPlayerSelectChannel(int channelType, int idType, const std::string& ipBroadcastId)
{
    Json::Value params;
    params["channelType"] = channelType;
    params["idType"] = idType;
    params["ipBroadcastID"] = ipBroadcastId;
    SendIPPlayerMessageToClients(MD_IPPLAYER_SELECT_CHANNEL, params);
}

void JsonRpcService::SendIPPlayerPlay(int sessionId) 
{
    SendIPPlayerMessageToClients(MD_IPPLAYER_PLAY, sessionId);
}

void JsonRpcService::SendIPPlayerPause(int sessionId) 
{
    SendIPPlayerMessageToClients(MD_IPPLAYER_PAUSE, sessionId);
}

void JsonRpcService::SendIPPlayerStop(int sessionId) 
{
    SendIPPlayerMessageToClients(MD_IPPLAYER_STOP, sessionId);
}

void JsonRpcService::SendIPPlayerResume(int sessionId) 
{
    SendIPPlayerMessageToClients(MD_IPPLAYER_RESUME, sessionId);
}

void JsonRpcService::SendIPPlayerSeek(int sessionId, int offset, int reference)
{
    Json::Value params;
    params["sessionID"] = sessionId;
    params["offset"] = offset;
    params["reference"] = reference;
    SendIPPlayerMessageToClients(MD_IPPLAYER_SEEK, params);
}

void JsonRpcService::SendIPPlayerSetVideoWindow(int sessionId, int x, int y, int width, int height)
{
    Json::Value params;
    params["sessionID"] = sessionId;
    params["x"] = x;
    params["y"] = y;
    params["width"] = width;
    params["height"] = height;
    SendIPPlayerMessageToClients(MD_IPPLAYER_SET_VIDEO_WINDOW, params);

}
void JsonRpcService::SendIPPlayerSetRelativeVolume(int sessionId, int volume) 
{
    Json::Value params;
    params["sessionID"] = sessionId;
    params["volume"] = volume;
    SendIPPlayerMessageToClients(MD_IPPLAYER_SET_RELATIVE_VOLUME, params);
}

void JsonRpcService::SendIPPlayerSelectComponents(int sessionId, const std::vector<int>& videoComponents,
        const std::vector<int>& audioComponents, const std::vector<int>& subtitleComponents)
{
    Json::Value params;
    params["sessionID"] = sessionId;
    JsonRpcServiceUtil::AddArrayToJson(params, "videoComponents", videoComponents);
    JsonRpcServiceUtil::AddArrayToJson(params, "audioComponents", audioComponents);
    JsonRpcServiceUtil::AddArrayToJson(params, "subtitleComponents", subtitleComponents);
    SendIPPlayerMessageToClients(MD_IPPLAYER_SELECT_COMPONENTS, params);
}

void JsonRpcService::SendIPPlayerResolveTimeline(int sessionId, const std::string& timelineSelector) 
{
    Json::Value params;
    params["sessionID"] = sessionId;
    params["timelineSelector"] = timelineSelector;
    SendIPPlayerMessageToClients(MD_IPPLAYER_RESOLVE_TIMELINE, params);
}

void JsonRpcService::GetIPPlayerConnectionIdsForMethod(std::vector<int> &availableIds, const std::string& method) 
{
    std::vector<int> connectionIds = GetAllConnectionIds();
    for (int connectionId : connectionIds)
    {
        Json::Value negotiateMethodsJson = GetConnectionData(connectionId,
            ConnectionDataType::NegotiateMethodsTerminalToApp);

        if (negotiateMethodsJson.isArray() 
            && JsonRpcServiceUtil::IsMethodInJsonArray(negotiateMethodsJson, method))
        {
            availableIds.push_back(connectionId);
        } else  
        {
            LOGE("Warning, connection data lost, parameter has wrong type.");
        }
    }
}

void JsonRpcService::SendIPPlayerMessageToClients(const std::string& method, const Json::Value &params) 
{
    LOGI("Sending IP Player message to clients with method: " << method);
    std::vector<int> connectionIds;
    GetIPPlayerConnectionIdsForMethod(connectionIds, method);
    for (int connectionId : connectionIds)
    {
        std::string id = GenerateId(connectionId);
        Json::Value request = JsonRpcServiceUtil::CreateClientRequest(id, method, params);
        SendJsonMessageToClient(connectionId, request);
    }
}

void JsonRpcService::SendIPPlayerMessageToClients(const std::string& method, const std::string &sessionId) {
    LOGI("Sending IP Player message to clients with method: " << method << ", sessionId: " << sessionId);
    Json::Value params;
    params["sessionID"] = sessionId;
    SendIPPlayerMessageToClients(method, params);
}

} // namespace network_services

} // namespace orb