#pragma once

#include <boost/sml.hpp>
#include <bitset>
#include <iostream>
#include <vector>

static bool printDebug = false;
static const int decoderwidth = 12;

struct decoderOutput
{
  std::bitset<decoderwidth> shortPress{};
  std::bitset<decoderwidth> longPress{};
  std::bitset<decoderwidth> doubleShortPress{};
  std::bitset<decoderwidth> doubleLongPress{};
  std::bitset<decoderwidth> shortLongPress{};
  std::bitset<decoderwidth> longShortPress{};

  std::vector<unsigned long> shortPressTime = std::vector<unsigned long>(decoderwidth, 0);
};

struct touchDecoderTimingConfig
{
  unsigned long minReleaseTime{};
  unsigned long shortPressTime{};
  unsigned long longPressTime{};
  unsigned long maxIdleShortTime{};
  unsigned long maxIdleLongTime{};
};

class state_machine
{
  using Self = state_machine;

public:
  explicit state_machine(const touchDecoderTimingConfig &tc) : tc{tc} {}

  // States
  struct Idle
  {
  };

  struct IdleShort
  {
    unsigned long entry_time{};
    int keyid{};
    unsigned long press_time{};
  };

  struct IdleLong
  {
    unsigned long entry_time{};
    int keyid{};
  };

  struct Touched
  {
    unsigned long entry_time{};
  };

  struct TouchedShort
  {
    unsigned long entry_time{};
  };

  struct TouchedLong
  {
    unsigned long entry_time{};
  };

  // Events
  struct no_event
  {
    int keyid{};
    unsigned long event_time{};
  };

  struct touch_event
  {
    int keyid{};
    unsigned long event_time{};
  };
  struct release_event
  {
    int keyid{};
    unsigned long event_time{};
  };
  // Transition table
  auto operator()() const
  {
    using namespace boost::sml;

    auto guard_min_touch_time = [this](const auto &event, Touched &state) {
      bool timeout = false;
      unsigned long timeDiff = event.event_time - state.entry_time;
      if (timeDiff < tc.minReleaseTime)
      {
        timeout = true;
      }
      if (printDebug)
        std::cout << "Key " << event.keyid << " - guard_min_touch_time (" << timeDiff << ") = " << timeout << std::endl;
      return timeout;
    };
    auto guard_short_press_time = [this](const auto &event, decoderOutput &dcOutput, Touched &state, IdleShort &dstState) {
      bool result = false;
      unsigned long timeDiff = event.event_time - state.entry_time;
      if (timeDiff >= tc.minReleaseTime && timeDiff < tc.shortPressTime)
      {
        dstState.press_time = timeDiff;
        result = true;
      }
      if (printDebug)
        std::cout << "Key " << event.keyid << " - guard_short_press_time (" << timeDiff << ") = " << result << std::endl;
      return result;
    };
    auto guard_long_press_time = [this](const auto &event, Touched &state) {
      bool result = false;
      unsigned long timeDiff = event.event_time - state.entry_time;
      if (timeDiff >= tc.longPressTime)
      {
        result = true;
      }
      if (printDebug)
        std::cout << "Key " << event.keyid << " - guard_long_press_time (" << timeDiff << ") = " << result << std::endl;
      return result;
    };

    auto guard_short_min_touch_time = [this](const auto &event, TouchedShort &state) {
      bool timeout = false;
      unsigned long timeDiff = event.event_time - state.entry_time;
      if (timeDiff < tc.minReleaseTime)
      {
        timeout = true;
      }
      if (printDebug)
        std::cout << "Key " << event.keyid << " - guard_short_min_touch_time (" << timeDiff << ") = " << timeout << std::endl;
      return timeout;
    };
    auto guard_short_short_press_time = [this](const auto &event, TouchedShort &state) {
      bool result = false;
      unsigned long timeDiff = event.event_time - state.entry_time;
      if (timeDiff >= tc.minReleaseTime && timeDiff < tc.shortPressTime)
      {
        result = true;
      }
      if (printDebug)
        std::cout << "Key " << event.keyid << " - guard_short_short_press_time (" << timeDiff << ") = " << result << std::endl;
      return result;
    };
    auto guard_short_long_press_time = [this](const auto &event, TouchedShort &state) {
      bool result = false;
      unsigned long timeDiff = event.event_time - state.entry_time;
      if (timeDiff >= tc.longPressTime)
      {
        result = true;
      }
      if (printDebug)
        std::cout << "Key " << event.keyid << " - guard_short_long_press_time (" << timeDiff << ") = " << result << std::endl;
      return result;
    };

    auto guard_long_min_touch_time = [this](const auto &event, TouchedLong &state) {
      bool timeout = false;
      unsigned long timeDiff = event.event_time - state.entry_time;
      if (timeDiff < tc.minReleaseTime)
      {
        timeout = true;
      }
      if (printDebug)
        std::cout << "Key " << event.keyid << " - guard_long_min_touch_time (" << timeDiff << ") = " << timeout << std::endl;
      return timeout;
    };
    auto guard_long_short_press_time = [this](const auto &event, TouchedLong &state) {
      bool result = false;
      unsigned long timeDiff = event.event_time - state.entry_time;
      if (timeDiff >= tc.minReleaseTime && timeDiff < tc.shortPressTime)
      {
        result = true;
      }
      if (printDebug)
        std::cout << "Key " << event.keyid << " - guard_long_short_press_time (" << timeDiff << ") = " << result << std::endl;
      return result;
    };
    auto guard_long_long_press_time = [this](const auto &event, TouchedLong &state) {
      bool result = false;
      unsigned long timeDiff = event.event_time - state.entry_time;
      if (timeDiff >= tc.longPressTime)
      {
        result = true;
      }
      if (printDebug)
        std::cout << "Key " << event.keyid << " - guard_long_long_press_time (" << timeDiff << ") = " << result << std::endl;
      return result;
    };

    auto guard_max_idleshort_time = [this](const auto &event, IdleShort &state, unsigned long &currentTime) {
      bool timeout = false;
      unsigned long timeDiff = currentTime - state.entry_time;
      if (timeDiff > tc.maxIdleShortTime)
      {
        timeout = true;
      }
      if (printDebug)
        std::cout << "Key " << state.keyid << " - guard_max_idleshort_time (" << timeDiff << ") = " << timeout << std::endl;
      return timeout;
    };

    auto guard_max_idlelong_time = [this](const auto &event, IdleLong &state, unsigned long &currentTime) {
      bool timeout = false;
      unsigned long timeDiff = currentTime - state.entry_time;
      if (timeDiff > tc.maxIdleLongTime)
      {
        timeout = true;
      }
      if (printDebug)
        std::cout << "Key " << state.keyid << " - guard_max_idlelong_time (" << timeDiff << ") = " << timeout << std::endl;
      return timeout;
    };

    const auto touch_action = [](const auto &event, Touched &state) {
      if (printDebug)
        std::cout << "Key " << event.keyid << " - Transition due to touch_event" << std::endl;
      state.entry_time = event.event_time;
    };
    const auto touch_short_action = [](const auto &event, TouchedShort &state) {
      if (printDebug)
        std::cout << "Key " << event.keyid << " - Transition due to touch_short_event" << std::endl;
      state.entry_time = event.event_time;
    };
    const auto touch_long_action = [](const auto &event, TouchedLong &state) {
      if (printDebug)
        std::cout << "Key " << event.keyid << " - Transition due to touch_long_event" << std::endl;
      state.entry_time = event.event_time;
    };

    const auto release_to_idle_short = [](decoderOutput &dcOutput, IdleShort &state, const auto &event) {
      if (printDebug)
        std::cout << "Key " << event.keyid << " - Transition due to release_to_idle_short" << std::endl;
      state.entry_time = event.event_time;
      state.keyid = event.keyid;
    };

    const auto release_to_idle_long = [](decoderOutput &dcOutput, IdleLong &state, const auto &event) {
      if (printDebug)
        std::cout << "Key " << event.keyid << " - Transition due to release_to_idle_long" << std::endl;
      state.entry_time = event.event_time;
      state.keyid = event.keyid;
    };

    const auto release_short_action = [](decoderOutput &dcOutput, IdleShort &state, const auto &event) {
      if (printDebug)
        std::cout << "Key " << state.keyid << " - Transition due to release_short_action" << std::endl;
      dcOutput.shortPressTime[state.keyid] = state.press_time;
      dcOutput.shortPress.set(state.keyid);
    };

    const auto release_double_short_action = [](decoderOutput &dcOutput, TouchedShort &state, const auto &event) {
      if (printDebug)
        std::cout << "Key " << event.keyid << " - Transition due to release_double_short_action" << std::endl;
      dcOutput.doubleShortPress.set(event.keyid);
    };

    const auto release_short_long_action = [](decoderOutput &dcOutput, TouchedShort &state, const auto &event) {
      if (printDebug)
        std::cout << "Key " << event.keyid << " - Transition due to release_short_long_action" << std::endl;
      dcOutput.shortLongPress.set(event.keyid);
    };

    const auto release_long_action = [](decoderOutput &dcOutput, IdleLong &state, const auto &event) {
      if (printDebug)
        std::cout << "Key " << state.keyid << " - Transition due to release_long_action" << std::endl;
      dcOutput.longPress.set(state.keyid);
    };

    const auto release_long_short_action = [](decoderOutput &dcOutput, TouchedLong &state, const auto &event) {
      if (printDebug)
        std::cout << "Key " << event.keyid << " - Transition due to release_long_short_action" << std::endl;
      dcOutput.longShortPress.set(event.keyid);
    };

    const auto release_double_long_action = [](decoderOutput &dcOutput, TouchedLong &state, const auto &event) {
      if (printDebug)
        std::cout << "Key " << event.keyid << " - Transition due to release_double_long_action" << std::endl;
      dcOutput.doubleLongPress.set(event.keyid);
    };

    const auto min_touch_time_cancel_action = [](decoderOutput &dcOutput, const auto &event) {
      if (printDebug)
        std::cout << "Key " << event.keyid << " - Transition due to min_touch_time_cancel_action" << std::endl;
    };

    // clang-format off
    return make_transition_table(
        * state<Idle>          + event<touch_event>                                  / (touch_action)                = state<Touched>
        , state<Touched>       + event<release_event>   [guard_min_touch_time]       / (min_touch_time_cancel_action)              = state<Idle>
        , state<Touched>       + event<release_event>   [guard_short_press_time]     / (release_to_idle_short)       = state<IdleShort>
        , state<Touched>       + event<release_event>   [guard_long_press_time]      / (release_to_idle_long)        = state<IdleLong>
        , state<IdleShort>                              [guard_max_idleshort_time]   / (release_short_action)        = state<Idle>
        , state<IdleLong>                               [guard_max_idlelong_time]    / (release_long_action)         = state<Idle>

        , state<IdleShort>     + event<touch_event>                                  / (touch_short_action)               = state<TouchedShort>
        , state<IdleLong>      + event<touch_event>                                  / (touch_long_action)                = state<TouchedLong>

        , state<TouchedShort>  + event<release_event>   [guard_short_min_touch_time]       / (min_touch_time_cancel_action)              = state<Idle>
        , state<TouchedShort>  + event<release_event>   [guard_short_short_press_time]     / (release_double_short_action) = state<Idle>
        , state<TouchedShort>  + event<release_event>   [guard_short_long_press_time]      / (release_short_long_action)   = state<Idle>

        , state<TouchedLong>  + event<release_event>   [guard_long_min_touch_time]       / (min_touch_time_cancel_action)               = state<Idle>
        , state<TouchedLong>  + event<release_event>   [guard_long_short_press_time]     / (release_long_short_action)    = state<Idle>
        , state<TouchedLong>  + event<release_event>   [guard_long_long_press_time]      / (release_double_long_action)   = state<Idle>
    );
    // clang-format on
  }

  touchDecoderTimingConfig tc{};
};

class TouchDecoder
{
public:
  TouchDecoder() : statemachine(state_machine{touchDecoderTimingConfig{30, 300, 300, 500, 1000}})
  {
    for (int i = 0; i < decoderwidth; i++)
    {
      vsm.push_back(new boost::sml::sm<state_machine>{statemachine, state_machine::Touched{0}, output, currentTime});
    }
  }

  TouchDecoder(touchDecoderTimingConfig);

  ~TouchDecoder()
  {
    for (int i = 0; i < decoderwidth; i++)
    {
      delete vsm[i];
    }
  }

  state_machine statemachine;

  std::vector<boost::sml::sm<state_machine> *> vsm;

  void push(uint16_t touchstate, unsigned long pushTime)
  {
    std::bitset<decoderwidth> newState = std::bitset<decoderwidth>(touchstate);
    _released = current & ~newState;
    _touched = ~current & newState;

    // reset output event registers to zero
    output = decoderOutput{};

    if (printDebug)
      std::cout << "PRE-SM  - current: " << current
                << ", touchstate: " << touchstate
                << ", _released: " << _released
                << ", _touched: " << _touched
                << ", shortPress: " << output.shortPress
                << ", longPress: " << output.longPress
                << ", doubleShort: " << output.doubleShortPress
                << ", shortLongPress: " << output.shortLongPress
                << ", longShortPress: " << output.longShortPress
                << ", doubleLong: " << output.doubleLongPress
                << std::endl;

    current = newState;
    currentTime = pushTime;

    for (int i = 0; i < decoderwidth; i++)
    {
      if (_touched.test(i))
      {
        vsm[i]->process_event(state_machine::touch_event{i, pushTime});
      }
    }

    for (int i = 0; i < decoderwidth; i++)
    {
      if (_released.test(i))
      {
        vsm[i]->process_event(state_machine::release_event{i, pushTime});
      }
    }

    for (int i = 0; i < decoderwidth; i++)
    {
      if (~_released.test(i) && ~_touched.test(i))
      {
        vsm[i]->process_event(state_machine::no_event{i, pushTime});
      }
    }
    if (printDebug)
      std::cout << "POST-SM - current: " << current
                << ", touchstate: " << touchstate
                << ", _released: " << _released
                << ", _touched: " << _touched
                << ", shortPress: " << output.shortPress
                << ", longPress: " << output.longPress
                << ", doubleShort: " << output.doubleShortPress
                << ", shortLongPress: " << output.shortLongPress
                << ", longShortPress: " << output.longShortPress
                << ", doubleLong: " << output.doubleLongPress
                << std::endl;
    int j = 3;
  }

  std::bitset<decoderwidth> released()
  {
    return _released;
  }

  std::bitset<decoderwidth> shortPress()
  {
    return output.shortPress;
  }

  std::vector<unsigned long> shortPressTime()
  {
    return output.shortPressTime;
  }

  std::bitset<decoderwidth> longPress()
  {
    return output.longPress;
  }

  std::bitset<decoderwidth> doubleShortPress()
  {
    return output.doubleShortPress;
  }

  std::bitset<decoderwidth> doubleLongPress()
  {
    return output.doubleLongPress;
  }

  std::bitset<decoderwidth> shortLongPress()
  {
    return output.shortLongPress;
  }
  std::bitset<decoderwidth> longShortPress()
  {
    return output.longShortPress;
  }
  int width()
  {
    return decoderwidth;
  }

private:
  unsigned long currentTime;
  int empMaxTouched; // max count of touches in buffer. If exceeded will clear buffer and set empDetected = true
  int _empTouchCount = 0;
  int bufSize;

  touchDecoderTimingConfig tc{10, 20};

  decoderOutput output;
  std::bitset<decoderwidth> current = 0;
  std::bitset<decoderwidth> _released;
  std::bitset<decoderwidth> _touched;

  // actions
};

TouchDecoder::TouchDecoder(touchDecoderTimingConfig tc) : statemachine{tc}
{
  for (int i = 0; i < decoderwidth; i++)
  {
    vsm.push_back(new boost::sml::sm<state_machine>{statemachine, state_machine::Touched{0}, output, currentTime});
  }
}