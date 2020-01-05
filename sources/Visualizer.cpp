#include "engine/tPixelGameEngine.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "NES.h"
#include "Visualizer.h"

using namespace std;

Visualizer::Visualizer(NES* nes) :
  m_nes(nes),
  m_redColor(1,0,0,1),
  m_greenColor(0,1,0,1),
  m_darkGrayColor(0.6f, 0.6f, 0.6f, 1)
{
  sAppName = "NES_Emulator";

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  if (Construct(400, 300, 2, 2))
    Start();
}

bool Visualizer::OnUserCreate()
{
  // Setup Platform/Renderer bindings
  ImGui_ImplWin32_Init(GetHWND());
  ImGui_ImplDX11_Init(GetDevice(), GetContext());

  return true;
}

bool Visualizer::OnUserUpdate(float fElapsedTime)
{
  DrawString(6, 6, "TUCNA", tDX::WHITE);
  DrawString(5, 5, "TUCNA", tDX::RED);

  return true;
}

bool Visualizer::OnUserUpdateEndFrame(float fElapsedTime)
{
  auto print = [&](bool flag)
  {
    flag ? ImGui::TextColored(m_greenColor, "Yes") : ImGui::TextColored(m_redColor, "No");
  };

  const Cpu& cpu = m_nes->GetCpu();

  // Start the Dear ImGui frame
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("CPU information");
  ImGui::Text("Bits");
  ImGui::Text("0 : Carry bit"); ImGui::SameLine(); print(cpu.GetB0());
  ImGui::Text("1 : Zero"); ImGui::SameLine(); print(cpu.GetB1());
  ImGui::Text("2 : Disable Interrupts"); ImGui::SameLine(); print(cpu.GetB2());
  ImGui::Text("3 : Decimal Mode"); ImGui::SameLine(); print(cpu.GetB3());
  ImGui::Text("4 : Break"); ImGui::SameLine(); print(cpu.GetB4());
  ImGui::Text("5 : UNUSED");
  ImGui::Text("6 : Overflow"); ImGui::SameLine(); print(cpu.GetB6());
  ImGui::Text("7 : Negative"); ImGui::SameLine(); print(cpu.GetB7());
  ImGui::Separator();
  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, 120);
  ImGui::Text("Program counter"); ImGui::NextColumn(); ImGui::TextColored(m_darkGrayColor, "$0000"); ImGui::NextColumn();
  ImGui::Text("Stack pointer"); ImGui::NextColumn(); ImGui::TextColored(m_darkGrayColor, "$0000"); ImGui::NextColumn();
  ImGui::Text("Accumulator"); ImGui::NextColumn(); ImGui::TextColored(m_darkGrayColor, "$0000"); ImGui::NextColumn();
  ImGui::Text("Register X"); ImGui::NextColumn(); ImGui::TextColored(m_darkGrayColor, "$0000"); ImGui::NextColumn();
  ImGui::Text("Register Y"); ImGui::NextColumn(); ImGui::TextColored(m_darkGrayColor, "$0000"); ImGui::NextColumn();
  ImGui::End();

  PrepareDisassembledCode(5);
  auto middleDis = m_disassembledCode.begin() + m_disassembledCode.size() / 2;

  ImGui::Begin("Disassembly");
  for (auto it = m_disassembledCode.begin(); it < middleDis; it++)
    ImGui::TextColored(m_darkGrayColor, (*it).data());
  ImGui::Separator();
  ImGui::Text((*middleDis++).data());
  ImGui::Separator();
  for (auto it = middleDis; it < m_disassembledCode.end(); it++)
    ImGui::TextColored(m_darkGrayColor, (*it).data());
  ImGui::End();

  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  return true;
}

bool Visualizer::OnUserDestroy()
{
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  return true;
}

void Visualizer::PrepareDisassembledCode(uint8_t lines)
{
  m_disassembledCode .clear();

  const Cpu& cpu = m_nes->GetCpu();
  const std::map<uint16_t, std::string>& assembly = m_nes->GetAssembly();

  auto it = assembly.find(cpu.GetProgramCounter());

  advance(it, -lines);

  if (it != assembly.end())
  {
    m_disassembledCode.push_back((*it).second);

    for (int i = 0; i < lines * 2; i++)
    {
      if (++it != assembly.end())
        m_disassembledCode.push_back((*it).second);
    }
  }
}
