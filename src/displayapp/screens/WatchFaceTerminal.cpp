#include <lvgl/lvgl.h>
#include "displayapp/screens/WatchFaceTerminal.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include <displayapp/Colors.h>

using namespace Pinetime::Applications::Screens;

WatchFaceTerminal::WatchFaceTerminal(Controllers::DateTime& dateTimeController,
                                     const Controllers::Battery& batteryController,
                                     const Controllers::Ble& bleController,
                                     Controllers::NotificationManager& notificationManager,
                                     Controllers::Settings& settingsController,
                                     Controllers::HeartRateController& heartRateController,
                                     Controllers::MotionController& motionController,
                                     Controllers::WeatherService& weatherService)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    weatherService {weatherService} {

  label_prompt_1 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_prompt_1, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -90);
  lv_label_set_text_static(label_prompt_1, "user@watch:~ $ now");

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_time, true);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -70);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_date, true);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -50);

  batteryValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(batteryValue, true);
  lv_obj_align(batteryValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -30);

  notificationPrefix = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(notificationPrefix, true);
  lv_label_set_text_static(notificationPrefix, "[NOTI]");
  lv_obj_align(notificationPrefix, nullptr, LV_ALIGN_IN_LEFT_MID, 0, -10);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(notificationIcon, true);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Orange));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_LEFT_MID, 72, -10);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(stepValue, true);
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 10);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(heartbeatValue, true);
  lv_obj_align(heartbeatValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 30);

  connectState = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(connectState, true);
  lv_obj_align(connectState, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 50);

  weatherStatePrefix = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(weatherStatePrefix, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 70);
  lv_label_set_text_static(weatherStatePrefix, "[WTHR]");

  weatherState = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(weatherState, LV_LABEL_LONG_SROLL_CIRC);
  lv_label_set_recolor(weatherState, true);
  lv_obj_align(weatherState, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 72, 70);
  lv_obj_set_width(weatherState, LV_HOR_RES - 75);

  label_prompt_2 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_prompt_2, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 90);
  lv_label_set_text_static(label_prompt_2, "user@watch:~ $");

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
    lv_label_set_text_fmt(batteryValue, "[BATT]#387b54 %d%% #", batteryPercentRemaining.Get());
    if (batteryController.IsPowerPresent()) {
      lv_label_ins_text(batteryValue, LV_LABEL_POS_LAST, " Charging");
    }
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    if (!bleRadioEnabled.Get()) {
      lv_label_set_text_static(connectState, "[STAT]#0082fc Disabled#");
    } else {
      if (bleState.Get()) {
        lv_label_set_text_static(connectState, "[STAT]#0082fc Connected#");
      } else {
        lv_label_set_text_static(connectState, "[STAT]#0082fc Disconnected#");
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
      lv_label_set_text_fmt(notificationIcon, "%lu messages", count);
    }
    else {
      lv_label_set_text_fmt(notificationIcon, "%lu message", count);
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
      lv_label_set_text_fmt(label_time, "[TIME]#11cc55 %02d:%02d:%02d %s#", hour, minute, second, ampmChar);
    } else {
      lv_label_set_text_fmt(label_time, "[TIME]#11cc55 %02d:%02d:%02d#", hour, minute, second);
    }

    currentDate = std::chrono::time_point_cast<days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      Controllers::DateTime::Months month = dateTimeController.Month();
      uint8_t day = dateTimeController.Day();
      auto* monthString = dateTimeController.MonthShortToStringLow(month);
      lv_label_set_text_fmt(label_date, "[DATE]#007fff %s, %02d.%s#", dateTimeController.DayOfWeekShortToStringLow(), day, monthString);
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_label_set_text_fmt(heartbeatValue, "[HBRT]#ee3311 %d bpm#", heartbeat.Get());
    } else {
      lv_label_set_text_static(heartbeatValue, "[HBRT]#ee3311 ---#");
    }
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "[STEP]#ee3377 %lu steps#", stepCount.Get());
  }

  if (weatherService.GetCurrentTemperature()->timestamp != 0 && weatherService.GetCurrentClouds()->timestamp != 0 &&
      weatherService.GetCurrentPrecipitation()->timestamp != 0) {
    nowTemp = (weatherService.GetCurrentTemperature()->temperature);
    clouds = (weatherService.GetCurrentClouds()->amount);
    precip = (weatherService.GetCurrentPrecipitation()->amount);
    lv_obj_align(weatherState, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 72, 70);
    if (nowTemp.IsUpdated()) {
      if ((clouds.Get() <= 30) && (precip.Get() == 0)) {
        lv_label_set_text_fmt(weatherState, "#be2bc1 %d.%d° clear#", nowTemp.Get() / 100, (nowTemp.Get() % 100) / 10);
      } else if ((clouds.Get() >= 70) && (clouds.Get() <= 90) && (precip.Get() == 1)) {
        lv_obj_align(weatherState, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 90, 70);
        lv_label_set_text_fmt(weatherState, "#be2bc1 %d.%d° sun/cloudy/rain#", nowTemp.Get() / 100, (nowTemp.Get() % 100) / 10);
      } else if ((clouds.Get() > 90) && (precip.Get() == 0)) {
        lv_label_set_text_fmt(weatherState, "#be2bc1 %d.%d° cloudy#", nowTemp.Get() / 100, (nowTemp.Get() % 100) / 10);
      } else if ((clouds.Get() > 70) && (precip.Get() >= 2)) {
        lv_label_set_text_fmt(weatherState, "#be2bc1 %d.%d° rain#", nowTemp.Get() / 100, (nowTemp.Get() % 100) / 10);
      } else {
        lv_obj_align(weatherState, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 90, 70);
        lv_label_set_text_fmt(weatherState, "#be2bc1 %d.%d° part.cloudy#", nowTemp.Get() / 100, (nowTemp.Get() % 100) / 10);
      };
    }    
  } else {
    lv_label_set_text_static(weatherState, "#be2bc1 ---#");
  }
}
