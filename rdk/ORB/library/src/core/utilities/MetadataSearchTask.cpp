/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "MetadataSearchTask.h"
#include "Channel.h"
#include "ORBEngine.h"
#include "JsonUtil.h"
#include "Logging.h"
#include <thread>

namespace orb {
std::thread s_thread;

static
std::string ToLowerCase(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){
            return std::tolower(c);
        });
    return s;
}

/**
 * Constructor.
 *
 * @param query              Shared pointer to the query
 * @param offset             The specified offset for the search results
 * @param count              The specified count for the search results
 * @param channelConstraints The additional channel constraints
 */
MetadataSearchTask::MetadataSearchTask(std::shared_ptr<Query> query, int offset, int count,
                                       std::vector<std::string> channelConstraints)
    : m_query(query)
    , m_offset(offset)
    , m_count(count)
    , m_channelConstraints(channelConstraints)
{
    ORB_LOG("queryId=%d", query->GetQueryId());
}

/**
 * Destructor.
 */
MetadataSearchTask::~MetadataSearchTask()
{
    ORB_LOG("queryId=%d", m_query->GetQueryId());
}

/**
 * Dispatch the MetadataSearch bridge event to the current page's JavaScript context.
 *
 * @param search        The search id
 * @param status        0 (Completed) or 3 (Aborted) or 4 (No resource found)
 * @param searchResults The list of JSON programme objects that match the search criteria
 */
void MetadataSearchTask::OnMetadataSearchCompleted(int search, int status,
    std::vector<std::string> searchResults)
{
    ORB_LOG("search=%d status=%d", search, status);

    // prepare event properties and request event dispatching
    json properties;
    properties["search"] = search;
    properties["status"] = status;

    json array;
    for (auto programme : searchResults)
    {
        json jsonProgramme = json::parse(programme);
        array.push_back(jsonProgramme);
    }

    properties["programmeList"] = array;

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "MetadataSearch", properties, "", true);
}

/**
 * @brief Run
 */
void MetadataSearchTask::Run()
{
    ORB_LOG_NO_ARGS();
    s_thread = std::thread(&MetadataSearchTask::Worker, this);
}

/**
 * Stop the search task thread.
 */
void MetadataSearchTask::Stop()
{
    ORB_LOG_NO_ARGS();
    pthread_cancel(s_thread.native_handle());
}

/**
 * @brief MetadataSearch::Worker
 *
 * Worker method that runs in a dedicated thread.
 *
 * @return infinite
 */
uint32_t MetadataSearchTask::Worker()
{
    // Get pointer to platform implementation
    ORBPlatform *platform = ORBEngine::GetSharedInstance().GetORBPlatform();
    if (platform == nullptr)
    {
        ORB_LOG("ERROR: ORB platform implementation not available");
        return 0;
    }

    // Get channels
    std::vector<Channel> channelList = platform->Broadcast_GetChannelList();

    // For each channel, if searchable, get programmes
    for (auto channel : channelList)
    {
        if (channel.IsHidden())
        {
            continue;
        }

        // Filter out channel if channelConstraints (1) is not empty, and (2) does not include the channel's ccid
        std::string constraint = "ccid:" + channel.GetCcid();
        if (!m_channelConstraints.empty())
        {
            if (std::find(m_channelConstraints.begin(), m_channelConstraints.end(), constraint) ==
                m_channelConstraints.end())
            {
                continue;
            }
        }
        std::vector<Programme> programmes = platform->Broadcast_GetProgrammes(channel.GetCcid());

        // For each programme, match against query
        for (auto programme : programmes)
        {
            // If programme matches add it to search results
            if (Match(m_query, programme, channel.GetCcid()))
            {
                if (m_offset > 0)
                {
                    m_offset--;
                }
                else
                {
                    // Add programme to search results
                    json jsonProgramme = JsonUtil::ProgrammeToJsonObject(programme);
                    m_searchResults.push_back(jsonProgramme.dump());
                    m_count--;
                    if (m_count == 0)
                    {
                        break;
                    }
                }
            }
        }
    }

    // Trigger notification
    OnMetadataSearchCompleted(m_query->GetQueryId(), SEARCH_STATUS_COMPLETED, m_searchResults);

    // Cleanup
    // Stop();
    ORBEngine::GetSharedInstance().RemoveMetadataSearchTask(m_query->GetQueryId());

    return 0;
}

/**
 * @brief MetadataSearchTask::Match
 *
 * Matches the specified programme against the specified query.
 *
 * @param query     The specified query
 * @param programme The specified programme
 * @param ccid      The ID of the channel that the specified programme belongs to
 *
 * @return true if the programme matches the query, or else false
 */
bool MetadataSearchTask::Match(
    std::shared_ptr<Query> query,
    Programme programme,
    std::string ccid
    )
{
    ORB_LOG("query=%s", query->ToString().c_str());
    bool result = false;
    switch (query->GetOperation())
    {
        case Query::Operation::OP_ID:
        {
            std::string field = query->GetField();
            if (field == "Programme.channelID")
            {
                result = CompareStringValues(query->GetComparison(),
                    ccid,
                    query->GetValue());
                break;
            }
            else if (field == "Programme.startTime")
            {
                result = CompareLongValues(query->GetComparison(),
                    programme.GetStartTime() / 1000,
                    std::stol(query->GetValue()));
                break;
            }
            else if (field == "Programme.endTime")
            {
                result = CompareLongValues(query->GetComparison(),
                    (programme.GetStartTime() / 1000) + programme.GetDuration(),
                    std::stol(query->GetValue()));
                break;
            }
            else if (field == "Programme.name")
            {
                result = CompareStringValues(query->GetComparison(),
                    programme.GetName(),
                    query->GetValue());
                break;
            }
            else if (field == "Programme.programmeID")
            {
                result = CompareStringValues(query->GetComparison(),
                    programme.GetProgrammeId(),
                    query->GetValue());
                break;
            }
            break;
        }
        case Query::Operation::OP_AND:
        {
            result = Match(query->GetOperator1(), programme, ccid) &&
                Match(query->GetOperator2(), programme, ccid);
            break;
        }
        case Query::Operation::OP_OR:
        {
            result = Match(query->GetOperator1(), programme, ccid) ||
                Match(query->GetOperator2(), programme, ccid);
            break;
        }
        case Query::Operation::OP_NOT:
        {
            result = !Match(query->GetOperator1(), programme, ccid);
            break;
        }
        default:
            break;
    }
    return result;
}

/**
 * @brief MetadataSearchTask::CompareStringValues
 *
 * Compare the given programme and query string values.
 *
 * @param comparison     The comparison type
 * @param programmeValue The programme value
 * @param queryValue     The query value
 *
 * @return true if the programme and query values match, or else false
 */
bool MetadataSearchTask::CompareStringValues(
    Query::Comparison comparison,
    std::string programmeValue,
    std::string queryValue)
{
    bool result = false;
    programmeValue = ToLowerCase(programmeValue);
    queryValue = ToLowerCase(queryValue);
    switch (comparison)
    {
        case Query::Comparison::CMP_EQUAL:
            result = (programmeValue == queryValue);
            break;
        case Query::Comparison::CMP_NOT_EQL:
            result = (programmeValue != queryValue);
            break;
        case Query::Comparison::CMP_MORE:
            result = (programmeValue.compare(queryValue) > 0);
            break;
        case Query::Comparison::CMP_MORE_EQL:
            result = (programmeValue.compare(queryValue) >= 0);
            break;
        case Query::Comparison::CMP_LESS:
            result = (programmeValue.compare(queryValue) < 0);
            break;
        case Query::Comparison::CMP_LESS_EQL:
            result = (programmeValue.compare(queryValue) <= 0);
            break;
        case Query::Comparison::CMP_CONTAINS:
            result = (programmeValue.find(queryValue) != std::string::npos);
            break;
        default:
            break;
    }
    return result;
}

/**
 * @brief MetadataSearchTask::CompareLongValues
 *
 * Compare the given programme and query long values.
 *
 * @param comparison     The comparison type
 * @param programmeValue The programme value
 * @param queryValue     The query value
 *
 * @return true if the programme and query values match, or else false
 */
bool MetadataSearchTask::CompareLongValues(Query::Comparison comparison,
    long programmeValue,
    long queryValue)
{
    bool result = false;
    switch (comparison)
    {
        case Query::Comparison::CMP_EQUAL:
            result = (programmeValue == queryValue);
            break;
        case Query::Comparison::CMP_NOT_EQL:
            result = (programmeValue != queryValue);
            break;
        case Query::Comparison::CMP_MORE:
            result = (programmeValue > queryValue);
            break;
        case Query::Comparison::CMP_MORE_EQL:
            result = (programmeValue >= queryValue);
            break;
        case Query::Comparison::CMP_LESS:
            result = (programmeValue < queryValue);
            break;
        case Query::Comparison::CMP_LESS_EQL:
            result = (programmeValue <= queryValue);
            break;
        case Query::Comparison::CMP_CONTAINS:
            result = (programmeValue == queryValue);
            break;
        default:
            break;
    }
    return result;
}
} // namespace orb