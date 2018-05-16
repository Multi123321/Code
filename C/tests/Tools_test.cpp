#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include "Tools.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

BOOST_AUTO_TEST_CASE(Toools_CumNorm_Test) {
  BOOST_CHECK_EQUAL(Tools::CumNorm(10.0), 1.0);
  BOOST_CHECK_EQUAL(Tools::CumNorm(-10.0), 0.0);
  BOOST_CHECK_EQUAL((int)(Tools::CumNorm(0) * 10000), (int)(0.49999997 * 10000.0));
  BOOST_CHECK_EQUAL((int)(Tools::CumNorm(0.5) * 10000), (int)(0.6914624526300104 * 10000.0));
  BOOST_CHECK_EQUAL((int)(Tools::CumNorm(1) * 10000), (int)(0.8413447326427189 * 10000.0));
  BOOST_CHECK_EQUAL((int)(Tools::CumNorm(-1) * 10000), (int)(0.15865526735728108 * 10000.0));
  BOOST_CHECK_EQUAL((int)(Tools::CumNorm(50) * 10000), (int)(1.0 * 10000.0));
  BOOST_CHECK_EQUAL((int)(Tools::CumNorm(0.0001) * 10000), (int)(0.5000398704475544 * 10000.0));
}