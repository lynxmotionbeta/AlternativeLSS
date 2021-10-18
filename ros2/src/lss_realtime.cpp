//
// Created by guru on 3/2/21.
//

#include "lss_hardware/lss_realtime.hpp"

#include "rclcpp/executors/single_threaded_executor.hpp"

#include <thread>


namespace lynxmotion {

static constexpr const char* FREQUENCY_PARAMETER = "frequency";
static constexpr const char* DIAGNOSTICS_PERIOD_PARAMETER = "diagnostic_period";


LynxmotionRealtime::SharedPtr default_realtime_ = nullptr;

rclcpp::executors::SingleThreadedExecutor::SharedPtr lss_rt_executor;
std::thread lss_rt_thread;


void free_default_realtime() {
    if(lss_rt_executor) {
        // stop realtime thread
        lss_rt_thread.join();
    }
    default_realtime_.reset();
}

void lss_rt_executor_add(rclcpp::Node::SharedPtr node) {
    if(!lss_rt_executor) {
        lss_rt_executor = rclcpp::executors::SingleThreadedExecutor::make_shared();
        lss_rt_thread = std::thread([] () {
            lss_rt_executor->spin();
        });
    }
    lss_rt_executor->add_node(node);
}

LynxmotionRealtime::SharedPtr LynxmotionRealtime::get_default() {
    if(!default_realtime_) {
        default_realtime_ = LynxmotionRealtime::make_shared();
        lss_rt_executor_add(default_realtime_);
        atexit(free_default_realtime);  // ensure the realtime node is freed on program exit to prevent leak
    }
    return default_realtime_;
}

LynxmotionRealtime::LynxmotionRealtime(const std::string& node_name, const rclcpp::NodeOptions& options)
    : Node(node_name, options), frequency(10), connection_rate_(4.0)
{
    RCLCPP_INFO(get_logger(), "LSS realtime bus node initialized");

    // declare parameters
    declare_parameter(FREQUENCY_PARAMETER, rclcpp::ParameterValue(10.0f));
    declare_parameter(DIAGNOSTICS_PERIOD_PARAMETER, rclcpp::ParameterValue((rcl_duration_value_t)1));

    // a method to calibrate the servos
    calibration_subscription_ = this->create_subscription<humanoid_model_msgs::msg::JointCalibration>(
            "joint_calibration", 10,
            std::bind(&LynxmotionRealtime::calibration_callback, this, std::placeholders::_1));

    // create update timer for communicating with servos
    rclcpp::Parameter frequency = get_parameter(FREQUENCY_PARAMETER);
    update_timer_ = create_wall_timer(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::duration<double>(1.0 / frequency.get_value<float>())),
            std::bind(&LynxmotionRealtime::update, this));

    rclcpp::Parameter diagnostic_period = get_parameter(DIAGNOSTICS_PERIOD_PARAMETER);
    if(diagnostic_period.get_value<rcl_duration_value_t>() > 0.0) {
        _diagnostics_pub = create_publisher<diagnostic_msgs::msg::DiagnosticArray>(
                node_name+"/diagnostics",
                get_diagnostics_qos()
                );
        diag_timer_ = create_wall_timer(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::duration<rcl_duration_value_t>(
                                diagnostic_period.get_value<rcl_duration_value_t>())),
                std::bind(&LynxmotionRealtime::publish_diagnostics, this));
    }
}

bool LynxmotionRealtime::add_bus(RealtimeBus& bus) {
    std::unique_lock<std::mutex> guard(bus_mutex, std::defer_lock);
    while (!guard.try_lock()) {
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }

    bus_.emplace_back(&bus);
    diags_.emplace_back(Diagnostics::make_shared());
    _diag_msg.reset();
    return true;
}

void LynxmotionRealtime::remove_bus(RealtimeBus& bus) {
    std::unique_lock<std::mutex> guard(bus_mutex, std::defer_lock);
    while (!guard.try_lock()) {
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }

    // find and remove iop from list
    auto p_itr = std::find(bus_.begin(), bus_.end(), &bus);
    if(p_itr != bus_.end()) {
        int pos = p_itr - bus_.end();
        // todo: we cant release this iop until we are sure the processing thread is done with it
        bus_.erase(p_itr);
        diags_.erase(diags_.begin() + pos);
        _diag_msg.reset();
    }
}


void LynxmotionRealtime::update()
{
    std::unique_lock<std::mutex> guard(bus_mutex, std::try_to_lock);
    if(!guard.owns_lock())
        return; // skip this iteration

    RealtimeArgs args;

    // perform a read of state
    size_t n = 0;
    for(auto& iop: bus_) {
        try {
            Diagnostics::SharedPtr diag = diags_[n];
            args.diagnostic = [&iop, diag](const Diagnostic& d) { *diag += d; };

            iop->read_state(args);
            n++;
        } catch (std::exception &e) {
            RCLCPP_ERROR(get_logger(), "exception occurred in IOP get state: %s: %s", iop->hw_name.c_str(),  e.what());
        }
    }

    // perform writes of command values
#if 0
    n = 0;
    for(auto& iop: bus_) {
        try {
            iop->write_command(args);

            if(n < diag_msg_count) {
                //diagnostic_msgs::msg::DiagnosticStatus &diag = diag_msg->status[n];
                //diag.values[0].value = 0;
            }

            n++;
        } catch (std::exception &e) {
            RCLCPP_ERROR(get_logger(), "exception occured in IOP write command: %s: %s", iop->hw_name.c_str(),  e.what());
        }
    }
#endif
}

void LynxmotionRealtime::calibration_callback(humanoid_model_msgs::msg::JointCalibration::SharedPtr msg)
{
    for(auto& b: bus_) {
        RCLCPP_INFO(get_logger(),
            "calling calibrate on bus %s", b->hw_name.c_str());
        b->calibrate(msg);
    }
}

diagnostic_msgs::msg::KeyValue diagnostic(const std::string& k, double v) {
    diagnostic_msgs::msg::KeyValue kv;
    kv.key = k;
    kv.value = v;
    return kv;
}

void LynxmotionRealtime::configure_diagnostics() {
    _diag_msg = std::make_shared<diagnostic_msgs::msg::DiagnosticArray>();

    // ensure diags_ equals bus_
    if(diags_.size() != bus_.size()) {
        // this shouldnt happen since we adjust in the add/remove bus methods
        RCLCPP_WARN(get_logger(),
                    "internal: mismatch in diagnostics between cached diagnostics message and aggregation filters. "
                    "this will be corrected.");
        diags_.resize(bus_.size());
    }

    diagnostic_msgs::msg::DiagnosticStatus status;
    status.level = diagnostic_msgs::msg::DiagnosticStatus::OK;
    status.name = "lss_realtime";

    // todo: loop through all buses here
    for(auto& b: bus_) {
        status.hardware_id = b->hw_name;

        status.message = "performance";
        status.values = {
                diagnostic("complete", 0),
                diagnostic("timeouts", 0),
                diagnostic("health", 0),
                diagnostic("ttf-max", 0),
                diagnostic("ttf-avg", 0),
                diagnostic("ttc-max", 0),
                diagnostic("ttc-avg", 0)
        };

        _diag_msg->status.emplace_back(status);
    }
}

void LynxmotionRealtime::publish_diagnostics() try {
    if(!_diag_msg || diags_.size() != _diag_msg->status.size())
        configure_diagnostics();

    for(size_t i = 0, _i = _diag_msg->status.size(); i < _i; i++) {
        auto& msg = _diag_msg->status[i];
        const auto& diag = diags_[i].get();
        assert(msg.values.size() == 7);

        double ttf_min, ttf_max, ttc_min, ttc_max;
        diag->time_to_first_packet.minmax(ttf_min, ttf_max);
        diag->time_to_complete.minmax(ttc_min, ttc_max);
        msg.values[0].value = std::to_string(diag->complete);
        msg.values[1].value = std::to_string(diag->timeouts);
        msg.values[2].value = (diag->complete > 0)
                ? (std::to_string(diag->complete * 100.0 /
                        (diag->complete + diag->timeouts)) + '%')
                : "100%";
        msg.values[3].value = std::to_string(ttf_max);
        msg.values[4].value = std::to_string(diag->time_to_first_packet.average());
        msg.values[5].value = std::to_string(ttc_max);
        msg.values[6].value = std::to_string(diag->time_to_complete.average());
    }

    _diag_msg->header.stamp = now();
    _diagnostics_pub->publish(*_diag_msg);

} catch (std::exception & e) {
    RCLCPP_ERROR_ONCE(get_logger(), "Failed to publish diagnostic data: %s", e.what());
}

} // ns:lynxmotion
