#pragma once

#include <iostream>
#include <mutex>

#include <android/binder_auto_utils.h>
#include "org/orbtv/orbservice/BnBridgeSession.h"
#include "org/orbtv/orbservice/IBrowserSession.h"

#ifdef NDK_AIDL
#define STATUS ndk::ScopedAStatus
#define SH_PTR std::shared_ptr
#else
#define STATUS ::android::binder::Status
#define SH_PTR ::android::sp
#endif

using namespace std;

#ifdef NDK_AIDL
namespace aidl {
#endif

namespace org::orbtv::orbservice {

class BridgeSession : public BnBridgeSession {
private:
    static BridgeSession* s_instance;
    static mutex s_mtx;
    BridgeSession() {}

public:
    BridgeSession(const BridgeSession& obj) = delete; // prevent copies
    void operator=(const BridgeSession &) = delete; // prevent assignments
    static BridgeSession* getInstance();

public:
  STATUS initialise(const SH_PTR<IBrowserSession>& browser) override;
  STATUS executeRequest(const vector<uint8_t>& jsonstr, vector<uint8_t>* result) override;

};

} // namespace org::orbtv::orbservice

#ifdef NDK_AIDL
} // namespace aidl
#endif
