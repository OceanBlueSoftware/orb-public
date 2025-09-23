#include "opapp.h"
#include "log.h"
#include "application_manager.h"

#define COUNT_DOWN_TIMEOUT 60000

namespace orb
{

static std::string opAppStateToString(const OpApp::E_OPAPP_STATE &state);

OpApp::OpApp(ApplicationSessionCallback *sessionCallback)
    : BaseApp(OPAPP_TYPE, sessionCallback)
    , m_countdown([&] () { SetState(OPAPP_BACKGROUND_STATE); })
    , m_state(OPAPP_BACKGROUND_STATE) // ETSI TS 103 606 V1.2.1 (2024-03) page 36
{
}

/**
 * Set the application state.
 *
 * @param state The desired state to transition to.
 * @returns true if transitioned successfully to the desired state, false otherwise.
 */
bool OpApp::SetState(const E_OPAPP_STATE &state)
{
    if (CanTransitionToState(state))
    {
        if (state != m_state)
        {
            std::string previous = opAppStateToString(m_state);
            std::string next = opAppStateToString(state);
            m_state = state;
            m_sessionCallback->DispatchOperatorApplicationStateChange(GetId(), previous, next);

            if (state == OPAPP_BACKGROUND_STATE)
            {
                m_sessionCallback->HideApplication(GetId());
            }
            else
            {
                m_sessionCallback->ShowApplication(GetId());
            }
        }

        if (state == OPAPP_TRANSIENT_STATE || state == OPAPP_OVERLAID_TRANSIENT_STATE)
        {
            m_countdown.start(std::chrono::milliseconds(COUNT_DOWN_TIMEOUT));
        }
        else
        {
            m_countdown.stop();
        }
        return true;
    }
    LOG(LOG_INFO, "Invalid state transition: %d -> %d", m_state, state);
    return false;
}

bool OpApp::CanTransitionToState(const E_OPAPP_STATE &state)
{
    if (state != m_state)
    {
        switch (m_state)
        {
            case OPAPP_FOREGROUND_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.2 Page 38
                // Allowed transitions from FOREGROUND_STATE
                if (state == OPAPP_BACKGROUND_STATE || state == OPAPP_TRANSIENT_STATE) {
                    return true;
                }
                break;

            case OPAPP_TRANSIENT_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.4 Page 41
            case OPAPP_OVERLAID_TRANSIENT_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.6 Page 42
            case OPAPP_OVERLAID_FOREGROUND_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.5 Page 41
                // Allowed transitions from these states
                if (state == OPAPP_FOREGROUND_STATE || state == OPAPP_BACKGROUND_STATE) {
                    return true;
                }
                break;

            default:
                break;
        }
        return false;
    }
    return true;
}


static std::string opAppStateToString(const OpApp::E_OPAPP_STATE &state)
{
    switch (state)
    {
        case OpApp::OPAPP_BACKGROUND_STATE: return "background";
        case OpApp::OPAPP_FOREGROUND_STATE: return "foreground";
        case OpApp::OPAPP_TRANSIENT_STATE: return "transient";
        case OpApp::OPAPP_OVERLAID_TRANSIENT_STATE: return "overlaid-transient";
        case OpApp::OPAPP_OVERLAID_FOREGROUND_STATE: return "overlaid-foreground";
        default: break;
    }
    // should never get here
    return "undefined";
}

} // namespace orb
