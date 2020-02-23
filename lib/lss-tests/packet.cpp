#include "lss-tests.hpp"


void test_parse_packet(const char* pkt, LynxPacket packet)
{
    LynxPacket p;
    REQUIRE(p.parse(pkt));
    REQUIRE(p.id == packet.id);
    REQUIRE(p.command == packet.command);
    REQUIRE(p.hasValue == packet.hasValue);
    if(packet.hasValue)
        REQUIRE(p.value == packet.value);
}

void fail_parse_packet(const char* pkt)
{
    LynxPacket p;
    REQUIRE(!p.parse(pkt));
    REQUIRE(p.id == 0);
    REQUIRE(p.command == LssInvalid);
    REQUIRE(!p.hasValue);
    REQUIRE(p.value == 0);
}

void test_serialize_packet(const char* pkt, LynxPacket packet)
{
    char buf[128];
    char* serialized = packet.serialize(buf);
    REQUIRE(serialized!=NULL);
    REQUIRE(strcmp(pkt, buf)==0);
}

#define TEST_PACKET_PARSE(str, ...)  TEST_CASE("parse packet (" str ")", "[packet,parse]") { test_parse_packet(str, LynxPacket(__VA_ARGS__)); }
#define FAIL_PACKET_PARSE(str)  TEST_CASE("fail to parse packet (" str ")", "[packet,parse]") { fail_parse_packet(str); }

#define TEST_SERIALIZE_PACKET(str, ...)  TEST_CASE("serialize packet (" str ")", "[packet,serialize]") { test_serialize_packet(str, LynxPacket(__VA_ARGS__)); }


/*
 *   Packet Parse Unit Tests
 */
TEST_PACKET_PARSE("1L", 1, LssLimp)
TEST_PACKET_PARSE("2Q", 2, LssStatus)
TEST_PACKET_PARSE("24LED", 24, LssLEDColor)
TEST_PACKET_PARSE("24LED5", 24, LssLEDColor, 5)
TEST_PACKET_PARSE("2D905", 2, LssPosition|LssDegrees, 905)
TEST_PACKET_PARSE("2D-905", 2, LssPosition|LssDegrees, -905)
TEST_PACKET_PARSE("02D-905", 2, LssPosition|LssDegrees, -905)

FAIL_PACKET_PARSE("-2Q")
FAIL_PACKET_PARSE("2@Q")
FAIL_PACKET_PARSE("2.Q")
FAIL_PACKET_PARSE(".2Q")
FAIL_PACKET_PARSE("Q2Q")
TEST_CASE("fail to parse empty packet", "[packet,parse]") { fail_parse_packet(""); }


/*
 *   Packet Serialization Tests
 */
TEST_SERIALIZE_PACKET("6QD1800", 6, LssQuery|LssPosition|LssDegrees, 1800);
TEST_SERIALIZE_PACKET("16QD1800", 16, LssQuery|LssPosition|LssDegrees, 1800);
TEST_SERIALIZE_PACKET("6QP3650", 6, LssQuery|LssPosition|LssPulse, 3650);
TEST_SERIALIZE_PACKET("16QP3650", 16, LssQuery|LssPosition|LssPulse, 3650);
TEST_SERIALIZE_PACKET("6QP-3650", 6, LssQuery|LssPosition|LssPulse, -3650);
TEST_SERIALIZE_PACKET("16QP-3650", 16, LssQuery|LssPosition|LssPulse, -3650);
TEST_SERIALIZE_PACKET("6P-3650", 6, LssPosition|LssPulse, -3650);
TEST_SERIALIZE_PACKET("16P-3650", 16, LssPosition|LssPulse, -3650);
TEST_SERIALIZE_PACKET("6D-3650", 6, LssPosition|LssDegrees, -3650);
TEST_SERIALIZE_PACKET("16D-3650", 16, LssPosition|LssDegrees, -3650);
TEST_SERIALIZE_PACKET("6CFD3650", 6, LssConfig|LssFirstPosition|LssDegrees, 3650);
TEST_SERIALIZE_PACKET("16CFD3650", 16, LssConfig|LssFirstPosition|LssDegrees, 3650);
TEST_SERIALIZE_PACKET("6CFD-650", 6, LssConfig|LssFirstPosition|LssDegrees, -650);
TEST_SERIALIZE_PACKET("16CFD-650", 16, LssConfig|LssFirstPosition|LssDegrees, -650);
TEST_SERIALIZE_PACKET("6CFP3650", 6, LssConfig|LssFirstPosition|LssPulse, 3650);
TEST_SERIALIZE_PACKET("16CFP3650", 16, LssConfig|LssFirstPosition|LssPulse, 3650);
TEST_SERIALIZE_PACKET("6CFP-650", 6, LssConfig|LssFirstPosition|LssPulse, -650);
TEST_SERIALIZE_PACKET("16CFP-650", 16, LssConfig|LssFirstPosition|LssPulse, -650);


TEST_CASE("test packet copy constructor", "[packet,construction]")
{
    LynxPacket a(16, LssQuery|LssPosition|LssDegrees, 125);
    LynxPacket b(a);
    REQUIRE(a == b);
}

TEST_CASE("test packet copy assignment", "[packet,construction]")
{
    LynxPacket a(16, LssQuery|LssPosition|LssDegrees, 125);
    LynxPacket b(5, LssTarget, 34);
    b = a;
    REQUIRE(a == b);
}






