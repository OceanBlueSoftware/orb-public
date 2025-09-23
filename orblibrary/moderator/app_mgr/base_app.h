#ifndef BASE_APP_H
#define BASE_APP_H

#include <string>

#include "ait.h"

namespace orb
{

class ApplicationSessionCallback;

class BaseApp
{
public:
    typedef enum
    {
        INVALID_APP_TYPE = 0,
        HBBTV_APP_TYPE,
        OPAPP_TYPE
    } E_APP_TYPE;

    BaseApp(E_APP_TYPE type, const std::string &url, ApplicationSessionCallback *sessionCallback)
        : m_sessionCallback(sessionCallback)
        , m_type(type)
        , m_id(++g_id)
        , m_loadedUrl(url)
        , m_baseUrl(url)
        , m_entryUrl(url)
    {}

    BaseApp(E_APP_TYPE type, ApplicationSessionCallback *sessionCallback)
        : m_sessionCallback(sessionCallback)
        , m_type(type)
        , m_id(++g_id)
    {}

    virtual ~BaseApp() {}

    BaseApp(const BaseApp&) = delete;
    BaseApp& operator=(const BaseApp&) = delete;

    E_APP_TYPE GetType() const { return m_type; }

    /**
     * Set URL of app from Ait description and URL params
     *
     * @param desc The AIT description the new App will use to set its initial state.
     * @param urlParams Additional url parameters that will be concatenated with the
     *      loaded url of the new App.
     * @param isNetworkAvailable The network availability. Will be used to
     *      determine the protocolId.
     */
    virtual void SetUrl(
        const Ait::S_AIT_APP_DESC &desc,
        const std::string &urlParams,
        bool isNetworkAvailable) {}

    int GetId() const { return m_id; }

    std::string GetLoadedUrl() const { return m_loadedUrl; }

    void SetLoadedUrl(const std::string& url) { m_loadedUrl = url; }

protected:
    ApplicationSessionCallback *m_sessionCallback;

private:
    static int g_id;
    const E_APP_TYPE m_type;
    int m_id;
    std::string m_loadedUrl;
    std::string m_baseUrl;
    std::string m_entryUrl;
};

} // namespace orb

#endif
