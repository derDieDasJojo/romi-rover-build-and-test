/*
  romi-rover

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include <exception>
#include <stdexcept>
#include <memory>
#include <atomic>
#include <syslog.h>
#include <thread>

#include <rcom/Linux.h>

#include <util/Clock.h>
#include <util/ClockAccessor.h>
#include <rover/RoverOptions.h>
#include <rpc/RemoteCamera.h>
#include <rpc/RcomClient.h>
#include <oquam/StepperSettings.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <session/Session.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>
#include <ui/ScriptList.h>
#include <rpc/ScriptHub.h>
#include <rpc/ScriptHubListener.h>
#include <rover/EventsAndStates.h>
#include <rover/RoverScriptEngine.h>
#include <ui/RemoteStateInputDevice.h>
#include <rover/RoverStateMachine.h>
#include <rover/RoverInterface.h>

#include "mock_inputdevice.h"
#include "mock_remotestateinputdevice.h"
#include "mock_display.h"
#include "mock_speedcontroller.h"
#include "mock_navigation.h"
#include "mock_eventtimer.h"
#include "mock_menu.h"
//#include "mock_scriptengine.h"
#include "mock_notifications.h"
#include "mock_weeder.h"
#include "mock_imager.h"


std::atomic<bool> quit(false);

void SignalHandler(int signal)
{
        if (signal == SIGSEGV){
                syslog(1, "rcom-registry segmentation fault");
                exit(signal);
        }
        else if (signal == SIGINT){
                r_info("Ctrl-C Quitting Application");
                perror("init_signal_handler");
                quit = true;
        }
        else{
                r_err("Unknown signam received %d", signal);
        }
}

int main(int argc, char** argv)
{
        std::shared_ptr<romi::IClock> clock = std::make_shared<romi::Clock>();
        romi::ClockAccessor::SetInstance(clock);

        int retval = 1;

        romi::RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();

        log_init();
        r_log_set_app("romi-rover");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);

        try {

                rcom::Linux linux;

                // Session
                r_info("main: Creating session");
                romi::RomiDeviceData romiDeviceData;
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationProvider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory =".";

                romi::Session session(linux, session_directory, romiDeviceData,
                                      softwareVersion, std::move(locationProvider));


                MockInputDevice mockInputDevice;
                EXPECT_CALL(mockInputDevice, get_next_event()).WillRepeatedly(testing::Return(0));

                MockDisplay mockDisplay;
                MockSpeedController mockSpeedController;
                MockNavigation mockNavigation;
                EXPECT_CALL(mockNavigation, move(testing::_,testing::_)).WillRepeatedly(testing::Return(true));
                EXPECT_CALL(mockNavigation, reset_activity()).WillRepeatedly(testing::Return(true));

                MockEventTimer mockEventTimer;
                EXPECT_CALL(mockEventTimer, get_next_event()).WillRepeatedly(testing::Return(0));
                MockMenu mockMenu;
    //            MockScriptEngine mockScriptEngine;
                MockNotifications mockNotifications;
                MockWeeder mockWeeder;
                EXPECT_CALL(mockWeeder, reset_activity()).WillRepeatedly(testing::Return(true));
                EXPECT_CALL(mockWeeder, power_up()).WillRepeatedly(testing::Return(true));
                MockImager mockImager;

                romi::RemoteStateInputDevice remoteStateInputDevice;
             //   MockRemoteStateInputDevice mockRemoteStateInputDevice;

                const std::string test_script_filename = "./test_data/scripts.json";
                romi::ScriptList scripts(test_script_filename);
                romi::RoverScriptEngine script_engine(scripts, romi::event_script_finished,
                                                  romi::event_script_error);

                romi::Rover rover(mockInputDevice, mockDisplay, mockSpeedController, mockNavigation, mockEventTimer,
                                  mockMenu, script_engine, mockNotifications, mockWeeder, mockImager, remoteStateInputDevice);

                auto scriptHubListener = std::make_shared<ScriptHubListener>(rover);
                ScriptHub scriptHub(scriptHubListener, ScriptHubListeningPort);
            // State machine
                r_info("main: Creating state machine");
                romi::RoverStateMachine state_machine(rover);

                // User interface
                r_info("main: Creating user interface");
                romi::RoverInterface user_interface(rover, state_machine);

                while (!quit) {
                        
                        try {
                            using namespace std::chrono_literals;
                            scriptHub.handle_events();
                            user_interface.handle_events();

                            std::this_thread::sleep_for(5ms);
                        } catch (std::exception& e) {
                            quit = true;
                                throw;
                        }
                }

                retval = 0;

                
        } catch (nlohmann::json::exception& je) {
                r_err("main: jsonobj Error: %s", je.what());
                
        } catch (std::exception& e) {
                r_err("main: exception: %s", e.what());
        }
        
        return retval;
}
