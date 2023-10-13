#include <lvgl/lvgl.h>
#include "displayapp/screens/WatchFaceTerminal.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include <displayapp/Colors.h>

using namespace Pinetime::Applications::Screens;

WatchFaceTerminal::WatchFaceTerminal(Controllers::DateTime& dateTimeController,
                                     const Controllers::Battery& batteryController,
                                     const Controllers::Ble& bleController,
                                     Controllers::NotificationManager& notificationManager,
                                     Controllers::Settings& settingsController,
                                     Controllers::MotionController& motionController,
                                     Controllers::WeatherService& weatherService)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    motionController {motionController},
    weatherService {weatherService} {

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 18, -80);

  label_prompt_1 = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_prompt_1, true);
  lv_obj_align(label_prompt_1, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -40);
  lv_label_set_text_static(label_prompt_1, "user@watch:~ $ stat");

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_date, true);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -20);

  batteryValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(batteryValue, true);
  lv_obj_align(batteryValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 0);

  notificationPrefix = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(notificationPrefix, true);
  lv_label_set_text_static(notificationPrefix, "[NOTI]");
  lv_obj_align(notificationPrefix, nullptr, LV_ALIGN_IN_LEFT_MID, 0, 20);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(notificationIcon, true);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Orange));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_LEFT_MID, 72, 20);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(stepValue, true);
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 40);

  connectState = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(connectState, true);
  lv_obj_align(connectState, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 60);

  weatherState = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(weatherState, true);
  lv_obj_align(weatherState, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 80);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceTerminal::~WatchFaceTerminal() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceTerminal::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated()) {
    if (batteryController.IsPowerPresent()) {
      lv_label_ins_text(batteryValue, LV_LABEL_POS_LAST, "[BATT]#11cc55 Charging#");
    } else {
      if (batteryPercentRemaining.Get() > 20) {
        lv_label_set_text_fmt(batteryValue, "[BATT]#11cc55 %d%% #", batteryPercentRemaining.Get());
      } else if (batteryPercentRemaining.Get() < 10) {
        lv_label_set_text_fmt(batteryValue, "[BATT]#ff0000 %d%% #", batteryPercentRemaining.Get());
      } else {
        lv_label_set_text_fmt(batteryValue, "[BATT]#ffa500 %d%% #", batteryPercentRemaining.Get());
      }
      
    }
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    if (!bleRadioEnabled.Get()) {
      lv_label_set_text_static(connectState, "[STAT]#387b54 Disabled#");
    } else {
      if (bleState.Get()) {
        lv_label_set_text_static(connectState, "[STAT]#387b54 Connected#");
      } else {
        lv_label_set_text_static(connectState, "[STAT]#387b54 Disconnected#");
      }
    }
  }

  notificationCount = notificationManager.NbNotifications();
  if (notificationCount.IsUpdated()) {
    size_t count = notificationCount.Get();
    lv_obj_set_style_local_text_color(notificationPrefix, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Orange));
    if (count == 0 || count > 1) {
      if(count == 0) {
        lv_obj_set_style_local_text_color(notificationPrefix, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      }
      lv_label_set_text_fmt(notificationIcon, "%d messages", count);
    }
    else {
      lv_label_set_text_fmt(notificationIcon, "%d message", count);
    }
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();
    uint8_t second = dateTimeController.Seconds();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      char ampmChar[3] = "AM";
      if (hour == 0) {
        hour = 12;
      } else if (hour == 12) {
        ampmChar[0] = 'P';
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar[0] = 'P';
      }
      lv_label_set_text_fmt(label_time, "%02d:%02d:%02d %s", hour, minute, second, ampmChar);
    } else {
      lv_label_set_text_fmt(label_time, "%02d:%02d:%02d", hour, minute, second);
    }

    currentDate = std::chrono::time_point_cast<days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      Controllers::DateTime::Months month = dateTimeController.Month();
      uint8_t day = dateTimeController.Day();
      auto* monthString = dateTimeController.MonthShortToStringLow(month);
      lv_label_set_text_fmt(label_date, "[DATE]#007fff %s, %02d.%s#", dateTimeController.DayOfWeekShortToStringLow(), day, monthString);
    }
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "[STEP]#ee3377 %lu steps#", stepCount.Get());
  }

  const auto& newTemperatureEvent = weatherService.GetCurrentTemperature();

  if (newTemperatureEvent->timestamp != 0) {
    nowTemp = (weatherService.GetCurrentTemperature()->temperature);
    if (nowTemp.IsUpdated()) {
      int16_t modulo = (nowTemp.Get() % 100) / 10;
      lv_label_set_text_fmt(weatherState, "[TEMP]#be2bc1 %d.%d° #", modulo < 5 ? nowTemp.Get() / 100 : (nowTemp.Get() / 100) + 1, modulo);
    }    
  } else {
    lv_label_set_text_static(weatherState, "[TEMP]#be2bc1 ---#");
  }
}
