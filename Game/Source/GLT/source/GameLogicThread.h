#pragma once

#include "GLT.h"
#include <thread>

class GameLogicThread : public Giperion::GLT::IGLT
{
public:
    void Initialize() override;
    void Deinitialize() override;

    bool IsInitialized() override;

private:
    void main(std::stop_token token);

    std::jthread GLThread;
};
