#include <Geode/Geode.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>
#include <Geode/modify/CCScheduler.hpp>

using namespace geode::prelude;

template <typename T>
T getCachedSettingValue(std::string name) {
    static std::unordered_map<std::string, T> cache;
    static std::mutex mutex;

    {
        std::lock_guard lock(mutex);
        if (!cache.contains(name)) {
            T val = Mod::get()->getSettingValue<T>(name);
            cache[name] = val;

            listenForSettingChangesV3<T>(name, [name](T newVal) {
            std::lock_guard lock(mutex);
            cache[name] = newVal;
            });
        }
        return cache[name];
    }
}

float globalFPS;
class $modify(CCScheduler) {
    void update(float dt) {
        globalFPS = 1 / dt;
        CCScheduler::update(dt);
    }
};

class $modify(FMODAudioEngine) {
    virtual void update(float delta) {
        bool modToggle = getCachedSettingValue<bool>("actual-toggle");
        if (!modToggle)
            return FMODAudioEngine::update(delta);

        bool forceVal = getCachedSettingValue<bool>("toggle");
        float simFPS;
        if (getCachedSettingValue<bool>("toggle"))
            simFPS = static_cast<float>(getCachedSettingValue<int>("sim-fps"));
        else
            simFPS = globalFPS;

        float quotient = simFPS / 60;

        int fullTicks = static_cast<int>(quotient);
        float remainder = quotient - fullTicks;
        for (int i = 0; i < fullTicks; i++)
            FMODAudioEngine::update(delta);

        if (remainder > 0.f)
            FMODAudioEngine::update(delta * remainder);
    }
};