constexpr uint32_t INGAMECLOCK_API_VERSION = 1;

struct IngameClockAPI
{
    uint32_t version;

    void (*ShowClock)();
    void (*HideClock)();
    void (*ToggleClock)();
    void (*RefreshSettings)();
    void (*SetClockPosition)(float x, float y, bool save);
    void (*SetClockColor)(const char* color, bool save);
    bool (*HasControlsDisableExternally)();
    void (*SetControlDisabler)(const char* modName, bool disable);
    float (*GetClockScale)();
    void (*SetClockScale)(float scale, bool save);
    const char* (*GetControllingModName)();
    bool (*IsClockVisible)();
};

// exported function name and signature
extern "C" __declspec(dllexport) IngameClockAPI* RequestIngameClockAPI();