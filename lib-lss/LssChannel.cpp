
#include "LssChannel.h"

//void * operator new (size_t size, void * ptr) { return ptr; }


LssChannel::LssChannel()
        : timeout_usec(TRANSACTION_TIMEOUT), unresponsive_request_limit(unresponsive_request_limit), unresponsive_disable_interval(UNRESPONSIVE_DISABLE_INTERVAL),
          txn_current(1), txn_next(1)
{
    pthread_mutex_init(&txlock, NULL);
    _driver = new LssPosixChannel(this);
}

LssChannel::~LssChannel()
{
    close();
    pthread_mutex_destroy(&txlock);
}

#if 0
// todo make this take a collection of servos
AsyncToken LssChannel::ReadAsyncAll(LssCommands commands)
{
    if(count>0) {
        AsyncToken rt;
        for (int i = 0; i < count; i++) {
            AsyncToken t = servos[i]->ReadAsync(commands);
            if (t.isActive())
                rt = t; // only accept if we got a valid active token, otherwise the servo is ignored and AsyncAll does its best
        }
        return rt;  // return the last active token, which will be the last token to finish and thus finishing our AsyncAll
    } else
        return AsyncToken();
}
#endif

ChannelDriverError LssChannel::begin(const char* devname, int baudrate)
{
    _driver = new LssPosixChannel(this);
    ChannelDriverError rv = (ChannelDriverError)(_driver->signal(OpenSignal, baudrate, devname));
    if( rv != DriverSuccess)
        delete _driver;
    return rv;
}

LssTransaction::Promise LssChannel::send(std::initializer_list<LynxPacket> packets)
{
    pthread_mutex_lock(&txlock);
    transactions.emplace_back(txn_next++, packets);
    auto& promise = transactions.back().promise;
    bool sendSignal = transactions.size() ==1;
    pthread_mutex_unlock(&txlock);
    if(sendSignal)
        _driver->signal(TransactionSignal, 0, nullptr);
    return promise;
}


#if 0
bool LssChannel::waitFor(const AsyncToken& token)
{
    unsigned long timeout = micros() + timeout_usec;
    unsigned long _txn = txn_current;
    while( micros() < timeout && token.isActive()) {
        if(_txn != txn_current) {
            // a servo responded, reset timeout
            timeout = micros() + timeout_usec;
            _txn = txn_current;
        }

        update(); // this call will process incoming data and update the servos
    }
    return token.isComplete();
}
#endif

void LssChannel::completeTransaction()
{
    auto &current = transactions.front();
    if(current.state == LssTransaction::Completed)
        current.promise.resolve(current);   // will call promises
    else
        current.promise.reject(current);   // will call promises

    pthread_mutex_lock(&txlock);
    transactions.pop_front();
    bool transmit_next = !transactions.empty();
    pthread_mutex_unlock(&txlock);

    // transmit the next transaction
    if(transmit_next)
        driverIdle();

}

void LssChannel::driverIdle()
{
    if(!transactions.empty()) {
        auto now = micros();
        auto &current = transactions.front();
        if(current.expired(now)) {
            // this transaction has waited too long
            current.expire();
            completeTransaction();
            return;
        }

        LynxPacket p;
        do
        {
            p = current.next();
            if(p.id) {
                LssChannel::transmit(p);
            }
        } while (p.id && (p.command & LssQuery)==0);

        if(current.state >= LssTransaction::Completed)
            completeTransaction();
    }
}

void LssChannel::driverDispatch(LynxPacket& p) {
    p.microstamp = micros();
    if (!transactions.empty()) {
        auto &current = transactions.front();
        current.dispatch(p);
        if(current.state >= LssTransaction::Completed) {
            completeTransaction();

#if 0
            // todo: update all servos in the transaction
            for(int i=0; i<count; i++) {
                if(servos[i]->id == packet.id) {
                    //servos[i]->dispatch(packet);
                    consecutive_errors = 0;
                    //dispatchPromises();


                    break;
                }
            }
#endif
        }
        driverIdle();
    }
}

void LssChannel::transmit(const LynxPacket &p)
{
    char buf[64];
    char* pend = buf;
    *pend++ = '#';
    if((pend=p.serialize(pend)) !=NULL) {
        char* pbegin = buf;
#if defined(LSS_LOGGING) && defined(LSS_LOG_PACKETS)
        LSS_LOGGING.print(">> ");
      LSS_LOGGING.println(pbegin);
#endif
        *pend++ = '\r';
        *pend=0;

        // p.microstamp = micros(); //todo: how can we have this set in the const transmit call?

        // transmit to LSS bus
        _driver->signal(TransmitSignal, pend - pbegin, pbegin);
    }
}

short LssChannel::scan(short beginId, short endId)
{
    // array to keep track of discovered devices
    short N = 0;
    short discovered[endId - beginId + 1];
    memset(discovered, 0, sizeof(discovered));

    printf("scanning is not implemented on posix");
# if 0
    // build a channel attached to same stream as current, but we will only add 1 servo
#if defined(ARDUINO)
    LssArduinoChannel sub_channel(this->name);
    sub_channel.begin(*this->serial);
#else
    LssPosixChannel sub_channel(this->name);
    // todo: this function must be rewritten not to create a Channel copy, maybe remove the servo list, then restore after scan...hell we end up calling create() anyway
#endif
    LynxServo servo(beginId);

    // set timeout much lower
    sub_channel.timeout_usec = 2000000UL;

    // add a single servo and we will keep incrementing the ID
    servo.stats = NULL;       // dont bother collecting servo stats
    sub_channel.add(servo);   // add our scan servo to the scan channel

    // iterate each ID on our single scan servo
    while (servo.id <= endId) {
        AsyncToken t = servo.ReadAsync(LssQuery);
        if (sub_channel.waitFor(t)) {
            // found a servo
            discovered[N++] = servo.id;
        }

        servo.id++; // scan next ID
        servo.timeouts = 0;
        servo.enableAfter_millis = 0;
    }

    // clear all servos (remove the servo we temporarily used to enumerate)
    sub_channel.free();

    // create servos of all discovered
    // for scans, the channel will own and must free the created LynxServo's
    if(N>0)
        create(discovered, N);
#endif
    return N;
}

void LssChannel::create(const short* ids, short N)
{
    short* shadowed_ids = (short*)calloc(N, sizeof(short));
    short shadowed_N = N;
#if 0
    memcpy(shadowed_ids, ids, N*sizeof(short));

    // clear out any servos that already exist
    if (count > 0) {
        // todo: this could be more efficient if continuous servo addition or bus-scanning really matters
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < count; j++) {
                if (servos[j] && ids[i] && ids[i] == servos[j]->id) {
                    // squelch adding this existing servo ID
                    shadowed_ids[i] = 0;
                    shadowed_N--;
                }
            }
        }
    }

    if (shadowed_N > 0) {
        // allocate new servos
        alloc(count + shadowed_N);
        for (int i = 0; i < N; i++) {
            if (shadowed_ids[i] > 0) {
                //LynxServo* servo = new (&_servos[i]) LynxServo(shadowed_ids[i]);
                LynxServo* servo = new LynxServo(shadowed_ids[i]);
                servo->channel = (LynxChannel*)this;
                servo->channel_owned = true;  // mark this object for ::free when channel deallocates
                servos[count++] = servo;
            }
        }
    }

    // free shadow mem
    ::free(shadowed_ids);
#endif
}