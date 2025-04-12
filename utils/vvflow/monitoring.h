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
class RingBuffer {
private:
    std::array<double, N> buffer;
    size_t buffer_pos;
    size_t buffer_size;
public:
    RingBuffer<N>(): buffer(), buffer_pos(), buffer_size() {}
    ~RingBuffer<N>() {}


    // size_t size() {
    //     return _size;
    // }
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
        init_timepoint(),
        init_wallclock(),
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

    struct Report {
        TWallclock wallclock_time;
        // TDuration last_step_duration;
        // double steps_per_second_mavg;
        TTime simulation_time;
        size_t vortex_count;
        // TWallclock eta;
    };

    void process(struct Event event) {
        event.emitted_timepoint = TTimepoint::clock::now(); //std::chrono::steady_clock::now();
        event.emitted_wallclock = TWallclock::clock::now(); // std::chrono::system_clock::now();

        events.insert({event.emitted_timepoint, event});
    }

    std::string report() {
        const auto& last_event = events.crbegin()->second;

        std::stringstream ss;
        char buf[256];
        memset(buf, 0, sizeof(buf));

        snprintf(buf, sizeof(buf), "%.1f", double(last_event.simulation_time)/finish*100);
        ss << "percent = " << buf << std::endl;

        std::time_t tt = TWallclock::clock::to_time_t(last_event.emitted_wallclock);
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&tt));
        ss << "wallclock_time = " << buf << std::endl;

        int digits = std::log10(last_event.simulation_time.timescale) + 1;
        snprintf(buf, sizeof(buf), "%.*f", digits, double(last_event.simulation_time));
        ss << "simulation_time = " << buf << std::endl;

        ss << "vortex_count = " << last_event.vortex_count << std::endl;
        return ss.str();
    }

private:
    TTimepoint init_timepoint;
    TWallclock init_wallclock;
    // TRingBuffer
    double finish;
    std::map<TTimepoint, struct Event> events;
};
