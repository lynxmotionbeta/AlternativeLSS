//
// Created by guru on 11/22/19.
//

#ifndef LYNXMOTIONLSS_LSSTRANSACTION_H
#define LYNXMOTIONLSS_LSSTRANSACTION_H


#include "LssPromise.h"
#include "LssCommunication.h"

#include <vector>

#include <algorithm>

struct _packet_order_by_busid {
    inline int operator()(const LynxPacket& lhs, const LynxPacket& rhs) const {
        return (lhs.id == rhs.id)
               // put value commands first (not a query)
               ? (lhs.hasValue == rhs.hasValue) ? 0 : (lhs.hasValue) ? -1 : +1
               // sort by bus ID
               : (lhs.id < rhs.id)
                 ? -1
                 : (lhs.id > rhs.id)
                   ? +1
                   : 0;
    }
    static struct _packet_order_by_busid sorter;
};


class LssTransaction {
public:
    typedef enum {
        Pending,
        Active,
        Completed,
        Expired
    } State;

    unsigned long txn;          // current transmission number for this request
    unsigned long timestamp;    // millis when first packet was sent
    unsigned long expireAt;     // timestamp when the read request will be considered expired
    unsigned long long nextQ;       // micros when next query transmission can be sent (set by last Q to give time for servo to reply without collision)
    static unsigned long Qwait;
    State state;

    // statistics
    unsigned long
        txt,             // transaction start time
        ttfr,            // time to first result
        ttc;             // time to complete

    // construct a new transaction with the given packets to  transmit
    LssTransaction(unsigned long _txn, std::initializer_list<LynxPacket> packets, unsigned long _expire_uSec=10000);

    template<class It>
    LssTransaction(unsigned long _txn, It first, It last, unsigned long _expire_uSec=10000)
            : txn(_txn), timestamp(micros()), expireAt(0), nextQ(0), state(Pending), txt(0), ttfr(0), ttc(0), expireInterval(_expire_uSec),
              _packets(first, last)
    {
        //std::sort(_packets.begin(), _packets.end(), _packet_order_by_busid::sorter);
        _tx = _packets.begin();
        _rx = _packets.begin();
    }

    template<class It>
    LssTransaction(It first, It last, unsigned long _expire_uSec=10000) : LssTransaction(0, first, last, _expire_uSec) {}

    // we cannot copy LssTransactions
    LssTransaction(const LssTransaction& copy) = delete;
    LssTransaction& operator=(const LssTransaction& copy) = delete;

    // clear a transaction so it can be sent again
    void reset();

    void expire();

    // returns true if the transaction should be expired
    bool expired(unsigned long long tsnow=0) const;

    inline const std::vector<LynxPacket>& packets() const { return _packets; }
    inline std::vector<LynxPacket>& packets() { return _packets; }

    const LynxPacket next();
    void dispatch(const LynxPacket& p);

protected:
    unsigned long expireInterval;

    // list of packets in this transaction
    std::vector<LynxPacket> _packets;

    // pointer to the next packet to be transmitted
    std::vector<LynxPacket>::iterator _tx;

    // pointer to the next query response we expect to receive
    std::vector<LynxPacket>::iterator _rx;

    void checkCompleteStatus();
};


#endif //LYNXMOTIONLSS_LSSTRANSACTION_H
