/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        MTA10_Server/mods/deathmatch/logic/lua/CLuaFunctionParseHelpers.cpp
*  PURPOSE:
*  DEVELOPERS:  Nobody knows
*
*****************************************************************************/

#include "StdInc.h"

//
// enum values <-> script strings
//

IMPLEMENT_ENUM_BEGIN( eLuaType )
    ADD_ENUM ( LUA_TNONE,           "none" )
    ADD_ENUM ( LUA_TNIL,            "nil" )
    ADD_ENUM ( LUA_TBOOLEAN,        "boolean" )
    ADD_ENUM ( LUA_TLIGHTUSERDATA,  "lightuserdata" )
    ADD_ENUM ( LUA_TNUMBER,         "number" )
    ADD_ENUM ( LUA_TSTRING,         "string" )
    ADD_ENUM ( LUA_TTABLE,          "table" )
    ADD_ENUM ( LUA_TFUNCTION,       "function" )
    ADD_ENUM ( LUA_TUSERDATA,       "userdata" )
    ADD_ENUM ( LUA_TTHREAD,         "thread" )
IMPLEMENT_ENUM_END( "lua-type" )

IMPLEMENT_ENUM_BEGIN( TrafficLight::EColor )
    ADD_ENUM ( TrafficLight::GREEN,           "green" )
    ADD_ENUM ( TrafficLight::YELLOW,          "yellow" )
    ADD_ENUM ( TrafficLight::RED,             "red" )
IMPLEMENT_ENUM_END( "traffic-light-color" )

IMPLEMENT_ENUM_BEGIN( TrafficLight::EState )
    ADD_ENUM ( TrafficLight::AUTO,            "auto" )
    ADD_ENUM ( TrafficLight::DISABLED,        "disabled" )
IMPLEMENT_ENUM_END( "traffic-light-state" )

IMPLEMENT_ENUM_BEGIN( CEasingCurve::eType )
    ADD_ENUM ( CEasingCurve::Linear,           "Linear" )
    ADD_ENUM ( CEasingCurve::InQuad,           "InQuad" )
    ADD_ENUM ( CEasingCurve::OutQuad,          "OutQuad" )
    ADD_ENUM ( CEasingCurve::InOutQuad,        "InOutQuad" )
    ADD_ENUM ( CEasingCurve::OutInQuad,        "OutInQuad" )
    ADD_ENUM ( CEasingCurve::InElastic,        "InElastic" )
    ADD_ENUM ( CEasingCurve::OutElastic,       "OutElastic" )
    ADD_ENUM ( CEasingCurve::InOutElastic,     "InOutElastic" )
    ADD_ENUM ( CEasingCurve::OutInElastic,     "OutInElastic" )
    ADD_ENUM ( CEasingCurve::InBack,           "InBack" )
    ADD_ENUM ( CEasingCurve::OutBack,          "OutBack" )
    ADD_ENUM ( CEasingCurve::InOutBack,        "InOutBack" )
    ADD_ENUM ( CEasingCurve::OutInBack,        "OutInBack" )
    ADD_ENUM ( CEasingCurve::InBounce,         "InBounce" )
    ADD_ENUM ( CEasingCurve::OutBounce,        "OutBounce" )
    ADD_ENUM ( CEasingCurve::InOutBounce,      "InOutBounce" )
    ADD_ENUM ( CEasingCurve::OutInBounce,      "OutInBounce" )
    ADD_ENUM ( CEasingCurve::SineCurve,        "SineCurve" )
    ADD_ENUM ( CEasingCurve::CosineCurve,      "CosineCurve" )
IMPLEMENT_ENUM_END_DEFAULTS( "easing-type", CEasingCurve::EASING_INVALID, "Invalid" )

IMPLEMENT_ENUM_BEGIN( eHudComponent )
    ADD_ENUM ( HUD_AMMO,            "ammo" )
    ADD_ENUM ( HUD_WEAPON,          "weapon" )
    ADD_ENUM ( HUD_HEALTH,          "health" )
    ADD_ENUM ( HUD_BREATH,          "breath" )
    ADD_ENUM ( HUD_ARMOUR,          "armour" )
    ADD_ENUM ( HUD_MONEY,           "money" )
    ADD_ENUM ( HUD_VEHICLE_NAME,    "vehicle_name" )
    ADD_ENUM ( HUD_AREA_NAME,       "area_name" )
    ADD_ENUM ( HUD_RADAR,           "radar" )
    ADD_ENUM ( HUD_CLOCK,           "clock" )
    ADD_ENUM ( HUD_RADIO,           "radio" )
    ADD_ENUM ( HUD_WANTED,          "wanted" )
    ADD_ENUM ( HUD_CROSSHAIR,       "crosshair" )
    ADD_ENUM ( HUD_ALL,             "all" )
IMPLEMENT_ENUM_END( "hud-component" )

IMPLEMENT_ENUM_BEGIN( eWeaponType )
    ADD_ENUM ( WEAPONTYPE_GRENADE,              "grenade" )
    ADD_ENUM ( WEAPONTYPE_TEARGAS,              "teargas" )
    ADD_ENUM ( WEAPONTYPE_MOLOTOV,              "molotov" )
    ADD_ENUM ( WEAPONTYPE_PISTOL,               "pistol" )
    ADD_ENUM ( WEAPONTYPE_PISTOL_SILENCED,      "silenced_pistol" )
    ADD_ENUM ( WEAPONTYPE_DESERT_EAGLE,         "desert_eagle" )
    ADD_ENUM ( WEAPONTYPE_SHOTGUN,              "shotgun" )
    ADD_ENUM ( WEAPONTYPE_SAWNOFF_SHOTGUN,      "sawnoff_shotgun" )
    ADD_ENUM ( WEAPONTYPE_SPAS12_SHOTGUN,       "spas12_shotgun" )
    ADD_ENUM ( WEAPONTYPE_MICRO_UZI,            "uzi" )
    ADD_ENUM ( WEAPONTYPE_MP5,                  "mp5" )
    ADD_ENUM ( WEAPONTYPE_AK47,                 "ak47" )
    ADD_ENUM ( WEAPONTYPE_M4,                   "m4" )   
    ADD_ENUM ( WEAPONTYPE_TEC9,                 "tec9" )
    ADD_ENUM ( WEAPONTYPE_COUNTRYRIFLE,         "rifle" )
    ADD_ENUM ( WEAPONTYPE_SNIPERRIFLE,          "sniper_rifle" )
    ADD_ENUM ( WEAPONTYPE_ROCKETLAUNCHER,       "rocket_launcher" )
    ADD_ENUM ( WEAPONTYPE_ROCKETLAUNCHER_HS,    "rocket_launcher_hs" )
    ADD_ENUM ( WEAPONTYPE_FLAMETHROWER,         "flamethrower" )
    ADD_ENUM ( WEAPONTYPE_MINIGUN,              "minigun" )
    ADD_ENUM ( WEAPONTYPE_REMOTE_SATCHEL_CHARGE,"satchel" )
    ADD_ENUM ( WEAPONTYPE_DETONATOR,            "detonator" )
    ADD_ENUM ( WEAPONTYPE_SPRAYCAN,             "spraycan" )
    ADD_ENUM ( WEAPONTYPE_EXTINGUISHER,         "extinguisher" )
    ADD_ENUM ( WEAPONTYPE_CAMERA,               "camera" )
IMPLEMENT_ENUM_END( "weapon-type" )

IMPLEMENT_ENUM_BEGIN( eWeaponProperty )
    ADD_ENUM ( WEAPON_WEAPON_RANGE,                     "weapon_range" )
    ADD_ENUM ( WEAPON_TARGET_RANGE,                     "target_range" )
    ADD_ENUM ( WEAPON_ACCURACY,                         "accuracy" )
    ADD_ENUM ( WEAPON_DAMAGE,                           "damage" )
    ADD_ENUM ( WEAPON_LIFE_SPAN,                        "life_span" )
    ADD_ENUM ( WEAPON_FIRING_SPEED,                     "firing_speed" )
    ADD_ENUM ( WEAPON_SPREAD,                           "spread" )
    ADD_ENUM ( WEAPON_MAX_CLIP_AMMO,                    "maximum_clip_ammo" )
    ADD_ENUM ( WEAPON_MOVE_SPEED,                       "move_speed" )
    ADD_ENUM ( WEAPON_FLAGS,                            "flags" )
    ADD_ENUM ( WEAPON_ANIM_GROUP,                       "anim_group" )

    // Set is Disabled >= FIRETYPE - Move above WEAPON_TYPE_SET_DISABLED to re-enable and move it above here to make it less confusing.
    ADD_ENUM ( WEAPON_FIRETYPE,                         "fire_type" )

    ADD_ENUM ( WEAPON_MODEL,                            "model" )
    ADD_ENUM ( WEAPON_MODEL2,                           "model2" ) 

    ADD_ENUM ( WEAPON_SLOT,                             "weapon_slot" ) 

    ADD_ENUM ( WEAPON_FIRE_OFFSET,                      "fire_offset" ) 

    ADD_ENUM ( WEAPON_SKILL_LEVEL,                      "skill_level" ) 
    ADD_ENUM ( WEAPON_REQ_SKILL_LEVEL,                  "required_skill_level" ) 

    ADD_ENUM ( WEAPON_ANIM_LOOP_START,                  "anim_loop_start" ) 
    ADD_ENUM ( WEAPON_ANIM_LOOP_STOP,                   "anim_loop_stop" ) 
    ADD_ENUM ( WEAPON_ANIM_LOOP_RELEASE_BULLET_TIME,    "anim_loop_bullet_fire" ) 

    ADD_ENUM ( WEAPON_ANIM2_LOOP_START,                 "anim2_loop_start" ) 
    ADD_ENUM ( WEAPON_ANIM2_LOOP_STOP,                  "anim2_loop_stop" ) 
    ADD_ENUM ( WEAPON_ANIM2_LOOP_RELEASE_BULLET_TIME,   "anim2_loop_bullet_fire" ) 

    ADD_ENUM ( WEAPON_ANIM_BREAKOUT_TIME,               "anim_breakout_time" ) 

    ADD_ENUM ( WEAPON_RADIUS,                           "radius" ) 

    ADD_ENUM ( WEAPON_AIM_OFFSET,                       "aim_offset" ) 

    ADD_ENUM ( WEAPON_DEFAULT_COMBO,                    "default_combo" ) 
    ADD_ENUM ( WEAPON_COMBOS_AVAILABLE,                 "combos_available" ) 
IMPLEMENT_ENUM_END( "weapon-property" )

IMPLEMENT_ENUM_BEGIN( eWeaponSkill )
    ADD_ENUM ( WEAPONSKILL_POOR,                            "poor" )
    ADD_ENUM ( WEAPONSKILL_STD,                             "std" )
    ADD_ENUM ( WEAPONSKILL_PRO,                             "pro" )
IMPLEMENT_ENUM_END( "weapon-skill" )

IMPLEMENT_ENUM_BEGIN( CAccessControlListRight::ERightType )
    ADD_ENUM ( CAccessControlListRight::RIGHT_TYPE_COMMAND,      "command" )
    ADD_ENUM ( CAccessControlListRight::RIGHT_TYPE_FUNCTION,     "function" )
    ADD_ENUM ( CAccessControlListRight::RIGHT_TYPE_RESOURCE,     "resource" )
    ADD_ENUM ( CAccessControlListRight::RIGHT_TYPE_GENERAL,      "general" )
IMPLEMENT_ENUM_END( "right-type" )


//
// Get best guess at name of userdata type
//
SString GetUserDataClassName ( void* ptr, lua_State* luaVM )
{
    // Try element
    if ( CElement* pElement = UserDataCast < CElement > ( (CElement*)NULL, ptr, NULL ) )
    {
        return pElement->GetTypeName ();
    }

    // Try xml node
    if ( CXMLNode* pXMLNode = UserDataCast < CXMLNode > ( (CXMLNode*)NULL, ptr, NULL ) )
    {
        return GetClassTypeName ( pXMLNode );
    }

    // Try timer
    if ( CLuaTimer* pLuaTimer = UserDataCast < CLuaTimer > ( (CLuaTimer*)NULL, ptr, luaVM ) )
    {
        return GetClassTypeName ( pLuaTimer );
    }

    // Try resource
    if ( CResource* pResource = UserDataCast < CResource > ( (CResource*)NULL, ptr, NULL ) )
    {
        return GetClassTypeName ( pResource );
    }

    return "";
}

//
// Read next as resource or resource name.  Result output as string
//
void MixedReadResourceString ( CScriptArgReader& argStream, SString& strOutResourceName )
{
    if ( !argStream.NextIsString () )
    {
        CResource* pResource;
        argStream.ReadUserData ( pResource );
        if ( pResource )
            strOutResourceName = pResource->GetName ();
    }
    else
        argStream.ReadString ( strOutResourceName );
}


//
// Read next as resource or resource name.  Result output as resource
//
void MixedReadResourceString ( CScriptArgReader& argStream, CResource*& pOutResource )
{
    if ( !argStream.NextIsString () )
    {
        argStream.ReadUserData ( pOutResource );
    }
    else
    {
        SString strResourceName;
        argStream.ReadString ( strResourceName );
        pOutResource = g_pGame->GetResourceManager ()->GetResource ( strResourceName );

        if ( !pOutResource )
            argStream.SetTypeError ( "resource", argStream.m_iIndex - 1 );
    }
}


///////////////////////////////////////////////////////////////
//
// StringToBool
//
// Convert a string into a best guess bool equivalent
//
///////////////////////////////////////////////////////////////
bool StringToBool ( const SString& strText )
{
    return ( strText == "true" || strText == "1" || strText == "yes" );
}
