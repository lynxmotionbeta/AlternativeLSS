
#pragma once

#include "LynxmotionLSS-Config.h"
#include "LssCommon.h"

#if defined(LSS_STATS)
#include "analytics/aggregate.h"
#endif

#ifndef BIT
// hopefully if this is already defined it performs the same function
#define BIT(n) (((unsigned long)1)<<n)
#endif

#define LssBroadcastAddress   (254)

typedef unsigned long LssModifiers;

typedef enum __attribute__ ((__packed__)) {
  LssAnyMode = 0,
  LssAction = BIT(1),
  LssQuery  = BIT(2),
  LssConfig = BIT(3)
} LssCommandMode;

typedef enum __attribute__ ((__packed__)) {
  LssAnyUnit = 0,
  LssDegrees = BIT(4),
  LssRPM     = BIT(5),
  LssPulse   = BIT(6)
} LssUnits;

typedef enum __attribute__ ((__packed__)) {
  LssInvalid =0,
  LssID,
  LssStatus,        // Simply the Q command with no sub-command id
  LssBaudRate,
  LssLimp,
  LssHaltAndHold,
  LssMove,
  LssPosition,
  LssTarget,
  LssWheelMode,
  LssMaxSpeed,
  LssAngularRange,
  LssAngularStiffness,
  LssAngularHoldingStiffness,
  LssOriginOffset,
  LssGyreDirection,
  LssLEDColor,
  LssCurrent,
  LssVoltage,
  LssTemperature,
  LssFirstPosition,
  LssMotionControl,
  LssFilterPoleCount,
  LssDefault,
  LssConfirm,
  LssMaxDuty,

  // todo: implement new commands from master
  LssAnalog,
  LssReset,
  LssModel
} LssCommandID;

// modifiers
#define  LssModTimedMove            BIT(2)
#define  LssModSpeed                BIT(3)
#define  LssModCurrentHaltAndHold   BIT(4)
#define  LssModCurrentHaltAndLimp   BIT(5)
#define  LssModifiersSet        (LssTimedMove|LssSpeed|LssModCurrentHaltAndHold)


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

/*class LssCommandParser {
public:
    LssCommandMode mode;
    LssUnits units;
    LssCommand command;
    LssModifiers modifiers;

    LssCommandParser();


    int value;
};*/


/* LssCommand class
 *
 * The new way to store an LSS command in integer form. We broke out the old bitmask to 3 fields that indicate the
 * command mode (Query, Config or Action) and units (Degrees, RMP or Pulses) and the Command ID.
 *
 * This class has many operator overloads to maintain compatibility with the old bitmask mode of defining LSS commands.
 * You can still define a command by bitmask OR'ing the Mode, Units and Command ID together.
 *
 */
class LssCommand {
public:
    LssCommandID id;
    LssCommandMode mode;
    LssUnits units;

    inline LssCommand(LssCommandID _id, LssCommandMode _mode, LssUnits _units)
        : id(_id), mode(_mode), units(_units) {}
    inline LssCommand(LssCommandID _id)
        : LssCommand(_id, LssAnyMode, LssAnyUnit) {}
    inline LssCommand()
        : LssCommand(LssInvalid, LssAnyMode, LssAnyUnit) {}

    inline LssCommand(const LssCommand& copy) : id(copy.id), mode(copy.mode), units(copy.units) {}

    inline LssCommand& operator=(const LssCommand& _copy) = default;
    inline LssCommand& operator=(LssCommandID _id) { id = _id; return *this; }
    inline LssCommand& operator=(LssCommandMode _mode) { mode = _mode; return *this; }

    inline LssCommand& operator|=(const LssCommand& rhs) {
        if(rhs.id != LssInvalid) id = rhs.id;
        if(rhs.mode != LssAnyMode) mode = rhs.mode;
        if(rhs.units != LssAnyUnit) units = rhs.units;
        return *this;
    }

    inline LssCommand& operator|(const LssCommandID _id) { id = _id; return *this; }
    inline LssCommand& operator|(const LssCommandMode _mode) { mode = _mode; return *this; }
    inline LssCommand& operator|(const LssUnits _units) { units = _units; return *this; }

    inline bool operator&(const LssUnits _units) const { return units & _units; }
    inline bool operator&(const LssCommandID _id) const { return id & _id; }
    inline bool operator&(const LssCommandMode _mode) const { return mode & _mode; }

    inline bool operator==(const LssCommand& rhs) const {
        return (id == rhs.id) && (rhs.mode==LssAnyMode || mode==rhs.mode) && (rhs.units==LssAnyUnit || units==rhs.units);
    }
    inline bool operator==(LssCommandID _id) const { return id == _id; }
    inline bool operator!=(const LssCommand& rhs) const { return !operator==(rhs); }

    // Parse a command string into mode, units and command ID fields
    bool parse(const char*& pkt);

    // converts the cmd into a string command code and places the result in 'out'
    // returns the end of the command code string within the 'out' memory, or NULL if an error
    char* commandCode(char* out) const;
};

inline LssCommand operator|(LssCommandMode mode, LssCommandID id) {
    return LssCommand(id, mode, LssAnyUnit);
}

inline LssCommand operator|(LssCommandID id, LssUnits units) {
    return LssCommand(id, LssAnyMode, units);
}

inline LssCommand operator|(LssCommandID id, LssCommandMode mode) {
    return LssCommand(id, mode, LssAnyUnit);
}

inline LssCommand operator|(LssCommandID id, LssCommand cmd) {
    return cmd | id;
}
inline LssCommand operator|(LssCommandMode mode, LssCommand cmd) {
    return cmd | mode;
}
inline LssCommand operator|(LssUnits units, LssCommand cmd) {
    return cmd | units;
}



class LynxPacket {
public:
    short id;
    unsigned long long microstamp;  // timestamp in microseconds the packet was last transmitted or received
    LssCommand command;
    LssModifiers modifiers;
    bool hasValue;
    long value;

    // modifier values
    int current;//, speed, timedMove;

    inline LynxPacket() : id(0), microstamp(0), command(LssInvalid), modifiers(0), hasValue(false), value(0) {}

    inline LynxPacket(short _id, LssCommand _command)
            : id(_id), microstamp(0), command(_command), modifiers(0), hasValue(false), value(0) {}
    inline LynxPacket(short _id, LssCommand _command, long _value)
            : id(_id), microstamp(0), command(_command), modifiers(0), hasValue(true), value(_value) {}


    explicit LynxPacket(const char* pkt);

    bool operator==(const LynxPacket& rhs) const;

    inline void clear() { value = 0; hasValue=false; }
    inline void set(long _value) { value=_value; hasValue=true; }

    inline LynxPacket& currentHaltAndHold(int _current) { modifiers |= LssModCurrentHaltAndHold; current = _current; return *this; }
    inline LynxPacket& currentHaltAndLimp(int _current) { modifiers |= LssModCurrentHaltAndLimp; current = _current; return *this; }

    bool parse(const char* pkt);

    char* serialize(char* out) const;

	inline bool matches(LssCommand _command) const { return (command == _command); }

    inline bool between(long min, long max) const { return hasValue && value >= min && value <= max; }

    inline bool broadcast() const { return id == 254; }

    // true if command is a query command
    inline bool query() const { return (command & LssQuery) >0; }

    // true if command requests value be written to flash (Config prefix)
    inline bool flash() const { return (command & LssConfig) >0; }

    // converts
    //static char* modifierCode(LssModifiers mods, char* out);


#if defined(HAVE_STRING)
    String toString() const;
#endif

private:
    int readValue(const char*& pkt, bool& _hasValue);
};
