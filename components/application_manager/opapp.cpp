#include "opapp.h"
#include "log.h"

#define COUNT_DOWN_TIMEOUT 60000

static std::string opAppStateToString(const OpApp::E_APP_STATE &state);

OpApp::OpApp(const std::string &url, std::shared_ptr<SessionCallback> sessionCallback)
    : App(url, sessionCallback)
{ 
    m_state = BACKGROUND_STATE; // ETSI TS 103 606 V1.2.1 (2024-03) page 36
}

OpApp::OpApp(const Ait::S_AIT_APP_DESC &desc, bool isNetworkAvailable, std::shared_ptr<SessionCallback> sessionCallback)
    : App(
        desc,
        Utils::MakeInvalidDvbTriplet(),
        isNetworkAvailable,
        "",
        true,
        false,
        sessionCallback)
{ 
    m_state = BACKGROUND_STATE; // ETSI TS 103 606 V1.2.1 (2024-03) page 36
}

void OpApp::SetState(const E_APP_STATE &state)
{
    if (state != OVERLAID && canTransitionToState(state))
    {
        if (state != m_state)
        {
            std::string previous = opAppStateToString(m_state);
            std::string next = opAppStateToString(state);
            m_state = state;
            static_cast<SessionCallback*>(m_sessionCallback.get())->DispatchOperatorApplicationStateChange(GetId(), previous, next);
            
            if (state == BACKGROUND_STATE)
            {
                m_sessionCallback->HideApplication(GetId());
            }
            else
            {
                m_sessionCallback->ShowApplication(GetId());
            }
        }
        if ((state | TRANSIENT_STATE) != 0)
        {
            m_countdown.start(std::chrono::milliseconds(COUNT_DOWN_TIMEOUT));
        }
        else
        {
            m_countdown.stop();
        }
    }
    else
    {
        LOG(LOG_INFO, "Invalid state transition: %d -> %d", m_state, state);
    }
}

bool OpApp::TransitionToBroadcastRelated()
{
    // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.2 Note 2
    if (m_state == FOREGROUND_STATE)
    {
        return App::TransitionToBroadcastRelated();
    }
    return false;
}

bool OpApp::canTransitionToState(const E_APP_STATE &state)
{
    if (state != m_state)
    {
        switch (m_state)
        {
            case FOREGROUND_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.2 Page 38
                return (state & (BACKGROUND_STATE | TRANSIENT_STATE)) != 0;
            case TRANSIENT_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.4 Page 41 
            case OVERLAID_TRANSIENT_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.6 Page 42 
            case OVERLAID_FOREGROUND_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.5 Page 41 
                return (state & (FOREGROUND_STATE | BACKGROUND_STATE)) != 0;
            
            default:
                break;
        }
    }
    return true;
}

static std::string opAppStateToString(const OpApp::E_APP_STATE &state)
{
    switch (state)
    {
        case OpApp::BACKGROUND_STATE: return "background";
        case OpApp::FOREGROUND_STATE: return "foreground";
        case OpApp::TRANSIENT_STATE: return "transient";
        case OpApp::OVERLAID_TRANSIENT_STATE: return "overlaid-transient";
        case OpApp::OVERLAID_FOREGROUND_STATE: return "overlaid-foreground";
        default: break;
    }
    // should never get here
    return "undefined";
}