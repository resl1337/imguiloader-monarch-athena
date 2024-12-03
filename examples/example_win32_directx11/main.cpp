#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"
#include "imgui_settings.h"
#include "imgui_freetype.h"

#include <D3DX11tex.h>
#pragma comment(lib, "D3DX11.lib")

#include "image.h"
#include "font.h"

#include <ctime>

#include <chrono>

#include <d3d11.h>
#include <tchar.h>
#include <dwmapi.h>
#include <string>

using namespace std;
using namespace std::chrono_literals;

static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

int page = 0;
int timer = 0;
int selector = 0;

int x_rand, y_rand;
float timer_invalid = 10;
int click = 0;

std::string login_correct = "name";
std::string pass_correct = "123";

namespace image
{
    ID3D11ShaderResourceView* logo = nullptr;

    ID3D11ShaderResourceView* close = nullptr;
    ID3D11ShaderResourceView* hide = nullptr;

    ID3D11ShaderResourceView* discord = nullptr;
    ID3D11ShaderResourceView* website = nullptr;
    ID3D11ShaderResourceView* telegram = nullptr;
    ID3D11ShaderResourceView* twitter = nullptr;

    ID3D11ShaderResourceView* call_of_duty = nullptr;
    ID3D11ShaderResourceView* warcraft = nullptr;
    ID3D11ShaderResourceView* valorant = nullptr;
    ID3D11ShaderResourceView* cs_go = nullptr;
    ID3D11ShaderResourceView* apex = nullptr;

    ID3D11ShaderResourceView* call_of_duty_preview = nullptr;
    ID3D11ShaderResourceView* warcraft_preview = nullptr;
    ID3D11ShaderResourceView* valorant_preview = nullptr;
    ID3D11ShaderResourceView* cs_go_preview = nullptr;
    ID3D11ShaderResourceView* apex_preview = nullptr;

}

namespace font
{
    ImFont* roleway_widget = nullptr;
    ImFont* roleway_desc = nullptr;
    ImFont* roleway_name = nullptr;

    ImFont* icomoon = nullptr;
    ImFont* inter_medium = nullptr;

}

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hwnd;
RECT rc;

void move_window() {

    ImGui::SetCursorPos(ImVec2(0, 0));
    if (ImGui::InvisibleButton("Move_detector", ImVec2(c::bg::size)));
    if (ImGui::IsItemActive()) {

        GetWindowRect(hwnd, &rc);
        MoveWindow(hwnd, rc.left + ImGui::GetMouseDragDelta().x, rc.top + ImGui::GetMouseDragDelta().y, c::bg::size.x, c::bg::size.y, TRUE);
    }

}

void CricleProgress(const char* name, float progress, float max, float radius)
{
    static float tickness = 5.f;
    static float position = 0.f;
    static float alpha_text = 1.f;

    ImVec4 circle_loading = ImColor(0, 0, 0, 20);

    position = ImLerp(position, progress / max * 6.28f, ImGui::GetIO().DeltaTime * 100.f);

    ImVec2 vecCenter = { ImGui::GetContentRegionMax() / 2 };

    ImGui::GetForegroundDrawList()->PathClear();
    ImGui::GetForegroundDrawList()->PathArcTo(vecCenter, radius, 0.f, 2.f * IM_PI, 120.f);
    ImGui::GetForegroundDrawList()->PathStroke(ImGui::GetColorU32(circle_loading), 0, tickness);

    ImGui::GetForegroundDrawList()->PathClear();
    ImGui::GetForegroundDrawList()->PathArcTo(vecCenter, radius, IM_PI * 1.5f, IM_PI * 1.5f + position, 120.f);
    ImGui::GetForegroundDrawList()->PathStroke(ImGui::GetColorU32(c::accent), 0, tickness);

    int procent = progress / (int)max * 100;

    std::string procent_str = std::to_string(procent) + "%";

    ImGui::PushFont(font::roleway_widget);

    ImGui::GetForegroundDrawList()->AddText(vecCenter - ImGui::CalcTextSize(procent_str.c_str()) / 2, ImGui::GetColorU32(c::text::text_active), procent_str.c_str());

    alpha_text = ImLerp(alpha_text, procent > 80 ? 0.f : 1.f, ImGui::GetIO().DeltaTime * 6);

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha_text);
    if (procent != 0) ImGui::GetForegroundDrawList()->AddText(ImVec2((ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(name).x) / 2, ImGui::GetContentRegionMax().y - alpha_text * 120), ImGui::GetColorU32(c::text::text), name);
    ImGui::PopStyleVar();

    ImGui::PopFont();

}

void RenderBlur(HWND hwnd)
{
    struct ACCENTPOLICY
    {
        int na;
        int nf;
        int nc;
        int nA;
    };
    struct WINCOMPATTRDATA
    { 
        int na;
        PVOID pd;
        ULONG ul;
    };

    const HINSTANCE hm = LoadLibrary(L"user32.dll");
    if (hm)
    {
        typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

        const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hm, "SetWindowCompositionAttribute");
        if (SetWindowCompositionAttribute)
        {
            ACCENTPOLICY policy = { 4, 0, 155, 0 };
            WINCOMPATTRDATA data = { 19, &policy,sizeof(ACCENTPOLICY) };
            SetWindowCompositionAttribute(hwnd, &data);
        }
        FreeLibrary(hm);
    }
}

int rotation_start_index;
void ImRotateStart()
{
    rotation_start_index = ImGui::GetBackgroundDrawList()->VtxBuffer.Size;
}

ImVec2 ImRotationCenter()
{
    ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX);

    const auto& buf = ImGui::GetBackgroundDrawList()->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

    return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2);
}

void ImRotateEnd(float rad, ImVec2 center = ImRotationCenter())
{
    float s = sin(rad), c = cos(rad);
    center = ImRotate(center, s, c) - center;

    auto& buf = ImGui::GetBackgroundDrawList()->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
}

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

    WNDCLASSEXW wc;
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = nullptr;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = L"ImGui";
    wc.lpszClassName = L"Example";
    wc.hIconSm = LoadIcon(0, IDI_APPLICATION);

    RegisterClassExW(&wc);
    hwnd = CreateWindowExW(NULL, wc.lpszClassName, L"Example", WS_POPUP, (GetSystemMetrics(SM_CXSCREEN) / 2) - (c::bg::size.x / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (c::bg::size.y / 2), c::bg::size.x, c::bg::size.y, 0, 0, 0, 0);

    SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    RenderBlur(hwnd);

    POINT mouse;
    rc = { 0 };
    GetWindowRect(hwnd, &rc);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   

    ImFontConfig cfg;
    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor | ImGuiFreeTypeBuilderFlags_Bitmap;

    font::roleway_widget = io.Fonts->AddFontFromMemoryTTF(roleway_semibold, sizeof(roleway_semibold), 15.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::roleway_name = io.Fonts->AddFontFromMemoryTTF(roleway_bold, sizeof(roleway_bold), 40.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::roleway_desc = io.Fonts->AddFontFromMemoryTTF(roleway_medium, sizeof(roleway_medium), 13.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::icomoon = io.Fonts->AddFontFromMemoryTTF(icomoon, sizeof(icomoon), 18.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

    font::inter_medium = io.Fonts->AddFontFromMemoryTTF(inter_medium, sizeof(inter_medium), 13.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };
    if (image::logo == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logo, sizeof(logo), &info, pump, &image::logo, 0);

    if (image::close == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, close_icon, sizeof(close_icon), &info, pump, &image::close, 0);
    if (image::hide == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, hide_icon, sizeof(hide_icon), &info, pump, &image::hide, 0);

    if (image::discord == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, discord, sizeof(discord), &info, pump, &image::discord, 0);
    if (image::website == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, website, sizeof(website), &info, pump, &image::website, 0);
    if (image::telegram == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, telegram, sizeof(telegram), &info, pump, &image::telegram, 0);
    if (image::twitter == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, twitter, sizeof(twitter), &info, pump, &image::twitter, 0);

    if (image::call_of_duty == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, call_of_duty, sizeof(call_of_duty), &info, pump, &image::call_of_duty, 0);
    if (image::warcraft == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, warcraft, sizeof(warcraft), &info, pump, &image::warcraft, 0);
    if (image::valorant == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, valorant, sizeof(valorant), &info, pump, &image::valorant, 0);
    if (image::cs_go == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, cs_go, sizeof(cs_go), &info, pump, &image::cs_go, 0);
    if (image::apex == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, apex, sizeof(apex), &info, pump, &image::apex, 0);

    if (image::call_of_duty_preview == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, call_of_duty_preview, sizeof(call_of_duty_preview), &info, pump, &image::call_of_duty_preview, 0);
    if (image::warcraft_preview == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, warcraft_preview, sizeof(warcraft_preview), &info, pump, &image::warcraft_preview, 0);
    if (image::valorant_preview == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, valorant_preview, sizeof(valorant_preview), &info, pump, &image::valorant_preview, 0);
    if (image::cs_go_preview == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, cs_go_preview, sizeof(cs_go_preview), &info, pump, &image::cs_go_preview, 0);
    if (image::apex_preview == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, apex_preview, sizeof(apex_preview), &info, pump, &image::apex_preview, 0);

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGuiStyle* style = &ImGui::GetStyle();

        style->WindowPadding = ImVec2(0, 0);
        style->ItemSpacing = ImVec2(20, 20);
        style->WindowBorderSize = 0;
        style->ScrollbarSize = 8.f;

        ImGui::SetNextWindowSize(ImVec2(c::bg::size));
        ImGui::SetNextWindowPos(ImVec2(0, 0));

            ImGui::Begin("MONARCH", nullptr, ImGuiWindowFlags_NoResize);                          
            {
                const ImVec2& pos = ImGui::GetWindowPos();
                const ImVec2& spacing = style->ItemSpacing;

                ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(c::bg::size), ImGui::GetColorU32(c::bg::background), c::bg::rounding);

                ImRotateStart();

                ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(c::bg::size.x / 2 - 180, -50), ImVec2(c::bg::size.x / 2 - 150, c::bg::size.y), ImGui::GetColorU32(c::bg::line), c::bg::rounding);

                ImRotateEnd(-2.01f);

                ImGui::GetWindowDrawList()->AddImage(image::logo, ImVec2(40, 40), ImVec2(40 + 67, 40 + 79), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::accent));

                ImGui::GetWindowDrawList()->AddImage(image::close, ImVec2(c::bg::size.x - 80, 55), ImVec2(c::bg::size.x - 40 , 95), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::accent));
                ImGui::GetWindowDrawList()->AddImage(image::hide, ImVec2(c::bg::size.x - 120, 55), ImVec2(c::bg::size.x - 80, 95), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::text::text));

                if (GetAsyncKeyState(VK_LBUTTON & 0x01) && ImGui::IsMouseHoveringRect(ImVec2(c::bg::size.x - 80, 55), ImVec2(c::bg::size.x - 40, 95), true)) PostQuitMessage(0);
                if (GetAsyncKeyState(VK_LBUTTON & 0x01) && ImGui::IsMouseHoveringRect(ImVec2(c::bg::size.x - 120, 55), ImVec2(c::bg::size.x - 80, 95), true)) ShowWindow(hwnd, SW_MINIMIZE);


                ImGui::PushFont(font::roleway_name);
                ImGui::GetWindowDrawList()->AddText(ImVec2((c::bg::size.x - ImGui::CalcTextSize("CatHook").x) / 2, (232 - ImGui::CalcTextSize("CatHook").y) / 2), ImGui::GetColorU32(c::accent), "Cat");

                ImGui::GetWindowDrawList()->AddText(ImVec2((c::bg::size.x - ImGui::CalcTextSize("CatHook").x) / 2 + ImGui::CalcTextSize("Cat").x, (232 - ImGui::CalcTextSize("CatHook").y) / 2), ImGui::GetColorU32(c::text::text_active), "Hook");
                ImGui::PopFont();

                ImGui::PushFont(font::roleway_desc);
                ImGui::GetWindowDrawList()->AddText(ImVec2((c::bg::size.x - ImGui::CalcTextSize("Unleash the power of cathook").x) / 2, (232 - ImGui::CalcTextSize(" cathook").y) / 2 + 40), ImGui::GetColorU32(c::text::text), "Unleash the power of cathook!");
                ImGui::PopFont();

                static float tab_alpha = 0.f; /* */ static float tab_add; /* */ static int active_tab = 0;

                tab_alpha = ImClamp(tab_alpha + (4.f * ImGui::GetIO().DeltaTime * (page == active_tab ? 1.f : -1.f)), 0.f, 1.f);

                if (tab_alpha == 0.f && tab_add == 0.f) active_tab = page;

                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * style->Alpha);

                if (active_tab == 0)
                {
                    ImGui::SetCursorPos(ImVec2((c::bg::size.x - 353), c::bg::size.y - (150 + spacing.y * 2)) / 2);
                    ImGui::BeginGroup();
                    {

                        static char login[24] = { "" };
                        ImGui::InputTextEx("a", "Login", login, 25, ImVec2(353, 50), NULL);

                        static char password[24] = { "" };
                        ImGui::InputTextEx("b", "Password", password, 25, ImVec2(353, 50), NULL);

                        ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((353 - ImGui::CalcTextSize("Forgot password? Reset").x) / 2, 0));
                        ImGui::TextColored(ImColor(ImGui::GetColorU32(c::text::text)), "Forgot password?");
                        ImGui::SameLine(0, 5);
                        ImGui::TextColored(ImColor(ImGui::GetColorU32(c::accent)), "Reset");
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);

                        if (ImGui::CustomButton("LAUNCH", "d", ImVec2(353, 50)) && login == login_correct && password == pass_correct) {
                            page = 2;
                        }
                        else if (ImGui::IsItemHovered() && ImGui::IsItemClicked()) {
                            timer_invalid = 0;
                        }

                        timer_invalid += 6.f * ImGui::GetIO().DeltaTime;

                        static float alpha_text = 0.f;
                        alpha_text = ImLerp(alpha_text, timer_invalid < 7 && login != login_correct && password != pass_correct ? 1.f : 0.f, ImGui::GetIO().DeltaTime * 8.f);

                        ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((353 - ImGui::CalcTextSize("The password is incorrect").x) / 2, 0));
                        ImGui::TextColored(ImColor(ImGui::GetColorU32(c::red, alpha_text)), "The password is incorrect");

                    }
                    ImGui::EndGroup();

                }
                else if (active_tab == 1)
                {
                    ImGui::SetCursorPos(ImVec2((c::bg::size.x - 353), c::bg::size.y - (200 + spacing.y * 2)) / 2);
                    ImGui::BeginGroup();
                    {

                        static char login[24] = { "" };
                        ImGui::InputTextEx("a", "Login", login, 25, ImVec2(353, 50), NULL);

                        static char password[24] = { "" };
                        ImGui::InputTextEx("b", "Password", password, 25, ImVec2(353, 50), NULL);

                        static char email[24] = { "" };
                        ImGui::InputTextEx("c", "Email address", email, 25, ImVec2(353, 50), NULL);

                        if (ImGui::CustomButton("REGISTER", "d", ImVec2(353, 50)));

                    }
                    ImGui::EndGroup();
                }
                
                if (active_tab < 2){
                ImGui::SetCursorPos(ImVec2(((c::bg::size.x - (200 + spacing.x * 3)) / 2), c::bg::size.y - 160));
                ImGui::BeginGroup();
                {

                    if (ImGui::icon_box(image::discord, ImVec2(50, 50), ImVec2(24, 24), ImGui::GetColorU32(c::widget::background)));
                    ImGui::SameLine();
                    if (ImGui::icon_box(image::website, ImVec2(50, 50), ImVec2(24, 24), ImGui::GetColorU32(c::widget::background)));
                    ImGui::SameLine();
                    if (ImGui::icon_box(image::telegram, ImVec2(50, 50), ImVec2(24, 24), ImGui::GetColorU32(c::widget::background)));
                    ImGui::SameLine();
                    if (ImGui::icon_box(image::twitter, ImVec2(50, 50), ImVec2(24, 24), ImGui::GetColorU32(c::widget::background)));

                    if (ImGui::Page(0 == page, "SIGN IN", ImVec2(120, 40), NULL)) page = 0;
                    ImGui::SameLine();
                    if (ImGui::Page(1 == page, "SIGN UP", ImVec2(120, 40), NULL)) page = 1;

                }
                ImGui::EndGroup();
                }

                if (active_tab == 2)
                {

                    if (timer > 500) {
                        page = 3;
                    }

                    else if (page == 2) timer++;

                    if (tab_alpha < 0.05f) timer = 0;

                    CricleProgress("The download is underway, stay tuned.", timer, 500, 80);
                 
                    ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(c::bg::size), ImGui::GetColorU32(c::accent, 0.f), ImGui::GetColorU32(c::accent, 0.f), ImGui::GetColorU32(c::accent, 0.1f), ImGui::GetColorU32(c::accent, 0.1f), c::bg::rounding);

                }
                else if (active_tab == 3)
                {

                    ImGui::SetCursorPos(ImVec2(spacing.x, 242));

                    ImGui::BeginGroup();
                    {

                        ImGui::BeginChild("SELECTOR", ImVec2((c::bg::size.x - spacing.x * 3) / 2, c::bg::size.y - (242 + spacing.y)));
                        {

                     //       if (ImGui::Selector(0 == selector, image::your_image, "Your Game name", ImVec2(size)) selector = 0;

                         

                            if (ImGui::Selector(0 == selector, image::call_of_duty, "Fortnite", ImVec2(ImGui::GetContentRegionMax().x - style->WindowPadding.x, 55))) selector = 0;

                        }
                        ImGui::EndChild();

                    }
                    ImGui::EndGroup();

                    ImGui::SameLine();

                    ImGui::BeginGroup();
                    {
                        ImGui::BeginChild("PROFILE", ImVec2((c::bg::size.x - spacing.x * 3) / 2, c::bg::size.y - (242 + spacing.y)));
                        {
                            ImTextureID textures[1] = {image::call_of_duty_preview};
                            const char* game[1] = {"Fortnite Chapter 5"};
                            const char* status[1] = { "UNDETECTED"};
                            const char* version[1] = { "1.0.4"};
                            const char* update[1] = { "22.12.2023"};

                            static float tab_alpha = 0.f; /* */ static float tab_add; /* */ static int active_tab = 0;

                            tab_alpha = ImClamp(tab_alpha + (8.f * ImGui::GetIO().DeltaTime * (selector == active_tab ? 1.f : -1.f)), 0.f, 1.f);

                            if (tab_alpha == 0.f && tab_add == 0.f) active_tab = selector;

                            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * style->Alpha);

                            ImGui::GetWindowDrawList()->AddImageRounded(textures[active_tab], ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + ImVec2(ImGui::GetContentRegionMax().x - style->WindowPadding.x, 140), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::text::text_active), c::widget::rounding);

                            ImGui::SetCursorPos(ImVec2(spacing.x, 160));

                            ImGui::status_list("GAME", game[active_tab], ImVec2(ImGui::GetContentRegionMax().x - style->WindowPadding.x, 40), NULL);

                            ImGui::status_list("STATUS", status[active_tab], ImVec2(ImGui::GetContentRegionMax().x - style->WindowPadding.x, 40), NULL);

                            ImGui::status_list("VERSION", version[active_tab], ImVec2(ImGui::GetContentRegionMax().x - style->WindowPadding.x, 40), NULL);

                            ImGui::status_list("UPDATED", update[active_tab], ImVec2(ImGui::GetContentRegionMax().x - style->WindowPadding.x, 40), NULL);

                            ImGui::PopStyleVar();

                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 26);

                            if (ImGui::CustomButton("INJECT", "d", ImVec2(ImGui::GetContentRegionMax().x - style->WindowPadding.x, 40))) page = 2;

                        }
                        ImGui::EndChild();
                    }
                    ImGui::EndGroup();
                }

                ImGui::PopStyleVar();


                move_window();
            }
            ImGui::End();

        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK) return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
