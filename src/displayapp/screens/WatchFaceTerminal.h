#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "utility/DirtyValue.h"
#include "components/ble/weather/WeatherData.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class MotionController;
    class WeatherService;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceTerminal : public Screen {
      public:
        WatchFaceTerminal(Controllers::DateTime& dateTimeController,
                          const Controllers::Battery& batteryController,
                          const Controllers::Ble& bleController,
                          Controllers::NotificationManager& notificationManager,
                          Controllers::Settings& settingsController,
                          Controllers::MotionController& motionController,
                          Controllers::WeatherService& weatherService);
        ~WatchFaceTerminal() override;

        void Refresh() override;

      private:
        Utility::DirtyValue<int> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<size_t> notificationCount {};
        using days = std::chrono::duration<int32_t, std::ratio<86400>>; // TODO: days is standard in c++20
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, days>> currentDate;
        Utility::DirtyValue<int16_t> nowTemp {};

        lv_obj_t* label_time;
        lv_obj_t* label_date;
        lv_obj_t* label_prompt_1;
        lv_obj_t* batteryValue;
        lv_obj_t* stepValue;
        lv_obj_t* notificationPrefix;
        lv_obj_t* notificationIcon;
        lv_obj_t* connectState;
        lv_obj_t* weatherState;

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::MotionController& motionController;
        Controllers::WeatherService& weatherService;

        lv_task_t* taskRefresh;
      };
    }
  }
}
