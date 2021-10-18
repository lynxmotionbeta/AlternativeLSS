#include "RequestBuffer.h"

#include <catch2/catch_all.hpp>

using namespace lss;

#if 0
template<int N>
class TestRequestBuffer : RequestBuffer<N> {
public:
  using index_t modulus(index_t i) const;
};
#endif

#if 0
TEST_CASE("create a request buffer", "[request-buffer]") {
  RequestBuffer<8> buf;
}

TEST_CASE("insert one request to head", "[request-buffer]") {
  RequestBuffer<8> buf;
  auto req = buf.insert();
  req->state = Parsing;
  REQUIRE(buf.count() == 1);
}

TEST_CASE("insert two requests to head", "[request-buffer]") {
  RequestBuffer<8> buf;
  auto req1 = buf.insert();
  REQUIRE(buf.count() == 1);
  auto req2 = buf.insert();
  REQUIRE(buf.count() == 2);
}

TEST_CASE("insert a requests and pop it", "[request-buffer]") {
  RequestBuffer<8> buf;
  auto req = buf.insert();
  REQUIRE(req->id == 0);
  req->state = Parsed;
  req->id = 5;
  REQUIRE(buf.count() == 1);
  // pop a request, but it wont reduce count until released
  auto req2 = buf.pop();
  REQUIRE(buf.count() == 1);
  REQUIRE(req2->state == InUse);
  REQUIRE(req2->id == 5);
  req2.release();
  REQUIRE(buf.count() == 0);
}
#endif
