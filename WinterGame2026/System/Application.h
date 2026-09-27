#pragma once
class Application
{
public:
	~Application();
	static Application& GetInstance();

	bool Init();

	void Run();

	void Terminate();

	void RequestExit();

private:
	Application();
	Application(const Application& app) = delete;
	void operator=(const Application& app) = delete;
	//ゲームの終了がリクエスト
	bool m_IsRequstendExit = false;

};

