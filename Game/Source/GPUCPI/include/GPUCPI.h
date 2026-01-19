#pragma once

#include <WickedEngine.h>  // подключение движка

namespace Giperion
{
    namespace GPUCPI
    {
        // Graphics Processing Units Compute Pipeline Interface
        class GPUCPI
        {
        public:
            virtual ~GPUCPI() = default;

            virtual void Initialize() = 0;
            virtual void Deinitialize() = 0;
            virtual bool IsInitialized() = 0;
        };

        void Init();
        bool IsInitialized();
        void Deinitialize();
        GPUCPI& Get();
    }  // namespace GPUCPI
}  // namespace Giperion
