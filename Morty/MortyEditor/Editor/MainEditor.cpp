#include "MainEditor.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


#include "MDirectX11Renderer.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "MObject.h"
#include "MMaterialResource.h"
#include "MMaterial.h"
#include "MMeshInstance.h"
#include "MMesh.h"

#include "Matrix.h"

MainEditor::MainEditor()
	: m_pMeshInstance(nullptr)
{

}

MainEditor::~MainEditor()
{

}

bool MainEditor::Initialize(MEngine* pEngine, const char* svWindowName)
{

	if (!MWindowsRenderView::Initialize(pEngine, svWindowName))
		return false;

// 	// Setup Dear ImGui context
// 	IMGUI_CHECKVERSION();
// 	ImGui::CreateContext();
// 	ImGuiIO& io = ImGui::GetIO(); (void)io;
// 	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
// 	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
// 
// 	// Setup Dear ImGui style
// 	ImGui::StyleColorsDark();
// 	//ImGui::StyleColorsClassic();
// 
// 	InitImGUI();
// 	InitImGUIDX11();


	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	MDirectX11Renderer* pRenderer = dynamic_cast<MDirectX11Renderer*>(m_pEngine->GetRenderer());

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(GetHWND());
	ImGui_ImplDX11_Init(pRenderer->GetDevice(), pRenderer->GetContext());


}

void MainEditor::Release()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();


	MWindowsRenderView::Release();
}

void MainEditor::OnRenderEnd()
{
// 	ImGuiIO& io = ImGui::GetIO();
// 	io.DisplaySize = ImVec2(GetViewWidth(), GetViewHeight());
// 
// 	// Start the Dear ImGui frame
// 	ImGui::NewFrame();
// 
// 	//Draw GUI
// 	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
// 	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
// 	ImGui::End();
// 
// 
// 
// 
// 	// Rendering
// 	ImGui::Render();
// 	DrawGUI(ImGui::GetDrawData());


	// Our state
	static bool show_demo_window = true;
	static bool show_another_window = false;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());



}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainEditor::ViewProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
		return true;

	return MWindowsRenderView::ViewProcessFunction(hwnd, message, wParam, lParam);
}

void MainEditor::DrawGUI(class ImDrawData* pDrawData)
{
	


	if (MMesh<ImDrawVert>* pMesh = dynamic_cast<MMesh<ImDrawVert>*>(m_pMeshInstance->GetMesh()))
	{

		pMesh->CreateVertices(pDrawData->TotalVtxCount);
		pMesh->CreateIndices(pDrawData->TotalIdxCount, 1);

		ImDrawVert* pVtx = pMesh->GetVertices();
		unsigned int* pItx = pMesh->GetIndices();
		for (int n = 0; n < pDrawData->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = pDrawData->CmdLists[n];
			memcpy(pVtx, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(pItx, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			pVtx += cmd_list->VtxBuffer.Size;
			pItx += cmd_list->IdxBuffer.Size;
		}

		pMesh->SetNeedUpload();

	}

	
	float L = pDrawData->DisplayPos.x;
	float R = pDrawData->DisplayPos.x + pDrawData->DisplaySize.x;
	float T = pDrawData->DisplayPos.y;
	float B = pDrawData->DisplayPos.y + pDrawData->DisplaySize.y;
	Matrix4 projMat(
		 2.0f / (R - L), 0.0f, 0.0f, 0.0f ,
		 0.0f, 2.0f / (T - B), 0.0f, 0.0f ,
		 0.0f, 0.0f, 0.5f, 0.0f ,
		 (R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f
	);


	for (MShaderParam& param : m_pMeshInstance->GetMaterial()->GetVertexShaderParams())
	{
		if (param.strName == "vertexBuffer")
		{
			param.var.GetStruct()->SetMember("ProjectionMatrix", projMat);
		}
	}
}

void MainEditor::InitImGUI()
{
	// Setup back-end capabilities flags
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
	io.BackendPlatformName = "imgui_impl_win32";
	io.ImeWindowHandle = GetHWND();

	// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
	io.KeyMap[ImGuiKey_Tab] = VK_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Space] = VK_SPACE;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
	io.KeyMap[ImGuiKey_A] = 'A';
	io.KeyMap[ImGuiKey_C] = 'C';
	io.KeyMap[ImGuiKey_V] = 'V';
	io.KeyMap[ImGuiKey_X] = 'X';
	io.KeyMap[ImGuiKey_Y] = 'Y';
	io.KeyMap[ImGuiKey_Z] = 'Z';
}

bool MainEditor::InitImGUIDX11()
{
	if (nullptr == m_pEngine)
		return false;

	MDirectX11Renderer* pRenderer = dynamic_cast<MDirectX11Renderer*>(m_pEngine->GetRenderer());
	if (nullptr == pRenderer)
		return false;

	ID3D11Device* pd3dDevice = pRenderer->GetDevice();
	if (nullptr == pd3dDevice)
		return false;


	MResource* pGUIVertexShader = m_pEngine->GetResourceManager()->Load("./Resource/Shader/ImGuiShader.mvs");
	MResource* pGUIPixelShader = m_pEngine->GetResourceManager()->Load("./Resource/Shader/ImGuiShader.mps");
	MMaterialResource* pMaterialRes = dynamic_cast<MMaterialResource*>(m_pEngine->GetResourceManager()->Create(MResourceManager::MEResourceType::Material));
	pMaterialRes->LoadVertexShader(pGUIVertexShader);
	pMaterialRes->LoadPixelShader(pGUIPixelShader);
	MMaterial* pMaterial = m_pEngine->GetObjectManager()->CreateObject<MMaterial>();
	pMaterial->Load(pMaterialRes);


	m_pMeshInstance = m_pEngine->GetObjectManager()->CreateObject<MMeshInstance>();

	m_pMeshInstance->SetMaterial(pMaterial);

	MMesh<ImDrawVert>* pMesh = new MMesh<ImDrawVert>();
	m_pMeshInstance->SetMesh(pMesh);

// 	// Create the blending setup
// 	{
// 		D3D11_BLEND_DESC desc;
// 		ZeroMemory(&desc, sizeof(desc));
// 		desc.AlphaToCoverageEnable = false;
// 		desc.RenderTarget[0].BlendEnable = true;
// 		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
// 		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
// 		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
// 		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
// 		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
// 		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
// 		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
// 		g_pd3dDevice->CreateBlendState(&desc, &g_pBlendState);
// 	}
// 	// Create depth-stencil State
// 	{
// 		D3D11_DEPTH_STENCIL_DESC desc;
// 		ZeroMemory(&desc, sizeof(desc));
// 		desc.DepthEnable = false;
// 		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
// 		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
// 		desc.StencilEnable = false;
// 		desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
// 		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
// 		desc.BackFace = desc.FrontFace;
// 		g_pd3dDevice->CreateDepthStencilState(&desc, &g_pDepthStencilState);
// 	}

//	ImGui_ImplDX11_CreateFontsTexture();

	return true;
}
