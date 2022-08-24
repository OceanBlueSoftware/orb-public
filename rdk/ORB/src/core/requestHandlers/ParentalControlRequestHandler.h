/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "RequestHandler.h"
#include "ParentalRating.h"

#include <map>
#include <vector>

namespace orb {
/**
 * @brief orb::ParentalControlRequestHandler
 *
 * RequestHandler implementation for handling ParentalControl-related requests issued by the WPE bridge.
 */
class ParentalControlRequestHandler : public RequestHandler {
public:

    ParentalControlRequestHandler();
    ~ParentalControlRequestHandler();

    virtual bool Handle(JsonObject token, std::string method, JsonObject params,
        JsonObject& response) override;

private:

    std::map<std::string, std::vector<ParentalRating> > GetRatingSchemes();
    std::shared_ptr<ParentalRating> GetThreshold(JsonObject params);
    bool IsRatingBlocked(JsonObject params);
}; // class ParentalControlRequestHandler
} // namespace orb
