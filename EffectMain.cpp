/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "effectFactory-impl/EffectFactory.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <android/binder_stability.h>
#include <system/audio_config.h>

#include <cstdlib>

__attribute__((weak)) extern "C" void _ZNSt3__122__libcpp_verbose_abortEPKcz(void) {
    abort();
}

/* WARNING: this function is not available for vendor partition, so patching libbinder_ndk.map.txt is required */
extern "C" void AIBinder_forceDowngradeToSystemStability(AIBinder* binder);

/** Default name of effect configuration file. */
static const char* kDefaultConfigName = "audio_effects_config.xml";

int main(int argc, const char **argv) {
    // This is a debug implementation, always enable debug logging.
    android::base::SetMinimumLogSeverity(::android::base::DEBUG);
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    std::string configFile;
    if (argc > 1 && access(argv[1], R_OK) == 0) {
        configFile = std::string(argv[1]);
    } else {
        configFile = android::audio_find_readable_configuration_file(kDefaultConfigName);
    }

    if (configFile == "") {
        LOG(ERROR) << __func__ << ": config file " << kDefaultConfigName << " not found!";
        return EXIT_FAILURE;
    }

    std::string implInstance = std::string() + aidl::android::hardware::audio::effect::IFactory::descriptor + "/default";

    auto implBinder = AServiceManager_waitForService(implInstance.c_str());
    if (implBinder == nullptr) {
        LOG(ERROR) << __func__ << ": impl instance " << implInstance << " is not available";
        return EXIT_FAILURE;
    }

    LOG(DEBUG) << __func__ << ": start factory with configFile: " << configFile;
    LOG(DEBUG) << __func__ << ": start factory with impl instance: " << implInstance;

    auto impl = aidl::android::hardware::audio::effect::IFactory::fromBinder(ndk::SpAIBinder(implBinder));
    auto effectFactory = ndk::SharedRefBase::make<aidl::android::hardware::audio::effect::Factory>(configFile, impl);

    std::string serviceName = std::string() + effectFactory->descriptor + "/aml";
    auto effectFactoryBinder = effectFactory->asBinder();

    AIBinder_forceDowngradeToSystemStability(effectFactoryBinder.get());

    binder_status_t status = AServiceManager_addService(effectFactoryBinder.get(), serviceName.c_str());
    CHECK_EQ(STATUS_OK, status);

    LOG(DEBUG) << __func__ << ": effectFactory: " << serviceName << " start";
    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
