#pragma once
#include <cstddef>
namespace Pinetime {
  namespace Applications {
    enum class Apps {
      None,
      Launcher,
      Clock,
      SysInfo,
      FirmwareUpdate,
      FirmwareValidation,
      NotificationsPreview,
      Notifications,
      Timer,
      Alarm,
      FlashLight,
      BatteryInfo,
      Music,
      Paint,
      Paddle,
      Twos,
      HeartRate,
      Navigation,
      StopWatch,
      Metronome,
      Motion,
      Steps,
      PassKey,
      QuickSettings,
      Settings,
      SettingWatchFace,
      SettingTimeFormat,
      SettingDisplay,
      SettingWakeUp,
      SettingSteps,
      SettingSetDateTime,
      SettingChimes,
      SettingBleDisconnectAlert,
      SettingShakeThreshold,
      SettingBluetooth,
      Error,
      Weather
    };

    template <Apps>
    struct AppTraits {};

    template <Apps... As>
    struct TypeList {
      static constexpr size_t Count = sizeof...(As);
    };

    using UserAppTypes = TypeList<Apps::Alarm,
                                  Apps::HeartRate,
                                  Apps::Music,
                                  Apps::Steps,
                                  Apps::StopWatch,
                                  Apps::Timer
                                  >;
  }
}
