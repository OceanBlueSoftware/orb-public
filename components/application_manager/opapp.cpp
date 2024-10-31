#include "opapp.h"
#include "log.h"

#define COUNT_DOWN_TIMEOUT 60000

OpApp::OpApp(const std::string &url, const F_STATE_CHANGE_CB &changeStateCb)
    : App(url),
    m_stateChangeCb(changeStateCb)
{ 
    m_state = BACKGROUND_STATE; // ETSI TS 103 606 V1.2.1 (2024-03) page 36
}

OpApp::OpApp(const Ait::S_AIT_APP_DESC &desc, bool isNetworkAvailable, const F_STATE_CHANGE_CB &changeStateCb)
    : App(
        desc,
        Utils::MakeInvalidDvbTriplet(),
        isNetworkAvailable,
        "",
        true,
        false),
    m_stateChangeCb(changeStateCb)
{ 
    m_state = BACKGROUND_STATE; // ETSI TS 103 606 V1.2.1 (2024-03) page 36
}

OpApp::OpApp(const OpApp &other)
    : App(other),
      m_stateChangeCb(other.m_stateChangeCb)
{
    if (!other.m_countdown.isStopped())
    {
        m_countdown.start(other.m_countdown.elapsed());
    }
}

void OpApp::SetState(const E_APP_STATE &state)
{
    if (state != OVERLAID && canTransitionToState(state))
    {
        if (state != m_state)
        {
            m_state = state;
            m_stateChangeCb(this);
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
