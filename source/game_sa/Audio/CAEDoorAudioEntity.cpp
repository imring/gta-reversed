#include "StdInc.h"

#include "CAEDoorAudioEntity.h"

#include "CAEAudioHardware.h"
#include "CAESoundManager.h"

// 0x447030
CAEDoorAudioEntity::CAEDoorAudioEntity() : CAEAudioEntity() {
    m_nTime = 0;
    m_nMovingDoorTime = 0;
    m_nField84 = 0;
}

// 0x5B9A80
void CAEDoorAudioEntity::StaticInitialise() {
    AEAudioHardware.LoadSoundBank(51, 31);
}

// 0x4DC6B0
void CAEDoorAudioEntity::Reset() {
    m_nTime = 0;
    m_nMovingDoorTime = 0;
    AESoundManager.CancelSoundsOwnedByAudioEntity(this, true);
}

// 0x4DC9F0
void CAEDoorAudioEntity::AddAudioEvent(eAudioEvents event, CPhysical* physical) {
    if (event == AE_ENTRY_EXIT_DOOR_MOVING &&
        !AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(AE_ENTRY_EXIT_DOOR_MOVING, this) &&
        CTimer::m_snTimeInMilliseconds > m_nMovingDoorTime + 4000
    ) {
        PlayDoorSound(1, AE_ENTRY_EXIT_DOOR_MOVING, physical->GetPosition(), 0.0f, 1.0f);
        m_nMovingDoorTime = CTimer::m_snTimeInMilliseconds;
    }
}

// 0x4DC860
void CAEDoorAudioEntity::AddAudioEvent(eAudioEvents event, CVector& posn, float volume, float speed) {
    switch (event) {
    case AE_GARAGE_DOOR_OPENING:
        if (AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(event, this)) {
            m_nTime = CTimer::m_snTimeInMilliseconds;
            break;
        }

        AESoundManager.CancelSoundsOwnedByAudioEntity(this, true);
        if (!(CTimer::m_snTimeInMilliseconds <= m_nTime + 300))
            PlayDoorSound(2, event, posn, volume, 0.79f);

        PlayDoorSound(0, event, posn, volume, speed);
        m_nTime = CTimer::m_snTimeInMilliseconds;
        break;
    case AE_GARAGE_DOOR_OPENED:
        if (!AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(event, this) &&
            AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(AE_GARAGE_DOOR_OPENING, this)
        ) {
            AESoundManager.CancelSoundsOwnedByAudioEntity(this, true);
            PlayDoorSound(2, event, posn, volume, 1.0f);
        }

        m_nTime = CTimer::m_snTimeInMilliseconds;
        break;
    case AE_GARAGE_DOOR_CLOSING:
        if (AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(event, this)) {
            m_nTime = CTimer::m_snTimeInMilliseconds;
            break;
        }

        if (!(CTimer::m_snTimeInMilliseconds <= m_nTime + 300))
            PlayDoorSound(2, event, posn, volume, 1.0f);

        PlayDoorSound(0, event, posn, volume, speed);
        m_nTime = CTimer::m_snTimeInMilliseconds;
        break;
    case AE_GARAGE_DOOR_CLOSED:
        if (!AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(event, this) &&
            AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(AE_GARAGE_DOOR_CLOSING, this)
        ) {
            AESoundManager.CancelSoundsOwnedByAudioEntity(this, true);
            PlayDoorSound(2, event, posn, volume, 0.79f);
        }
        m_nTime = CTimer::m_snTimeInMilliseconds;
        break;
    default:
        return;
    }
}

// 0x4DC6D0
void CAEDoorAudioEntity::PlayDoorSound(short sfxId, eAudioEvents event, CVector& posn, float volumeDelta, float speed) {
    if (AEAudioHardware.IsSoundBankLoaded(51, 31)) {
        CVector position;
        bool enabled = false;
        if (posn.x == -1000.0f && posn.y == -1000.0f && posn.z == -1000.0f ||
            posn.x == 0.0f && posn.y == 0.0f && posn.z == 0.0f
        ) {
            position.Set(0.0f, 1.0f, 0.0f);
            enabled = true;
        } else {
            position = posn;
        }

        const float eventVolume = CAEAudioEntity::m_pAudioEventVolumes[event];
        const float volume = eventVolume + volumeDelta;
        CAESound sound;
        sound.Initialise(31, sfxId, this, position, volume, 2.0f, speed, 1.0f, 0, SOUND_REQUEST_UPDATES, 0.0f, 0);
        sound.SetIndividualEnvironment(SOUND_FRONT_END, enabled);
        sound.m_nEvent = event;
        AESoundManager.RequestNewSound(&sound);
    } else {
        StaticInitialise();
    }
}

// 0x4DCA60
void CAEDoorAudioEntity::UpdateParameters(CAESound* sound, short curPlayPos) {
    if (curPlayPos <= 0)
        return;

#ifdef ANDROID_CODE
    if (sound->m_nEvent == AE_GARAGE_DOOR_OPENING) {
        if (m_nTime + 10000 >= CTimer::m_snTimeInMilliseconds) {
            auto playing = AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(AE_GARAGE_DOOR_OPENED, this);
            if (playing)
                return sound->StopSoundAndForget();

            return;
        }
        AddAudioEvent(AE_GARAGE_DOOR_OPENED, sound->m_vecCurrPosn, 0.0f, 1.0f);
        return sound->StopSoundAndForget();
    }

    if (sound->m_nEvent != AE_GARAGE_DOOR_CLOSING)
        return;

    if (m_nTime + 10000 < CTimer::m_snTimeInMilliseconds) {
        AddAudioEvent(AE_GARAGE_DOOR_CLOSED, sound->m_vecCurrPosn, 0.0f, 1.0f);
        return sound->StopSoundAndForget();
    }

    auto playing = AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(AE_GARAGE_DOOR_CLOSED, this);
    if (playing)
        return sound->StopSoundAndForget();
#else
    eAudioEvents event;
    if (sound->m_nEvent == AE_GARAGE_DOOR_OPENING) {
        event = AE_GARAGE_DOOR_OPENED;
        if (CTimer::m_snTimeInMilliseconds > (m_nTime + 10000)) {
            auto playing = (
                !AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(AE_GARAGE_DOOR_OPENED, this) &&
                AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(AE_GARAGE_DOOR_OPENING, this)
            );
            if (playing) {
                AESoundManager.CancelSoundsOwnedByAudioEntity(this, 1);
                PlayDoorSound(2, AE_GARAGE_DOOR_OPENED, sound->m_vecCurrPosn, 0.0f, 1.0f);
            }
            m_nTime = CTimer::m_snTimeInMilliseconds;
            sound->StopSoundAndForget();
            return;
        }
    } else {
        if (sound->m_nEvent != AE_GARAGE_DOOR_CLOSING)
            return;

        event = AE_GARAGE_DOOR_CLOSED;
        if (CTimer::m_snTimeInMilliseconds > (m_nTime + 10000)) {
            auto playing = (
                !AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(AE_GARAGE_DOOR_CLOSED, this) &&
                AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(AE_GARAGE_DOOR_CLOSING, this)
            );
            if (playing) {
                AESoundManager.CancelSoundsOwnedByAudioEntity(this, 1);
                PlayDoorSound(2, AE_GARAGE_DOOR_CLOSED, sound->m_vecCurrPosn, 0.0f, 0.79f);
            }
            m_nTime = CTimer::m_snTimeInMilliseconds;
            sound->StopSoundAndForget();
            return;
        }
    }

    if (AESoundManager.AreSoundsOfThisEventPlayingForThisEntity(event, this))
        sound->StopSoundAndForget();
#endif
}

void CAEDoorAudioEntity::InjectHooks() {
    ReversibleHooks::Install("CAEDoorAudioEntity", "StaticInitialise", 0x5B9A80, &CAEDoorAudioEntity::StaticInitialise);
    ReversibleHooks::Install("CAEDoorAudioEntity", "Reset", 0x4DC6B0, &CAEDoorAudioEntity::Reset);
    ReversibleHooks::Install("CAEDoorAudioEntity", "AddAudioEvent_1", 0x4DC9F0, static_cast<void (CAEDoorAudioEntity::*)(eAudioEvents, CPhysical*)>(&CAEDoorAudioEntity::AddAudioEvent));
    ReversibleHooks::Install("CAEDoorAudioEntity", "AddAudioEvent_2", 0x4DC860, static_cast<void (CAEDoorAudioEntity::*)(eAudioEvents, CVector&, float, float)>(&CAEDoorAudioEntity::AddAudioEvent));
    ReversibleHooks::Install("CAEDoorAudioEntity", "PlayDoorSound", 0x4DC6D0, &CAEDoorAudioEntity::PlayDoorSound);
    ReversibleHooks::Install("CAEDoorAudioEntity", "UpdateParameters", 0x4DCA60, &CAEDoorAudioEntity::UpdateParameters_Reversed);
}

void CAEDoorAudioEntity::UpdateParameters_Reversed(CAESound* sound, short curPlayPos) {
    CAEDoorAudioEntity::UpdateParameters(sound, curPlayPos);
}