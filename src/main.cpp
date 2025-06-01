#include "extern/IngameClockAPI.h"

static const char* plugin_name = "ChaosClock";
typedef IngameClockAPI* (*_RequestIngameClockAPI)();

IngameClockAPI* g_clockAPI = nullptr;
void RegisterClockAPI()
{
	auto handle = REX::W32::GetModuleHandleW(L"ingame-clock.dll");
	if (!handle) {
		// Clock mod not loaded
		return;
	}

	auto requestAPI = (_RequestIngameClockAPI)GetProcAddress(handle, "RequestIngameClockAPI");
	if (!requestAPI)
		return;

	g_clockAPI = requestAPI();

	if (!g_clockAPI || g_clockAPI->version != INGAMECLOCK_API_VERSION) {
		// Version mismatch or failed fetch
		g_clockAPI = nullptr;
	}
}

struct Randomiser {
	static std::mt19937& GetRNG() {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		return gen;
	}

	static std::string GetRandomColorCode() {
		std::uniform_int_distribution<int> distrib(0x000000, 0xFFFFFF);
		std::stringstream ss;
		ss << std::uppercase << std::setfill('0') << std::setw(6)
			<< std::hex << distrib(GetRNG());
		return ss.str();
	}

	static float GetRandomFloat(float a_min, float a_max) {
		std::uniform_real_distribution<float> distrib(a_min, a_max);
		return std::roundf((distrib(GetRNG()) * 100)) / 100;
	}
};

class Timer
{
public:
	void Start() 
	{
		if (!running)
		{
			startTime = std::chrono::steady_clock::now();
			running = true;
		}
	};

	void Stop() 
	{
		running = false;
	};

	void Reset() 
	{
		running = false;
	};

	double ElapsedSeconds() 
	{
		if (!running)
			return 0.0; // If stopped, return 0
		auto now = std::chrono::steady_clock::now();
		return std::chrono::duration<double>(now - startTime).count();
	};

	bool IsRunning() const 
	{
		return running;
	};

private:
	std::chrono::steady_clock::time_point startTime{};
	bool running{false};
};

struct ChaosClock : public REX::Singleton<ChaosClock>
{
	Timer timerColorPos;
	Timer timerToggle;

	float lastX{ 500.0f };
	float lastY{ 500.0f };

	std::mt19937 rng{ std::random_device{}() };
	std::uniform_real_distribution<float> posDistX{ 0.0f, 1600.0f };
	std::uniform_real_distribution<float> posDistY{ 0.0f, 900.0f };
	std::uniform_int_distribution<int> colorDist{ 0, 255 };

	// These two will be called by your own mod's FrameUpdate hook
	void RunChaosMode()
	{
		if (!timerColorPos.IsRunning())
			timerColorPos.Start();

		if (timerColorPos.ElapsedSeconds() >= Randomiser::GetRandomFloat(0.5f, 1.5f)) {
			RandomisePositionAndColor();
			timerColorPos.Reset();
		}
	}

	// Internal randomisation logic for ChaosMode
	void RandomisePositionAndColor()
	{
		float newX = lastX;
		float newY = lastY;

		while (std::abs(newX - lastX) < 200.0f)
			newX = Randomiser::GetRandomFloat(0.0f, 1600.0f);
		while (std::abs(newY - lastY) < 200.0f)
			newY = Randomiser::GetRandomFloat(0.0f, 900.0f);

		lastX = newX;
		lastY = newY;
        const std::string colorStr = Randomiser::GetRandomColorCode();
		float new_scale = Randomiser::GetRandomFloat(1.0f, 4.0f);
        if (g_clockAPI) {
			g_clockAPI->SetClockScale(new_scale, true);
           g_clockAPI->SetClockColor(colorStr.c_str(), true);
           g_clockAPI->SetClockPosition(newX, newY, true);		   
        }		
	}
};

static bool was_disabled{false};

namespace OnFrameHook {

	
	static int32_t OnFrameUpdate(float a_delta) 
	{
		
		if (!was_disabled && g_clockAPI) {
			g_clockAPI->SetControlDisabler(plugin_name, true);
			was_disabled = true;
		}			

		ChaosClock::GetSingleton()->RunChaosMode();
		return int32_t(a_delta);
	};
	inline static REL::Hook func{ REL::ID(36564), 0xc26, OnFrameUpdate };
}

void InitListener(SKSE::MessagingInterface::Message* a_message) {

	if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {		
		RegisterClockAPI();
	}
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse, { .trampoline = true, .trampolineSize  = 14 });
	SKSE::GetMessagingInterface()->RegisterListener(InitListener);
	REX::INFO("Welcome to the Chaos");	
	return true;
}