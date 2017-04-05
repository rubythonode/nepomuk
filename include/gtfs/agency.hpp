#ifndef TRANSIT_GTFS_AGENCY_HPP_
#define TRANSIT_GTFS_AGENCY_HPP_

#include "id/agency.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <boost/optional.hpp>

namespace transit
{
namespace gtfs
{

struct Agency
{
    std::string name;
    std::string url;
    std::string timezone;
    boost::optional<AgencyID> id;
    boost::optional<std::string> language;
    boost::optional<std::string> phone;
    boost::optional<std::string> fare_url;
    boost::optional<std::string> email;
};

bool checkAgencyCSVHeader(std::map<std::string, std::size_t> const &header);
Agency makeAgency(std::map<std::string, std::size_t> const &header,
                  std::vector<std::string> &values);

} // namespace gtfs
} // namespace transit

#endif // TRANSIT_GTFS_AGENCY_HPP_
