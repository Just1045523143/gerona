#ifndef ROBOTCONTROLLERACKERMANNPID_H
#define ROBOTCONTROLLERACKERMANNPID_H

/// ROS
#include <geometry_msgs/Twist.h>

/// THIRD PARTY
#include <Eigen/Core>

/// PROJECT
#include <path_follower/controller/robotcontroller.h>
#include <path_follower/utils/pidcontroller.hpp>
#include <path_follower/utils/visualizer.h>
#include <path_follower/utils/parameters.h>

class Behaviour;

class RobotController_Legacy_Ackermann_Pid : public RobotController
{
public:
    RobotController_Legacy_Ackermann_Pid(PathFollower *path_driver);

    virtual void stopMotion();

    virtual void reset();
    virtual void start();

    virtual void initOnLine();
    virtual void initApproachTurningPoint();
    virtual void behaveOnLine();
    virtual bool behaveApproachTurningPoint();

protected:
    virtual MoveCommandStatus computeMoveCommand(MoveCommand* cmd);
    virtual void publishMoveCommand(const MoveCommand &cmd) const;

    void switchBehaviour(Behaviour* next_behaviour);

private:
    struct Command
    {
        float velocity;
        float steer_front;
        float steer_back; // currently unused

        Command()
        {
            // initialize to zero
            velocity = steer_front = steer_back = 0.0;
        }

        /* ramaxx_msg commented for the moment, as it would limit the use of this node to the rabots.
         * Reimplement this, if you want to use a robot with front and back steerting.
         */
        /*
        operator ramaxx_msgs::RamaxxMsg()
        {
            ramaxx_msgs::RamaxxMsg msg;
            msg.data.resize(3);
            msg.data[0].key = ramaxx_msgs::RamaxxMsg::CMD_STEER_FRONT_DEG;
            msg.data[1].key = ramaxx_msgs::RamaxxMsg::CMD_STEER_REAR_DEG;
            msg.data[2].key = ramaxx_msgs::RamaxxMsg::CMD_SPEED;
            msg.data[0].value = steer_front * 180.0/M_PI;
            msg.data[1].value = steer_back * 180.0/M_PI;
            msg.data[2].value = v;
            return msg;
        }
        */

        operator MoveCommand()
        {
            MoveCommand mcmd;
            mcmd.setDirection(steer_front);
            mcmd.setVelocity(velocity);
            return mcmd;
        }

        bool isValid()
        {
            if ( isnan(velocity) || isinf(velocity)
                 || isnan(steer_front) || isinf(steer_front)
                 || isnan(steer_back) || isinf(steer_back) )
            {
                // fix this instantly, to avoid further problems.
                ROS_FATAL("Non-numerical values in command: %d,%d,%d,%d,%d,%d",
                          isnan(velocity), isinf(velocity),
                          isnan(steer_front), isinf(steer_front),
                          isnan(steer_back), isinf(steer_back));
                velocity = 0.0;
                steer_front = 0.0;
                steer_back = 0.0;

                return false;
            } else {
                return true;
            }
        }
    };

    struct ControllerParameters : public Parameters
    {
        P<double> dead_time;
        P<double> l;
        P<float> pid_ta;
        P<float> pid_kp;
        P<float> pid_ki;
        P<float> pid_kd;
        //P<double> pid_i_max;

        ControllerParameters():
            dead_time(this, "~dead_time", 0.1, ""),
            l(this, "~l", 0.38, "Not sure... distance between front and rear wheels?"),
            pid_ta(this, "~pid/ta", 0.03, "Update interval of the PID controller."),
            pid_kp(this, "~pid/kp", 1.0, "Proportional coefficient of the PID controller."),
            pid_ki(this, "~pid/ki", 0.001, "Integral coefficient of the PID controller."),
            pid_kd(this, "~pid/kd", 0, "Derivative coefficient of the PID controller.")
            //pid_i_max(this, "~pid/i_max", 0.0, "Limit to the integral part of the PID controller.") //TODO: not used
        {}
    } opt_;

    Behaviour* active_behaviour_;

    PidController<1> pid_;
    Command cmd_;
    Visualizer *visualizer_;

    //! Step counter for behaviour ApproachTurningPoint.
    int atp_step_;

    float filtered_speed_;

    void configure();

    bool setCommand(float error, float speed);

    void predictPose(Eigen::Vector2d &front_pred, Eigen::Vector2d &rear_pred);
    double calculateCourse();
    double calculateLineError();
    double calculateDistanceError();

    /**
     * @brief Visualize a carrot.
     *
     * This method is located here and not in the Visualizer class, as it needs access to the node class for
     * a tf transform (Visualizer should be independent of such things).
     *
     * @param carrot
     * @param id    ID of the marker.
     * @param r     Marker color, red.
     * @param g     Marker color, green.
     * @param b     Marker color, blue.
     */
    void visualizeCarrot(const Eigen::Vector2d &carrot, int id, float r, float g, float b);
};

#endif // ROBOTCONTROLLERACKERMANNPID_H
