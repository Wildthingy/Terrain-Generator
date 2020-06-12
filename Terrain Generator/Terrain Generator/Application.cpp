#include "Application.h"
#include "Terrain.h"

Application* Application::s_pApp = NULL;

const int CAMERA_MAP = 0;
const int CAMERA_MAX = 4;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool Application::HandleStart()
{
	s_pApp = this;

	this->SetWindowTitle("Terrain Generator");

	m_bWireframe = false;

	width = 100;
	length = 100;
	iterations = 100;
	method = 0;
	snowHeight = 0.2;

	m_pTerrain = new Terrain(width, length, iterations, method, snowHeight);

	m_cameraZ = 50.0f;
	m_rotationAngle = 0.f;

	if(!this->CommonApp::HandleStart())
		return false;

	this->SetRasterizerState(false, m_bWireframe);

	m_cameraState = CAMERA_MAP;

	ImGui_ImplDX11_Init(this->m_hWnd, this->m_pD3DDevice, this->m_pD3DDeviceContext);

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleStop()
{
	delete m_pTerrain;

	ImGui_ImplDX11_Shutdown();

	this->CommonApp::HandleStop();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleUpdate()
{
	m_rotationAngle += .002f;

	if(m_cameraState == CAMERA_MAP)
	{
		if(this->IsKeyPressed('Q'))
			m_cameraZ -= 2.0f;

		if(this->IsKeyPressed('A'))
			m_cameraZ += 2.0f;
	}

	static bool dbC = false;

	if(this->IsKeyPressed('C'))
	{
		if(!dbC)
		{
			if(++m_cameraState == CAMERA_MAX)
				m_cameraState = CAMERA_MAP;

			dbC = true;
		}
	}
	else
	{
		dbC = false;
	}

	static bool dbW = false;
	if(this->IsKeyPressed('W'))
	{
		if(!dbW)
		{
			m_bWireframe = !m_bWireframe;
			this->SetRasterizerState(false, m_bWireframe);
			dbW = true;
		}
	}
	else
	{
		dbW = false;
	}

	if (regenerating)
	{
		m_pTerrain = new Terrain(width, length, iterations, method, snowHeight);
		regenerating = false;
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleRender()
{
	ImGui_ImplDX11_NewFrame();

	XMFLOAT3 vUpVector(0.0f, 1.0f, 0.0f);
	XMFLOAT3 vCamera, vLookat;
	
	vCamera = XMFLOAT3(sin(m_rotationAngle) * m_cameraZ, m_cameraZ / 4, cos(m_rotationAngle) * m_cameraZ);
	vLookat = XMFLOAT3(0.0f, 4.0f, 0.0f);

	XMMATRIX matView;
	matView = XMMatrixLookAtLH(XMLoadFloat3(&vCamera), XMLoadFloat3(&vLookat), XMLoadFloat3(&vUpVector));

	XMMATRIX matProj;
	matProj = XMMatrixPerspectiveFovLH(float(XM_PI / 4), 2, 1.5f, 5000.0f);

	this->SetViewMatrix(matView);
	this->SetProjectionMatrix(matProj);

	this->EnablePointLight(0, XMFLOAT3(100.0f, 100.f, -100.f), XMFLOAT3(1.f, 1.f, 1.f));
	this->SetLightAttenuation(0, 200.f, 2.f, 2.f, 2.f);
	this->EnableDirectionalLight(1, XMFLOAT3(-1.f, -1.f, -1.f), XMFLOAT3(0.65f, 0.55f, 0.65f));

	this->Clear(XMFLOAT4(.2f, .2f, .6f, 1.f));

	XMMATRIX matWorld;
	matWorld = XMMatrixIdentity();
	this->SetWorldMatrix(matWorld);

	m_pTerrain->Draw();

	ImGui::Text("Terrain Size:");
	ImGui::InputInt("Width", &width, 10, 1000);
	ImGui::InputInt("Length", &length, 10, 1000);

	ImGui::Separator();

	ImGui::Text("\nGeneration Algorithm:");
	ImGui::RadioButton("Fault", &method, 0); ImGui::SameLine();
	ImGui::RadioButton("Circles", &method, 1); ImGui::SameLine();
	ImGui::RadioButton("Particles (x10 iter advised)", &method, 2);
	ImGui::InputInt("Iterations", &iterations, 100, 1000);

	ImGui::Separator();

	ImGui::Text("\nOther:");
	ImGui::SliderFloat("Snow Height", &snowHeight, 0.001f, 0.5f);

	ImGui::Separator();

	ImGui::Text("\n");
	if (ImGui::Button("Regenerate Terrain"))
		regenerating = true;
	ImGui::Text("\n");

	ImGui::Separator();

	ImGui::Text("\nCamera Controls:");
	ImGui::Text("Move forwards:    Q");
	ImGui::Text("Move back:        A");
	ImGui::Text("Toggle Wireframe: W");

	// Render dear imgui into screen
	ImGui::Render();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	Application application;

	Run(&application);

	return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
