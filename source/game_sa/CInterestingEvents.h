#pragma once

#include "CEntity.h"

struct TInterestingEvent {
    int32_t type;
    int32_t time;
    CEntity* entity;
};

class CInterestingEvents {
    CInterestingEvents* Constructor();
    CInterestingEvents* Destructor();

public:
    TInterestingEvent m_events[8];
    int8_t byte60[29];
    int16_t word7E[29];
    int32_t dwordB8[29];
    int8_t byte12C;
    int8_t gap12D[3];
    int32_t m_nLastFrameUpdate;
    uint32_t m_nLastScanTimeUpdate;
    float dword138;
    CVector vec13C[2];
    char byte154;

    CInterestingEvents();
    ~CInterestingEvents();

    enum EType {

    };

    void Add(EType type, CEntity* entity);
    void ScanForNearbyEntities();
    void GetInterestingEvent();
    void InvalidateEvent(const TInterestingEvent* event);
    void InvalidateNonVisibleEvents();

    static void InjectHooks();
};

extern CInterestingEvents& g_InterestingEvents;
