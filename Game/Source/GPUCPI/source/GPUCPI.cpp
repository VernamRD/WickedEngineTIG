#include <GPUCPI.h>

namespace Giperion
{
    namespace GPUCPI
    {
        static std::unique_ptr<GPUCPI> GLTPtr = nullptr;

        void Init()
        {
            // GLTPtr = std::make_unique<GameLogicThread>();
            // GLTPtr->Initialize();
        }

        bool IsInitialized() { return GLTPtr && GLTPtr->IsInitialized(); }

        void Deinitialize()
        {
            GLTPtr->Deinitialize();
            GLTPtr.reset();
        }

        GPUCPI& Get() { return *GLTPtr; }
    }  // namespace GPUCPI
}  // namespace Giperion