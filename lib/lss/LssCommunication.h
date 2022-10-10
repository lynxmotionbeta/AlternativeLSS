
#pragma once

#ifdef ARDUINO
#include <stdint.h>
#else
#include <cstdint>
#endif

#include "LynxmotionLSS-Config.h"
#include "LssCommon.h"

#if defined(LSS_STATS)
#include "analytics/aggregate.h"
#endif

#ifndef BIT
// hopefully if this is already defined it performs the same function
#define BIT(n) (((uint64_t)1)<<n)
#endif

typedef uint64_t LssCommands;
typedef unsigned long LssModifiers;

// todo: These bitmask defines were updated in a consolidated branch
//       to use enums.
//       https://github.com/lynxmotionbeta/AlternativeLSS/commit/53bdc709f43f7d223e82563645f3d5f4603411a9
//          consolidate branch:
//       https://github.com/lynxmotionbeta/AlternativeLSS/compare/feature/consolidate?expand=1
//
#define  LssInvalid          0
#define  LssAction           BIT(1)
#define  LssQuery            BIT(2)
#define  LssConfig           BIT(3)
#define  LssCommandModes     (LssAction|LssQuery|LssConfig)

#define  LssDegrees          BIT(4)
#define  LssRPM              BIT(5)
#define  LssPulse            BIT(6)
#define  LssUnits            (LssDegrees|LssRPM|LssPulse)

#define  LssID               BIT(7)
#define  LssBaudRate         BIT(8)
#define  LssLimp             BIT(9)
#define  LssHaltAndHold      BIT(10)
#define  LssMove             BIT(12)
#define  LssPosition         BIT(13)
#define  LssTarget           BIT(14)
#define  LssWheelMode        BIT(16)
#define  LssMaxSpeed         BIT(17)
#define  LssSpeed            BIT(33)
#define  LssAngularRange     BIT(18)
#define  LssAngularStiffness BIT(19)
#define  LssAngularHoldingStiffness BIT(31)
#define  LssOriginOffset     BIT(20)
#define  LssGyreDirection    BIT(21)
#define  LssLEDColor         BIT(22)
#define  LssCurrent          BIT(23)
#define  LssVoltage          BIT(24)
#define  LssTemperature      BIT(25)
#define  LssFirstPosition    BIT(26)
#define  LssMotionControl    BIT(27)
#define  LssFilterPoleCount  BIT(28)
#define  LssDefault          BIT(29)
#define  LssConfirm          BIT(30)
#define  LssMaxDuty          BIT(32)
#define  LssAnalog           BIT(34)
#define  LssReset            BIT(35)
#define  LssModel            BIT(36)

#define  LssCommandSet       (LssCommands)((0xffffffffffffULL & ~(LssCommandModes|LssUnits)) | LssQuery)

// modifiers
#define  LssModTimedMove            BIT(2)
#define  LssModSpeed                BIT(3)
#define  LssModCurrentHaltAndHold   BIT(4)
#define  LssModCurrentHaltAndLimp   BIT(5)
#define  LssModifiersSet        (LssTimedMove|LssSpeed|LssModCurrentHaltAndHold)

// commands that are part of servo configuration
// you probably dont need these for normal control operations
#define  LssConfigCommandSet  ( \
       LssWheelMode \
      |LssMaxSpeed \
      |LssAngularRange \
      |LssAngularStiffness \
      |LssLEDColor \
      |LssBaudRate \
      |LssGyreDirection \
      |LssOriginOffset \
      |LssID )

  // commands that support asynchronous queries
#define  LssAsyncCommandSet  ( \
       LssQuery \
      |LssPosition \
      |LssTarget \
      |LssMaxSpeed \
      |LssVoltage \
      |LssCurrent \
      |LssConfigCommandSet )


typedef enum {
  LssLedOff  = 0,
  LssRed     = 1,
  LssGreen   = 2,
  LssBlue    = 3,
  LssYellow  = 4,
  LssCyan    = 5,
  LssMagenta = 6,
  LssWhite   = 7
} LssColors;

class LssValue {
public:
    typedef enum {
        None=0,
        I,
    } TypeID;

public:
    inline TypeID type() const { return _type; }
    inline short length() const { return _length; }
    inline short capacity() const { return _capacity; }

    inline bool valid() const { return _type != None; }

    inline int operator[](short idx) const {
        switch(_type) {
            case None: return 0;
            case I: return (idx < _length)
                ? (_capacity<=1) ? v.i : v.i_ptr[idx]
                : 0;
            default: return 0;
        }
    }

    inline operator int() const { return operator[](0); }

private:
    TypeID _type;
    short _length;
    short _capacity;

    union {
        int *i_ptr;
        int i;
    } v;
};


class LynxPacket {
public:
    typedef enum {
        Action,
        Query,
        Config
    } Method;

  typedef uint32_t value_type;

  public:
    short id;
    unsigned long long microstamp;  // timestamp in microseconds the packet was last transmitted or received
    LssCommands command;
    LssModifiers modifiers;
    bool hasValue;
	bool enable_;
	value_type value;

    // modifier values
    int current;//, speed, timedMove;

    inline LynxPacket() : id(0), microstamp(0), command(LssInvalid), modifiers(0), hasValue(false), enable_(true), value(0) {}
    inline LynxPacket(short _id, LssCommands _command) : id(_id), microstamp(0), command(_command), modifiers(0), hasValue(false), enable_(true), value(0) {}
    inline LynxPacket(short _id, LssCommands _command, value_type _value) : id(_id),
                                                                   microstamp(0), command(_command), modifiers(0), hasValue(true), enable_(true), value(_value) {}

    explicit LynxPacket(const char* pkt);

    bool operator==(const LynxPacket& rhs) const;

    inline void clear() { value = 0; hasValue=false;  }
    inline void set(value_type _value) { value=_value; hasValue=true; enable_ =
          true; }

    inline bool isEnabled() const { return command!=0 && enable_; }
	inline void enable(bool e = true) { enable_ = e; }

    inline LynxPacket& currentHaltAndHold(int _current) { modifiers |= LssModCurrentHaltAndHold; current = _current; return *this; }
    inline LynxPacket& currentHaltAndLimp(int _current) { modifiers |= LssModCurrentHaltAndLimp; current = _current; return *this; }

    bool parse(const char* pkt);

    char* serialize(char* out) const;

    inline bool matches(LssCommands bits) const { return (command & bits) == bits; }

    inline bool between(value_type min, value_type max) const { return hasValue && value >= min && value <= max; }

    inline bool broadcast() const { return id == 254; }

    // true if command is a query command
    inline bool query() const { return (command & LssQuery) >0; }

    // true if command requests value be written to flash (Config prefix)
    inline bool flash() const { return (command & LssConfig) >0; }

    static LssCommands parseCommand(const char*& pkt);
    
    // converts the cmd into a string command code and places the result in 'out'
    // returns the end of the command code string within the 'out' memory, or NULL if an error
    static char* commandCode(LssCommands cmd, char* out);

    // converts
    static char* modifierCode(LssModifiers mods, char* out);


#if defined(HAVE_STRING)
    String toString() const;
#endif

private:
    int readValue(const char*& pkt, bool& _hasValue);
};
