// #pragma once

// #include <CPI.h>
// #include <thread>

// namespace Giperion
// {
//     namespace CPI
//     {
//         class CPI_Impl : public Giperion::CPI::CPI
//         {
//         public:
//             void Initialize() override;
//             void Deinitialize() override;

//             bool IsInitialized() override;

//         private:
//             void main(std::stop_token token);

//             std::jthread GLThread;
//         };
//     }  // namespace CPI

// }  // namespace Giperion