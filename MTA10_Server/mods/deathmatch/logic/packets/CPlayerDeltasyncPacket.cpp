/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/packets/CPlayerDeltasyncPacket.cpp
*  PURPOSE:     Player delta synchronization packet class
*  DEVELOPERS:  Alberto Alonso <rydencillo@gmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "net/SyncStructures.h"

extern CGame* g_pGame;

bool CPlayerDeltasyncPacket::Read ( NetBitStreamInterface& BitStream )
{
    if ( m_pSourceElement )
    {
        CPlayer * pSourcePlayer = static_cast < CPlayer * > ( m_pSourceElement );
        SPlayerDeltaSyncData& delta = pSourcePlayer->GetReceivedDeltaSyncData ();
        bool bChanged;

        delta.vehicle.lastWasVehicleSync = false;

        // Read out the delta context
        unsigned char ucDeltaContext = 0;
        if ( !BitStream.ReadBits ( reinterpret_cast<char *>(&ucDeltaContext), SPlayerDeltaSyncData::DELTA_CONTEXT_BITCOUNT ) )
            return false;
        if ( ! delta.IsValidDelta ( ucDeltaContext ) )
            return false;

        // Read out the time context
        unsigned char ucTimeContext = 0;
        if ( !BitStream.Read ( ucTimeContext ) )
            return false;
        delta.lastSyncTimeContext = ucTimeContext;

        // Only read this packet if it matches the current time context that
        // player is in.
        if ( !pSourcePlayer->CanUpdateSync ( ucTimeContext ) )
        {
            return false;
        }

        // Read out keys
        if ( !BitStream.ReadBit ( bChanged ) )
            return false;

        CControllerState ControllerState = delta.lastControllerState;
        if ( bChanged )
        {
            ReadFullKeysync ( ControllerState, BitStream );
            pSourcePlayer->GetPad ()->NewControllerState ( ControllerState );
            delta.lastControllerState = ControllerState;
        }

        // Read the flags
        SPlayerPuresyncFlags flags;
        if ( !BitStream.ReadBit ( bChanged ) )
            return false;
        if ( bChanged )
        {
            if ( !BitStream.Read ( &flags ) )
                return false;

            pSourcePlayer->SetInWater ( flags.data.bIsInWater );
            pSourcePlayer->SetOnGround ( flags.data.bIsOnGround );
            pSourcePlayer->SetHasJetPack ( flags.data.bHasJetPack );
            pSourcePlayer->SetDucked ( flags.data.bIsDucked );
            pSourcePlayer->SetWearingGoggles ( flags.data.bWearsGoogles );
            pSourcePlayer->SetChoking ( flags.data.bIsChoking );
            pSourcePlayer->SetAkimboArmUp ( flags.data.bAkimboTargetUp );
            pSourcePlayer->SetOnFire ( flags.data.bIsOnFire );
            pSourcePlayer->SetStealthAiming ( flags.data.bStealthAiming );

            delta.lastFlags = flags;
        }
        else
            flags = delta.lastFlags;

        // Contact element
        CElement* pContactElement = NULL;
        ElementID Temp = delta.lastContact;
        if ( !BitStream.ReadBit ( bChanged ) )
            return false;
        if ( bChanged )
        {
            if ( !BitStream.Read ( Temp ) )
                return false;
            delta.lastContact = Temp;
        }

        if ( Temp != INVALID_ELEMENT_ID )
            pContactElement = CElementIDs::GetElement ( Temp );

        CElement * pPreviousContactElement = pSourcePlayer->GetContactElement ();
        pSourcePlayer->SetContactElement ( pContactElement );

        if ( pPreviousContactElement != pContactElement )
        {
            // Call our onPlayerContact event
            CLuaArguments Arguments;
            if ( pPreviousContactElement )
                Arguments.PushElement ( pPreviousContactElement );
            else
                Arguments.PushNil ();
            if ( pContactElement )
                Arguments.PushElement ( pContactElement );
            else
                Arguments.PushNil ();

            pSourcePlayer->CallEvent ( "onPlayerContact", Arguments );
        }

        // Player position
        SDeltaPositionSync position ( delta.lastPosition );
        if ( !BitStream.Read ( &position ) )
            return false;
        delta.lastPosition = position.data.vecPosition;

        if ( pContactElement )
        {
            pSourcePlayer->SetContactPosition ( position.data.vecPosition );

            // Get the true position
            CVector vecTempPos = pContactElement->GetPosition ();
            position.data.vecPosition += vecTempPos;
        }
        pSourcePlayer->SetPosition ( position.data.vecPosition );

        // Player rotation
        SPedRotationSync rotation;
        if ( !BitStream.Read ( &rotation ) )
            return false;
        pSourcePlayer->SetRotation ( rotation.data.fRotation );

        // Move speed vector
        if ( flags.data.bSyncingVelocity )
        {
            SVelocitySync velocity;
            if ( !BitStream.Read ( &velocity ) )
                return false;
            pSourcePlayer->SetVelocity ( velocity.data.vecVelocity );
        }

        if ( !BitStream.ReadBit ( bChanged ) )
            return false;

        float fHealth = delta.lastHealth;
        float fArmorLoss = 0.0f;
        if ( bChanged )
        {
            // Health ( stored with damage )
            SPlayerHealthSync health;
            if ( !BitStream.Read ( &health ) )
                return false;
            fHealth = health.data.fValue;

            // Armor
            SPlayerArmorSync armor;
            if ( !BitStream.Read ( &armor ) )
                return false;

            float fArmor = armor.data.fValue;
            float fOldArmor = pSourcePlayer->GetArmor ();
            fArmorLoss = fOldArmor - fArmor;

            pSourcePlayer->SetArmor ( fArmor );

            delta.lastHealth = fHealth;
            delta.lastArmor = fArmor;
        }

        // Read out and set the camera rotation
        float fCameraRotation;
        if ( !BitStream.Read ( fCameraRotation ) )
            return false;
        pSourcePlayer->SetCameraRotation ( fCameraRotation );

        if ( flags.data.bHasAWeapon )
        {
            if ( !BitStream.ReadBit ( bChanged ) )
                return false;

            bool bWeaponCorrect = true;
            unsigned int uiSlot = delta.weapon.lastSlot;
            // Read client weapon data, but only apply it if the weapon matches with the server
            uchar ucUseWeaponType = pSourcePlayer->GetWeaponType ();

            if ( bChanged )
            {
                // Check client has the weapon we think he has
                unsigned char ucClientWeaponType;
                if ( !BitStream.Read ( ucClientWeaponType ) )
                    return false;

                if ( pSourcePlayer->GetWeaponType () != ucClientWeaponType )
                {
                    bWeaponCorrect = false;                 // Possibly old weapon data.
                    ucUseWeaponType = ucClientWeaponType;   // Use the packet supplied weapon type to skip over the correct amount of data
                }

                // Update check counts
                pSourcePlayer->SetWeaponCorrect ( bWeaponCorrect );

                // Current weapon slot
                SWeaponSlotSync slot;
                if ( !BitStream.Read ( &slot ) )
                    return false;
                uiSlot = slot.data.uiSlot;

                // Set weapon slot
                if ( bWeaponCorrect )
                    pSourcePlayer->SetWeaponSlot ( uiSlot );
                
                delta.weapon.lastWeaponType = ucUseWeaponType;
                delta.weapon.lastSlot = uiSlot;
            }

            if ( CWeaponNames::DoesSlotHaveAmmo ( uiSlot ) )
            {
                if ( !BitStream.ReadBit ( bChanged ) )
                    return false;

                unsigned short usAmmoInClip = delta.weapon.lastAmmoInClip;
                unsigned short usTotalAmmo = delta.weapon.lastAmmoTotal;
                if ( bChanged )
                {
                    // Read out the ammo states
                    SWeaponAmmoSync ammo ( ucUseWeaponType, true, true );
                    if ( !BitStream.Read ( &ammo ) )
                        return false;
                    usAmmoInClip = ammo.data.usAmmoInClip;
                    usTotalAmmo = ammo.data.usTotalAmmo;
                    delta.weapon.lastAmmoInClip = usAmmoInClip;
                    delta.weapon.lastAmmoTotal = usTotalAmmo;
                }

                // Read out the aim data
                SWeaponAimSync sync ( CWeaponNames::GetWeaponRange ( ucUseWeaponType ), ( ControllerState.RightShoulder1 || ControllerState.ButtonCircle ) );
                if ( !BitStream.Read ( &sync ) )
                    return false;

                if ( bWeaponCorrect )
                {
                    // Set the ammo states
                    pSourcePlayer->SetWeaponAmmoInClip ( usAmmoInClip );
                    pSourcePlayer->SetWeaponTotalAmmo ( usTotalAmmo );

                    // Set the arm directions and whether or not arms are up
                    pSourcePlayer->SetAimDirection ( sync.data.fArm );

                    // Read the aim data only if he's shooting or aiming
                    if ( sync.isFull() )
                    {
                        pSourcePlayer->SetSniperSourceVector ( sync.data.vecOrigin );
                        pSourcePlayer->SetTargettingVector ( sync.data.vecTarget );
                    }
                }
            }
            else
            {
                if ( bWeaponCorrect )
                {
                    pSourcePlayer->SetWeaponAmmoInClip ( 1 );
                    pSourcePlayer->SetWeaponTotalAmmo ( 1 );
                }
            }
        }
        else
        {
            pSourcePlayer->SetWeaponSlot ( 0 );
            pSourcePlayer->SetWeaponAmmoInClip ( 1 );
            pSourcePlayer->SetWeaponTotalAmmo ( 1 );
        }

        // Read out damage info if changed
        if ( BitStream.ReadBit () == true )
        {
            ElementID DamagerID;
            if ( !BitStream.Read ( DamagerID ) )
                return false;

            SWeaponTypeSync weaponType;
            if ( !BitStream.Read ( &weaponType ) )
                return false;

            SBodypartSync bodyPart;
            if ( !BitStream.Read ( &bodyPart ) )
                return false;

            pSourcePlayer->SetDamageInfo ( DamagerID, weaponType.data.ucWeaponType, bodyPart.data.uiBodypart );
        }

        // If we know the player's dead, make sure the health we send on is 0
        if ( pSourcePlayer->IsDead () )
            fHealth = 0.0f;

        float fOldHealth = pSourcePlayer->GetHealth ();
        float fHealthLoss = fOldHealth - fHealth;
        pSourcePlayer->SetHealth ( fHealth );

        // Less than last packet's frame?
        if ( fHealthLoss > 0 || fArmorLoss > 0 )
        {
            float fDamage = 0.0f;
            if ( fHealthLoss > 0 ) fDamage += fHealthLoss;
            if ( fArmorLoss > 0 ) fDamage += fArmorLoss;

            // Call the onPlayerDamage event
            CLuaArguments Arguments;
            CElement* pKillerElement = CElementIDs::GetElement ( pSourcePlayer->GetPlayerAttacker () );
            if ( pKillerElement ) Arguments.PushElement ( pKillerElement );
            else Arguments.PushNil ();
            Arguments.PushNumber ( pSourcePlayer->GetAttackWeapon () );
            Arguments.PushNumber ( pSourcePlayer->GetAttackBodyPart () );
            Arguments.PushNumber ( fDamage );

            pSourcePlayer->CallEvent ( "onPlayerDamage", Arguments );
        }

        // Success
        return true;
    }

    return false;
}

bool CPlayerDeltasyncPacket::Write ( NetBitStreamInterface& BitStream ) const
{
    if ( m_pSourceElement )
    {
        CPlayer * pSourcePlayer = static_cast < CPlayer * > ( m_pSourceElement );
        const SPlayerDeltaSyncData& delta = m_delta;
        SPlayerDeltaSyncData& newDelta = pSourcePlayer->GetSentDeltaSyncData ();

        newDelta.vehicle.lastWasVehicleSync = false;

        ElementID PlayerID = pSourcePlayer->GetID ();
        unsigned short usLatency = pSourcePlayer->GetPing ();
        const CControllerState& ControllerState = pSourcePlayer->GetPad ()->GetCurrentControllerState ();
        CElement* pContactElement = pSourcePlayer->GetContactElement ();

        // Get current weapon slot
        unsigned char ucWeaponSlot = pSourcePlayer->GetWeaponSlot ();

        // Flags
        SPlayerPuresyncFlags flags;
        flags.data.bIsInWater       = ( pSourcePlayer->IsInWater () == true );
        flags.data.bIsOnGround      = ( pSourcePlayer->IsOnGround () == true );
        flags.data.bHasJetPack      = ( pSourcePlayer->HasJetPack () == true );
        flags.data.bIsDucked        = ( pSourcePlayer->IsDucked () == true );
        flags.data.bWearsGoogles    = ( pSourcePlayer->IsWearingGoggles () == true );
        flags.data.bHasContact      = ( pContactElement != NULL );
        flags.data.bIsChoking       = ( pSourcePlayer->IsChoking () == true );
        flags.data.bAkimboTargetUp  = ( pSourcePlayer->IsAkimboArmUp () == true );
        flags.data.bIsOnFire        = ( pSourcePlayer->IsOnFire () == true );
        flags.data.bHasAWeapon      = ( ucWeaponSlot != 0 );
        flags.data.bSyncingVelocity = ( !flags.data.bIsOnGround || pSourcePlayer->IsSyncingVelocity () );
        flags.data.bStealthAiming   = ( pSourcePlayer->IsStealthAiming () == true );

        CVector vecPosition = pSourcePlayer->GetPosition ();
        if ( pContactElement )
            pSourcePlayer->GetContactPosition ( vecPosition );
        float fCameraRotation = pSourcePlayer->GetCameraRotation ();

        BitStream.Write ( PlayerID );

        // Write the delta context
        BitStream.WriteBits ( reinterpret_cast<const char*>(&delta.deltaSyncContext), SPlayerDeltaSyncData::DELTA_CONTEXT_BITCOUNT );

        // Write the time context
        BitStream.Write ( pSourcePlayer->GetSyncTimeContext () );
        newDelta.lastSyncTimeContext = pSourcePlayer->GetSyncTimeContext ();

        if ( usLatency != delta.lastLatency )
        {
            BitStream.WriteBit ( true );
            BitStream.WriteCompressed ( usLatency );
        }
        else
            BitStream.WriteBit ( false );
        newDelta.lastLatency = usLatency;

        if ( ControllerState != delta.lastControllerState )
        {
            BitStream.WriteBit ( true );
            WriteFullKeysync ( ControllerState, BitStream );
        }
        else
            BitStream.WriteBit ( false );
        newDelta.lastControllerState = ControllerState;

        if ( flags != delta.lastFlags )
        {
            BitStream.WriteBit ( true );
            BitStream.Write ( &flags );
        }
        else
            BitStream.WriteBit ( false );
        newDelta.lastFlags = flags;

        if ( flags.data.bHasContact )
        {
            if ( delta.lastContact != pContactElement->GetID() )
            {
                BitStream.WriteBit ( true );
                BitStream.Write ( pContactElement->GetID () );
                newDelta.lastContact = pContactElement->GetID ();
            }
            else
                BitStream.WriteBit ( false );
        }
        else
        {
            if ( delta.lastContact != INVALID_ELEMENT_ID )
            {
                BitStream.WriteBit ( true );
                BitStream.Write ( ElementID ( INVALID_ELEMENT_ID ) );
                newDelta.lastContact = INVALID_ELEMENT_ID;
            }
            else
                BitStream.WriteBit ( false );
        }
            

        SDeltaPositionSync position ( delta.lastPosition );
        position.data.vecPosition = vecPosition;
        BitStream.Write ( &position );
        newDelta.lastPosition = vecPosition;

        SPedRotationSync rotation;
        rotation.data.fRotation = pSourcePlayer->GetRotation ();
        BitStream.Write ( &rotation );

        if ( flags.data.bSyncingVelocity )
        {
            SVelocitySync velocity;
            pSourcePlayer->GetVelocity ( velocity.data.vecVelocity );
            BitStream.Write ( &velocity );
        }

        // Player health and armor
        if ( fabs ( delta.lastHealth - pSourcePlayer->GetHealth() ) > 0.01 ||
             fabs ( delta.lastArmor - pSourcePlayer->GetArmor() ) > 0.01 )
        {
            BitStream.WriteBit ( true );

            SPlayerHealthSync health;
            health.data.fValue = pSourcePlayer->GetHealth ();
            BitStream.Write ( &health );
            newDelta.lastHealth = pSourcePlayer->GetHealth ();

            SPlayerArmorSync armor;
            armor.data.fValue = pSourcePlayer->GetArmor ();
            BitStream.Write ( &armor );
            newDelta.lastArmor = pSourcePlayer->GetArmor ();
        }
        else
            BitStream.WriteBit ( false );

        BitStream.Write ( fCameraRotation );

        if ( flags.data.bHasAWeapon )
        {
            unsigned int uiSlot = ucWeaponSlot;
            if ( uiSlot != delta.weapon.lastSlot )
            {
                BitStream.WriteBit ( true );

                SWeaponSlotSync slot;
                slot.data.uiSlot = uiSlot;
                BitStream.Write ( &slot );

                newDelta.weapon.lastSlot = ucWeaponSlot;
            }
            else
                BitStream.WriteBit ( false );

            if ( CWeaponNames::DoesSlotHaveAmmo ( uiSlot ) )
            {
                unsigned short usWeaponAmmoInClip = pSourcePlayer->GetWeaponAmmoInClip ();

                if ( delta.weapon.lastAmmoInClip != usWeaponAmmoInClip )
                {
                    BitStream.WriteBit ( true );

                    SWeaponAmmoSync ammo ( pSourcePlayer->GetWeaponType (), false, true );
                    ammo.data.usAmmoInClip = usWeaponAmmoInClip;
                    BitStream.Write ( &ammo );

                    newDelta.weapon.lastAmmoInClip = usWeaponAmmoInClip;
                }
                else
                    BitStream.WriteBit ( false );

                SWeaponAimSync aim ( 0.0f, ( ControllerState.RightShoulder1 || ControllerState.ButtonCircle ) );
                aim.data.fArm = pSourcePlayer->GetAimDirection ();

                // Write the aim data only if he's aiming or shooting
                if ( aim.isFull() )
                {
                    aim.data.vecOrigin = pSourcePlayer->GetSniperSourceVector ();
                    pSourcePlayer->GetTargettingVector ( aim.data.vecTarget );
                }
                BitStream.Write ( &aim );
            }
        }

        // Success
        return true;
    }

    return false;
}

void CPlayerDeltasyncPacket::PrepareToSendDeltaSync ()
{
    if ( m_pSourceElement )
    {
        CPlayer * pSourcePlayer = static_cast < CPlayer * > ( m_pSourceElement );
        m_delta = pSourcePlayer->GetSentDeltaSyncData ();
    }
}