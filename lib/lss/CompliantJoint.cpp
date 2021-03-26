//
// Created by guru on 11/24/19.
//

#include "CompliantJoint.h"


CompliantJoint::CompliantJoint(short joint_lssId, std::string _name)
    : joint(joint_lssId), name(_name), state(ComplianceLimp), stateChanged(0), current(0),gravityBias(0), currentLimit(170), cpr(0), cpr_changed(false) //, targetUpdated(false)
{
    //fudge.filter(2);
}

const char* CompliantJoint::stateName(JointState state) {
    switch(state) {
        case Disabled: return "Disabled";
        case Limp: return "Limp";
        case Holding: return "Holding";
        case Moving: return "Moving";
        case PositiveCompliance: return "+Compliance";
        case NegativeCompliance: return "-Compliance";
        case ComplianceLimp: return "ComplianceLimp";
        default:
            return "?state?";
    }
}


void CompliantJoint::enable(bool _enable) {
    if(isEnabled() != _enable) {
        transitionTo(_enable ? Limp : Disabled);
    }
}

void CompliantJoint::transitionTo(JointState new_state) {
    if(new_state == state)
         return; // No change

     prevState = state;
     stateChanged = millis();

    // we get a chance to do something on the transition
    auto action = transitionActions.find( StateTransition(state, new_state) );
    if(action != transitionActions.end()) {
        // call the action
        if(action->second) {
            state = action->second(state, new_state);
            return;
        }
    }
    state = new_state;
}

CompliantJoint& CompliantJoint::moveTo(int _position, bool _transition) {
    position.target(_position);
    //targetUpdated = true;
    if(_transition)
        transitionTo( (position.current().velocity() > 10) ? Moving : Holding );
    return *this;
}

void CompliantJoint::update(unsigned long tsnow)
{
    if(state <= Limp)
        return;
    bool entering = state != prevState;
    int w;

    if(!isLimp() && tsnow > 1200) {
        int dx = position.current().velocity();
        bool pos_polarity = dx > 0;
        bool neg_polarity = dx < 0;

        currentBias = neg_polarity
                ? currentPositiveBias.value()
                : pos_polarity
                    ? currentNegativeBias.value()
                    : 0;

        // shadow it for gain
        int _gravityBias = gravityBias * 6;

        int gravityLim = 0;
        if(pos_polarity && _gravityBias>0)
            gravityLim += _gravityBias;
        else if(neg_polarity && _gravityBias<0)
            gravityLim -= _gravityBias;     // double minus cancel
        else if (!neg_polarity && !pos_polarity)
            gravityLim += abs(_gravityBias);    // if we are holding then we still need to add gravityBias (just doenst matter the direction)


        int limp_limit = (currentLimit + gravityLim) * 1.4;        // our limp limit
        int duty_limit = (currentLimit + gravityLim) * 3;        // servo commanded limit
        bool limit_exceeded = current.current().value() > limp_limit;
        //int amps_velocity = -current.current().velocity();

#if 0
        if(name == "J14")
            printf("%s  dx:%d bias:%d |%c| Gb%d => Cb%d | %d+%d:%d (%4.1f)\n",
                    name.c_str(),
                    dx, old_currentBias, pos_polarity ? '+' : neg_polarity ? '-' : '*', gravityBias, currentBias,
                    current.current().value(), currentBias, currentLimit, (double)unbiasedCurrent*100/currentLimit
            );
#elif 0
        if(name == "J14*" || name == "J13")
            printf("%s  p:%d dx:%d mA:%d | bias: [%c] B%d G%d | lim: L%d+G%d=>%d duty:%d %s\n",
                   name.c_str(),
                   /*    p: */  position.current().value(), dx, current.current().value(),
                   /* bias: */  pos_polarity ? '+' : neg_polarity ? '-' : '*', currentBias, _gravityBias,
                   /*  lim: */  currentLimit, gravityLim, limp_limit, duty_limit, limit_exceeded ? "limit" : ""
            );
#endif

        unsigned long stateTime = millis() - stateChanged;
        bool idle = position.current().velocity()==0 && !limit_exceeded;

        switch(state) {
            case Holding:
                // check if we are over current limit
                // todo: switch this to asymmetric MMD command using #[id]MMD[dutyPos],[dutyNeg]\r
                mmd.target(clamp(duty_limit, 500, 1200));
                CPR(3);
                if(limit_exceeded) {
                    if(pos_polarity)
                        transitionTo(PositiveCompliance);
                    else if (neg_polarity)
                        transitionTo(NegativeCompliance);
                }
                break;

            case Moving:
                mmd.target(1023);
                CPR(3);
                //if(delta.spread()<2 && delta.average()==0 && !limit)
                if(idle)
                        transitionTo(Holding);
                break;


            // seems wierd to split the state, then switch on the same code but we still use discrete polarity state to prevent oscillation
            case NegativeCompliance:
            case PositiveCompliance:
                CPR(1);
                mmd.target(255);

                fudge = clamp(current.current().value() / 23, 0, 50);
                w = (state == NegativeCompliance)
                    ? -fudge.value()
                    : fudge.value();

                // just keep adding to target
                //if (current.current().acceleration() < -25) {
                //if (current.current().velocity() > 1000) {
                //if (fudge.velocity() > 1000) {
                if(fudge > 15 && stateTime > 60) {//} || ((position.current().acceleration() < -50) && stateTime > 200)) {
                    CPR(1); // reduce filter since we cant update FIR in limp mode
                    transitionTo(ComplianceLimp);
                } else {
                    // half way between target and current (set fudge divider to 12 and limp threshold to 30)
                    //moveTo( 0.5* ((position.target() + w) + position.current()), false); // todo: try adding measure velocity here

                    //moveTo(position.target() + w + position.current().velocity()/5, false); // todo: try adding measure velocity here
                    moveTo(position.target() + w, false); // use fudge divider of 20, limp threshold of 15
                }

                if(idle && stateTime>40)
                //if(position.current().acceleration() < -10)
                    transitionTo(Holding);

                break;

            case ComplianceLimp:
                // keep target tracking current position for when we exit compliance
                fudge.clear();
                current.current().clear();
                moveTo( position.current(), false);

                // if our average is low, and the spread of numbers is narrow, then human is holding us in stable position, retake control
                if(idle && stateTime > 200)
                    transitionTo(Holding); // was go to compliance, but what polarity?
                break;
        }

    }
}

