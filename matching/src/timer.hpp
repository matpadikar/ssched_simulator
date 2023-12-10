#include <chrono>

class StopWatch{
private:
    bool running;
    std::chrono::time_point<std::chrono::steady_clock> start_time, end_time;
public:
    explicit StopWatch(bool with_start = true){
        end_time = start_time;
        running = false;
        if (with_start){
            start();
        }
    }

    bool is_running() const {
        return running;
    }

    bool start(){
        if (running){
            // clock is already running
            return false;
        } else {
            running = true;
            start_time = std::chrono::steady_clock::now();
            return true;
        }
    }

    bool stop(){
        if (!running){
            // clock is already stopped
            return false;
        } else {
            running = false;
            end_time = std::chrono::steady_clock::now();
            return true;
        }
    }

    double peek(){
        if (running){
            end_time = std::chrono::steady_clock::now();
        }
        std::chrono::duration<double> diff = end_time - start_time;
        return diff.count();
    }

    void reset_and_start(){
        running = true;
        start_time = std::chrono::steady_clock::now();
    }
};