#include "Stream.hpp"

using namespace glasgow_ustream;

class NumberSource : public Pollable<uint32_t> {

    std::list<uint32_t> *values;
    int pos = 0;

public:
    explicit NumberSource(std::list<uint32_t> *values) {
        this->values = values;
    }

    virtual ~NumberSource() {}

    uint32_t getData(PolledSource<uint32_t> *caller) {

        std::list<uint32_t>::iterator ptr;
        int i = 0;

        for (i = 0, ptr = values->begin(); i < pos && ptr != values->end(); i++, ptr++);

        if (ptr == values->end()) {
            caller->stop();
            return 0;
        } else {
            pos++;
            return *ptr;
        }
    }
};


int main(int, char **) {

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //  Lambda Functions
    auto print_int_sink = [](uint32_t val) {
        std::cout << "Received value " << val << std::endl;
    };

    std::pair<uint32_t, void*> (*int_to_byte_array) (uint32_t) = [] (uint32_t val) {
        uint32_t *int_ptr = (uint32_t *) malloc(sizeof(uint32_t));
        *int_ptr = val;
        return std::pair<uint32_t, void*>(sizeof(val), int_ptr);
    };

    optional<uint32_t> (*byte_array_to_int) (std::pair<uint32_t, void*>) = [] (std::pair<uint32_t, void*> data) {
        return optional<uint32_t>(*((uint32_t*) data.second));
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Topology *topology = new Topology(std::chrono::seconds(1));

    // Create two data sources that will feed the topology.
    std::list<uint32_t> values = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    NumberSource *numberSource = new NumberSource(&values);
    Source<uint32_t> *int_source = topology->addFixedDataSource(values);
    Source<uint32_t> *int_source2 = topology->addPolledSource(std::chrono::seconds(1), numberSource);

    // Union the two data sources and sink them into the network stream "numbers"
    std::list<Subscribeable<uint32_t> *> subscribers = {(Subscribeable<uint32_t> *) int_source2};
    auto *union_stream = int_source->union_streams(subscribers);
    union_stream->networkSink(topology, "numbers", int_to_byte_array);

    // Create a new topology source that will read data from the network (potentially from a different sensor)
    // This call fails if another source exists with the same stream_id.
    optional<NetworkSource<uint32_t> *> opt_network_temperature_source = topology->addNetworkSource("numbers", byte_array_to_int);

    // Sink the network stream into std.out.
    NetworkSource<uint32_t> *networkTemperatureSource = opt_network_temperature_source.value();


    networkTemperatureSource->sink(print_int_sink);

    std::cout << "Topology built." << std::endl;

    while (!topology->peers_connected()) {
        std::cout << "Waiting for connections..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "Running..." << std::endl;
    topology->run_with_threads();

    topology->shutdown();
    delete (topology);
    delete (numberSource);

    return 0;
}
