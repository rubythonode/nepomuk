// make sure we get a new main function here
#define BOOST_TEST_MAIN
#include "fixtures.h"
#include <boost/test/unit_test.hpp>

#include "gtfs/dataset.hpp"
#include "gtfs/read_csv.hpp"
#include "id/stop.hpp"

#include "geometric/bounding_box.hpp"
#include "geometric/coordinate.hpp"
#include "search/coordinate_to_stop.hpp"

#include <iostream>

using namespace nepomuk;

BOOST_AUTO_TEST_CASE(lookup_lines_from_stops)
{
    // This needs to be replaced by a dedicated fixture (see
    // https://github.com/mapbox/nepomuk/issues/37)
    gtfs::CSVDiscSource source(TRANSIT_THREE_LINES_EXAMPLE_FIXTURE);
    auto dataset = gtfs::readCSV(source);

    auto make_coordinate_lookup = [&]() {
        std::vector<std::pair<geometric::WGS84Coordinate, StopID>> data;
        std::for_each(dataset.stops.begin(), dataset.stops.end(), [&](auto const &element) {
            if (!element.location_type || *element.location_type == gtfs::LocationType::STOP)
                data.push_back(std::make_pair(element.location, element.id));
        });
        return search::CoordinateToStop(data);
    };

    auto coordinate_lookup = make_coordinate_lookup();

    geometric::WGS84Coordinate coordinate(
        geometric::makeLatLonFromDouble<geometric::FixedLongitude>(0.00501),
        geometric::makeLatLonFromDouble<geometric::FixedLatitude>(0.00511));

    auto closest_stop = coordinate_lookup.nearest(coordinate);
    BOOST_CHECK_EQUAL(closest_stop.first, StopID{0});

    geometric::WGS84Coordinate coordinate2(
        geometric::makeLatLonFromDouble<geometric::FixedLongitude>(0.004),
        geometric::makeLatLonFromDouble<geometric::FixedLatitude>(0.005));

    auto distance = geometric::distance(closest_stop.second, coordinate);

    auto const coords_in_circle = coordinate_lookup.all(coordinate, distance * 1.1);
    BOOST_CHECK(coords_in_circle.size() == 1 && coords_in_circle.front().first == StopID{0});

    auto const count_nearest = coordinate_lookup.nearest(coordinate, 1);
    BOOST_CHECK_EQUAL(count_nearest.front().first, StopID{0});

    geometric::WGS84BoundingBox bbox(coordinate2, coordinate);

    auto all = coordinate_lookup.all(bbox);
    BOOST_CHECK(all.size() == 1);
    BOOST_CHECK_EQUAL(all.front().first, StopID{0});
}
