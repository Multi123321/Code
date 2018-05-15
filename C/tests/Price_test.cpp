#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include "Price.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

BOOST_AUTO_TEST_CASE(Price_constructor_test) {
  Price price(10.2f, 15.2f, 20l);

  BOOST_CHECK_EQUAL(price.getBid(), 10.2f);
}

// BOOST_AUTO_TEST_CASE(point_operator_test) {
//   point pt(2, 3);
//   BOOST_CHECK_EQUAL(pt + point(3, 4), point(5, 7));
//   BOOST_CHECK_EQUAL(pt - point(3, 4), point(-1, -1));
// }

// BOOST_AUTO_TEST_CASE(point_ostream_test) {
//   boost::test_tools::output_test_stream output;
//   output << point(3, 2);
//   BOOST_CHECK(output.is_equal("point(3,2)"));
// }

