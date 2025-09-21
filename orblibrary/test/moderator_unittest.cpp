#include <iostream>
#include <string>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/orb/orblibrary/include/Moderator.h"
#include "third_party/orb/orblibrary/test/MockOrbBrowser.h"
#include "third_party/orb/orblibrary/include/OrbConstants.h"

TEST(OrbModerator, TestModeratorInvalidRequest)
{
  // GIVEN: a MockOrbBrowser instance and orb::Moderator object
  MockOrbBrowser mockBrowser;
  orb::Moderator moderator(&mockBrowser, orb::APP_TYPE_HBBTV);

  // WHEN: handleOrbRequest is called with an empty string
  std::string response = moderator.handleOrbRequest("");

  // THEN: an invalid JSON error reponse is returned
  EXPECT_EQ(response, R"({"error": "Invalid Request"})");

  // OR WHEN: handleOrbRequest is called without a method argument
  response = moderator.handleOrbRequest(R"({ "NotAMethod": { "Some": "Value" }})");

  // THEN: an invalid method reponse is returned
  EXPECT_EQ(response, R"({"error": "No method"})");

  // OR WHEN: handleOrbRequest is called with valid JSON with an invalid method parameter,
  std::string json_request = R"({ "method": { "Some": "Value" }})";

  response = moderator.handleOrbRequest(json_request);

  // THEN: a JSON error reponse is returned
  EXPECT_EQ(response, R"({"error": "No method"})");
}

TEST(OrbModerator, TestModeratorErrorRequest)
{
  // GIVEN: a MockOrbBrowser instance and orb::Moderator object
  MockOrbBrowser mockBrowser;
  orb::Moderator moderator(&mockBrowser, orb::APP_TYPE_HBBTV);

  // WHEN: handleOrbRequest is called with an "error" parameter string
  std::string response = moderator.handleOrbRequest(R"({ "error": { "Some": "Value" }})");

  // THEN: a valid JSON error reponse is returned
  EXPECT_EQ(response, R"({"error": "Error Request"})");
}

TEST(OrbModerator, TestModeratorInvalidMethod)
{
  // GIVEN: a MockOrbBrowser instance and orb::Moderator object
  MockOrbBrowser mockBrowser;
  orb::Moderator moderator(&mockBrowser, orb::APP_TYPE_HBBTV);

  // WHEN: handleOrbRequest is called with a invalid method request
  std::string response = moderator.handleOrbRequest(R"({ "method": "some method" })");

  // THEN: a valid JSON reponse is returned indicating an invalid method
  EXPECT_EQ(response, R"({"error": "Invalid method"})");
}

TEST(OrbModerator, TestModeratorValidMethod_AppManager)
{
  // GIVEN: a MockOrbBrowser instance and orb::Moderator object
  MockOrbBrowser mockBrowser;
  orb::Moderator moderator(&mockBrowser, orb::APP_TYPE_HBBTV);

  // WHEN: handleOrbRequest is called with a valid 'Manager' component payload
  // but invalid 'method'
  std::string response = moderator.handleOrbRequest(R"({ "method": "Manager.SomeMethod" })");

  // THEN: a valid JSON reponse is returned indicating an invalid method
  EXPECT_EQ(response, R"({"error": "AppManager request [SomeMethod] invalid method"})");

  // AND WHEN: handleOrbRequest is called with a valid 'Manager' method payload
  std::vector<std::string> requests = {
    "createApplication",
    "destroyApplication",
    "showApplication",
    "hideApplication",
    "searchOwner",
    "getFreeMem",
    "getKeyIcon",
    "setKeyValue",
    "getKeyMaximumValue",
    "getKeyValues",
    "getApplicationScheme",
    "getApplicationUrl",
    "getRunningAppIds"
  };

  for (const auto& request : requests)
  {
    std::string json_request = R"({ "method": "Manager.)" + request + R"(" })";
    response = moderator.handleOrbRequest(json_request);

    // THEN: a valid JSON reponse is returned
    EXPECT_EQ(response, R"({"Response": "AppManager request [)" + request + R"(] not implemented"})");
  }
}

TEST(OrbModerator, TestModeratorValidMethod_Network)
{
  // GIVEN: a MockOrbBrowser instance and orb::Moderator object
  MockOrbBrowser mockBrowser;
  orb::Moderator moderator(&mockBrowser, orb::APP_TYPE_HBBTV);

  // WHEN: handleOrbRequest is called with a valid 'Network' component payload
  // but invalid 'method'
  std::string response = moderator.handleOrbRequest(R"({ "method": "Network.SomeMethod" })");

  // THEN: a valid JSON reponse is returned indicating an invalid method
  EXPECT_EQ(response, R"({"error": "Network request [SomeMethod] invalid method"})");

  // AND WHEN: handleOrbRequest is called with a valid 'Network' method payload
  std::vector<std::string> requests = {
    "resolveHostAddress"
  };

  for (const auto& request : requests)
  {
    std::string json_request = R"({ "method": "Network.)" + request + R"(" })";
    response = moderator.handleOrbRequest(json_request);

    // THEN: a valid JSON reponse is returned
    EXPECT_EQ(response, R"({"Response": "Network request [)" + request + R"(] not implemented"})");
  }
}

TEST(OrbModerator, TestModeratorValidMethod_MediaSynchroniser)
{
  // GIVEN: a MockOrbBrowser instance and orb::Moderator object
  MockOrbBrowser mockBrowser;
  orb::Moderator moderator(&mockBrowser, orb::APP_TYPE_HBBTV);

  // WHEN: handleOrbRequest is called with a valid 'MediaSynchroniser' component payload
  // but invalid 'method'
  std::string response = moderator.handleOrbRequest(R"({ "method": "MediaSynchroniser.SomeMethod" })");

  // THEN: a valid JSON reponse is returned indicating an invalid method
  EXPECT_EQ(response, R"({"error": "MediaSynchroniser request [SomeMethod] invalid method"})");

  // AND WHEN: handleOrbRequest is called with a valid 'Network' method payload
  std::vector<std::string> requests = {
    "instantiate",
    "initialise",
    "destroy",
    "enableInterDeviceSync",
    "disableInterDeviceSync",
    "nrOfSlaves",
    "interDeviceSyncEnabled",
    "getContentIdOverride",
    "getBroadcastCurrentTime",
    "startTimelineMonitoring",
    "stopTimelineMonitoring",
    "setContentIdOverride",
    "setContentTimeAndSpeed",
    "updateCssCiiProperties",
    "setTimelineAvailability"
  };

  for (const auto& request : requests)
  {
    std::string json_request = R"({ "method": "MediaSynchroniser.)" + request + R"(" })";
    response = moderator.handleOrbRequest(json_request);

    // THEN: a valid JSON reponse is returned
    EXPECT_EQ(response, R"({"Response": "MediaSynchroniser request [)" + request + R"(] not implemented"})");
  }
}


// TODO add tests for

// - DVB client? Null DVB client

TEST(OrbModeratorWithMockBrowser, TestModeratorConstructionWithHbbTVAppType)
{
  // GIVEN: a MockOrbBrowser instance
  MockOrbBrowser mockBrowser;

  // WHEN: constructing a Moderator with the mock browser and HbbTV app type
  orb::Moderator moderator(&mockBrowser, orb::APP_TYPE_HBBTV);

  // THEN: the moderator should be constructed successfully
  // (This test verifies that the constructor doesn't throw exceptions)
  SUCCEED();
}

TEST(OrbModeratorWithMockBrowser, TestModeratorRequestHandlingWithMockBrowser)
{
  // GIVEN: a MockOrbBrowser instance and Moderator with HbbTV app type
  MockOrbBrowser mockBrowser;
  orb::Moderator moderator(&mockBrowser, orb::APP_TYPE_HBBTV);

  // WHEN: handleOrbRequest is called with an empty string
  std::string response = moderator.handleOrbRequest("");

  // THEN: an invalid JSON error response is returned
  EXPECT_EQ(response, R"({"error": "Invalid Request"})");

  // AND WHEN: handleOrbRequest is called with a valid method
  std::string json_request = R"({ "method": "Manager.createApplication" })";
  response = moderator.handleOrbRequest(json_request);

  // THEN: a valid JSON response is returned
  EXPECT_EQ(response, R"({"Response": "AppManager request [createApplication] not implemented"})");
}

