/* (c) 2020 KAI OS TECHNOLOGIES (HONG KONG) LIMITED All rights reserved. This
 * file or any portion thereof may not be reproduced or used in any manner
 * whatsoever without the express written permission of KAI OS TECHNOLOGIES
 * (HONG KONG) LIMITED. KaiOS is the trademark of KAI OS TECHNOLOGIES (HONG
 * KONG) LIMITED or its affiliate company and may be registered in some
 * jurisdictions. All other trademarks are the property of their respective
 * owners.
 */


#include <android/hardware/ISensorPrivacyListener.h>
#include <android/hardware/ISensorPrivacyManager.h>
#include <binder/BinderService.h>
#include <binder/IPCThreadState.h>
#include <binder/IProcessInfoService.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>

namespace android {

using binder::Status;

class BnSensorPrivacyService : public BnInterface<hardware::ISensorPrivacyManager> {
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0) = 0;
};

class FakeSensorPrivacyService :
    public BinderService<FakeSensorPrivacyService>,
    public BnSensorPrivacyService {
public:
    FakeSensorPrivacyService();
    virtual ~FakeSensorPrivacyService();
    static const char *getServiceName() { return "sensor_privacy"; }

    virtual Status addSensorPrivacyListener(const sp<hardware::ISensorPrivacyListener>& listener);
    virtual Status removeSensorPrivacyListener(const sp<hardware::ISensorPrivacyListener>& listener);
    virtual Status isSensorPrivacyEnabled(bool* _aidl_return);
    virtual Status setSensorPrivacy(bool enable);

    virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                uint32_t flags) override;

};

FakeSensorPrivacyService::FakeSensorPrivacyService()
 : BnSensorPrivacyService() {
}

FakeSensorPrivacyService::~FakeSensorPrivacyService() {
}

Status
FakeSensorPrivacyService::addSensorPrivacyListener(const sp<hardware::ISensorPrivacyListener>& listener) {
    (void)listener;

    return Status::ok();
}

Status
FakeSensorPrivacyService::removeSensorPrivacyListener(const sp<hardware::ISensorPrivacyListener>& listener) {
    (void)listener;

    return Status::ok();
}

Status
FakeSensorPrivacyService::isSensorPrivacyEnabled(bool* _aidl_return) {
    /* Do not enable sensor privacy for b2g*/
    *_aidl_return = false;

    return Status::ok();
}

Status
FakeSensorPrivacyService::setSensorPrivacy(bool enable) {
    (void)enable;

    return Status::ok();
}

status_t
FakeSensorPrivacyService::onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                   uint32_t flags) {
    switch (code) {
        case IBinder::FIRST_CALL_TRANSACTION + 0: /* addSensorPrivacyListener */ {
            CHECK_INTERFACE(hardware::ISensorPrivacyManager, data, reply);
            sp<hardware::ISensorPrivacyListener> listener;
            data.readStrongBinder(&listener);
            Status status(addSensorPrivacyListener(listener));
            return status.writeToParcel(reply);
        }
        case IBinder::FIRST_CALL_TRANSACTION + 1: /* removeSensorPrivacyListener */ {
            CHECK_INTERFACE(hardware::ISensorPrivacyManager, data, reply);
            sp<hardware::ISensorPrivacyListener> listener;
            data.readStrongBinder(&listener);
            Status status(removeSensorPrivacyListener(listener));
            return status.writeToParcel(reply);
        }
        case IBinder::FIRST_CALL_TRANSACTION + 2: /* isSensorPrivacyEnabled */ {
            CHECK_INTERFACE(hardware::ISensorPrivacyManager, data, reply);
            bool enabled;
            Status status(isSensorPrivacyEnabled(&enabled));
            status_t result = status.writeToParcel(reply);
            reply->writeBool(enabled);
            return result;
        }
        case IBinder::FIRST_CALL_TRANSACTION + 3: /* setSensorPrivacy */ {
            CHECK_INTERFACE(hardware::ISensorPrivacyManager, data, reply);
            bool enabled;
            data.readBool(&enabled);
            Status status(setSensorPrivacy(enabled));
            return status.writeToParcel(reply);
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

class BnProcessInfoService : public BnInterface<IProcessInfoService> {
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0) = 0;
};

class GonkProcessInfoService :
    public BinderService<GonkProcessInfoService>,
    public BnProcessInfoService {
public:
    GonkProcessInfoService();
    virtual ~GonkProcessInfoService();
    static const char *getServiceName() { return "processinfo"; }

    virtual status_t getProcessStatesFromPids( size_t length,
                                        /*in*/ int32_t* pids,
                                       /*out*/ int32_t* states) override;

    virtual status_t getProcessStatesAndOomScoresFromPids( size_t length,
                                                    /*in*/ int32_t* pids,
                                                   /*out*/ int32_t* states,
                                                   /*out*/ int32_t* scores) override;

    virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                uint32_t flags) override;
};

GonkProcessInfoService::GonkProcessInfoService()
  : BnProcessInfoService() {
}

GonkProcessInfoService::~GonkProcessInfoService() {
}

status_t
GonkProcessInfoService::getProcessStatesFromPids( size_t length,
                                           /*in*/ int32_t* pids,
                                          /*out*/ int32_t* states) {
    return getProcessStatesAndOomScoresFromPids(length, pids, states, nullptr);
}

#define PROCESS_STATE_UNKNOWN -1;
#define PROCESS_STATE_TOP_SLEEPING 13;
#define PROCESS_STATE_NONEXISTENT 19;
status_t
GonkProcessInfoService::getProcessStatesAndOomScoresFromPids( size_t length,
                                                       /*in*/ int32_t* pids,
                                                      /*out*/ int32_t* states,
                                                      /*out*/ int32_t* scores) {
    // sanity check for parameters
    if (length <= 0 || !pids || !states) {
        return -ENOSYS;
    }

    for (int i = 0; i < length; i++) {
        FILE *fp;
        char cmd[80], result[80];
        { /* handle process states */
            sprintf(cmd, "cat proc/%d/stat", pids[i]);
            /* Open the command for reading. */
            fp = popen(cmd, "r");
            if (fp == NULL) {
                return -ENOSYS;
            }

            /* Read the output a argument each time - output it. */
            fscanf(fp, "%s", result); /* process id, skip */
            fscanf(fp, "%s", result); /* process name, skip */
            while(!strchr(result, ')')) { /* keep finding ')' for the end of name */
                fscanf(fp, "%s", result); /* process name, skip */
            }
            fscanf(fp, "%s", result); /* process state */
            states[i] = PROCESS_STATE_NONEXISTENT;
            if (result[0] != '\0') {
                switch(result[0]) {
                    case 'S':
                        states[i] = PROCESS_STATE_TOP_SLEEPING;
                        break;
                    default:
                        states[i] = PROCESS_STATE_UNKNOWN;
                }
            }
            pclose(fp);
        }

        if (scores) { /* handle process oom_score */
            sprintf(cmd, "cat proc/%d/oom_adj", pids[i]);
            /* Open the command for reading. */
            fp = popen(cmd, "r");
            if (fp == NULL) {
                return -ENOSYS;
            }

            /* Read the output a argument each time - output it. */
            fscanf(fp, "%s", result); /* process oom_score */
            if (result[0] != '\0') {
                scores[i] = atoi(result);
            }
            pclose(fp);
        }
    }

    return NO_ERROR;
}

status_t
GonkProcessInfoService::onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                   uint32_t flags) {
    switch (code) {
        case GET_PROCESS_STATES_FROM_PIDS: {
            CHECK_INTERFACE(IProcessInfoService, data, reply);
            int32_t length = data.readInt32();
            if (length <= 0) {
                return NO_ERROR;
            }
            int32_t pids[length];
            status_t result = data.read((void*)pids, length * sizeof(int32_t));
            if (result != NO_ERROR) {
                return result;
            }
            int32_t statesLength = data.readInt32();
            if (length != statesLength) {
                return -ENOSYS;
            }
            int32_t states[length];
            result = getProcessStatesFromPids(length, pids, states);
            if (result != NO_ERROR) {
                return result;
            }
            reply->writeNoException();
            reply->writeInt32Array(statesLength, states);
            reply->writeInt32(result);
            return NO_ERROR;
            break;
        }
        case GET_PROCESS_STATES_AND_OOM_SCORES_FROM_PIDS: {
            CHECK_INTERFACE(IProcessInfoService, data, reply);
            int32_t length = data.readInt32();
            if (length <= 0) {
                return NO_ERROR;
            }
            int32_t pids[length];
            status_t result = data.read((void*)pids, length * sizeof(int32_t));
            if (result != NO_ERROR) {
                return result;
            }
            int32_t statesLength = data.readInt32();
            int32_t scoresLength = data.readInt32();
            if (length != statesLength || statesLength != scoresLength) {
                return -ENOSYS;
            }
            int32_t states[length], scores[length];
            result = getProcessStatesAndOomScoresFromPids(length, pids, states, scores);
            if (result != NO_ERROR) {
                return result;
            }
            reply->writeNoException();
            reply->writeInt32Array(statesLength, states);
            reply->writeInt32Array(scoresLength, scores);
            reply->writeInt32(result);
            return NO_ERROR;
            break;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    android::FakeSensorPrivacyService::instantiate();
    android::GonkProcessInfoService::instantiate();
    android::ProcessState::self()->startThreadPool();
    android::IPCThreadState::self()->joinThreadPool();
    return 0;
}
