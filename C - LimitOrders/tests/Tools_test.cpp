#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include "Tools.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

BOOST_AUTO_TEST_CASE(Toools_CumNorm_Test, * boost::unit_test::tolerance(0.0001)) 
{
  BOOST_CHECK_EQUAL(Tools::CumNorm(10.0), 1.0);
  BOOST_CHECK_EQUAL(Tools::CumNorm(-10.0), 0.0);
  BOOST_TEST(Tools::CumNorm(0) == 0.49999997);
  BOOST_TEST(Tools::CumNorm(0.5) == 0.6914624526300104);
  BOOST_TEST(Tools::CumNorm(1) == 0.8413447326427189);
  BOOST_TEST(Tools::CumNorm(-1) == 0.15865526735728108);
  BOOST_TEST(Tools::CumNorm(50) == 1.0);
  BOOST_TEST(Tools::CumNorm(0.0001) == 0.5000398704475544);
}