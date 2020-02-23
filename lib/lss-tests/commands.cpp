#include "lss-tests.hpp"

void test_parse_command(const char* cmd, LssCommand parsed)
{
  LssCommand val;
  REQUIRE(val.parse(cmd));
  REQUIRE( *cmd == 0 /* must not have extra unparsed characters */);
  REQUIRE(val == (LssCommand)parsed);
}

void test_parse_command_extra(const char* cmd, char c, LssCommand parsed)
{
    LssCommand val;
    REQUIRE(val.parse(cmd));
    REQUIRE( *cmd == c /* must not have extra unparsed characters */);
    REQUIRE(val == (LssCommand) parsed);
}

void fail_parse_command(const char* cmd, LssCommand parsed)
{
    LssCommand val;
    if(!val.parse(cmd))
        return; // successful fail
    if(*cmd != 0)
        return;      // left over characters are a fail (success fail test)
    REQUIRE(val != parsed);
}

void test_serialize_command(const char* pkt, LssCommand cmd)
{
  char buf[128];
  char* serialized = cmd.commandCode(buf);
  REQUIRE(NULL != serialized);
  REQUIRE(0 == strcmp(pkt, buf));
}

#define TEST_PARSE_COMMAND(str, ...)  TEST_CASE("parse command (" str ")", "[command,parse]") { test_parse_command(str, __VA_ARGS__); test_parse_command_extra(str "-", '-', __VA_ARGS__); }
#define TEST_SERIALIZE_COMMAND(str, ...)  TEST_CASE("serialize command (" str ")", "[command,serialize]") { test_serialize_command(str, __VA_ARGS__); }

#define FAIL_PARSE_COMMAND(str, ...)  TEST_CASE("fail to parse command (" str ")", "[command,parse]") { fail_parse_command(str, __VA_ARGS__); }

// action commands
TEST_PARSE_COMMAND("L", LssLimp);
TEST_PARSE_COMMAND("H", LssHaltAndHold);
//TEST_PARSE_COMMAND("T", LssTimedMove);
//TEST_PARSE_COMMAND("S", LssSpeed);
//TEST_PARSE_COMMAND("MD", LssMove|LssDegrees);
TEST_PARSE_COMMAND("O", LssOriginOffset);
TEST_PARSE_COMMAND("AR", LssAngularRange);
TEST_PARSE_COMMAND("P", LssPosition|LssPulse);
TEST_PARSE_COMMAND("D", LssPosition|LssDegrees);
TEST_PARSE_COMMAND("WD", LssWheelMode|LssDegrees);
TEST_PARSE_COMMAND("WR", LssWheelMode|LssRPM);
TEST_PARSE_COMMAND("SD", LssMaxSpeed|LssDegrees);
TEST_PARSE_COMMAND("SR", LssMaxSpeed|LssRPM);
TEST_PARSE_COMMAND("AS", LssAngularStiffness);
TEST_PARSE_COMMAND("LED", LssLEDColor);
TEST_PARSE_COMMAND("B", LssBaudRate);
TEST_PARSE_COMMAND("G", LssGyreDirection);

// query commands
TEST_PARSE_COMMAND("QO", LssQuery|LssOriginOffset);
TEST_PARSE_COMMAND("QAR", LssQuery|LssAngularRange);
TEST_PARSE_COMMAND("QP", LssQuery|LssPosition|LssPulse);
TEST_PARSE_COMMAND("QD", LssQuery|LssPosition|LssDegrees);
TEST_PARSE_COMMAND("QWD", LssQuery|LssWheelMode|LssDegrees);
TEST_PARSE_COMMAND("QWR", LssQuery|LssWheelMode|LssRPM);
TEST_PARSE_COMMAND("QSD", LssQuery|LssMaxSpeed|LssDegrees);
TEST_PARSE_COMMAND("QSR", LssQuery|LssMaxSpeed|LssRPM);
TEST_PARSE_COMMAND("QAS", LssQuery|LssAngularStiffness);
TEST_PARSE_COMMAND("QLED", LssQuery|LssLEDColor);
TEST_PARSE_COMMAND("QID", LssQuery|LssID);
TEST_PARSE_COMMAND("QB", LssQuery|LssBaudRate);
TEST_PARSE_COMMAND("QG", LssQuery|LssGyreDirection);
//TEST_PARSE_COMMAND("QFP", LssQuery|LssPowerUpPosition|LssPulse);
//TEST_PARSE_COMMAND("QFD", LssQuery|LssPowerUpPosition|LssDegrees);
TEST_PARSE_COMMAND("QDT", LssQuery|LssTarget);
//TEST_PARSE_COMMAND("QM", LssQuery|LssModel);
//TEST_PARSE_COMMAND("QN", LssQuery|LssSerial);
//TEST_PARSE_COMMAND("QF", LssQuery|LssFirmware);
TEST_PARSE_COMMAND("Q", LssStatus);
TEST_PARSE_COMMAND("QV", LssQuery|LssVoltage);
TEST_PARSE_COMMAND("QT", LssQuery|LssTemperature);
TEST_PARSE_COMMAND("QC", LssQuery|LssCurrent);

// lower case commands
TEST_PARSE_COMMAND("q", LssStatus);
TEST_PARSE_COMMAND("qv", LssQuery|LssVoltage);
TEST_PARSE_COMMAND("qt", LssQuery|LssTemperature);
TEST_PARSE_COMMAND("qc", LssQuery|LssCurrent);
TEST_PARSE_COMMAND("p", LssPosition|LssPulse);
TEST_PARSE_COMMAND("d", LssPosition|LssDegrees);
TEST_PARSE_COMMAND("cled", LssConfig|LssLEDColor);
TEST_PARSE_COMMAND("csd", LssConfig|LssMaxSpeed|LssDegrees);



// config commands
TEST_PARSE_COMMAND("CO", LssConfig|LssOriginOffset);
TEST_PARSE_COMMAND("CAR", LssConfig|LssAngularRange);
TEST_PARSE_COMMAND("CSD", LssConfig|LssMaxSpeed|LssDegrees);
TEST_PARSE_COMMAND("CSR", LssConfig|LssMaxSpeed|LssRPM);
TEST_PARSE_COMMAND("CAS", LssConfig|LssAngularStiffness);
TEST_PARSE_COMMAND("CLED", LssConfig|LssLEDColor);
TEST_PARSE_COMMAND("CID", LssConfig|LssID);
TEST_PARSE_COMMAND("CB", LssConfig|LssBaudRate);
TEST_PARSE_COMMAND("CG", LssConfig|LssGyreDirection);
//TEST_PARSE_COMMAND("CFP", LssQuery|LssPowerUpPosition|LssPulse);
//TEST_PARSE_COMMAND("CFD", LssQuery|LssPowerUpPosition|LssDegrees);

// test some commands with unparsed characters left over
FAIL_PARSE_COMMAND("PX", LssPosition|LssPulse);
FAIL_PARSE_COMMAND("QBy", LssQuery|LssBaudRate);
FAIL_PARSE_COMMAND("QLEDx", LssQuery|LssLEDColor);


// action commands
TEST_SERIALIZE_COMMAND("L", LssLimp);
TEST_SERIALIZE_COMMAND("H", LssHaltAndHold);
//TEST_SERIALIZE_COMMAND("T", LssTimedMove);
//TEST_SERIALIZE_COMMAND("S", LssSpeed);
//TEST_SERIALIZE_COMMAND("MD", LssMove|LssDegrees);
TEST_SERIALIZE_COMMAND("O", LssOriginOffset);
TEST_SERIALIZE_COMMAND("AR", LssAngularRange);
TEST_SERIALIZE_COMMAND("P", LssPosition|LssPulse);
TEST_SERIALIZE_COMMAND("D", LssPosition|LssDegrees);
TEST_SERIALIZE_COMMAND("WD", LssWheelMode|LssDegrees);
TEST_SERIALIZE_COMMAND("WR", LssWheelMode|LssRPM);
TEST_SERIALIZE_COMMAND("SD", LssMaxSpeed|LssDegrees);
TEST_SERIALIZE_COMMAND("SR", LssMaxSpeed|LssRPM);
TEST_SERIALIZE_COMMAND("AS", LssAngularStiffness);
TEST_SERIALIZE_COMMAND("LED", LssLEDColor);
TEST_SERIALIZE_COMMAND("B", LssBaudRate);
TEST_SERIALIZE_COMMAND("G", LssGyreDirection);

// query commands
TEST_SERIALIZE_COMMAND("QO", LssQuery|LssOriginOffset);
TEST_SERIALIZE_COMMAND("QAR", LssQuery|LssAngularRange);
TEST_SERIALIZE_COMMAND("QP", LssQuery|LssPosition|LssPulse);
TEST_SERIALIZE_COMMAND("QD", LssQuery|LssPosition|LssDegrees);
TEST_SERIALIZE_COMMAND("QWD", LssQuery|LssWheelMode|LssDegrees);
TEST_SERIALIZE_COMMAND("QWR", LssQuery|LssWheelMode|LssRPM);
TEST_SERIALIZE_COMMAND("QSD", LssQuery|LssMaxSpeed|LssDegrees);
TEST_SERIALIZE_COMMAND("QSR", LssQuery|LssMaxSpeed|LssRPM);
TEST_SERIALIZE_COMMAND("QAS", LssQuery|LssAngularStiffness);
TEST_SERIALIZE_COMMAND("QLED", LssQuery|LssLEDColor);
TEST_SERIALIZE_COMMAND("QID", LssQuery|LssID);
TEST_SERIALIZE_COMMAND("QB", LssQuery|LssBaudRate);
TEST_SERIALIZE_COMMAND("QG", LssQuery|LssGyreDirection);
TEST_SERIALIZE_COMMAND("QFP", LssQuery|LssFirstPosition|LssPulse);
TEST_SERIALIZE_COMMAND("QFD", LssQuery|LssFirstPosition|LssDegrees);
TEST_SERIALIZE_COMMAND("QDT", LssQuery|LssTarget);
//TEST_SERIALIZE_COMMAND("QM", LssQuery|LssModel);
//TEST_SERIALIZE_COMMAND("QN", LssQuery|LssSerial);
//TEST_SERIALIZE_COMMAND("QF", LssQuery|LssFirmware);
TEST_SERIALIZE_COMMAND("Q", LssQuery|LssStatus);
TEST_SERIALIZE_COMMAND("QV", LssQuery|LssVoltage);
TEST_SERIALIZE_COMMAND("QT", LssQuery|LssTemperature);
TEST_SERIALIZE_COMMAND("QC", LssQuery|LssCurrent);

// config commands
TEST_SERIALIZE_COMMAND("CO", LssConfig|LssOriginOffset);
TEST_SERIALIZE_COMMAND("CAR", LssConfig|LssAngularRange);
TEST_SERIALIZE_COMMAND("CSD", LssConfig|LssMaxSpeed|LssDegrees);
TEST_SERIALIZE_COMMAND("CSR", LssConfig|LssMaxSpeed|LssRPM);
TEST_SERIALIZE_COMMAND("CAS", LssConfig|LssAngularStiffness);
TEST_SERIALIZE_COMMAND("CLED", LssConfig|LssLEDColor);
TEST_SERIALIZE_COMMAND("CID", LssConfig|LssID);
TEST_SERIALIZE_COMMAND("CB", LssConfig|LssBaudRate);
TEST_SERIALIZE_COMMAND("CG", LssConfig|LssGyreDirection);
//TEST_SERIALIZE_COMMAND("CFP", LssQuery|LssPowerUpPosition|LssPulse);
//TEST_SERIALIZE_COMMAND("CFD", LssQuery|LssPowerUpPosition|LssDegrees);
