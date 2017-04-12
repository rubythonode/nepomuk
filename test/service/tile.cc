// make sure we get a new main function here
#define BOOST_TEST_MAIN
#include "fixtures.h"
#include <boost/test/unit_test.hpp>

#include "service/master.hpp"
#include "service/tile.hpp"
#include "service/tile_parameters.hpp"

using namespace transit;

BOOST_AUTO_TEST_CASE(render_tiles)
{
    service::Master master_service(TRANSIT_THREE_LINES_EXAMPLE_FIXTURE);

    service::Tile tileservice(master_service);
    service::ServiceParameters parameters = service::TileParameters(16384, 16383, 15);
    service::ServiceParameters invalid_parameters = service::TileParameters(1 << 10, 1 << 10, 11);
    service::ServiceParameters empty_parameters = service::TileParameters(0, 0, 12);

    // compute a tile
    {
        auto status = tileservice(parameters);
        BOOST_CHECK(status == service::ServiceStatus::SUCCESS);
    }
    {
        auto status = tileservice(invalid_parameters);
        BOOST_CHECK(status == service::ServiceStatus::ERROR);
    }
    {
        auto status = tileservice(empty_parameters);
        BOOST_CHECK(status == service::ServiceStatus::SUCCESS);
    }

    BOOST_CHECK(
        ((std::string)boost::get<service::TileParameters>(parameters).result()).length() >
        ((std::string)boost::get<service::TileParameters>(empty_parameters).result()).length());
}
