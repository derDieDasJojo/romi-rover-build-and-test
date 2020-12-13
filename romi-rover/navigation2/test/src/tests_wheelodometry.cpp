#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WheelOdometry.h"

using namespace std;
using namespace testing;
using namespace romi;

class wheelodometry_tests : public ::testing::Test
{
protected:
        JSON config;
        
	wheelodometry_tests() {
                const char * config_string = "{"
                        "'wheel_diameter': 1.0,"
                        "'wheel_base': 1.0,"
                        "'encoder_steps': 1000.0,"
                        "'maximum_speed': 3.0 }";
                config = JSON::parse(config_string);
	}

	~wheelodometry_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(wheelodometry_tests, test_initialisation)
{
        RoverConfiguration rover(config);

        WheelOdometry odometry(rover, 1000.0, 1000.0, 0.0);
        double x, y, orientation;
        
        //Assert
        odometry.get_location(x, y);
        orientation = odometry.get_orientation();
                
        ASSERT_EQ(x, 0.0);
        ASSERT_EQ(y, 0.0);
        ASSERT_EQ(orientation, 0.0);
}

TEST_F(wheelodometry_tests, test_update_encoders_displacement)
{
        RoverConfiguration rover(config);

        WheelOdometry odometry(rover, 1000.0, 1000.0, 0.0);
        double x, y, orientation;

        // 1000 steps = 1 revolution = 3.14 meters
        odometry.set_encoders(2000.0, 2000.0, 1.0);
        
        //Assert
        odometry.get_location(x, y);
        orientation = odometry.get_orientation();
                
        ASSERT_EQ(x, M_PI);
        ASSERT_EQ(y, 0.0);
        ASSERT_EQ(orientation, 0.0);
}

TEST_F(wheelodometry_tests, test_update_encoders_orientation_360degrees)
{
        RoverConfiguration rover(config);

        WheelOdometry odometry(rover, 0.0, 0.0, 0.0);
        double x, y, orientation;

        // left: 1000 steps = 1 revolution = 3.14 meters
        // right: 1000 steps = 1 revolution = -3.14 meters
        // -> full rotation in the same spot (wheel base = 1m)
        odometry.set_encoders(1000.0, -1000.0, 1.0);
        
        //Assert
        odometry.get_location(x, y);
        orientation = odometry.get_orientation();
                
        ASSERT_EQ(x, 0);
        ASSERT_EQ(y, 0.0);
        ASSERT_EQ(orientation, -2.0 * M_PI);
}

TEST_F(wheelodometry_tests, test_update_encoders_orientation_90degrees)
{
        RoverConfiguration rover(config);

        WheelOdometry odometry(rover, 0.0, 0.0, 0.0);
        double x, y, orientation;

        // 90° rotation
        odometry.set_encoders(-250.0, 250.0, 1.0);
        
        //Assert
        odometry.get_location(x, y);
        orientation = odometry.get_orientation();
                
        ASSERT_EQ(x, 0);
        ASSERT_EQ(y, 0.0);
        ASSERT_EQ(orientation, M_PI / 2.0);
}

TEST_F(wheelodometry_tests, test_update_encoders_move_and_turn_90degrees)
{
        RoverConfiguration rover(config);

        WheelOdometry odometry(rover, 0.0, 0.0, 0.0);
        double x, y, orientation;

        odometry.set_encoders(1000.0, 1000.0, 1.0);
        odometry.set_encoders(1000.0 - 250.0, 1000.0 + 250.0, 1.0);
        
        //Assert
        odometry.get_location(x, y);
        orientation = odometry.get_orientation();
                
        ASSERT_EQ(x, M_PI);
        ASSERT_EQ(y, 0.0);
        ASSERT_EQ(orientation, M_PI / 2.0);
}
