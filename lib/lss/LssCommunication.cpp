
#include "LssCommunication.h"

#define ACCEPT(cmdid)  { operator|=(cmdid); return (cmdid) != LssInvalid; }
#define SWITCH(cmdid)  if(*pkt==0 || !isalpha(*pkt)) ACCEPT(cmdid) else switch (toupper(*pkt++))


bool LssCommand::parse(const char*& pkt)
{
  /*  This code might be a bit confusing but it is much faster than a bunch of string
   *  comparisons. This is technically called a "Trie" or prefix tree.
   *  https://en.wikipedia.org/wiki/Trie
   *  
   *  To  simplify the code I use the above two macros SWITCH and ACCEPT:
   *  ACCEPT(cmdid) -- immediately return with a parsed command value of cmdid.
   *  SWITCH(cmdid) -- test if next character is a stop char (null char or non-alpha 
   *                   char) and if so accept it by immediately returning cmdid. If
   *                   not, then test the character as the next character in the Trie.
   *                   I.e. the next character in the reduced set of commands. 
   *                   
   *  NOTE The SWITCH(cmdid) macro uses standard switch() logic for testing characters, 
   *  but don't confuse the macro argument for the argument of a standard switch(), the 
   *  argument to SWITCH is the cmdid if we find a stop char and SWITCH is internally 
   *  aware the char to test is *pkt.
   */

  if(toupper(*pkt) == 'Q') {
      pkt++;
      goto queries;
  } else if(toupper(*pkt) == 'C') {
      pkt++;
      goto config;
  }

actions:
  SWITCH(LssInvalid) {
    case 'L': SWITCH(LssLimp) {
      case 'E': SWITCH(LssInvalid) {
        case 'D': ACCEPT(LssLEDColor);
      }
    }
    case 'H': ACCEPT(LssHaltAndHold);
    case 'O': ACCEPT(LssOriginOffset);
    case 'P': ACCEPT(LssPosition|LssPulse);
    case 'D': SWITCH(LssPosition|LssDegrees) {
      case 'E': SWITCH(LssInvalid) {
      case 'F': SWITCH(LssInvalid) {
      case 'A': SWITCH(LssInvalid) {
      case 'U': SWITCH(LssInvalid) {
      case 'L': SWITCH(LssInvalid) {
      case 'T': SWITCH(LssDefault) {
      }}}}}}
    }
    case 'A': SWITCH(LssInvalid) {
      case 'R': ACCEPT(LssAngularRange);
      case 'S': ACCEPT(LssAngularStiffness);
      case 'H': ACCEPT(LssAngularHoldingStiffness);
    }
    case 'E': SWITCH(LssInvalid) {
      case 'M': ACCEPT(LssMotionControl);
    }
    case 'F': SWITCH(LssInvalid) {
      case 'P': SWITCH(LssInvalid) {
        case 'C': ACCEPT(LssFilterPoleCount);
      }
    }
    case 'M': SWITCH(LssInvalid) {
      case 'D': ACCEPT(LssMove|LssDegrees);
    }
    case 'W': SWITCH(LssInvalid) {
      case 'D': ACCEPT(LssWheelMode|LssDegrees);
      case 'R': ACCEPT(LssWheelMode|LssRPM);
    }
    case 'B': ACCEPT(LssBaudRate);
    case 'G': ACCEPT(LssGyreDirection);
    case 'S': SWITCH(LssInvalid) {
      case 'D': ACCEPT(LssMaxSpeed|LssDegrees);
      case 'R': ACCEPT(LssMaxSpeed|LssRPM);
    }      
  }
  return false;
  
queries:
  mode = LssQuery;
  SWITCH(LssStatus) {
    case 'O': ACCEPT(LssOriginOffset);
    case 'A': SWITCH(LssInvalid) {
      case 'R': ACCEPT(LssAngularRange);
      case 'S': ACCEPT(LssAngularStiffness);
      case 'H': ACCEPT(LssAngularHoldingStiffness);
    }
    case 'P': ACCEPT(LssPosition|LssPulse);
    case 'D': SWITCH(LssPosition|LssDegrees) {
      case 'T': ACCEPT(LssTarget);
    }
    case 'W': SWITCH(LssInvalid) {
      case 'D': ACCEPT(LssWheelMode|LssDegrees);
      case 'R': ACCEPT(LssWheelMode|LssRPM);
    }
    case 'S': SWITCH(LssInvalid) {
      case 'D': ACCEPT(LssMaxSpeed|LssDegrees);
      case 'R': ACCEPT(LssMaxSpeed|LssRPM);
    }
    case 'L': SWITCH(LssInvalid) {
      case 'E': SWITCH(LssInvalid) {
        case 'D': ACCEPT(LssLEDColor);
      }
    }
    case 'I': SWITCH(LssInvalid) {
      case 'D': ACCEPT(LssID);
    }
    case 'B': ACCEPT(LssBaudRate);
    case 'G': ACCEPT(LssGyreDirection);
    // FirstPOsition Pulse/Degrees
    // Midel, SerialNumber, FirmwareVersion
    case 'V': ACCEPT(LssVoltage);
    case 'T': ACCEPT(LssTemperature);
    case 'C': ACCEPT(LssCurrent);
  }
  return false;

config:
  mode = LssConfig;
  SWITCH(LssInvalid) {
    case 'O': SWITCH(LssOriginOffset) {
      case 'N': SWITCH(LssInvalid) {
      case 'F': SWITCH(LssInvalid) {
      case 'I': SWITCH(LssInvalid) {
      case 'R': SWITCH(LssInvalid) {
      case 'M': SWITCH(LssConfirm) {
      }}}}}
    }
    case 'A': SWITCH(LssInvalid) {
      case 'R': ACCEPT(LssAngularRange);
      case 'S': ACCEPT(LssAngularStiffness);
      case 'H': ACCEPT(LssAngularHoldingStiffness);
    }
    case 'S': SWITCH(LssInvalid) {
      case 'D': ACCEPT(LssMaxSpeed|LssDegrees);
      case 'R': ACCEPT(LssMaxSpeed|LssRPM);
    }
    case 'L': SWITCH(LssInvalid) {
      case 'E': SWITCH(LssInvalid) {
        case 'D': ACCEPT(LssLEDColor);
      }
    }
    case 'M': SWITCH(LssInvalid) {
      case 'M': SWITCH(LssInvalid) {
         case 'D': ACCEPT(LssMaxDuty);
      }
    }
    case 'I': SWITCH(LssInvalid) {
      case 'D': ACCEPT(LssID);
    }
    case 'B': ACCEPT(LssBaudRate);
    case 'G': ACCEPT(LssGyreDirection);
    // FirstPOsition Pulse/Degrees
  }
  return false;
}

char* LssCommand::commandCode(char* out) const
{
  char* pout = out;
  switch(mode) {
      case LssQuery: *pout++ = 'Q'; break;
      case LssConfig: *pout++ = 'C'; break;
  }

  // filter out the member flag
  switch(id) {
    case LssStatus: // LssQuery command
      break;
    case LssID:
      *pout++ = 'I';
      *pout++ = 'D';
      break;
    case LssLimp:
      *pout++ = 'L';
      break;
    case LssHaltAndHold:
      *pout++ = 'H';
      break;
    case LssPosition:
      *pout++ = (units == LssPulse) ? 'P' : 'D';
      break;
    case LssTarget:
      *pout++ = 'D';
      *pout++ = 'T';
      break;
    case LssFirstPosition:
      *pout++ = 'F';
      *pout++ = (units == LssPulse) ? 'P' : 'D';
      break;
    case LssWheelMode:
      *pout++ = 'W';
      *pout++ = (units == LssRPM) ? 'R' : 'D';
      break;
    case LssMaxSpeed:
      *pout++ = 'S';
      *pout++ = (units == LssRPM) ? 'R' : 'D';
      break;
    case LssVoltage:
      *pout++ = 'V';
      break;
    case LssCurrent:
      *pout++ = 'C';
      break;
    case LssTemperature:
      *pout++ = 'T';
      break;
    case LssAngularRange:
      *pout++ = 'A';
      *pout++ = 'R';
      break;
    case LssAngularStiffness:
      *pout++ = 'A';
      *pout++ = 'S';
      break;
  case LssAngularHoldingStiffness:
      *pout++ = 'A';
      *pout++ = 'H';
      break;
    case LssLEDColor:
      *pout++ = 'L';
      *pout++ = 'E';
      *pout++ = 'D';
      break;
    case LssBaudRate:
      *pout++ = 'B';
      break;
    case LssGyreDirection:
      *pout++ = 'G';
      break;
    case LssOriginOffset:
      *pout++ = 'O';
      break;
  case LssMotionControl:
      *pout++ = 'E';
      *pout++ = 'M';
      break;
  case LssMaxDuty:
      *pout++ = 'M';
      *pout++ = 'M';
      *pout++ = 'D';
      break;
  case LssFilterPoleCount:
      *pout++ = 'F';
      *pout++ = 'P';
      *pout++ = 'C';
      break;
    case LssDefault:
      *pout++ = 'D';
      *pout++ = 'E';
      *pout++ = 'F';
      *pout++ = 'A';
      *pout++ = 'U';
      *pout++ = 'L';
      *pout++ = 'T';
      break;
    case LssConfirm:
      *pout++ = 'C';
      *pout++ = 'O';
      *pout++ = 'N';
      *pout++ = 'F';
      *pout++ = 'I';
      *pout++ = 'R';
      *pout++ = 'M';
      break;
    default:
      // cannot serialize, unknown command code
      return NULL;
  }

  *pout =0;
  return pout;
}

#if defined(HAVE_STRING)
String LynxPacket::toString() const {
  char buf[32];
  if(serialize(buf) !=NULL)
    return String(buf);
  return String();
}
#endif

char* LynxPacket::serialize(char* out) const
{
  // print ID efficiently
  unsigned char x = id;
  if(x>=100) {
      *out++ = '0'+(x/100);
      x %= 100;
      if(x<10)
          *out++ = '0';   // number is 2 digits, with a zero in the middle
  }
  if(x>=10) {
    *out++ = '0'+(x/10);
    *out++ = '0'+(x%10);
  } else {
    *out++ = '0'+x;
  }

  // print command code
  out = command.commandCode(out);
  if(out==NULL)
    return NULL;

  // use platform to convert value
  if(hasValue) {
    // if(NULL == itoa(value, out, 10)
    if (snprintf(out, 8, "%d", value) == -1)
      return NULL;
    while(*out) out++;  // skip to end
  } else
    *out=0;

    // filter out the member flag
    if (modifiers & (LssModCurrentHaltAndHold | LssModCurrentHaltAndLimp)) {
        if(modifiers & LssModCurrentHaltAndLimp) {
            // halt and limp will take precedence over hold
            *out++ = 'C';
            *out++ = 'L';
        } else {
            *out++ = 'C';
            *out++ = 'H';
        }
        if (snprintf(out, 8, "%d", current) == -1)
            return NULL;
        while(*out) out++;  // skip to end
    }

  return out;
}

LynxPacket::LynxPacket(const char* pkt)
  : id(0), microstamp(0), command(LssInvalid), modifiers(0), hasValue(false), value(0)
{
  parse(pkt);
}

int LynxPacket::readValue(const char*& pkt, bool& _hasValue)
{
    int _value=0;
    if(isdigit(*pkt) || *pkt=='-') {
        bool isNegative = false;
        if(*pkt=='-') {
            isNegative=true;
            pkt++;
        }

        while (*pkt && isdigit(*pkt)) {
            _value *= 10;
            _value += (int)(*pkt++ - '0');
        }
        if(isNegative)
            _value *= -1;
        _hasValue = true;
    }
    return _value;
}


bool LynxPacket::parse(const char* pkt)
{
  // we parse into local variables and then set instance members
  // when we are sure we've successfully parsed.
  short _id=0;
  LssCommand _command;
  bool _hasValue=false;
  int _value=0;
#if defined(LSS_LOGGING)
  const char* begin = pkt;
#endif

  if(!isdigit(*pkt))
    goto bad_read;
    
  // read ID
  while (*pkt && isdigit(*pkt)) {
      _id *= 10;
      _id += (short)(*pkt++ - '0');
  }

  if(!_command.parse(pkt))
    goto bad_read;

  _value = readValue(pkt, _hasValue);

  id=_id;
  command = _command;
  hasValue = _hasValue;
  value = _value;

  while(*pkt != '\r') {
    if(*pkt == 'C' && (*(pkt+1) == 'H' || *(pkt+1) == 'L')) {
       // current halt and hold
       pkt++;
       bool hv=false;
       _value = readValue(pkt, hv);
       if(hv) {
           modifiers |= (*pkt == 'L') ? LssModCurrentHaltAndLimp : LssModCurrentHaltAndHold;
           current = _value;
       }
       pkt++;
    } else
        pkt++;
  }

  return true;
    
bad_read:
#if defined(LSS_LOGGING)
  LSS_LOGGING.print("E@");
  LSS_LOGGING.print(pkt - begin);
  LSS_LOGGING.print(" ");
  while(begin <= pkt ) {
    if(isprint(*begin))
      LSS_LOGGING.print(*begin++);
    else
      LSS_LOGGING.print('[');
      LSS_LOGGING.print((short)*begin++, HEX);
      LSS_LOGGING.print(']');
  }
  LSS_LOGGING.println();
#endif
  return false;
}

bool LynxPacket::operator==(const LynxPacket& rhs) const
{
    return id==rhs.id && command==rhs.command && hasValue==rhs.hasValue && (!hasValue || (value==rhs.value));
}
