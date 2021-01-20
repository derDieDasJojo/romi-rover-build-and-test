#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "JsonCpp.h"
#include "DefaultNavigation.h"
#include "../mock/mock_motordriver.h"

using namespace std;
using namespace testing;
using namespace romi;

class defaultnavigation_tests : public ::testing::Test
{
protected:
        JsonCpp config;
        MockMotorDriver driver;
        
	defaultnavigation_tests() {
                const char * config_string = "{"
                        "'wheel_diameter': 1.0,"
                        "'wheel_base': 1.0,"
                        "'encoder_steps': 1000.0,"
                        "'maximum_speed': 3.0 }";
                config = JsonCpp::parse(config_string);
	}

	~defaultnavigation_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(defaultnavigation_tests, test_moveat)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);

        EXPECT_CALL(driver, moveat(0.1, 0.2))
                .WillOnce(Return(true));

        navigation.moveat(0.1, 0.2);
}

TEST_F(defaultnavigation_tests, move_fails_with_invalid_speed_1)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);

        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true));

        bool success = navigation.move(1.0, 2.0);
        ASSERT_EQ(success, false);
}

TEST_F(defaultnavigation_tests, move_fails_with_invalid_speed_2)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);

        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true));

        bool success = navigation.move(1.0, -2.0);
        ASSERT_EQ(success, false);
}

TEST_F(defaultnavigation_tests, move_fails_with_invalid_distance_1)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);

        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true));

        bool success = navigation.move(51.0, 1.0);
        ASSERT_EQ(success, false);
}

TEST_F(defaultnavigation_tests, move_fails_with_invalid_distance_2)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);

        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true));

        bool success = navigation.move(-51.0, 1.0);
        ASSERT_EQ(success, false);
}

TEST_F(defaultnavigation_tests, successful_move)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);

        {
                InSequence seq;
                
                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(0.0),
                                        SetArgReferee<1>(0.0),
                                        SetArgReferee<2>(0.0),
                                        Return(true)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, moveat(1.0, 1.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(3.0 * 1000.0 / M_PI),
                                        SetArgReferee<1>(3.0 * 1000.0 / M_PI),
                                        SetArgReferee<2>(1.0),
                                        Return(true)))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, moveat(0.0, 0.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

        }

        // Move 3 meter, at the maximum speed of 3 m/s.
        bool success = navigation.move(3.0, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(defaultnavigation_tests, successfully_move_distance_with_negative_speed)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);

        {
                InSequence seq;
                
                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(0.0),
                                        SetArgReferee<1>(0.0),
                                        SetArgReferee<2>(0.0),
                                        Return(true)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, moveat(-1.0, -1.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(-3.0 * 1000.0 / M_PI),
                                        SetArgReferee<1>(-3.0 * 1000.0 / M_PI),
                                        SetArgReferee<2>(1.0),
                                        Return(true)))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, moveat(0.0, 0.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

        }

        // Move 3 meter, at the maximum speed of 3 m/s.
        bool success = navigation.move(3.0, -1.0);
        ASSERT_EQ(success, true);
}

TEST_F(defaultnavigation_tests, successfully_move_negative_distance_with_positive_speed)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);

        {
                InSequence seq;
                
                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(0.0),
                                        SetArgReferee<1>(0.0),
                                        SetArgReferee<2>(0.0),
                                        Return(true)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, moveat(-1.0, -1.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(-3.0 * 1000.0 / M_PI),
                                        SetArgReferee<1>(-3.0 * 1000.0 / M_PI),
                                        SetArgReferee<2>(1.0),
                                        Return(true)))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, moveat(0.0, 0.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

        }

        // Move 3 meter, at the maximum speed of 3 m/s.
        bool success = navigation.move(-3.0, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(defaultnavigation_tests, successfully_move_negative_distance_with_negative_speed)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);

        {
                InSequence seq;
                
                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(0.0),
                                        SetArgReferee<1>(0.0),
                                        SetArgReferee<2>(0.0),
                                        Return(true)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, moveat(-1.0, -1.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(-3.0 * 1000.0 / M_PI),
                                        SetArgReferee<1>(-3.0 * 1000.0 / M_PI),
                                        SetArgReferee<2>(1.0),
                                        Return(true)))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, moveat(0.0, 0.0))
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

        }

        // Move 3 meter, at the maximum speed of 3 m/s.
        bool success = navigation.move(-3.0, -1.0);
        ASSERT_EQ(success, true);
}

TEST_F(defaultnavigation_tests, move_fails_on_zero_speed)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        bool success = navigation.move(3.0, 0.0);
        ASSERT_EQ(success, false);
}

TEST_F(defaultnavigation_tests, move_returns_true_on_zero_distance)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        bool success = navigation.move(0.0, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(defaultnavigation_tests, move_fails_on_bad_speed_1)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        bool success = navigation.move(3.0, 2.0);
        ASSERT_EQ(success, false);
}

TEST_F(defaultnavigation_tests, move_fails_on_bad_speed_2)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        bool success = navigation.move(3.0, -1.1);
        ASSERT_EQ(success, false);
}

TEST_F(defaultnavigation_tests, move_fails_on_bad_speed_3)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        bool success = navigation.move(3.0, 0);
        ASSERT_EQ(success, false);
}

TEST_F(defaultnavigation_tests, move_returns_false_on_failing_stop)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(false))
                .RetiresOnSaturation();
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(false))
                .RetiresOnSaturation();
        
        bool success = navigation.move(3.0, 0.5);
        ASSERT_EQ(success, false);
}

TEST_F(defaultnavigation_tests, move_returns_false_on_failing_get_encoders)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
                
        EXPECT_CALL(driver, get_encoder_values(_,_,_))
                .WillOnce(Return(false))
                .RetiresOnSaturation();
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        
        bool success = navigation.move(3.0, 0.5);
        ASSERT_EQ(success, false);
}

TEST_F(defaultnavigation_tests, move_returns_false_when_moveat_fails)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);

        {
                InSequence seq;
                
                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, get_encoder_values(_,_,_))
                        .WillOnce(DoAll(SetArgReferee<0>(0.0),
                                        SetArgReferee<1>(0.0),
                                        SetArgReferee<2>(0.0),
                                        Return(true)))
                        .RetiresOnSaturation();
                
                EXPECT_CALL(driver, moveat(1.0, 1.0))
                        .WillOnce(Return(false))
                        .RetiresOnSaturation();

                EXPECT_CALL(driver, stop())
                        .WillOnce(Return(true))
                        .RetiresOnSaturation();

        }

        // Move 3 meter, at the maximum speed of 3 m/s.
        bool success = navigation.move(3.0, 1.0);
        ASSERT_EQ(success, false);
}

TEST_F(defaultnavigation_tests, stop_succeeds)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(true))
                .RetiresOnSaturation();

        bool success = navigation.stop();
        ASSERT_EQ(success, true);
}

TEST_F(defaultnavigation_tests, stop_fails_when_stop_driver_fails)
{
        NavigationSettings rover(config);
        DefaultNavigation navigation(driver, rover);
        
        EXPECT_CALL(driver, stop())
                .WillOnce(Return(false))
                .RetiresOnSaturation();

        bool success = navigation.stop();
        ASSERT_EQ(success, false);
}
