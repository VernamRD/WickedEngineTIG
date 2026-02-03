// #pragma once

// #include <WickedEngine.h>
// #include "Tasks/CPITaskFactory.h"

// namespace Giperion
// {
//     namespace CPI
//     {
//         // Graphics Processing Units Compute Pipeline Interface
//         class CPI
//         {
//         public:
//             virtual ~CPI() = default;

//             virtual void Initialize() = 0;
//             virtual void Deinitialize() = 0;
//             virtual bool IsInitialized() = 0;
//         };

//         void Init();
//         bool IsInitialized();
//         void Deinitialize();
//         CPI& Get();
    
        
//         class CPIShader
//         {
//         public:
//             virtual void Initialize(wi::graphics::GraphicsDevice* device, wi::graphics::CommandList cmd) = 0;
//             virtual void Execute(wi::graphics::GraphicsDevice* device, wi::graphics::CommandList cmd) = 0;

//             bool IsInitialized() const { return m_initialized; }

//         protected:
//             std::atomic_bool m_initialized = false;
//         };
//     }  // namespace CPI
// }  // namespace Giperion
