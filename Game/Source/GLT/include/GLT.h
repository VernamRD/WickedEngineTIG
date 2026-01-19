#pragma once

#include <WickedEngine.h>  // подключение движка

namespace Giperion
{
    namespace GLT
    {
        class IGLT
        {
        public:
            virtual ~IGLT() = default;

            virtual void Initialize() = 0;
            virtual void Deinitialize() = 0;
            virtual bool IsInitialized() = 0;
        };

        void Init();
        bool IsInitialized();
        void Deinitialize();
        IGLT& Get();
    }  // namespace GLT
}  // namespace Giperion
