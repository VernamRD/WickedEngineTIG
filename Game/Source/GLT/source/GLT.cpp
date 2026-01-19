#include "GLT.h"
#include "GameLogicThread.h"

namespace Giperion
{
    namespace GLT
    {
        static std::unique_ptr<IGLT> GLTPtr = nullptr;

        void Init()
        {
            GLTPtr = std::make_unique<GameLogicThread>();
            GLTPtr->Initialize();
        }

        bool IsInitialized() { return GLTPtr && GLTPtr->IsInitialized(); }

        void Deinitialize()
        {
            GLTPtr->Deinitialize();
            GLTPtr.reset();
        }

        IGLT& Get() { return *GLTPtr; }
    }  // namespace GLT
}  // namespace Giperion