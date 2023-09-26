#include <counoscore/version.h>

#include <config/counosh-config.h>
#include <test/util/setup_common.h>

#include <boost/test/unit_test.hpp>

#include <string>

BOOST_FIXTURE_TEST_SUITE(counoscore_version_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(version_comparison)
{
    BOOST_CHECK(COUNOSCORE_VERSION >= 80000000); // Counos Core v0.8.0
}

/**
 * The following tests are very unhandy, because any version bump
 * breaks the tests.
 *
 * TODO: might be removed completely.
 */

BOOST_AUTO_TEST_CASE(version_string)
{
    BOOST_CHECK_EQUAL(CounosCoreVersion(), "0.11.0");
}

BOOST_AUTO_TEST_CASE(version_number)
{
    BOOST_CHECK_EQUAL(COUNOSCORE_VERSION, 110000000);
}

BOOST_AUTO_TEST_CASE(config_package_version)
{
    // the package version is used in the file names:
    BOOST_CHECK_EQUAL(PACKAGE_VERSION, "0.11.0");
}


BOOST_AUTO_TEST_SUITE_END()
