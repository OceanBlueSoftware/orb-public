/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#include <cstdio>
#include <string>
#include <wpe/webkit-web-extension.h>
#include <jsc/jsc.h>
#include <thread>
#include <chrono>
#include <memory>

#include "ORBGenericClient.h"
#include "ORBLogging.h"

using namespace orb;

// The ORB client to be used for both synchronous and asynchronous communication with the ORB service.
static std::shared_ptr<ORBGenericClient> s_orbClient;

// The JavaScript context to be used for evaluating custom JavaScript code.
static JSCContext *s_jsContext;

/******************************************************************************
 * Helper method to evaluate JavaScript from the ORB event listener thread
 ******************************************************************************/

/**
 * Dispatch JavaScript event to the frame indicated by the passed user data.
 *
 * @param userData Pointer to the frame
 *
 * @return Always false to prevent from continuously running in a loop
 */
static gboolean EvaluateJavaScript(gpointer javascript)
{
    ORB_LOG_NO_ARGS();
    std::string kScript((const char *)javascript);
    g_autoptr(JSCValue) result = jsc_context_evaluate(s_jsContext, kScript.c_str(),
        kScript.length());
    g_clear_object(&result);

    return false;
}

/******************************************************************************
 * ORB Client callback methods
 ******************************************************************************/

/**
 * Called back by the ORB client upon receipt of the javaScriptEventDispatchRequested notification
 * from the ORB service.
 *
 * @param name       The name (type) of the bridge event to be dispatched
 * @param properties The properties of the bridge event to be dispatched
 */
static void OnJavaScriptEventDispatchRequested(std::string name, std::string properties)
{
    ORB_LOG("name=%s properties=%s", name.c_str(), properties.c_str());
    std::string kScript = "document.dispatchBridgeEvent('" + name + "', " + properties + ")";
    char *javascript = strdup(kScript.c_str());
    g_main_context_invoke_full(nullptr, G_PRIORITY_HIGH_IDLE, EvaluateJavaScript,
        (gpointer) javascript, (GDestroyNotify) free);
}

/**
 * Called back by the ORB client upon receipt of the dvbURLLoaded notification from the ORB service.
 *
 * @param requestId     The original request id
 * @param content       The downloaded DVB URL content
 * @param contentLength The downloaded DVB URL content length (in number of bytes)
 */
static void OnDvbUrlLoaded(int requestId, unsigned char *content, unsigned int contentLength)
{
    // No need to implement this callback method as the web extension shall not be
    // responsible for dealing with DSM-CC
    ORB_LOG("<Not supported>");
}

/**
 * Called back by the ORB client upon receipt of the inputKeyGenerated notification from the ORB
 * service.
 *
 * @param keyCode The key code to be generated on the main frame's JavaScript context
 */
static void OnInputKeyGenerated(int keyCode)
{
    ORB_LOG("keyCode=%d", keyCode);
    std::string kScript = "document.dispatchEvent(new KeyboardEvent('keydown',{keyCode:" +
        std::to_string(keyCode) + "}));";
    g_main_context_invoke(nullptr, EvaluateJavaScript, (gpointer) kScript.c_str());
}

/******************************************************************************
 * wpeBridge.request method implementation in C++
 ******************************************************************************/

/**
 * Execute the specified bridge request.
 *
 * @param request String containing the JSON request
 *
 * @return String containing the JSON response
 */
static char* ExecuteBridgeRequest(const char *request)
{
    ORB_LOG("request=%s", request);
    gchar *response = nullptr;
    if (!s_orbClient)
    {
        ORB_LOG("ORB client not initialised");
        response = g_strdup("{}");
        return response;
    }
    std::string jsonRequest(request);
    std::string jsonResponse = s_orbClient->ExecuteBridgeRequest(jsonRequest);
    ORB_LOG("response=%s", jsonResponse);
    response = g_strdup(jsonResponse.c_str());
    return response;
}

/******************************************************************************
 * Helper methods called upon the window-object-cleared signal
 ******************************************************************************/

/**
 * Expose the wpeBridge object in the specified JavaScript context.
 *
 * @param jsContext The JavaScript context
 */
static void ExposeBridge(JSCContext *jsContext)
{
    ORB_LOG_NO_ARGS();

    std::string objectName = "wpeBridge";
    std::string methodName = "request";

    g_autoptr(JSCValue) jsObject = NULL;
    g_autoptr(JSCValue) jsFunction = NULL;

    // initialise empty named object
    jsObject = jsc_value_new_from_json(jsContext, "{}");
    jsc_context_set_value(jsContext, objectName.c_str(), jsObject);

    // create function and add it to the object
    jsFunction = jsc_value_new_function
        (
        /* context */ jsContext,
        /* name */ methodName.c_str(),
        /* callback */ G_CALLBACK(ExecuteBridgeRequest),
        /* userData */ NULL,
        /* destroyNotify */ NULL,
        /* returnType */ G_TYPE_STRING,
        /* paramCount */ 1,
        /* args */ G_TYPE_STRING
        );
    jsc_value_object_set_property(jsObject, methodName.c_str(), jsFunction);

    // cleanup
    g_clear_object(&jsFunction);
    g_clear_object(&jsObject);
}

/**
 * Expose the document.token object in the specified JavaScript context.
 *
 * @param jsContext The JavaScript context
 * @param uri       The page URI
 */
static void ExposeToken(JSCContext *jsContext, const char *uri)
{
    ORB_LOG_NO_ARGS();
    if (!s_orbClient)
    {
        ORB_LOG("ORB client not initialised");
        return;
    }
    std::string token = s_orbClient->CreateToken(uri);
    std::string javascript = "document.token = " + token + ";";
    g_autoptr(JSCValue) result = jsc_context_evaluate(jsContext, javascript.c_str(),
        javascript.length());
    g_clear_object(&result);
}

/******************************************************************************
 * WPE/WebKit signal handlers
 ******************************************************************************/

/**
 * Callback connected to the window-object-cleared signal.
 *
 * @param world    Pointer to the world script
 * @param page     Pointer to the web page
 * @param frame    Pointer to the frame
 * @param userData Pointer to the user data (or nullptr)
 */
static void OnWindowObjectCleared(WebKitScriptWorld *world, WebKitWebPage *page, WebKitFrame *frame,
    gpointer userData)
{
    ORB_LOG_NO_ARGS();

    if (!webkit_frame_is_main_frame(frame))
    {
        ORB_LOG("Not in main frame, return immediately");
        return;
    }

    if (s_jsContext)
    {
        g_clear_object(&s_jsContext);
    }
    s_jsContext = webkit_frame_get_js_context_for_script_world(frame, world);

    ExposeBridge(s_jsContext);
    ExposeToken(s_jsContext, webkit_frame_get_uri(frame));

    ORB_LOG("uri=%s\n", webkit_frame_get_uri(frame));
    if (s_orbClient)
    {
        s_orbClient->NotifyApplicationPageChanged(webkit_frame_get_uri(frame));
    }
}

/**
 * Callback connected to the 'page-created' signal of the web extension.
 *
 * @param extension Pointer to the web extension
 * @param webPage   Pointer to the created web page
 */
static void OnPageCreated(WebKitWebExtension *extension, WebKitWebPage *webPage, gpointer userData)
{
    ORB_LOG("page_id=%ld uri=%s\n",
        webkit_web_page_get_id(webPage),
        webkit_web_page_get_uri(webPage)
        );
}

/******************************************************************************
 * ORB WPE web extension initialisation method
 ******************************************************************************/

/**
 * Initialise the ORB wpe web extension.
 *
 * @param extension Pointer to the web extension
 * @param userData  Pointer to the user data (or nullptr)
 */
extern "C"
void webkit_web_extension_initialize_with_user_data(WebKitWebExtension *extension, const
    GVariant *userData)
{
    ORB_LOG_NO_ARGS();

    g_signal_connect(webkit_script_world_get_default(), "window-object-cleared", G_CALLBACK(
        OnWindowObjectCleared), (gpointer)userData);
    g_signal_connect(extension, "page-created", G_CALLBACK(OnPageCreated), nullptr);

    // Create the ORB client and subscribe to the ORB events of interest
    if (!s_orbClient)
    {
        ORB_LOG("Create ORB client");
        s_orbClient = CreateORBClient(
            OnJavaScriptEventDispatchRequested,
            OnDvbUrlLoaded,
            OnInputKeyGenerated
            );
        s_orbClient->SubscribeToJavaScriptEventDispatchRequestedEvent();
        s_orbClient->SubscribeToInputKeyGeneratedEvent();
    }
}