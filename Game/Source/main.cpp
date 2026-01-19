#include "WickedEngine.h"
#include "wiScene_BindLua.h"

#include "GLT.h"

class Game : public wi::Application
{
    void Initialize() override;

    void Update(float dt) override;
    void Exit() override;
};

class GameRender : public wi::RenderPath3D
{
public:
    wi::Sprite sprite;

    GameRender()
    {
        sprite = wi::Sprite(wi::helper::GetRootDir() + "Content\\logo_small.png");

        sprite.params.pos = XMFLOAT3(100, 100, 0);
        sprite.params.siz = XMFLOAT2(256, 256);

        AddSprite(&sprite);
    }
};

Game application;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // process command line string:
    wi::arguments::Parse(lpCmdLine);

    // Win32 window and message loop setup:
    static auto WndProc = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        switch (message)
        {
            case WM_SIZE:
            case WM_DPICHANGED:
                if (application.is_window_active) application.SetWindow(hWnd);
                break;
            case WM_CHAR:
                switch (wParam)
                {
                    case VK_BACK: wi::gui::TextInputField::DeleteFromInput(); break;
                    case VK_RETURN: break;
                    default:
                    {
                        const wchar_t c = (const wchar_t)wParam;
                        wi::gui::TextInputField::AddInput(c);
                    }
                    break;
                }
                break;
            case WM_INPUT: wi::input::rawinput::ParseMessage((void*)lParam); break;
            case WM_KILLFOCUS: application.is_window_active = false; break;
            case WM_SETFOCUS: application.is_window_active = true; break;
            case WM_DESTROY: PostQuitMessage(0); break;
            default: return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    };
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"Game";
    wcex.hIconSm = NULL;
    RegisterClassExW(&wcex);
    HWND hWnd = CreateWindowW(wcex.lpszClassName, wcex.lpszClassName, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    ShowWindow(hWnd, SW_SHOWDEFAULT);

    // set Win32 window to engine:
    application.SetWindow(hWnd);

    // just show some basic info:
    application.infoDisplay.active = true;
    application.infoDisplay.watermark = true;
    application.infoDisplay.resolution = true;
    application.infoDisplay.fpsinfo = true;

    GameRender Render;
    application.ActivatePath(&Render);

    wi::eventhandler::SetVSync(false);
    wi::profiler::SetEnabled(true);

    wi::renderer::SetShaderSourcePath(wi::helper::GetCurrentPath() + "/shaders/");

    MSG msg = {0};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            application.Run();
        }
    }

    wi::jobsystem::ShutDown();  // waits for jobs to finish before shutdown

    return (int)msg.wParam;
}

void Game::Initialize()
{
    Application::Initialize();

    Giperion::GLT::Init();

    // CreateBullet(500000);

    // Shader.initialize(wi::graphics::GetDevice(), wi::graphics::GetDevice()->BeginCommandList());

    // UAVShader.Initialize(wi::graphics::GetDevice());

    auto emitter_entity = wi::ecs::CreateEntity();
    wi::scene::LoadModel2(wi::helper::GetRootDir() + "/Content/BulletEmitter.wiscene",
        XMMatrixIdentity(), emitter_entity);

    wi::scene::LoadModel2(wi::helper::GetRootDir() + "/Content/Camera.wiscene", XMMatrixIdentity());
    auto camera_comp = &wi::scene::GetScene().cameras[0];
    static_cast<wi::RenderPath3D*>(application.activePath)->camera = camera_comp;
}

void Game::Update(float dt)
{
    Application::Update(dt);

    constexpr bool b_GPU_Compute = false;

    // if (b_GPU_Compute)
    // {
    // 	auto scope = wi::profiler::BeginRangeCPU("BulletsUpdate");
    //
    // 	UAVShader.Execute(wi::graphics::GetDevice(), dt);
    // 	if (auto dataPtr = UAVShader.ReadBackResults(wi::graphics::GetDevice()))
    // 	{
    // 		wilog("X: %f", dataPtr[0].Data.x);
    // 	}
    //
    // 	// auto cmd = wi::graphics::GetDevice()->BeginCommandList(wi::graphics::QUEUE_COMPUTE);
    // 	// Shader.Execute(dt, wi::graphics::GetDevice(), cmd);
    // 	//
    // 	// if (auto dataPtr = Shader.ReadBackResults(wi::graphics::GetDevice(), cmd))
    // 	// {
    // 	// 	wilog("X: %f", dataPtr[0].position.x);
    // 	//
    // 	// 	// for (int32_t i = 0; i < Shader.ELEMENTS_COUNT; ++i)
    // 	// 	// {
    // 	// 	// 	wilog("%i:X: %f", i, dataPtr[0].position.x);
    // 	// 	// 	wilog("%i:Y: %f", i, dataPtr[0].position.y);
    // 	// 	// 	wilog("%i:Z: %f", i, dataPtr[0].position.z);
    // 	// 	// }
    // 	// }
    //
    // 	wi::profiler::EndRange(scope);
    // }
    // else
    // {
    // 	auto scope = wi::profiler::BeginRangeCPU("BulletsUpdate");
    //
    // 	auto Bullets =
    // wi::scene::GetScene().componentLibrary.Get<BulletComponent>(BulletComponentManager_Default)->
    // 	                                     GetEntityArray();
    // 	for (auto& BulletEntity : Bullets)
    // 	{
    // 		auto BulletEntityComponent =
    // wi::scene::GetScene().componentLibrary.Get<BulletComponent>(
    // 			BulletComponentManager_Default)->GetComponent(BulletEntity);
    // 		++BulletEntityComponent->BulletID;
    // 	}
    //
    // 	wi::profiler::EndRange(scope);
    // }
}

void Game::Exit()
{
    Giperion::GLT::Deinitialize();
    Application::Exit();
}
