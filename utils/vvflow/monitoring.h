#pragma once

#include "TTime.hpp"
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <map>
#include <array>
#include <sstream>

using TTimepoint = std::chrono::steady_clock::time_point;
using TWallclock = std::chrono::system_clock::time_point;
using TDuration = std::chrono::steady_clock::duration;

std::string datetimefmt(TWallclock t) {
    std::time_t tt = TWallclock::clock::to_time_t(t);
    std::string s(30, '\0');
    std::strftime(&s[0], s.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&tt));
    return s;
}

template<size_t N>
class TRingBuffer {
private:
    std::array<double, N> buffer;
    size_t buffer_pos;
    size_t buffer_size;
public:
    TRingBuffer<N>(): buffer(), buffer_pos(), buffer_size() {}
    ~TRingBuffer<N>() {}

    void record(double val) {
        buffer[buffer_pos] = val;
        buffer_size = std::min(buffer_size + 1, N);
        buffer_pos = (buffer_pos + 1) % N;
    }

    double average() {
        double result = 0;
        for (auto i = 0; i < buffer_size; i++) {
            result += buffer[i];
        }
        return result / buffer_size;
    }
};

class Monitoring {
public:
    Monitoring():
        init_timepoint(TTimepoint::clock::now()),
        init_wallclock(TWallclock::clock::now()),
        step_durations(),
        finish(),
        events()
        {}
    ~Monitoring() {}

    void setFinish(double _finish) {
        finish = _finish;
    }

    struct Event {
        TTimepoint emitted_timepoint;
        TWallclock emitted_wallclock;
        TTime simulation_time;
        size_t vortex_count;
    };

    void process(struct Event event) {
        event.emitted_timepoint = TTimepoint::clock::now();
        event.emitted_wallclock = TWallclock::clock::now();

        const auto& it = events.crbegin();
        if (it != events.crend()) {
            const auto& last_event = it->second;
            auto last_step_duration = event.emitted_timepoint - last_event.emitted_timepoint;
            step_durations.record(std::chrono::duration<double>(last_step_duration).count());
        }

        events.insert({event.emitted_timepoint, event});
    }

    std::string report() {
        const auto& last_event = events.crbegin()->second;

        std::stringstream ss;
        char buf[256];
        memset(buf, 0, sizeof(buf));

        /* percent */ if (finish > 0) {
            snprintf(buf, sizeof(buf), "%.1f", double(last_event.simulation_time)/finish*100);
            ss << "percent = " << buf << std::endl;
        }

        /* wallclock_time */ {
            std::time_t tt = TWallclock::clock::to_time_t(last_event.emitted_wallclock);
            std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&tt));
            ss << "wallclock_time = " << buf << std::endl;
        }

        /* simulation_time */ {
            int digits = std::log10(last_event.simulation_time.timescale) + 1;
            snprintf(buf, sizeof(buf), "%.*f", digits, double(last_event.simulation_time));
            ss << "simulation_time = " << buf << std::endl;
        }

        /* vortex_count */ {
            ss << "vortex_count = " << last_event.vortex_count << std::endl;
        }

        /* elapsed */ {
            double elapsed = std::chrono::duration<double>(last_event.emitted_timepoint - init_timepoint).count();
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d", int(elapsed/3600), int(elapsed/60)%60, int(elapsed)%60);
            ss << "elapsed = " << buf << std::endl;
        }

        /* step_duration */ {
            double step_duration = step_durations.average();
            if (step_duration > 0.2) {
                snprintf(buf, sizeof(buf), "%.1f", step_duration);
            } else {
                snprintf(buf, sizeof(buf), "%e", step_duration);
            }
            ss << "step_duration = " << step_duration << std::endl;
        }

        return ss.str();
    }

private:
    TTimepoint init_timepoint;
    TWallclock init_wallclock;
    TRingBuffer<10> step_durations;
    double finish;
    std::map<TTimepoint, struct Event> events;
};
