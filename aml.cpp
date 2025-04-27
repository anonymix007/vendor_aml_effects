#include <cstring>

#include <android/binder_manager.h>
#define LOG_TAG "AML"
#include <log/log.h>

#define DEFAULT_INSTANCE "android.hardware.audio.effect.IFactory/default"
#define HOOKED_INSTANCE  "android.hardware.audio.effect.IFactory/aml"

extern "C" AIBinder* AMLServiceManager_waitForService(const char* instance) {
    ALOGI("%s: %s requested", __func__, instance);
    if (instance && strcmp(instance, DEFAULT_INSTANCE) == 0) {
        ALOGI("%s: return %s instead", __func__, HOOKED_INSTANCE);
        return AServiceManager_waitForService(HOOKED_INSTANCE);
    }
    return AServiceManager_waitForService(instance);
}
