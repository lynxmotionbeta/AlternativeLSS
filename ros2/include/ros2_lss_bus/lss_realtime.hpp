//
// Created by guru on 3/2/21.
//

#ifndef LSS_HARDWARE_LSS_REALTIME_H
#define LSS_HARDWARE_LSS_REALTIME_H

#pragma once


#include <rclcpp/rclcpp.hpp>
#include <rclcpp/rate.hpp>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>

//#include "humanoid_model_msgs/msg/compliant_joint_params.hpp"
#include <humanoid_model_msgs/msg/joint_calibration.hpp>

#include <memory>
#include <string>

#include "raf.h"

namespace lynxmotion {
    class Diagnostic {
    public:
        double time_to_first_packet;
        double time_to_complete;
        int complete;
        int timeouts;

        inline Diagnostic()
                : time_to_first_packet(NAN), time_to_complete(NAN), complete(0), timeouts(0) {}
        inline Diagnostic(double ttf, double ttc, int _complete, int _overruns)
                : time_to_first_packet(ttf), time_to_complete(ttc), complete(_complete), timeouts(_overruns) {}
    };

    class RealtimeInfo {
    public:
    };

    class RealtimeArgs {
    public:
        std::function<void(const Diagnostic&)> diagnostic;
    };

    class RealtimeBus {
    public:
        std::string hw_name;

        virtual unsigned long read_state(RealtimeArgs& args)=0;
        virtual unsigned long write_command(RealtimeArgs& args)=0;

        virtual void calibrate(humanoid_model_msgs::msg::JointCalibration::SharedPtr msg)=0;
    };

    class LynxmotionRealtime : public rclcpp::Node {
    public:
        RCLCPP_SHARED_PTR_DEFINITIONS(LynxmotionRealtime)

        class Diagnostics {
        public:
            RCLCPP_SHARED_PTR_DEFINITIONS(Diagnostics)

            RollingAverage<double> time_to_first_packet;
            RollingAverage<double> time_to_complete;
            int complete;
            int timeouts;

            inline Diagnostics()
                    : time_to_first_packet(10), time_to_complete(10), complete(0), timeouts(0) {}
#if 0
            Diagnostics operator + (const Diagnostics& rhs) const {
                Diagnostics out;
                out.time_to_first_packet =  time_to_first_packet + rhs.time_to_first_packet;
                out.time_to_last_packet =  time_to_last_packet + rhs.time_to_last_packet;
                out.overuns =  overuns + rhs.overuns;
                return out;
            }
            Diagnostics operator / (const Diagnostics& rhs) const {
                Diagnostics out;
                out.time_to_first_packet = time_to_first_packet / rhs.time_to_first_packet;
                out.time_to_last_packet = time_to_last_packet / rhs.time_to_last_packet;
                out.overuns = overuns / rhs.overuns;
                return out;
            }
            Diagnostics operator / (double n) const {
                Diagnostics out;
                out.time_to_first_packet = time_to_first_packet / n;
                out.time_to_last_packet = time_to_last_packet / n;
                out.overuns = overuns / n;
                return out;
            }
            Diagnostics operator * (const Diagnostics& rhs) const {
                Diagnostics out;
                out.time_to_first_packet = time_to_first_packet / rhs.time_to_first_packet;
                out.time_to_last_packet = time_to_last_packet / rhs.time_to_last_packet;
                out.overuns = overuns / rhs.overuns;
                return out;
            }
            Diagnostics operator * (double n) const {
                Diagnostics out;
                out.time_to_first_packet = time_to_first_packet * n;
                out.time_to_last_packet = time_to_last_packet * n;
                out.overuns = overuns * n;
                return out;
            }
#endif
            Diagnostics& operator+=(const Diagnostic& d) {
                time_to_first_packet.add(d.time_to_first_packet);
                time_to_complete.add(d.time_to_complete);
                complete += d.complete;
                timeouts += d.timeouts;
                return *this;
            }
        };

        LynxmotionRealtime(const std::string& node_name, const rclcpp::NodeOptions& options);
        inline LynxmotionRealtime() : LynxmotionRealtime("lss_realtime", rclcpp::NodeOptions()) {}

        static LynxmotionRealtime::SharedPtr get_default();

        bool add_bus(RealtimeBus& bus);
        void remove_bus(RealtimeBus& bus);

    // parameters
    protected:
        double frequency;      // how fast we will query/command the servos

        std::mutex bus_mutex;
        std::vector<RealtimeBus*> bus_;
        std::vector<Diagnostics::SharedPtr> diags_;

        // called when its time to communicate with the servos
        void update();


    // Ros2 pub/sub related
    protected:
        rclcpp::TimerBase::SharedPtr update_timer_, diag_timer_;
        rclcpp::GenericRate<std::chrono::system_clock> connection_rate_;
        Diagnostics diag_;

        //rclcpp::Subscription<humanoid_model_msgs::msg::CompliantJointParams>::SharedPtr compliance_params_subscription_;
        //void compliance_params_callback(humanoid_model_msgs::msg::CompliantJointParams::SharedPtr msg) const;

        rclcpp::Subscription<humanoid_model_msgs::msg::JointCalibration>::SharedPtr calibration_subscription_;
        void calibration_callback(humanoid_model_msgs::msg::JointCalibration::SharedPtr msg);

        void configure_diagnostics();
        void publish_diagnostics();
        rclcpp::Publisher<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr _diagnostics_pub;
        diagnostic_msgs::msg::DiagnosticArray::SharedPtr _diag_msg;

        static rclcpp::QoS get_diagnostics_qos() {
            rmw_qos_profile_t diag_qos_profile = rmw_qos_profile_sensor_data;
            diag_qos_profile.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
            diag_qos_profile.depth = 5;
            diag_qos_profile.reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;
            diag_qos_profile.durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;
            return rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(diag_qos_profile));
        }
    };

} // ns:lynxmotion

#endif //LSS_HARDWARE_LSS_REALTIME_H
