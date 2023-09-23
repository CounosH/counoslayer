#include <counoscore/counoscore.h>
#include <counoscore/rules.h>

#include <base58.h>
#include <chainparams.h>
#include <key_io.h>
#include <test/util/setup_common.h>

#include <boost/test/unit_test.hpp>

#include <limits>

using namespace mastercore;

BOOST_FIXTURE_TEST_SUITE(counoscore_exodus_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(exodus_address_mainnet)
{
    BOOST_CHECK(CTxDestination(DecodeDestination("cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam")) ==
                ExodusAddress());
    BOOST_CHECK(!(CTxDestination(DecodeDestination("1rDQWR9yZLJY7ciyghAaF7XKD9tGzQuP6")) ==
                ExodusAddress()));
}

BOOST_AUTO_TEST_CASE(exodus_crowdsale_address_mainnet)
{
    BOOST_CHECK(CTxDestination(DecodeDestination("cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam")) ==
                ExodusCrowdsaleAddress(0));
    BOOST_CHECK(CTxDestination(DecodeDestination("cch1q6gj3g8v4zekfq9eygwynj5fjr9mukjvaclzgam")) ==
                ExodusCrowdsaleAddress(std::numeric_limits<int>::max()));
    BOOST_CHECK(!(CTxDestination(DecodeDestination("1rDQWR9yZLJY7ciyghAaF7XKD9tGzQuP6")) ==
                ExodusCrowdsaleAddress(0)));
    BOOST_CHECK(!(CTxDestination(DecodeDestination("1rDQWR9yZLJY7ciyghAaF7XKD9tGzQuP6")) ==
                ExodusCrowdsaleAddress(std::numeric_limits<int>::max())));
}

BOOST_AUTO_TEST_CASE(exodus_address_testnet)
{
    SelectParams(CBaseChainParams::TESTNET);

    BOOST_CHECK(CTxDestination(DecodeDestination("cch1q5h8klytg2yr6q8wsvfnw5kc035jkh0val52jgu")) ==
                ExodusAddress());
    BOOST_CHECK(!(CTxDestination(DecodeDestination("cch1qrhk2ea3ce8uw76alqr69glwng84np2nvde0lq3")) ==
                ExodusAddress()));

    SelectParams(CBaseChainParams::MAIN);
}

BOOST_AUTO_TEST_CASE(exodus_address_regtest)
{
    SelectParams(CBaseChainParams::REGTEST);

    BOOST_CHECK(CTxDestination(DecodeDestination("cch1q5h8klytg2yr6q8wsvfnw5kc035jkh0val52jgu")) ==
                ExodusAddress());
    BOOST_CHECK(!(CTxDestination(DecodeDestination("cch1qrhk2ea3ce8uw76alqr69glwng84np2nvde0lq3")) ==
                ExodusAddress()));

    SelectParams(CBaseChainParams::MAIN);
}

BOOST_AUTO_TEST_CASE(exodus_crowdsale_address_testnet)
{
    SelectParams(CBaseChainParams::TESTNET);

    BOOST_CHECK(CTxDestination(DecodeDestination("cch1q5h8klytg2yr6q8wsvfnw5kc035jkh0val52jgu")) ==
                ExodusCrowdsaleAddress(0));
    BOOST_CHECK(CTxDestination(DecodeDestination("cch1q5h8klytg2yr6q8wsvfnw5kc035jkh0val52jgu")) ==
                ExodusCrowdsaleAddress(MONEYMAN_TESTNET_BLOCK-1));
    BOOST_CHECK(!(CTxDestination(DecodeDestination("cch1qrhk2ea3ce8uw76alqr69glwng84np2nvde0lq3")) ==
                ExodusCrowdsaleAddress(0)));
    BOOST_CHECK(!(CTxDestination(DecodeDestination("cch1qrhk2ea3ce8uw76alqr69glwng84np2nvde0lq3")) ==
                ExodusCrowdsaleAddress(MONEYMAN_TESTNET_BLOCK-1)));
    BOOST_CHECK(CTxDestination(DecodeDestination("cch1qrhk2ea3ce8uw76alqr69glwng84np2nvde0lq3")) ==
                ExodusCrowdsaleAddress(MONEYMAN_TESTNET_BLOCK));
    BOOST_CHECK(CTxDestination(DecodeDestination("cch1qrhk2ea3ce8uw76alqr69glwng84np2nvde0lq3")) ==
                ExodusCrowdsaleAddress(std::numeric_limits<int>::max()));
    BOOST_CHECK(!(CTxDestination(DecodeDestination("cch1q5h8klytg2yr6q8wsvfnw5kc035jkh0val52jgu")) ==
                ExodusCrowdsaleAddress(MONEYMAN_TESTNET_BLOCK)));
    BOOST_CHECK(!(CTxDestination(DecodeDestination("cch1q5h8klytg2yr6q8wsvfnw5kc035jkh0val52jgu")) ==
                ExodusCrowdsaleAddress(std::numeric_limits<int>::max())));

    SelectParams(CBaseChainParams::MAIN);
}

BOOST_AUTO_TEST_CASE(exodus_crowdsale_address_regtest)
{
    SelectParams(CBaseChainParams::REGTEST);

    BOOST_CHECK(CTxDestination(DecodeDestination("cch1q5h8klytg2yr6q8wsvfnw5kc035jkh0val52jgu")) ==
                ExodusCrowdsaleAddress(0));
    BOOST_CHECK(CTxDestination(DecodeDestination("cch1q5h8klytg2yr6q8wsvfnw5kc035jkh0val52jgu")) ==
                ExodusCrowdsaleAddress(MONEYMAN_REGTEST_BLOCK-1));
    BOOST_CHECK(!(CTxDestination(DecodeDestination("cch1qrhk2ea3ce8uw76alqr69glwng84np2nvde0lq3")) ==
                ExodusCrowdsaleAddress(0)));
    BOOST_CHECK(!(CTxDestination(DecodeDestination("cch1qrhk2ea3ce8uw76alqr69glwng84np2nvde0lq3")) ==
                ExodusCrowdsaleAddress(MONEYMAN_REGTEST_BLOCK-1)));
    BOOST_CHECK(CTxDestination(DecodeDestination("cch1qrhk2ea3ce8uw76alqr69glwng84np2nvde0lq3")) ==
                ExodusCrowdsaleAddress(MONEYMAN_REGTEST_BLOCK));
    BOOST_CHECK(CTxDestination(DecodeDestination("cch1qrhk2ea3ce8uw76alqr69glwng84np2nvde0lq3")) ==
                ExodusCrowdsaleAddress(std::numeric_limits<int>::max()));
    BOOST_CHECK(!(CTxDestination(DecodeDestination("cch1q5h8klytg2yr6q8wsvfnw5kc035jkh0val52jgu")) ==
                ExodusCrowdsaleAddress(MONEYMAN_REGTEST_BLOCK)));
    BOOST_CHECK(!(CTxDestination(DecodeDestination("cch1q5h8klytg2yr6q8wsvfnw5kc035jkh0val52jgu")) ==
                ExodusCrowdsaleAddress(std::numeric_limits<int>::max())));

    SelectParams(CBaseChainParams::MAIN);
}


BOOST_AUTO_TEST_SUITE_END()
