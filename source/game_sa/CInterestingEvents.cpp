#include "StdInc.h"

CInterestingEvents& g_InterestingEvents = *(CInterestingEvents*)0xC0B058;

void CInterestingEvents::InjectHooks() {
    ReversibleHooks::Install("CInterestingEvents", "Constructor", 0x6023A0, &CInterestingEvents::Constructor);
    ReversibleHooks::Install("CInterestingEvents", "Destructor", 0x856880, &CInterestingEvents::Destructor);
    //    ReversibleHooks::Install("CInterestingEvents", "Add", 0x602590, &CInterestingEvents::Add);
    //    ReversibleHooks::Install("CInterestingEvents", "ScanForNearbyEntities", 0x605A30, &CInterestingEvents::ScanForNearbyEntities);
    ReversibleHooks::Install("CInterestingEvents", "GetInterestingEvent", 0x6028A0, &CInterestingEvents::GetInterestingEvent);
    ReversibleHooks::Install("CInterestingEvents", "InvalidateEvent", 0x602960, &CInterestingEvents::InvalidateEvent);
    ReversibleHooks::Install("CInterestingEvents", "InvalidateNonVisibleEvents", 0x6029C0, &CInterestingEvents::InvalidateNonVisibleEvents);
}

// 0x6023A0
CInterestingEvents::CInterestingEvents() {
    byte12C = byte12C & 0xFE | 0xE;
    dword138 = 30.f;
    m_nLastScanTimeUpdate = 0;
    m_nInterestingEvent = -1;
    m_nLastFrameUpdate = CTimer::m_FrameCounter - 1;
    memset(m_events, 0, sizeof(m_events));
    memset(m_nEndsOfTime, 0, sizeof(m_nEndsOfTime));

#define SET_OPTIONS(index, priority, delay, end)                                                                                                                                   \
    m_nPrioritys[index] = priority;                                                                                                                                                \
    m_nDelays[index] = delay;                                                                                                                                                      \
    m_nEndsOfTime[index] = end

    SET_OPTIONS(0, 5, 2000, 0);
    SET_OPTIONS(1, 1, 5000, 0);
    SET_OPTIONS(2, 1, 5000, 0);
    SET_OPTIONS(3, 1, 5000, 0);
    SET_OPTIONS(4, 2, 3000, 0);
    SET_OPTIONS(5, 2, 3000, 0);
    SET_OPTIONS(6, 2, 3000, 0);
    SET_OPTIONS(7, 2, 3000, 0);
    SET_OPTIONS(8, 4, 3000, 0);
    SET_OPTIONS(9, 4, 3000, 0);
    SET_OPTIONS(10, 5, 6000, 0);
    SET_OPTIONS(11, 6, 6000, 0);
    SET_OPTIONS(12, 6, 8000, 0);
    SET_OPTIONS(13, 6, 5000, 0);
    SET_OPTIONS(14, 5, 6000, 0);
    SET_OPTIONS(15, 9, 6000, 0);
    SET_OPTIONS(16, 9, 6000, 0);
    SET_OPTIONS(17, 8, 6000, 0);
    SET_OPTIONS(18, 7, 6000, 0);
    SET_OPTIONS(19, 6, 5000, 0);
    SET_OPTIONS(20, 7, 6000, 0);
    SET_OPTIONS(21, 8, 8000, 0);
    SET_OPTIONS(22, 9, 5000, 0);
    SET_OPTIONS(23, 9, 6000, 0);
    SET_OPTIONS(24, 9, 6000, 0);
    SET_OPTIONS(25, 9, 6000, 0);
    SET_OPTIONS(26, 9, 8000, 0);
    SET_OPTIONS(27, 10, 4000, 0);
    SET_OPTIONS(28, 10, 4000, 0);

#undef SET_OPTIONS
}

CInterestingEvents* CInterestingEvents::Constructor() {
    this->CInterestingEvents::CInterestingEvents();
    return this;
}

// 0x856880
CInterestingEvents::~CInterestingEvents() {
    for (int8_t i = 0; i < MAX_INTERESTING_EVENTS; i++) {
        TInterestingEvent& e = g_InterestingEvents.m_events[i];
        if (e.entity) {
            e.entity->CleanUpOldReference(&e.entity);
            e.entity = nullptr;
        }
    }
}

CInterestingEvents* CInterestingEvents::Destructor() {
    this->CInterestingEvents::~CInterestingEvents();
    return this;
}

// 0x602590
void CInterestingEvents::Add(CInterestingEvents::EType type, CEntity* entity) {
    plugin::CallMethod<0x602590, CInterestingEvents*, CInterestingEvents::EType, CEntity*>(this, type, entity);
}

// 0x605A30
void CInterestingEvents::ScanForNearbyEntities() {
    plugin::CallMethod<0x605A30, CInterestingEvents*>(this);
}

// 0x6028A0
TInterestingEvent* CInterestingEvents::GetInterestingEvent() {
    uint32_t start = CTimer::m_snTimeInMilliseconds, end = CTimer::m_snTimeInMilliseconds;
    if (!(byte12C & 4) && m_nInterestingEvent != -1)
        return nullptr;

    TInterestingEvent* result = &m_events[m_nInterestingEvent];
    if (result->entity && CTimer::m_snTimeInMilliseconds < result->time + static_cast<uint32_t>(m_nDelays[result->type]))
        return result;

    // update
    uint8_t prevpriority = 0;
    int8_t interesting = -1;
    for (int8_t i = 0; i < MAX_INTERESTING_EVENTS; i++, start = end) {
        TInterestingEvent& e = m_events[i];
        if (e.entity && start < e.time + static_cast<uint32_t>(m_nDelays[result->type]) && m_nPrioritys[e.type] > prevpriority || static_cast<uint16_t>(rand()) < 0x80) {
            prevpriority = m_nPrioritys[e.type];
            interesting = i;
        }
    }
    m_nInterestingEvent = interesting;

    if (interesting == -1)
        return nullptr;
    return result;
}

// 0x602960
void CInterestingEvents::InvalidateEvent(const TInterestingEvent* event) {
    for (int8_t index = 0; index < MAX_INTERESTING_EVENTS; index++) {
        TInterestingEvent& e = m_events[index];
        if (&e != event)
            continue;

        e.time = 0;
        if (e.entity) {
            e.entity->CleanUpOldReference(&e.entity);
            e.entity = nullptr;
        }

        if (m_nInterestingEvent == index)
            m_nInterestingEvent = -1;
    }
}

// 0x6029C0
void CInterestingEvents::InvalidateNonVisibleEvents() {
    CVector source = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecSource;
    for (int8_t i = 0; i < MAX_INTERESTING_EVENTS; i++) {
        TInterestingEvent& e = m_events[i];
        if (!e.entity)
            continue;

        CVector pos = e.entity->GetPosition();
        if (CWorld::GetIsLineOfSightClear(source, pos, true, false, false, false, false, true, false))
            continue;
        e.time = 0;
        e.entity->CleanUpOldReference(&e.entity);
        if (m_nInterestingEvent == i)
            m_nInterestingEvent = -1;
    }
}
