// #include <CPI.h>

// namespace wi
// {
//     namespace CPI
//     {
//         static std::unique_ptr<CPI> CPIThreadPtr = nullptr;

//         void Init()
//         {
//             // CPIThreadPtr = std::make_unique<CPI>();
//             // CPIThreadPtr->Initialize();
//         }

//         bool IsInitialized() { return CPIThreadPtr && CPIThreadPtr->IsInitialized(); }

//         void Deinitialize()
//         {
//             CPIThreadPtr->Deinitialize();
//             CPIThreadPtr.reset();
//         }

//         CPI& Get() { return *CPIThreadPtr; }
//     }  // namespace CPI
// }  // namespace wi