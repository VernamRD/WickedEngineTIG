#pragma once

#include <GPUCPI.h>
#include <thread>

namespace Giperion
{
    namespace GPUCPI
    {
        class GPUCPI_Impl : public Giperion::GPUCPI::GPUCPI
        {
        public:
            void Initialize() override;
            void Deinitialize() override;

            bool IsInitialized() override;

        private:
            void main(std::stop_token token);

            std::jthread GLThread;
        };
    }  // namespace GPUCPI

}  // namespace Giperion