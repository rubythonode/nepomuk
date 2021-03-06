#include "service/master.hpp"

#include "geometric/coordinate.hpp"
#include "gtfs/read_csv.hpp"
#include "id/stop.hpp"

#include "adaptor/dictionary.hpp"
#include "annotation/geometry_factory.hpp"
#include "annotation/line_factory.hpp"
#include "search/stop_to_line_factory.hpp"
#include "timetable/graph_adaptor.hpp"
#include "timetable/timetable_factory.hpp"

#include "tool/status/progress.hpp"

#include <boost/assert.hpp>
#include <utility>
#include <vector>

namespace nepomuk
{
namespace service
{

Master::Master(std::string const &path)
    : base_data(gtfs::readCSV(gtfs::CSVDiscSource(path.c_str())))
{
    // the constructor only reads the base data. All other classes / look-ups are constructed on
    // demand
    base_data.filter_unreachable_stops();
    base_data.connect_stops_into_stations(100);
}

// routing data
timetable::TimeTable const &Master::timetable()
{
    if (!_timetable)
    {
        tool::status::FunctionTimingGuard guard("Timetable creation");
        _trip_offsets_by_line = std::make_unique<std::vector<std::size_t>>();
        BOOST_ASSERT(_trip_offsets_by_line);
        _timetable = std::make_unique<timetable::TimeTable>(
            timetable::TimeTableFactory::produce(base_data, *_trip_offsets_by_line));
    }

    return *_timetable;
}

std::vector<std::size_t> const &Master::trip_offsets_by_line()
{
    if (!_trip_offsets_by_line)
    {
        // the trip_offsets_by_line is a byproduct of computing the line tables (as part of the
        // timetable creation). We cannot easily compute them in advance/independent of the
        // timetable creation. Therefore we delegate the creation and compute timetables first.
        timetable();
        BOOST_ASSERT(_trip_offsets_by_line);
    }
    return *_trip_offsets_by_line;
}

// look-ups
search::StopToLine const &Master::stop_to_line()
{
    if (!_stop_to_line)
    {
        tool::status::FunctionTimingGuard guard("Stop To Line Lookup-creation");
        _stop_to_line = std::make_unique<search::StopToLine>(
            search::StopToLineFactory::produce(base_data.stops.size(), timetable().lines()));
    }

    return *_stop_to_line;
}

search::CoordinateToStop const &Master::coordinate_to_stop()
{
    if (!_coordinate_to_stop)
    {
        tool::status::FunctionTimingGuard guard("Coordinate to Stop Lookup-creation");
        auto make_coordinate_lookup = [&]() {
            std::vector<std::pair<geometric::WGS84Coordinate, StopID>> data;
            std::for_each(base_data.stops.begin(), base_data.stops.end(), [&](auto const &element) {
                data.push_back(std::make_pair(element.location, element.id));
            });
            return search::CoordinateToStop(data);
        };

        _coordinate_to_stop = std::make_unique<search::CoordinateToStop>(make_coordinate_lookup());
    }
    return *_coordinate_to_stop;
}

// plain data
tool::container::StringTable const &Master::dictionary()
{
    if (!_dictionary)
    {
        tool::status::FunctionTimingGuard guard("Dictionary Creation");
        auto const message = adaptor::Dictionary::encode(base_data.dictionary);
        _dictionary = std::make_unique<tool::container::StringTable>();
        adaptor::Dictionary::decode_into(*_dictionary, message);
        // release the full dictionary, we don't need it anymore
        base_data.dictionary = {};
    }
    return *_dictionary;
}

timetable::SegmentTable const &Master::segment_table()
{
    if (!_segment_table)
    {
        tool::status::FunctionTimingGuard guard("Segment Table Creation");
        _segment_table =
            std::make_unique<timetable::SegmentTable>(base_data.shapes_as_indexed_vector());
    }
    return *_segment_table;
}

// annotation
annotation::Geometry const &Master::geometry_annotation()
{
    if (!_geometry_annotation)
    {
        tool::status::FunctionTimingGuard guard("Geometry Annotation - creation");
        _geometry_annotation = std::make_unique<annotation::Geometry>(
            annotation::GeometryFactory::produce(base_data.stops,
                                                 base_data.trips,
                                                 trip_offsets_by_line(),
                                                 stop_to_line(),
                                                 timetable().lines(),
                                                 segment_table()));
    }
    return *_geometry_annotation;
}

annotation::Stop const &Master::stop_annotation()
{
    if (!_stop_annotation)
    {
        tool::status::FunctionTimingGuard guard("Stop Annotation creation");
        _stop_annotation = std::make_unique<annotation::Stop>(base_data.stops);
    }

    return *_stop_annotation;
}

annotation::Line const &Master::line_annotation()
{
    if (!_line_annotation)
    {
        tool::status::FunctionTimingGuard guard("Line Annotation creation");

        _line_annotation = std::make_unique<annotation::Line>(annotation::LineFactory::produce(
            trip_offsets_by_line(), base_data.routes, base_data.trips));
    }

    return *_line_annotation;
}

annotation::API const &Master::api_annotation()
{
    if (!_api_annotation)
    {
        tool::status::FunctionTimingGuard guard("API Annotation - creation");
        _api_annotation = std::make_unique<annotation::API>(
            dictionary(), geometry_annotation(), stop_annotation(), line_annotation());
    }

    return *_api_annotation;
}

annotation::PBF const &Master::pbf_annotation()
{
    if (!_pbf_annotation)
    {
        tool::status::FunctionTimingGuard guard("PBF Annotation - creation");
        _pbf_annotation = std::make_unique<annotation::PBF>(
            dictionary(), geometry_annotation(), stop_annotation(), line_annotation());
    }

    return *_pbf_annotation;
}

algorithm::StronglyConnectedComponent const &Master::components()
{
    if (!_components)
    {
        tool::status::FunctionTimingGuard guard("Components creation");
        auto graph = timetable::TimetableToGraphAdaptor::adapt(timetable(), stop_to_line());
        _components =
            std::make_unique<algorithm::StronglyConnectedComponent>(algorithm::computeSCC(graph));
    }
    return *_components;
}

} // namespace service
} // namespace nepomuk
