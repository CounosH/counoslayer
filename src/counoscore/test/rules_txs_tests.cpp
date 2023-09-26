#include <counoscore/counoscore.h>
#include <counoscore/rules.h>

#include <test/util/setup_common.h>

#include <stdint.h>
#include <limits>

#include <boost/test/unit_test.hpp>

using namespace mastercore;

BOOST_FIXTURE_TEST_SUITE(counoscore_rules_txs_tests, BasicTestingSetup)

const int MAX_BLOCK = std::numeric_limits<int>::max();
const int MAX_VERSION = std::numeric_limits<uint16_t>::max();

BOOST_AUTO_TEST_CASE(simple_send_restrictions)
{
    int MSC_SEND_BLOCK = ConsensusParams().MSC_SEND_BLOCK;

    BOOST_CHECK(!IsTransactionTypeAllowed(0,                COUNOS_PROPERTY_CCH,  MSC_TYPE_SIMPLE_SEND, MP_TX_PKT_V0));
    BOOST_CHECK(!IsTransactionTypeAllowed(MSC_SEND_BLOCK,   COUNOS_PROPERTY_CCH,  MSC_TYPE_SIMPLE_SEND, MP_TX_PKT_V0));
    BOOST_CHECK(!IsTransactionTypeAllowed(MAX_BLOCK,        COUNOS_PROPERTY_CCH,  MSC_TYPE_SIMPLE_SEND, MAX_VERSION));
    BOOST_CHECK(!IsTransactionTypeAllowed(0,                COUNOS_PROPERTY_MSC,  MSC_TYPE_SIMPLE_SEND, MAX_VERSION));
    BOOST_CHECK(!IsTransactionTypeAllowed(MSC_SEND_BLOCK-1, COUNOS_PROPERTY_MSC,  MSC_TYPE_SIMPLE_SEND, MP_TX_PKT_V0));
    BOOST_CHECK(!IsTransactionTypeAllowed(MAX_BLOCK,        COUNOS_PROPERTY_MSC,  MSC_TYPE_SIMPLE_SEND, MP_TX_PKT_V1));
    BOOST_CHECK(!IsTransactionTypeAllowed(MSC_SEND_BLOCK,   COUNOS_PROPERTY_TMSC, MSC_TYPE_SIMPLE_SEND, MAX_VERSION));
    BOOST_CHECK(!IsTransactionTypeAllowed(MAX_BLOCK,        COUNOS_PROPERTY_TMSC, MSC_TYPE_SIMPLE_SEND, MP_TX_PKT_V1));

    BOOST_CHECK(IsTransactionTypeAllowed(MSC_SEND_BLOCK,    COUNOS_PROPERTY_MSC,  MSC_TYPE_SIMPLE_SEND, MP_TX_PKT_V0));
    BOOST_CHECK(IsTransactionTypeAllowed(MAX_BLOCK,         COUNOS_PROPERTY_MSC,  MSC_TYPE_SIMPLE_SEND, MP_TX_PKT_V0));
    BOOST_CHECK(IsTransactionTypeAllowed(0,                 COUNOS_PROPERTY_TMSC, MSC_TYPE_SIMPLE_SEND, MP_TX_PKT_V0));
    BOOST_CHECK(IsTransactionTypeAllowed(MAX_BLOCK,         COUNOS_PROPERTY_TMSC, MSC_TYPE_SIMPLE_SEND, MP_TX_PKT_V0));
}


BOOST_AUTO_TEST_SUITE_END()
