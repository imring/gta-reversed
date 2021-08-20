#pragma once

#include "CEntity.h"

#define MAX_INTERESTING_EVENTS 8
#define MAX_INTERESTING_EVENT_TYPES 29

struct TInterestingEvent {
    int32_t type;
    uint32_t time;
    CEntity* entity;
};

class CInterestingEvents {
    CInterestingEvents* Constructor();
    CInterestingEvents* Destructor();

public:
    TInterestingEvent m_events[MAX_INTERESTING_EVENTS];
    uint8_t m_nPrioritys[MAX_INTERESTING_EVENT_TYPES]; // not sure
    uint16_t m_nDelays[MAX_INTERESTING_EVENT_TYPES]; // delays for update (e.g. GetInterestingEvent)
    uint32_t m_nEndsOfTime[MAX_INTERESTING_EVENT_TYPES];
    int8_t byte12C;
    int8_t gap12D[3];
    uint32_t m_nLastFrameUpdate;
    uint32_t m_nLastScanTimeUpdate;
    float dword138;
    CVector vec13C;
    CVector vec148;
    int8_t m_nInterestingEvent;

    CInterestingEvents();
    ~CInterestingEvents();

    enum EType {

    };

    void Add(EType type, CEntity* entity);
    void ScanForNearbyEntities();
    TInterestingEvent* GetInterestingEvent();
    void InvalidateEvent(const TInterestingEvent* event);
    void InvalidateNonVisibleEvents();

    static void InjectHooks();
};

extern CInterestingEvents& g_InterestingEvents;
