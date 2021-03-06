#ifndef GU_EDGENT_POLLEDSOURCE_H
#define GU_EDGENT_POLLEDSOURCE_H

#include <iostream>
#include "../StreamTypes.hpp"

namespace glasgow_ustream {

    template<typename T>
    class PolledSource;

    /**
     * Extend this class to create a PolledSource node.
     * @tparam T
     */
    template<typename T>
    class Pollable {

    public:
        /**
         * This function should return the next data value for the source.
         * @param caller This can be used to request the PolledSource to stop.
         * @return the next value in the stream.
         */
        virtual T getData(PolledSource<T> *caller) = 0;
    };


    template<typename T>
    class PolledSource : public Source<T> {

        std::chrono::duration<double> interval;

        std::thread thread;

        Pollable<T> *pollable;

    private:

        void poll() {
            while (should_run) {
                T val = this->pollable->getData(this);

                // If the getData function requests the thread to stop we should not publish the result.
                if (!this->should_run) break;

                this->publish(val);
                std::this_thread::sleep_for(interval);
            }
        }

    protected:

        bool should_run = false;

    public:

        PolledSource(std::chrono::duration<double> interval, Pollable<T> *pollable) : interval(interval),
                                                                                      pollable(pollable) {};


        void start() override {
            this->should_run = true;
            if (thread.joinable()) thread.join();
            this->thread = std::thread(&PolledSource::poll, this);
        }

        void join() override {
            if (thread.joinable()) thread.join();
        }

        void stop() {
            this->should_run = false;
        }

        ~PolledSource() {
            this->should_run = false;
            if (thread.joinable()) thread.join();
            // Do not delete the pollable as it might be desirable to have two PolledSources polling the same pollable.
        }
    };

}

#endif //GU_EDGENT_POLLEDSOURCE_H
