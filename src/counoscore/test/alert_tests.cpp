#include <counoscore/notifications.h>
#include <counoscore/version.h>

#include <util/system.h>
#include <test/util/setup_common.h>
#include <tinyformat.h>

#include <boost/test/unit_test.hpp>

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

using namespace mastercore;

BOOST_FIXTURE_TEST_SUITE(counoscore_alert_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(alert_positive_authorization)
{
    // Confirm authorized sources for mainnet
    BOOST_CHECK(CheckAlertAuthorization("17xr7sbehYY4YSZX9yuJe6gK9rrdRrZx26"));  // Craig   <craig@counos.foundation>
    BOOST_CHECK(CheckAlertAuthorization("1883ZMsRJfzKNozUBJCCHxQ7EaiNioNDWz"));  // Zathras <zathras@counos.foundation>
    BOOST_CHECK(CheckAlertAuthorization("1HHv91gRxqBzQ3gydMob3LU8hqXcWoLfvd"));  // dexX7   <dexx@bitwatch.co>
    BOOST_CHECK(CheckAlertAuthorization("16oDZYCspsczfgKXVj3xyvsxH21NpEj94F"));  // Adam    <adam@counos.foundation>
}

BOOST_AUTO_TEST_CASE(alert_unauthorized_source)
{
    // Confirm unauthorized sources are not accepted
    BOOST_CHECK(!CheckAlertAuthorization("1JwSSubhmg6iPtRjtyqhUYYH7bZg3Lfy1T"));
}

BOOST_AUTO_TEST_CASE(alert_manual_sources)
{
    // Add 1JwSSu as allowed source for alerts
    gArgs.ForceSetArg("-counosalertallowsender", "1JwSSubhmg6iPtRjtyqhUYYH7bZg3Lfy1T");
    BOOST_CHECK_EQUAL(gArgs.GetArgs("-counosalertallowsender").size(), 1);
    BOOST_CHECK(CheckAlertAuthorization("1JwSSubhmg6iPtRjtyqhUYYH7bZg3Lfy1T"));

    // Then ignore some sources explicitly
    gArgs.ForceSetArg("-counosalertignoresender", "1JwSSubhmg6iPtRjtyqhUYYH7bZg3Lfy1T");
    BOOST_CHECK(CheckAlertAuthorization("1HHv91gRxqBzQ3gydMob3LU8hqXcWoLfvd")); // should still be authorized
    BOOST_CHECK(!CheckAlertAuthorization("1JwSSubhmg6iPtRjtyqhUYYH7bZg3Lfy1T"));
    gArgs.ForceSetArg("-counosalertignoresender", "16oDZYCspsczfgKXVj3xyvsxH21NpEj94F");
    BOOST_CHECK(!CheckAlertAuthorization("16oDZYCspsczfgKXVj3xyvsxH21NpEj94F"));

    gArgs.ForceSetArg("-counosalertallowsender", "");
    gArgs.ForceSetArg("-counosalertignoresender", "");
}

BOOST_AUTO_TEST_CASE(alert_authorize_any_source)
{
    // Allow any source (e.g. for tests!)
    gArgs.ForceSetArg("-counosalertallowsender", "any");
    BOOST_CHECK(CheckAlertAuthorization("1JwSSubhmg6iPtRjtyqhUYYH7bZg3Lfy1T"));
    BOOST_CHECK(CheckAlertAuthorization("137uFtQ5EgMsreg4FVvL3xuhjkYGToVPqs"));
    BOOST_CHECK(CheckAlertAuthorization("16oDZYCspsczfgKXVj3xyvsxH21NpEj94F"));

    gArgs.ForceSetArg("-counosalertallowsender", "");
}

BOOST_AUTO_TEST_SUITE_END()
