/*
 * HAimbot.cpp
 *
 *  Created on: Oct 9, 2016
 *      Author: nullifiedcat
 */

#include "Aimbot.h"

#include <globalvars_base.h>
#include <icliententity.h>
#include <inputsystem/ButtonCode.h>
#include <inputsystem/iinputsystem.h>
#include <mathlib/vector.h>
#include <cmath>

#include "../aftercheaders.h"
#include "../common.h"
#include "../conditions.h"
#include "../crits.h"
#include "../cvwrapper.h"
#include "../drawing.h"
#include "../entitycache.h"
#include "../globals.h"
#include "../helpers.h"
#include "../hoovy.hpp"
#include "../interfaces.h"
#include "../localplayer.h"
#include "../netvars.h"
#include "../playerlist.hpp"
#include "../prediction.h"
#include "../sdk/in_buttons.h"
#include "../targethelper.h"
#include "../usercmd.h"
#include "AntiAim.h"
#include "ESP.h"
#include "FollowBot.h"

namespace hacks { namespace shared { namespace aimbot {

EAimbotState state { EAimbotState::DISABLED };
int target_eid { 0 };
CachedEntity* target_highest = 0;
int minigun_fix_ticks { 0 };
bool projectile_mode { false };
float cur_proj_speed { 0.0f };
float cur_proj_grav { 0.0f };
bool headonly { false };
int last_target { -1 };
bool silent_huntsman { false };
// This array will store calculated projectile/hitscan predictions
// for current frame, to avoid performing them again
AimbotCalculatedData_s calculated_data_array[2048] {};

static CatVar huntsman_full_auto(CV_SWITCH, "aimbot_full_auto_huntsman", "1", "Auto Huntsman", "Autoshoot will pull huntsman's string");
static CatVar ignore_hoovy(CV_SWITCH, "aimbot_ignore_hoovy", "0", "Ignore Hoovies", "Aimbot won't attack hoovies");
static CatVar wait_for_charge(CV_SWITCH, "aimbot_charge", "0", "Wait for sniper rifle charge", "Aimbot waits until it has enough charge to kill");
static CatVar respect_vaccinator(CV_SWITCH, "aimbot_respect_vaccinator", "1", "Respect Vaccinator", "Hitscan weapons won't fire if enemy is vaccinated against bullets");
static CatVar aimkey(CV_KEY, "aimbot_aimkey", "0", "Aimkey", "Aimkey. Look at Aimkey Mode too!");
static CatEnum aimkey_modes_enum({ "DISABLED", "AIMKEY", "REVERSE", "TOGGLE" });
static CatVar aimkey_mode(aimkey_modes_enum, "aimbot_aimkey_mode", "1", "Aimkey mode", "DISABLED: aimbot is always active\nAIMKEY: aimbot is active when key is down\nREVERSE: aimbot is disabled when key is down\nTOGGLE: pressing key toggles aimbot");
static CatEnum hitbox_mode_enum({ "AUTO-HEAD", "AUTO-CLOSEST", "STATIC" });
static CatVar hitbox_mode(hitbox_mode_enum, "aimbot_hitboxmode", "0", "Hitbox Mode", "Defines hitbox selection mode");
static CatVar enabled(CV_SWITCH, "aimbot_enabled", "0", "Enable Aimbot", "Main aimbot switch");
static CatVar fov(CV_FLOAT, "aimbot_fov", "0", "Aimbot FOV", "FOV range for aimbot to lock targets. \"Smart FOV\" coming eventually.", 360.0f);
static CatEnum hitbox_enum({
		"HEAD", "PELVIS", "SPINE 0", "SPINE 1", "SPINE 2", "SPINE 3", "UPPER ARM L", "LOWER ARM L",
		"HAND L", "UPPER ARM R", "LOWER ARM R", "HAND R", "HIP L", "KNEE L", "FOOT L", "HIP R",
		"KNEE R", "FOOT R" });
static CatVar hitbox(hitbox_enum, "aimbot_hitbox", "0", "Hitbox", "Hitbox to aim at. Ignored if AutoHitbox is on");
static CatVar lerp(CV_SWITCH, "aimbot_interp", "1", "Latency interpolation", "Enable basic latency interpolation");
static CatVar autoshoot(CV_SWITCH, "aimbot_autoshoot", "1", "Autoshoot", "Shoot automatically when the target is locked, isn't compatible with 'Enable when attacking'");
static CatVar silent(CV_SWITCH, "aimbot_silent", "1", "Silent", "Your screen doesn't get snapped to the point where aimbot aims at");
static CatVar zoomed_only(CV_SWITCH, "aimbot_zoomed", "1", "Zoomed only", "Don't autoshoot with unzoomed rifles");
static CatVar teammates(CV_SWITCH, "aimbot_teammates", "0", "Aim at teammates", "Aim at your own team. Useful for HL2DM");
static CatVar huntsman_autoshoot(CV_FLOAT, "aimbot_huntsman_charge", "0.5", "Huntsman autoshoot", "Minimum charge for autoshooting with huntsman.\n"
		"Set it to 0.01 if you want to shoot as soon as you start pulling the arrow", 0.01f, 1.0f);
static CatVar max_range(CV_INT, "aimbot_maxrange", "0", "Max distance",
		"Max range for aimbot\n"
		"900-1100 range is efficient for scout/widowmaker engineer", 4096.0f);
//CatVar* v_iMaxAutoshootRange; // TODO IMPLEMENT
static CatVar respect_cloak(CV_SWITCH, "aimbot_respect_cloak", "1", "Respect cloak", "Don't aim at invisible enemies");
static CatVar attack_only(CV_SWITCH, "aimbot_enable_attack_only", "0", "Active when attacking", "Basically makes Mouse1 an AimKey, isn't compatible with AutoShoot");
static CatVar projectile_aimbot(CV_SWITCH, "aimbot_projectile", "1", "Projectile aimbot", "If you turn it off, aimbot won't try to aim with projectile weapons");
static CatVar proj_speed(CV_FLOAT, "aimbot_proj_speed", "0", "Projectile speed",
		"Force override projectile speed.\n"
		"Can be useful for playing with MvM upgrades or on x10 servers "
		"since there is no \"automatic\" projectile speed detection in "
		"cathook. Yet.");
static CatVar proj_gravity(CV_FLOAT, "aimbot_proj_gravity", "0", "Projectile gravity",
		"Force override projectile gravity. Useful for debugging.", 1.0f);
static CatVar buildings(CV_SWITCH, "aimbot_buildings", "1", "Aim at buildings", "Should aimbot aim at buildings?");
static CatVar only_can_shoot(CV_SWITCH, "aimbot_only_when_can_shoot", "1", "Active when can shoot", "Aimbot only activates when you can instantly shoot, sometimes making the autoshoot invisible for spectators");
static CatEnum priority_mode_enum({ "SMART", "FOV", "DISTANCE", "HEALTH" });
static CatVar priority_mode(priority_mode_enum, "aimbot_prioritymode", "0", "Priority mode", "Priority mode.\n"
		"SMART: Basically Auto-Threat. Will be tweakable eventually. "
		"FOV, DISTANCE, HEALTH are self-explainable. HEALTH picks the weakest enemy");
static CatVar proj_visibility(CV_SWITCH, "aimbot_proj_vispred", "0", "Projectile visibility prediction", "If enabled, projectile aimbot will perform additional visibility checking and won't try to predict enemies behind walls");
static CatVar proj_fov(CV_SWITCH, "aimbot_proj_fovpred", "0", "Projectile FOV mode", "If disabled, FOV restrictions apply to current target position");
static CatVar auto_spin_up(CV_SWITCH, "aimbot_spin_up", "0", "Auto Spin Up", "Spin up minigun if you can see target, useful for followbots");
static CatVar auto_zoom(CV_SWITCH, "aimbot_auto_zoom", "0", "Auto Zoom", "Automatically zoom in if you can see target, useful for followbots");
static CatVar engine_predict(CV_SWITCH, "aimbot_engine_pred", "0", "Engine Prediction", "Improves accuracy by preforming engine prediction\nKnown bugs: Crash on disconnect, breaks bhop");
//Initialize vars for slow aim
static CatVar slowaim(CV_SWITCH, "aimbot_slow", "0", "Slow Aim", "Slowly moves your crosshair onto the targets face\nDoesn't work with Silent or Anti-aim");
static CatVar slowaim_smoothing(CV_INT, "aimbot_slow_smooth", "10", "Slow Aim Smooth", "How slow the slow aim's aiming should be", 50);
static CatVar slowaim_autoshoot(CV_INT, "aimbot_slow_autoshoot", "10", "Slow Aim Threshhold", "Distance to autoshoot while smooth aiming", 25);
static CatVar aimbot_debug(CV_SWITCH, "aimbot_debug", "0", "Aimbot Debug", "Print some internal state info for aimbot");
	



	

bool slowCanShoot = false;
/*//Salting vars that need to be saved due to them being time based
static CatVar slowaim_salting(CV_SWITCH, "aimbot_slow_salt", "1", "Slow Aim Smooth", "Makes the slowaim more random", 5);
float saltWait = 0;
int saltRandom = 0;*/
static CatVar instant_rezoom_enabled(CV_SWITCH, "aimbot_instant_rezoom_enabled", "0", "Instant rezoom", "Allows you to instantly zoom after you shoot\nGreat for pre-charging charged shots\nOccasionally fails");
bool instant_rezoom_shoot = false;

CachedEntity* CurrentTarget() {
	if (state == EAimbotState::TARGET_FOUND || state == EAimbotState::AIMING)
		return ENTITY(target_eid);
	return nullptr;
}

int ClosestHitbox(CachedEntity* target) {
	PROF_SECTION(CM_aimbot_closesthitbox);

	// FIXME this will break multithreading if it will be ever implemented. When implementing it, these should be made non-static
	int closest;
	float closest_fov, fov;

	//If you can see the spine, no need to check for another hitbox
    if ((int)hitbox_mode == 0) {
        if (target->hitboxes.VisibilityCheck(hitbox_t::spine_1)) return hitbox_t::spine_1;
    }
	closest = -1;
	closest_fov = 256;
	for (int i = 0; i < target->hitboxes.GetNumHitboxes(); i++) {
		fov = GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye, target->hitboxes.GetHitbox(i)->center);
		if (fov < closest_fov || closest == -1) {
			closest = i;
			closest_fov = fov;
		}
	}
	return closest;
}

static EAimbotLocalState local_state_last;

void CreateMove() {
	
	EAimbotLocalState local_state;
	float target_highest_score, scr, begincharge, charge;
	CachedEntity* ent;
	EAimbotTargetState tg;
	int huntsman_ticks = 0;

	target_highest = 0;
	if (!enabled) {
		state = EAimbotState::DISABLED;
		return;
	} else {
		state = EAimbotState::ENABLED;
	}
	if (engine_predict) RunEnginePrediction(RAW_ENT(LOCAL_E), g_pUserCmd);
	local_state = ShouldAim();

	if (aimbot_debug) {
		local_state_last = local_state;
	}

	if (local_state != EAimbotLocalState::GOOD) {
		state = EAimbotState::INACTIVE;
	} else {
		state = EAimbotState::ACTIVE;
	}

	headonly = false;

	projectile_mode = (GetProjectileData(g_pLocalPlayer->weapon(), cur_proj_speed, cur_proj_grav));
	if (proj_speed)
		cur_proj_speed = (float)proj_speed;
	if (proj_gravity)
		cur_proj_grav = (float)proj_gravity;
	// TODO priority modes (FOV, Smart, Distance, etc)
	target_highest_score = -256;
	{
		PROF_SECTION(CM_aimbot_finding_target);
		for (int i = 0; i < HIGHEST_ENTITY; i++) {
			ent = ENTITY(i);
			if (CE_BAD(ent)) continue;
			tg = TargetState(ent);
			if (tg == EAimbotTargetState::GOOD) {
				if (GetWeaponMode() == weaponmode::weapon_melee || (int)priority_mode == 2) {
					scr = 4096.0f - calculated_data_array[i].aim_position.DistTo(g_pLocalPlayer->v_Eye);
					if (scr > target_highest_score) {
						target_highest_score = scr;
						target_highest = ent;
					}
				} else {
					switch ((int)priority_mode) {
					case 0: {
						scr = GetScoreForEntity(ent);
						if (scr > target_highest_score) {
							target_highest_score = scr;
							target_highest = ent;
						}
					} break;
					case 1: {
						scr = 360.0f - calculated_data_array[ent->m_IDX].fov;
						if (scr > target_highest_score) {
							target_highest_score = scr;
							target_highest = ent;
						}
					} break;
					case 3: {
						scr = 450.0f - ent->m_iHealth;
						if (scr > target_highest_score) {
							target_highest_score = scr;
							target_highest = ent;
						}
					}
					}
				}
			} else {
				if (aimbot_debug) {
					if (CE_GOOD(ent)) {
						hacks::shared::esp::AddEntityString(ent, format("AIMBOT_STATE: ", static_cast<int>(tg)));
					}
				}
				//if (tg != 26)
				//	logging::Info("Shouldn't target ent %i %i", ent->m_IDX, tg);
			}
		}
	}
	if (huntsman_ticks) {
		g_pUserCmd->buttons |= IN_ATTACK;
		huntsman_ticks = max(0, huntsman_ticks - 1);
	}

	if (CE_GOOD(target_highest)) {
		state = EAimbotState::TARGET_FOUND;
		hacks::shared::esp::SetEntityColor(target_highest, colors::pink);
		if (local_state == EAimbotLocalState::GOOD) {
			PROF_SECTION(CM_aimbot_aiming);
			last_target = target_highest->m_IDX;
			if (g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFCompoundBow)) { // There is no Huntsman in TF2C.
				begincharge = CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flChargeBeginTime);
				charge = 0;
				if (begincharge != 0) {
					charge = g_GlobalVars->curtime - begincharge;
					if (charge > 1.0f) charge = 1.0f;
					silent_huntsman = true;
				}
				if (charge >= (float)huntsman_autoshoot) {
					g_pUserCmd->buttons &= ~IN_ATTACK;
					hacks::shared::antiaim::SetSafeSpace(3);
				} else if (autoshoot && huntsman_full_auto) {
					huntsman_ticks = 3;
					g_pUserCmd->buttons |= IN_ATTACK;
				}
				if (!(g_pUserCmd->buttons & IN_ATTACK) && silent_huntsman) {
					Aim(target_highest);
					silent_huntsman = false;
				}
			} else {
				Aim(target_highest);
			}
			if (g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFMinigun))
				minigun_fix_ticks = 40;
		}
	}
	if (minigun_fix_ticks > 0) {
		minigun_fix_ticks--;
		g_pUserCmd->buttons |= IN_ATTACK;
	}
	if (g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFMinigun) &&
			target_highest == 0 &&
			IDX_GOOD(last_target) &&
			minigun_fix_ticks && (local_state == EAimbotLocalState::GOOD)) {
		Aim(ENTITY(last_target));
	}
	
	if (silent) g_pLocalPlayer->bUseSilentAngles = true;
	return;
}

void Reset() {
	last_target = -1;
	projectile_mode = false;
}


const Vector& PredictEntity(CachedEntity* entity) {
	AimbotCalculatedData_s& cd = calculated_data_array[entity->m_IDX];
	Vector& result = cd.aim_position;
	if (cd.predict_tick == tickcount) return result;
	if ((entity->m_Type == ENTITY_PLAYER)) {
		if (projectile_mode) {
			result = ProjectilePrediction(entity, cd.hitbox, cur_proj_speed, cur_proj_grav, PlayerGravityMod(entity));			
		} else {
			if (lerp)
				result = SimpleLatencyPrediction(entity, cd.hitbox);
			else
				GetHitbox(entity, cd.hitbox, result);
		}
	} else if (entity->m_Type == ENTITY_BUILDING) {
		result = GetBuildingPosition(entity);
	} else {
		result = entity->m_vecOrigin;
	}
	cd.predict_tick = tickcount;
	cd.fov = GetFov(g_pLocalPlayer->v_OrigViewangles, g_pLocalPlayer->v_Eye, result);
	return result;
}

bool VischeckPredictedEntity(CachedEntity* entity) {
	AimbotCalculatedData_s& cd = calculated_data_array[entity->m_IDX];
	if (cd.vcheck_tick == tickcount) return cd.visible;
	cd.vcheck_tick = tickcount;
	//if (entity->m_Type == ENTITY_PLAYER)
	//	cd.visible = IsVectorVisible(g_pLocalPlayer->v_Eye, PredictEntity(entity));
	//else
		cd.visible = IsEntityVectorVisible(entity, PredictEntity(entity));
	return cd.visible;
}

EAimbotTargetState TargetState(CachedEntity* entity) {
	float bdmg;
	weaponmode mode;
	Vector resultAim;
	int hitbox;
	int team;

	if (entity->m_Type == ENTITY_PLAYER) {
		if (entity == LOCAL_E) return EAimbotTargetState::LOCAL;
		if (!entity->m_bAlivePlayer) return EAimbotTargetState::DEAD;
		if (!entity->m_bEnemy && !teammates) return EAimbotTargetState::TEAMMATE;
		if (EffectiveTargetingRange()) {
			if (entity->m_flDistance > EffectiveTargetingRange()) return EAimbotTargetState::OUT_OF_RANGE;
		}
		IF_GAME (IsTF()) {
			if (wait_for_charge && g_pLocalPlayer->holding_sniper_rifle) {
				bdmg = CE_FLOAT(g_pLocalPlayer->weapon(), netvar.flChargedDamage);
				if (g_GlobalVars->curtime - g_pLocalPlayer->flZoomBegin <= 1.0f) bdmg = 50.0f;
				if ((bdmg * 3) < (HasDarwins(entity) ? (entity->m_iHealth * 1.15) : entity->m_iHealth)) {
					return EAimbotTargetState::NOT_ENOUGH_CHARGE;
				}
			}
			if (ignore_taunting && HasCondition<TFCond_Taunting>(entity)) return EAimbotTargetState::TAUNTING;
			if (IsPlayerInvulnerable(entity)) return EAimbotTargetState::INVULNERABLE;
			if (respect_cloak && IsPlayerInvisible(entity)) return EAimbotTargetState::INVISIBLE;
			mode = GetWeaponMode();
			if (mode == weaponmode::weapon_hitscan || LOCAL_W->m_iClassID == CL_CLASS(CTFCompoundBow))
				if (respect_vaccinator && HasCondition<TFCond_UberBulletResist>(entity)) return EAimbotTargetState::VACCINATED;
		}
		if (playerlist::IsFriendly(playerlist::AccessData(entity).state)) return EAimbotTargetState::FRIENDLY;
		IF_GAME (IsTF()) {
			if (ignore_hoovy && IsHoovy(entity)) {
				return EAimbotTargetState::HOOVY;
			}
		}
		hitbox = BestHitbox(entity);
		AimbotCalculatedData_s& cd = calculated_data_array[entity->m_IDX];
		cd.hitbox = hitbox;
		if (!VischeckPredictedEntity(entity)) return EAimbotTargetState::VCHECK_FAILED;
		if ((float)fov > 0.0f && cd.fov > (float)fov) return EAimbotTargetState::FOV_CHECK_FAILED;
		return EAimbotTargetState::GOOD;
	} else if (entity->m_Type == ENTITY_BUILDING) {
		if (!buildings) return EAimbotTargetState::BUILDING_AIMBOT_DISABLED;
		team = CE_INT(entity, netvar.iTeamNum);
		if (team == g_pLocalPlayer->team) return EAimbotTargetState::TEAMMATE;
		if (EffectiveTargetingRange()) {
			if (entity->m_flDistance > (int)EffectiveTargetingRange()) return EAimbotTargetState::OUT_OF_RANGE;
		}
		AimbotCalculatedData_s& cd = calculated_data_array[entity->m_IDX];
		if (!VischeckPredictedEntity(entity)) return EAimbotTargetState::VCHECK_FAILED;
		if ((float)fov > 0.0f && cd.fov > (float)fov) return EAimbotTargetState::FOV_CHECK_FAILED;
		return EAimbotTargetState::GOOD;
	} else {
		return EAimbotTargetState::INVALID_ENTITY;
	}
	return EAimbotTargetState::IMPOSSIBLE_ERROR;
}

void slowAim(Vector &inputAngle, Vector userAngle) {
    //Initialize vars for slow aim
    int slowfliptype;
    int slowdir;
    float changey;
    float changex;
    
    /*//Use rand to randomize the change speed
    if (slowaim_salting) {
        if ((g_GlobalVars->curtime - 0.15F) > saltWait) {
            saltWait = g_GlobalVars->curtime;
            saltRandom = rand() % 3;
        }
    }*/
    
    //Angle clamping for when the aimbot chooses a too high of value
    if (inputAngle.y > 180) inputAngle.y = inputAngle.y - 360;
    if (inputAngle.y < -180) inputAngle.y = inputAngle.y + 360;
        
    //Determine whether to move the mouse at all for the yaw
    if (userAngle.y != inputAngle.y) {
        
        //Fliping The main axis to prevent 360s from happening when the bot trys to cross -180y and 180y
        slowfliptype = 0;
        if ( ((inputAngle.y < -90) && (userAngle.y > 90)) && (slowfliptype == 0) ) {
            slowfliptype = 1;
            inputAngle.y = inputAngle.y - 90;
            userAngle.y = userAngle.y + 90;
        }
        if ( ((inputAngle.y > 90) && (userAngle.y < -90)) && (slowfliptype == 0) ) {
            slowfliptype = 2;
            inputAngle.y = inputAngle.y + 90;
            userAngle.y = userAngle.y - 90;
        }
        
        //Math to calculate how much to move the mouse
        changey = (std::abs(userAngle.y - inputAngle.y)) / ((int)slowaim_smoothing) ;
        //Use stronger shunting due to the flip
        if (slowfliptype != 0) changey = ((( std::abs(userAngle.y - inputAngle.y) ) / ((int)slowaim_smoothing * (int)slowaim_smoothing)) / (int)slowaim_smoothing) ;
        
        //Determine the direction to move in before reseting the flipped angles
        slowdir = 0;
        if ((userAngle.y > inputAngle.y) && (slowdir == 0)) slowdir = 1;
        if ((userAngle.y < inputAngle.y) && (slowdir == 0)) slowdir = 2;

        //Reset Flipped angles and fix directions
        if (slowfliptype == 1) {
            inputAngle.y = inputAngle.y + 90;
            userAngle.y = userAngle.y - 90;
            slowdir = 2;
        }
        if (slowfliptype == 2) {
            inputAngle.y = inputAngle.y - 90;
            userAngle.y = userAngle.y + 90;
            slowdir = 1;
        }
        
        /*//If salted, then randomize the speed here
        if (slowaim_salting) {
            if (saltRandom == 0) changey = changey - (changey/2);
            if (saltRandom == 1) changey = changey - (changey/2.5);  
            if (saltRandom == 2) changey = changey + (changey/4);
            if (saltRandom == 3) changey = changey + (changey/3);  
        }*/
        
        //Move in the direction determined before the fliped angles
        if (slowdir == 1) inputAngle.y = userAngle.y - changey;
        if (slowdir == 2) inputAngle.y = userAngle.y + changey;
    }
    
    //Angle clamping for when the aimbot chooses a too high of value, fixes for when players are above your player
    if (inputAngle.x > 89) inputAngle.x = inputAngle.x - 360;
    
    //Determine whether to move the mouse at all for the pitch
    if (userAngle.x != inputAngle.x) {
        changex = (std::abs(userAngle.x - inputAngle.x)) / ((int)slowaim_smoothing) ;
        
        /*//If salted, then randomize the speed here
        if (slowaim_salting) {
            if (saltRandom == 0) changex = changex - (changex/2);
            if (saltRandom == 1) changex = changex - (changex/2.5);  
            if (saltRandom == 2) changex = changex + (changex/4);
            if (saltRandom == 3) changex = changex + (changex/3);  
        }*/
        
        //Determine the direction to move in
        if (userAngle.x > inputAngle.x) inputAngle.x = userAngle.x - changex; 
        if (userAngle.x < inputAngle.x) inputAngle.x = userAngle.x + changex;
    }

    //Check if can autoshoot with slowaim
    slowCanShoot = false;
    if (changey < (0.02*(int)slowaim_autoshoot) && changex < (0.02*(int)slowaim_autoshoot)) slowCanShoot = true;
}

bool Aim(CachedEntity* entity) {
	Vector angles, tr;
	int hitbox, weapon_class;
	bool attack;
	static int forbiddenWeapons[] = { CL_CLASS(CTFCompoundBow), CL_CLASS(CTFKnife) };

	state = EAimbotState::AIMING;
	if (CE_BAD(entity)) return false;
/*	if (entity->m_Type == ENTITY_PLAYER) {
		GetHitbox(entity, hitbox, hit);
		if (lerp) SimpleLatencyPrediction(entity, hitbox);
	} else if (entity->m_Type == ENTITY_BUILDING) {
		hit = GetBuildingPosition(entity);
	}
	if (projectile_mode) {
		hit = ProjectilePrediction(entity, hitbox, cur_proj_speed, cur_proj_grav, PlayerGravityMod(entity));
	}*/

	tr = (PredictEntity(entity) - g_pLocalPlayer->v_Eye);
	VectorAngles(tr, angles);
    
    //Slow the aiming to the aimpoint if true
	if (slowaim && !silent) slowAim(angles, g_pUserCmd->viewangles);
    
	//Set angles
    fClampAngle(angles);
    g_pUserCmd->viewangles = angles;

	if (silent) {
		g_pLocalPlayer->bUseSilentAngles = true;
	}
	if (autoshoot) {
		IF_GAME (IsTF()) {
			if (g_pLocalPlayer->clazz == tf_class::tf_sniper) {
				if (g_pLocalPlayer->holding_sniper_rifle) {
					if (zoomed_only && !CanHeadshot()) return true;
				}
			}
		}

		// Don't autoshoot with the knife!
		weapon_class = g_pLocalPlayer->weapon()->m_iClassID;
		attack = true;
        for (int i = 0; i < 2; i++) {
			if (weapon_class == forbiddenWeapons[i]) {
				attack = false;
				break;
			}
		}
		//Autoshoot breaks Slow aimbot, so use a workaround to detect when it can
		if (slowaim && !slowCanShoot) attack = false;

        if ( attack ) {
        	g_pUserCmd->buttons |= IN_ATTACK;
        }
        
        //Tell reset conds to function
        IF_GAME (IsTF()) {
        	if (instant_rezoom_enabled) {
				if (attack && g_pLocalPlayer->bZoomed && !instant_rezoom_shoot) {
					instant_rezoom_shoot = true;
				}
			}
        }
	}
	return true;
}

bool UpdateAimkey() {
	static bool aimkey_flip = false;
	static bool pressed_last_tick = false;
	bool key_down;
	if (aimkey && aimkey_mode) {
		key_down = g_IInputSystem->IsButtonDown((ButtonCode_t)(int)aimkey);
		switch (static_cast<EAimKeyMode>((int)aimkey_mode)) {
		case EAimKeyMode::PRESS_TO_ENABLE:
			if (!key_down) {
				state = EAimbotState::INACTIVE;
			}
			break;
		case EAimKeyMode::PRESS_TO_DISABLE:
			if (key_down) {
				state = EAimbotState::INACTIVE;
			}
			break;
		case EAimKeyMode::PRESS_TO_TOGGLE:
			if (!pressed_last_tick && key_down) aimkey_flip = !aimkey_flip;
			if (!aimkey_flip) {
				state = EAimbotState::INACTIVE;
			}
		}
		pressed_last_tick = key_down;
	}
	return state != EAimbotState::INACTIVE;
}


float EffectiveTargetingRange() {
	if (GetWeaponMode() == weapon_melee) {
		return 100.0f;
	}
	return (float)max_range;
}

float EffectiveShootingRange() {
	return 0.0f;
}

EAimbotLocalState ShouldAim() {
	PROF_SECTION(CM_aimbot_calc_localstate);

	bool do_minigun_checks;
	int weapon_state;
	// Checks should be in order: cheap -> expensive
	if (attack_only && !(g_pUserCmd->buttons & IN_ATTACK)) {
		return EAimbotLocalState::NOT_ATTACKING;
	}
	if (g_pUserCmd->buttons & IN_USE) return EAimbotLocalState::USE_BUTTON;
	if (g_pLocalPlayer->using_action_slot_item) return EAimbotLocalState::ACTION_SLOT_ITEM;
	if (!UpdateAimkey()) return EAimbotLocalState::AIMKEY_RELEASED;
	IF_GAME (IsTF2()) {
		if (CE_BYTE(g_pLocalPlayer->entity, netvar.m_bCarryingObject)) return EAimbotLocalState::CARRYING_BUILDING;
		if (CE_BYTE(g_pLocalPlayer->entity, netvar.m_bFeignDeathReady)) return EAimbotLocalState::DEAD_RINGER_OUT;
		if (zoomed_only && g_pLocalPlayer->holding_sniper_rifle) {
			if (!g_pLocalPlayer->bZoomed && !(g_pUserCmd->buttons & IN_ATTACK)) return EAimbotLocalState::NOT_ZOOMED;
		}
		if (HasCondition<TFCond_Taunting>(g_pLocalPlayer->entity)) return EAimbotLocalState::TAUNTING;
		if (IsPlayerInvisible(g_pLocalPlayer->entity)) return EAimbotLocalState::CLOAKED;
		if (g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFPipebombLauncher)) return EAimbotLocalState::DISABLED_FOR_THIS_WEAPON;
	}
	if (only_can_shoot) {
		// Miniguns should shoot and aim continiously. TODO smg
		if (g_pLocalPlayer->weapon()->m_iClassID != CL_CLASS(CTFMinigun)) {
			// Melees are weird, they should aim continiously like miniguns too.
			if (GetWeaponMode() != weaponmode::weapon_melee) {
				// Finally, CanShoot() check.
				if (!CanShoot()) return EAimbotLocalState::CANT_SHOOT;
			}
		}
	}
	IF_GAME (IsTF2()) {
		switch (GetWeaponMode()) {
		case weapon_hitscan:
		case weapon_melee:
			break;
		case weapon_projectile:
			if (!projectile_aimbot) return EAimbotLocalState::DISABLED_FOR_THIS_WEAPON;
			break;
		default:
			return EAimbotLocalState::DISABLED_FOR_THIS_WEAPON;
		};
	}
	IF_GAME (IsTF()) {
		if (g_pLocalPlayer->bZoomed) {
			if (!(g_pUserCmd->buttons & (IN_ATTACK | IN_ATTACK2))) {
				if (!CanHeadshot()) return EAimbotLocalState::SNIPER_RIFLE_DELAY;
			}
		}
	}
	IF_GAME (IsTF2()) {
		if (!AmbassadorCanHeadshot()) return EAimbotLocalState::AMBASSADOR_COOLDOWN;
	}
	do_minigun_checks = true;
#ifdef IPC_ENABLED
	if (hacks::shared::followbot::bot) {
		CachedEntity* player = ENTITY(hacks::shared::followbot::following_idx);
		if (CE_GOOD(player)) {
			if (HasCondition<TFCond_Slowed>(player)) {
				do_minigun_checks = false;
			}
		}
	}
#endif
	IF_GAME (IsTF()) {
		if (do_minigun_checks && g_pLocalPlayer->weapon()->m_iClassID == CL_CLASS(CTFMinigun)) {
			weapon_state = CE_INT(g_pLocalPlayer->weapon(), netvar.iWeaponState);
			if ((weapon_state == MinigunState_t::AC_STATE_IDLE || weapon_state == MinigunState_t::AC_STATE_STARTFIRING) && !auto_spin_up) {
				return EAimbotLocalState::MINIGUN_IDLE;
			}
			if (!(g_pUserCmd->buttons & (IN_ATTACK2 | IN_ATTACK))) {
				return EAimbotLocalState::MINIGUN_BUTTON_RELEASED;
			}
			// This doesn't belong here
			/*if (minigun_fix_ticks > 0) {
				minigun_fix_ticks--;
				cmd->buttons |= IN_ATTACK;
			}*/
		}
		if (!AllowAttacking())
			return EAimbotLocalState::CRIT_HACK_LOCKS_ATTACK;
	}
	return EAimbotLocalState::GOOD;
}

void PaintTraverse() {
	if (!aimbot_debug) return;
	if (!enabled) return;
	AddSideString(format("AimbotState: ", static_cast<int>(local_state_last)));
}

int BestHitbox(CachedEntity* target) {
	PROF_SECTION(CM_aimbot_besthitbox);

	int preferred, ci, flags, bdmg;
	float cdmg, bodmg;
	bool ground;
	preferred = hitbox;
	switch ((int)hitbox_mode) {
	case 0: { // AUTO-HEAD
		ci = g_pLocalPlayer->weapon()->m_iClassID;
		IF_GAME (IsTF()) {
			preferred = hitbox_t::pelvis;
			if (g_pLocalPlayer->holding_sniper_rifle) {
				headonly = CanHeadshot();
			} else if (ci == CL_CLASS(CTFCompoundBow)) {
				headonly = true;
			} else if (IsAmbassador(g_pLocalPlayer->weapon())) {
				headonly = true;
			} else if (ci == CL_CLASS(CTFRocketLauncher) ||
					ci == CL_CLASS(CTFRocketLauncher_AirStrike) ||
					ci == CL_CLASS(CTFRocketLauncher_DirectHit) ||
					ci == CL_CLASS(CTFRocketLauncher_Mortar)) {
				preferred = hitbox_t::hip_L;
			}
			flags = CE_INT(target, netvar.iFlags);
			ground = (flags & (1 << 0));
			if (!ground) {
				if (GetWeaponMode() == weaponmode::weapon_projectile) {
					if (g_pLocalPlayer->weapon()->m_iClassID != CL_CLASS(CTFCompoundBow)) {
						preferred = hitbox_t::spine_3;
					}
				}
			}
			if (g_pLocalPlayer->holding_sniper_rifle) {
				cdmg = CE_FLOAT(LOCAL_W, netvar.flChargedDamage);
				bodmg = 50;
				//Darwins damage correction
				if (target->m_iMaxHealth == 150 && target->m_iClassID == tf_sniper) {
					bodmg = (bodmg * .85) - 1;
					cdmg = (cdmg * .85) - 1;
				}
				//Vaccinator damage correction
				if (HasCondition<TFCond_UberBulletResist>(target)) {
					bodmg = (bodmg * .25) - 1;
					cdmg = (cdmg * .25) - 1;
				} else if (HasCondition<TFCond_SmallBulletResist>(target)) {
					bodmg = (bodmg * .90) - 1;
					cdmg = (cdmg * .90) - 1;
				}
				//Invis damage correction
				if (IsPlayerInvisible(target)) {
					bodmg = (bodmg * .80) - 1;
					cdmg = (cdmg * .80) - 1;
				}
				//If can headshot and if bodyshot kill from charge damage, or if crit boosted and they have 150 health, or if player isnt zoomed, or if the enemy has less than 40, due to darwins, and only if they have less than 150 health will it try to bodyshot
				if (CanHeadshot() && (cdmg >= target->m_iHealth || IsPlayerCritBoosted(g_pLocalPlayer->entity) || !g_pLocalPlayer->bZoomed || target->m_iHealth <= bodmg)  && target->m_iHealth <= 150) {
					preferred = ClosestHitbox(target);
					headonly = false;
				}
			}
		} else IF_GAME (IsCSS()) {
			headonly = true;
		}
		if (headonly) {
			IF_GAME (IsTF())
				return hitbox_t::head;
			IF_GAME (IsCSS())
				return 12;
		}
		if (target->hitboxes.VisibilityCheck(preferred)) return preferred;
		for (int i = projectile_mode ? 1 : 0; i < target->hitboxes.GetNumHitboxes(); i++) {
			if (target->hitboxes.VisibilityCheck(i)) return i;
		}
	} break;
	case 1: { // AUTO-CLOSEST
		return ClosestHitbox(target);
	} break;
	case 2: { // STATIC
		return (int)hitbox;
	} break;
	}
	return -1;
}
	
void Draw() {
	//Fov ring to represent when a target will be shot
	//Not perfect but does a good job of representing where its supposed to be
	if (fov_draw) {
		//It cant use fovs greater than 180, so we check for that
		if ((int)fov < 180 && fov) {
			//Dont show ring while player is dead
			if (!LOCAL_E->m_bAlivePlayer) {
				//Grab the screen resolution and save to some vars
				int width, height;
				g_IEngine->GetScreenSize(width, height);
				//Grab the cvar for fov_desired and attach to another var
				static ConVar *realFov = g_ICvar->FindVar("fov_desired");
				//Some math to find radius of the fov circle
				float radius = tanf(DEG2RAD((float)fov) / 2) / tanf(DEG2RAD((int)realFov)/ 2) * width;
				//Draw a circle with our newfound circle
				draw::DrawCircle( width / 2 ,height / 2, radius, 35, GUIColor());
			}
		}
	}	
}

}}}
 
