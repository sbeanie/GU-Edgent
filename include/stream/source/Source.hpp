#ifndef GU_EDGENT_SOURCE_H
#define GU_EDGENT_SOURCE_H

#include "../StreamTypes.hpp"


class Startable {

public:
    virtual void start() = 0;
};

template <typename T>
class Source : public Stream<T>, public Startable {

public:

    virtual void start() = 0;
};


#include "FixedDataSource.hpp"
#include "PolledSource.hpp"
#include "NetworkSource.hpp"

#endif //GU_EDGENT_SOURCE_H