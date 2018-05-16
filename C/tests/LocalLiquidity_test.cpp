#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include "LocalLiquidity.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>


BOOST_AUTO_TEST_CASE(LocalLiquidity_constructor_test) 
{
  LocalLiquidity localLiquidity(10.2f, 15.2f, 20.0f);

  BOOST_CHECK_EQUAL(localLiquidity.deltaUp, 10.2f);
  BOOST_CHECK_EQUAL(localLiquidity.deltaDown, 10.2f);
  BOOST_CHECK_EQUAL(localLiquidity.delta, 10.2f);
  BOOST_CHECK_EQUAL(localLiquidity.deltaStar, 15.2f);
  BOOST_CHECK_EQUAL(localLiquidity.alpha, 20.0f);
}

BOOST_AUTO_TEST_CASE(LocalLiquidity_computeH1H2exp_test, * boost::unit_test::tolerance(0.0001)) 
{
  LocalLiquidity localLiquidity(10.2f, 15.2f, 20.0f);

  localLiquidity.computeH1H2exp();

  BOOST_TEST(localLiquidity.H1 == 0.5335697698511845);
  BOOST_TEST(localLiquidity.H2 == 0.16518869903954936);

  LocalLiquidity localLiquidity2(0.1f, 0.1f, 0.1f);

  localLiquidity2.computeH1H2exp();


  BOOST_TEST(localLiquidity2.H1 == 0.6578174303942945);
  BOOST_TEST(localLiquidity2.H2 == -0.1978316799191401);
}

BOOST_AUTO_TEST_CASE(LocalLiquidity_computation_test) 
{
  //TODO
}