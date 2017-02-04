/*
 * HBunnyhop.cpp
 *
 *  Created on: Oct 6, 2016
 *      Author: nullifiedcat
 */

#include "Bunnyhop.h"

#include "../common.h"
#include "../sdk.h"

DEFINE_HACK_SINGLETON(Bunnyhop);

const char* Bunnyhop::GetName() {
	return "BUNNYHOP";
}

Bunnyhop::Bunnyhop() {
	this->v_bEnabled = new CatVar(CV_SWITCH, "bhop_enabled", "0", "Enable", NULL, "Enable Bunnyhop");
	this->v_bAutoJump = new CatVar(CV_SWITCH, "bhop_autojump", "0", "AutoJump", NULL, "Autojump if you reach certain speed");
	this->v_iAutoJumpSpeed = new CatVar(CV_INT, "bhop_autojump_speed", "300", "AutoJump speed", NULL, "Minimum autojump speed");
}

bool bDoubleJumpFix = false;
int iTicksFlying = 0;
int iTicksLastJump = 0;

bool Bunnyhop::CreateMove(void* thisptr, float sampling, CUserCmd* cmd) {
	if (!this->v_bEnabled->GetBool()) return true;
	if (HasCondition(g_pLocalPlayer->entity, TFCond_GrapplingHook)) return true;
	int flags = CE_INT(g_pLocalPlayer->entity, netvar.iFlags);

	if (v_bAutoJump->GetBool()) {
		Vector vel = CE_VECTOR(g_pLocalPlayer->entity, netvar.vVelocity);
		if ((vel.x * vel.x + vel.y * vel.y) > SQR(v_iAutoJumpSpeed->GetInt())) {
			cmd->buttons |= IN_JUMP;
		}
	}

	bool ground = (flags & (1 << 0));
	bool jump = (cmd->buttons & IN_JUMP);

	if (ground) {
		iTicksFlying = 0;
	} else {
		iTicksFlying++;
	}

	if (!ground && jump) {
		if (iTicksLastJump++ >= 20) cmd->buttons = cmd->buttons &~ IN_JUMP;
	}
	if (!jump) iTicksLastJump = 0;

	/*if (jump && !ground) {
		if (!bDoubleJumpFix) {
			cmd->buttons = cmd->buttons &~ IN_JUMP;
		}
		bDoubleJumpFix = false;
	}
	if (!jump && !ground) {
		bDoubleJumpFix = true;
	}
	if (ground) {
		bDoubleJumpFix = false;
	}*/
	return true;
}

void Bunnyhop::PaintTraverse(void*, unsigned int, bool, bool) {}

void Bunnyhop::LevelInit(const char*) {}
void Bunnyhop::LevelShutdown() {}