/*===========================================================================

Club N69 - old school style FPS game parody on "Duke Nukem 3D" and "Wolfenstein 3D".

Copyright (C) 2011-2014 Andrey Korotkov. 

This file is part of the Club N69 GPL Source Code ("Club N69 Source Code").  

Club N69 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Club N69 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Club N69 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Club N69 Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following
the terms and conditions of the GNU General Public License which accompanied the
Club N69 Source Code.

If you have questions concerning this license or application, you may contact
Andrey Korotkov using this email: dron@dronprogs.org.

Visit <http://dronprogs.org> for additional information about this project.

===========================================================================*/

#include "Common.h"
#include "Utils.h"
#include "Level.h"

// Game config and balance

#define KILLS_SOUNDS_COUNT 8
#define ENTERING_SOUNDS_COUNT 3
#define SOLDIER_DEAD_SOUNDS_COUNT 2
#define SCRIPT_SOUNDS_COUNT 4

#define WORLD_UNIT 1.f /*basic 3d world size*/
#define CELL_SIZE (10.f * WORLD_UNIT)
#define UNIT_SIZE (4.f * WORLD_UNIT)
#define BULLETS_Z (6.5f * WORLD_UNIT)
#define CAMERA_Z (7.f * WORLD_UNIT)
#define PARTICLE_SCALE (2.5f * WORLD_UNIT)

#define SPRITE_ANIM_STEP 0.25f

#define BULLET_DAMAGE 20
#define SOLDIER_HEALTH 100
#define DEMON_HEALTH 200
#define ENEMY_SHOT_DELAY 20
#define DEMON_SPREAD 2.5f
#define SOLDIER_SPREAD 10.f
#define ENEMY_MOVING_SPEED (WORLD_UNIT * 0.2f)
#define ENEMY_CHANCE_TO_ATTACK 1 /*in percents*/
#define DEMON_VIEWING_DISTANCE 125.f
#define SOLDIER_VIEWING_DISTANCE 75.f
#define ENEMY_HEALTH_TRESHOLD_TO_EXPLODE -40

#define PLAYER_MOVING_SPEED (WORLD_UNIT * 0.3f)
#define PLAYER_HEALTH 100
#define MEDKIT_HEAL 50
#define PISTOL_AMMO 10
#define SHOTGUN_AMMO 15
#define PISTOL_SPREAD 3.f
#define SHOTGUN_POWER 8
#define SHOTGUN_SPREAD 7.5f

// in update steps
#define PLAYER_AFTER_SHOT_DELAY 8
#define HIT_RED_FLASH_DELAY 150

// Variables

IEngineCore	*pEngineCore;
IRender *pRender;
IRender2D *pRender2D;
IRender3D *pRender3D;
IInput *pInput;

uint uiCounter = 0;
uint uiRedScreenCounter = 0;
uint64 ui64PrevAmmoTick;

uint uiEnvironmentType, uiCurrentLevelId;

IBitmapFont *pSystemFont, *pMainFont;

ITexture *pTexBackdrop, *pTexHUD, *pTexPistol, *pTexShotgun, *pTexSparks, *pTexSoldier, *pTexBlood, *pTexDemon,
	*pTexFloor[2], *pTexCeilings[2], *pTexWalls[10], *pTexSprites[12];

IMesh *pMeshBox;

ISoundSample *pSndDemonPain, *pSndDemonShoot, *pSndDemonDie, *pSndSoldierPain, *pSndSoldierShoot, *pSndSoldierDies[2],
	*pSndPistol, *pSndShotgun, *pSndPlayerPain, *pSndPlayerDie, *pSndShotgunPickup, *pSndMedkit,
	*pSndEnding, *pSndLevels[3], *pSndScripts[4], *pSndKills[8];

bool bIsInGame = false;

TPoint2 stPlayerPos;
float fPlayerAngle;
float fShotAnimFrame;

int iPlayerHealth;
uint uiPistolAmmo, uiShotgunAmmo;

// to save progress between levels
int iSavePlayerHealth;
uint uiSavePistolAmmo, uiSaveShotgunAmmo;

uint8 ui8Map[MAP_SIZE][MAP_SIZE];

struct TSparks { TPoint2 pos; float anim; };
std::vector<TSparks> vecSparks;

struct TBlood { TPoint3 pos; float angle; };
std::vector<TBlood> vecBlood;

struct TEnemy
{
	enum ENEMY_TYPE { SOLDIER, DEMON };
	enum STATE { WALK, ATTACK, DEAD, SHOOT };

	ENEMY_TYPE type;
	STATE state;

	TPoint2 pos;
	float anim, angle;
	int health;
	
	uint sound; // sound played flag: 1 - on hit 2 - on dead
	uint shooting; // delay counter between enemy shots
};

std::vector<TEnemy> vecEnemies;

TMatrix4x4Stack clTransformStack(MatrixIdentity());

void LoadMap(uint uiMapId)
{
	vecSparks.clear();
	vecEnemies.clear();
	vecBlood.clear();

	if (uiMapId > MAPS_COUNT)
	{
		bIsInGame = false;
		
		pSndEnding->Play();
		
		pTexBackdrop->Free();
		
		IResourceManager *p_res_man;
		pEngineCore->GetSubSystem(ESS_RESOURCE_MANAGER, (IEngineSubSystem *&)p_res_man);
		
		p_res_man->Load(RESOURCE_PATH"bg_ending.tga", (IEngineBaseObject *&)pTexBackdrop, TLF_FILTERING_NONE);
		
		return;
	}

	memcpy(ui8Map, &ui8MapsData[uiMapId - 1][0][0], MAP_SIZE * MAP_SIZE * sizeof(uint8));

	fPlayerAngle = ui8MapsData[uiMapId - 1][MAP_SIZE][0] * 2;
	
	uiEnvironmentType = ui8MapsData[uiMapId - 1][MAP_SIZE][1];
	
	if (ui8MapsData[uiMapId - 1][MAP_SIZE][3] != 0)
	{
		pRender3D->ToggleFog(true);
		pRender3D->SetFogColor(uiEnvironmentType == ENV_BASEMENT ? ColorGray() : ColorBlack());
		const float fog_end = (float)ui8MapsData[uiMapId - 1][MAP_SIZE][4] * WORLD_UNIT;
		pRender3D->SetLinearFogBounds(fog_end / 2.f, fog_end);
	}
	else
		pRender3D->ToggleFog(false);

	const uint snd_id = ui8MapsData[uiMapId - 1][MAP_SIZE][2];
	if (snd_id < ENTERING_SOUNDS_COUNT)
		pSndLevels[snd_id]->Play();

	for (uint i = 0; i < MAP_SIZE; ++i)
		for (uint j = 0; j < MAP_SIZE; ++j)
		{
			TEnemy t;

			switch (ui8Map[i][j])
			{
			case SOLDIER_UNIT:
				
				t.type = TEnemy::SOLDIER;
				t.pos = TPoint2(i * CELL_SIZE, j * CELL_SIZE);
				t.anim = 0.f;
				t.angle = RandomFloatInRange(0.f, 360.f);
				t.health = SOLDIER_HEALTH;
				t.shooting = ENEMY_SHOT_DELAY;
				t.sound = 0;
				t.state = TEnemy::WALK;
				vecEnemies.push_back(t);

				ui8Map[i][j] = 0;
				
				break;

			case DEMON_UNIT:
				t.type = TEnemy::DEMON;
				t.pos = TPoint2(i * CELL_SIZE, j * CELL_SIZE);
				t.anim = 0.f;
				t.angle = RandomFloatInRange(0.f, 360.f);

				// make demons stronger on last level
				if (uiCurrentLevelId == MAPS_COUNT)
					t.health = DEMON_HEALTH * 2;
				else
					t.health = DEMON_HEALTH;

				t.shooting = ENEMY_SHOT_DELAY;
				t.state = TEnemy::ATTACK;
				vecEnemies.push_back(t);
				
				ui8Map[i][j] = 0;
				
				break;

			case PLAYER_RESPAWN:
				
				stPlayerPos = TPoint2(i * CELL_SIZE, j * CELL_SIZE);
				fShotAnimFrame = 0.f;
				
				iPlayerHealth = iSavePlayerHealth;
				uiPistolAmmo = uiSavePistolAmmo;
				uiShotgunAmmo = uiSaveShotgunAmmo;

				if (iSavePlayerHealth < PLAYER_HEALTH / 2)
					iSavePlayerHealth += MEDKIT_HEAL / 5;

				if (uiSaveShotgunAmmo == 0 && uiSavePistolAmmo < PISTOL_AMMO * 4)
					uiSavePistolAmmo += PISTOL_AMMO;

				ui8Map[i][j] = 0;
				
				break;
			}
		}
}

bool TraceBullet(const TPoint2 &pos, float angle, bool check = false) // returns true if bullet hits player
{
	const float trace_step = 0.1f;

	// for offset needed to draw sparks and blood outside walls and units
	const float particle_trace_back = 5.f;

	// offset needed to move bullet forward not to collide with shooter
	const float offset = 3.f; 
	
	TPoint2 bullet_pos = pos + TPoint2(sin(ToRadians(angle)) * offset, cos(ToRadians(angle)) * offset);

	while ((bullet_pos - pos).Length() < MAP_SIZE * MAP_SIZE)
	{
		bullet_pos += TPoint2(sin(ToRadians(angle)) * trace_step, cos(ToRadians(angle)) * trace_step);

		for (uint i = 0; i < MAP_SIZE; ++i)
			for (uint j = 0; j < MAP_SIZE; ++j)
				if (
					(ui8Map[i][j] != 0 && ui8Map[i][j] < SCRIPTS_SECTION_BEGIN && ui8Map[i][j] != SHOTGUN_ITEM && ui8Map[i][j] != MEDKIT_ITEM && IsSpriteAnObstacle(ui8Map[i][j])) &&
					(ui8Map[i][j] > MAX_WALL_CODE ?
					TRectF(i * CELL_SIZE - UNIT_SIZE / 2.f, j * CELL_SIZE - UNIT_SIZE / 2.f, UNIT_SIZE, UNIT_SIZE).PointInRect(bullet_pos) :
					TRectF(i * CELL_SIZE - CELL_SIZE / 2.f, j * CELL_SIZE - CELL_SIZE / 2.f, CELL_SIZE, CELL_SIZE).PointInRect(bullet_pos))
					)
				{
					if (!check)
					{
						TSparks t;
						t.pos = bullet_pos - TPoint2(sin(ToRadians(angle)) * trace_step * particle_trace_back, cos(ToRadians(angle)) * trace_step * particle_trace_back);
						t.anim = 0.f;
						vecSparks.push_back(t);
					}

					return false;
				}

		if (!check)
			for (uint i = 0; i < vecEnemies.size(); ++i)
			{
				TEnemy &enemy = vecEnemies[i];

				if (enemy.state != TEnemy::DEAD && TRectF(enemy.pos.x - UNIT_SIZE / 2, enemy.pos.y - UNIT_SIZE / 2, UNIT_SIZE, UNIT_SIZE).PointInRect(bullet_pos))
				{
					TBlood t;
					t.pos = TPoint3(bullet_pos - TPoint2(sin(ToRadians(angle)) * trace_step * particle_trace_back, cos(ToRadians(angle)) * trace_step * particle_trace_back));
					t.pos.z = -BULLETS_Z;
					t.angle = RandomFloatInRange(0.f, 360.f);
					vecBlood.push_back(t);

					enemy.health -= BULLET_DAMAGE;

					if (enemy.health > 0)
					{
						if (enemy.sound == 0)
						{
							enemy.sound = 1;

							if (enemy.type == TEnemy::SOLDIER)
								pSndSoldierPain->Play();
							else
								if (enemy.type == TEnemy::DEMON)
									pSndDemonPain->Play();
						}

						if (rand() % 3 > 1 && enemy.state != TEnemy::SHOOT) // in most cases enemy becomes angry after being hit
							enemy.state = TEnemy::ATTACK;
					}
					else
					{
						if (enemy.sound < 2)
						{
							enemy.sound = 2;

							if (rand() % 5 == 2) // say something but not too often
								pSndKills[rand() % KILLS_SOUNDS_COUNT]->Play();
							else
							{
								if (enemy.type == TEnemy::DEMON)
									pSndDemonDie->Play();
								else
									if (enemy.type == TEnemy::SOLDIER)
										pSndSoldierDies[rand() % SOLDIER_DEAD_SOUNDS_COUNT]->Play();
							}
						}
					}

					return false;
				}
			}
		
		if (iPlayerHealth > 0 && TRectF(stPlayerPos.x - UNIT_SIZE / 2, stPlayerPos.y - UNIT_SIZE / 2, UNIT_SIZE, UNIT_SIZE).PointInRect(bullet_pos))
		{
			if (!check)
			{
				TBlood t;
				t.pos = TPoint3(bullet_pos - TPoint2(sin(ToRadians(angle)) * trace_step * particle_trace_back, cos(ToRadians(angle)) * trace_step * particle_trace_back));
				t.pos.z = -RandomFloatInRange(BULLETS_Z, BULLETS_Z + BULLETS_Z / 2.f);
				t.angle = RandomFloatInRange(0.f, 360.f);
				vecBlood.push_back(t);

				uiRedScreenCounter = HIT_RED_FLASH_DELAY;

				iPlayerHealth -= BULLET_DAMAGE / 2;

				if (iPlayerHealth > 0)
					pSndPlayerPain->Play();
				else
					pSndPlayerDie->Play();
			}

			return true;
		}
	}

	return false;
}

bool TestMapCollisionBox(const TPoint2 &pos, bool player = false)
{
	// make object collider little bit bigger while checking, not to stuck objects in each other
	const TRectF collider = TRectF(pos.x - (UNIT_SIZE + 1.f) / 2.f, pos.y - (UNIT_SIZE + 1.f) / 2.f, UNIT_SIZE + 1.f, UNIT_SIZE + 1.f);

	for (uint i = 0; i < MAP_SIZE; ++i)
		for (uint j = 0; j < MAP_SIZE; ++j)
			if (ui8Map[i][j] != 0 &&
				(ui8Map[i][j] > MAX_WALL_CODE ?
				TRectF(i * CELL_SIZE - UNIT_SIZE / 2.f, j * CELL_SIZE - UNIT_SIZE / 2.f, UNIT_SIZE, UNIT_SIZE).IntersectRect(collider) :
				TRectF(i * CELL_SIZE - CELL_SIZE / 2.f, j * CELL_SIZE - CELL_SIZE / 2.f, CELL_SIZE, CELL_SIZE).IntersectRect(collider))
				)
			{
				if (!IsSpriteAnObstacle(ui8Map[i][j]))
					return false;

				if (ui8Map[i][j] == NEXT_LEVEL_TRIGGER && player)
				{
					iSavePlayerHealth = iPlayerHealth;
					uiSavePistolAmmo = uiPistolAmmo + PISTOL_AMMO;
					uiSaveShotgunAmmo = uiShotgunAmmo;

					LoadMap(++uiCurrentLevelId);
					return false;
				}

				if (ui8Map[i][j] == SHOTGUN_ITEM)
				{
					if (player)
					{
						pSndShotgunPickup->Play();
						ui8Map[i][j] = 0;
						uiShotgunAmmo += SHOTGUN_AMMO;
					}

					return false;
				}

				if (ui8Map[i][j] == MEDKIT_ITEM)
				{
					if (player && iPlayerHealth < PLAYER_HEALTH)
					{
						ui8Map[i][j] = 0;

						iPlayerHealth += MEDKIT_HEAL;

						if (iPlayerHealth > PLAYER_HEALTH)
							iPlayerHealth = PLAYER_HEALTH;

						pSndMedkit->Play();
					}

					return false;
				}

				if (ui8Map[i][j] >= SCRIPTS_SECTION_BEGIN && ui8Map[i][j] <= SCRIPTS_SECTION_BEGIN + ENTERING_SOUNDS_COUNT && player)
				{
					const uint script_sound_id = ui8Map[i][j] - SCRIPTS_SECTION_BEGIN;

					if (script_sound_id < SCRIPT_SOUNDS_COUNT)
						pSndScripts[script_sound_id]->Play();

					ui8Map[i][j] = 0;
				}

				return true;
			}

	return false;
}

void DrawMap()
{
	clTransformStack.Push();

	clTransformStack.MultLocal(MatrixScale(TPoint3(CELL_SIZE, CELL_SIZE, CELL_SIZE)));

	for (uint i = 0; i < MAP_SIZE; ++i)
		for (uint j = 0; j < MAP_SIZE; ++j)
		{
			clTransformStack.Push();

			clTransformStack.MultLocal(MatrixTranslate(TPoint3(-(float)i, -(float)j, 0.f)));
			
			if (ui8Map[i][j] != 0 && ui8Map[i][j] <= MAX_WALL_CODE)
			{
				clTransformStack.MultLocal(MatrixTranslate(TPoint3(0.f, 0.f, -(WORLD_UNIT / 2.f))));
				
				const bool single_wall = IsSingleBlockWall(ui8Map[i][j]);

				if (single_wall)
				{
					clTransformStack.MultLocal(MatrixTranslate(TPoint3(0.f, 0.f, -(WORLD_UNIT / 2.f))));
					clTransformStack.MultLocal(MatrixScale(TPoint3(WORLD_UNIT, WORLD_UNIT, WORLD_UNIT * 2.f)));
				}

				pRender3D->SetMatrix(clTransformStack.Top());

				pTexWalls[ui8Map[i][j] - 1]->Bind();
				pMeshBox->Draw();

				if (!single_wall)
				{
					clTransformStack.MultLocal(MatrixTranslate(TPoint3(0.f, 0.f, -WORLD_UNIT)));
					pRender3D->SetMatrix(clTransformStack.Top());
					
					if (ui8Map[i][j] == 8) // this is hack to draw block with door correctly
						pTexWalls[0]->Bind();
					
					pMeshBox->Draw();
				}
			}
			else
			{
				if (ui8Map[i][j] != 0 && ui8Map[i][j] < SOLDIER_UNIT)
				{
					pRender3D->ToggleAlphaTest(true);
					
					clTransformStack.Push();

					clTransformStack.MultLocal(GetSpriteLocalTransform(ui8Map[i][j]) * MatrixTranslate(TPoint3(0.f, 0.f, -(WORLD_UNIT / 2.f))));
					
					pRender3D->SetMatrix(MatrixBillboard(clTransformStack.Top()));

					pTexSprites[ui8Map[i][j] - (MAX_WALL_CODE + 1)]->Draw3D(GetSpriteAnimationFrame(ui8Map[i][j], uiCounter));

					clTransformStack.Pop();
					
					pRender3D->ToggleAlphaTest(false);
				
					if (SpriteHasBase(ui8Map[i][j]))
					{
						clTransformStack.Push();
						
						pRender3D->SetMatrix(clTransformStack.Top());
						
						pTexFloor[uiEnvironmentType]->Bind();
						pMeshBox->Draw();
						
						clTransformStack.Pop();
					}
				}

				clTransformStack.Push();
				clTransformStack.MultLocal(MatrixRotate(180.f, TVector3(1.f, 0.f, 0.f)));
				pRender3D->SetMatrix(clTransformStack.Top());
				pTexFloor[uiEnvironmentType]->Draw3D();
				clTransformStack.Pop();

				clTransformStack.MultLocal(MatrixTranslate(TPoint3(0.f, 0.f, -(WORLD_UNIT * 2.f))));
				pRender3D->SetMatrix(clTransformStack.Top());
				pTexCeilings[uiEnvironmentType]->Draw3D();
			}

			clTransformStack.Pop();
		}

		clTransformStack.Pop();
}

void DGLE_API Init(void *pParameter)
{
	pEngineCore->GetTimer(ui64PrevAmmoTick);

	pEngineCore->GetSubSystem(ESS_RENDER, (IEngineSubSystem *&)pRender);
	
	pRender->GetRender2D(pRender2D);
	pRender2D->SetResolutionCorrection(GAME_VIEWPORT_WIDTH, GAME_VIEWPORT_HEIGHT, false);

	pRender->GetRender3D(pRender3D);
	pRender3D->SetPerspective(GAME_FOV, 0.05f, MAP_SIZE * MAP_SIZE * CELL_SIZE);

	pEngineCore->GetSubSystem(ESS_INPUT, (IEngineSubSystem *&)pInput);
	pInput->Configure((E_INPUT_CONFIGURATION_FLAGS)(ICF_HIDE_CURSOR | ICF_CURSOR_BEYOND_SCREEN | ICF_EXCLUSIVE));
	
	IResourceManager *p_res_man;
	pEngineCore->GetSubSystem(ESS_RESOURCE_MANAGER, (IEngineSubSystem *&)p_res_man);

	IMusic *p_music;
	p_res_man->Load(RESOURCE_PATH"music.mp3",(IEngineBaseObject *&)p_music);
	p_music->Play();
	p_music->SetVolume(60);

	p_res_man->GetDefaultResource(EOT_MESH, (IEngineBaseObject *&)pMeshBox);
	
	p_res_man->GetDefaultResource(EOT_BITMAP_FONT, (IEngineBaseObject *&)pSystemFont);

	p_res_man->Load(RESOURCE_PATH"font.dft", (IEngineBaseObject *&)pMainFont, BFLF_FILTERING_NONE);

	// Load sounds

	p_res_man->Load(RESOURCE_PATH"pistol_shot.wav", (IEngineBaseObject *&)pSndPistol);
	p_res_man->Load(RESOURCE_PATH"shotgun_shot.wav", (IEngineBaseObject *&)pSndShotgun);
	p_res_man->Load(RESOURCE_PATH"pain.wav", (IEngineBaseObject *&)pSndPlayerPain);
	p_res_man->Load(RESOURCE_PATH"pdeath.wav", (IEngineBaseObject *&)pSndPlayerDie);
	p_res_man->Load(RESOURCE_PATH"shotgun_pickup.wav", (IEngineBaseObject *&)pSndShotgunPickup);
	p_res_man->Load(RESOURCE_PATH"medkit.wav", (IEngineBaseObject *&)pSndMedkit);
	p_res_man->Load(RESOURCE_PATH"soldier_pain.wav", (IEngineBaseObject *&)pSndSoldierPain);
	p_res_man->Load(RESOURCE_PATH"soldier_shoot.wav", (IEngineBaseObject *&)pSndSoldierShoot);
	p_res_man->Load(RESOURCE_PATH"ending.wav", (IEngineBaseObject *&)pSndEnding);
	
	char buffer[256];

	for (uint i = 0; i < SOLDIER_DEAD_SOUNDS_COUNT; ++i)
	{
		sprintf(buffer, RESOURCE_PATH"%s%d.wav", "soldier_death_", i);
		p_res_man->Load(buffer, (IEngineBaseObject *&)pSndSoldierDies[i]);
	}

	for (uint i = 0; i < ENTERING_SOUNDS_COUNT; ++i)
	{
		sprintf(buffer, RESOURCE_PATH"%s%d.wav", "level_", i);
		p_res_man->Load(buffer, (IEngineBaseObject *&)pSndLevels[i]);
	}

	for (uint i = 0; i < SCRIPT_SOUNDS_COUNT; ++i)
	{
		sprintf(buffer, RESOURCE_PATH"%s%d.wav", "script_", i);
		p_res_man->Load(buffer, (IEngineBaseObject *&)pSndScripts[i]);
	}

	for (uint i = 0; i < KILLS_SOUNDS_COUNT; ++i)
	{
		sprintf(buffer, RESOURCE_PATH"%s%d.wav", "kill_", i);
		p_res_man->Load(buffer, (IEngineBaseObject *&)pSndKills[i]);
	}

	p_res_man->Load(RESOURCE_PATH"demon_pain.wav", (IEngineBaseObject *&)pSndDemonPain);
	p_res_man->Load(RESOURCE_PATH"demon_die.wav", (IEngineBaseObject *&)pSndDemonDie);
	p_res_man->Load(RESOURCE_PATH"demon_shoot.wav", (IEngineBaseObject *&)pSndDemonShoot);
	
	// Load textures

	p_res_man->Load(RESOURCE_PATH"bg.tga", (IEngineBaseObject *&)pTexBackdrop, TLF_FILTERING_NONE);
	p_res_man->Load(RESOURCE_PATH"hud.tga", (IEngineBaseObject *&)pTexHUD, TLF_FILTERING_NONE);

	p_res_man->Load(RESOURCE_PATH"pistol.tga", (IEngineBaseObject *&)pTexPistol, TLF_FILTERING_NONE);
	pTexPistol->SetFrameSize(90, 128);
	
	p_res_man->Load(RESOURCE_PATH"shotgun.tga", (IEngineBaseObject *&)pTexShotgun, TLF_FILTERING_NONE);
	pTexShotgun->SetFrameSize(161, 196);
	
	p_res_man->Load(RESOURCE_PATH"sparks.tga", (IEngineBaseObject *&)pTexSparks, TLF_FILTERING_NONE);
	pTexSparks->SetFrameSize(14, 16);
	
	p_res_man->Load(RESOURCE_PATH"soldier.tga", (IEngineBaseObject *&)pTexSoldier, TLF_FILTERING_NONE);
	pTexSoldier->SetFrameSize(59, 64);
	
	p_res_man->Load(RESOURCE_PATH"demon.tga", (IEngineBaseObject *&)pTexDemon, TLF_FILTERING_NONE);
	pTexDemon->SetFrameSize(64, 64);

	p_res_man->Load(RESOURCE_PATH"blood.tga", (IEngineBaseObject *&)pTexBlood, TLF_FILTERING_NONE);
	
	// Load map environment and walls

	for (uint i = 0; i < ENV_COUNT; ++i)
	{
		sprintf(buffer, RESOURCE_PATH"%s%d.tga", "floor_", i);
		p_res_man->Load(buffer, (IEngineBaseObject *&)pTexFloor[i], (uint)(TLF_FILTERING_NONE | TLF_GENERATE_MIPMAPS));
		
		sprintf(buffer, RESOURCE_PATH"%s%d.tga", "ceiling_", i);
		p_res_man->Load(buffer, (IEngineBaseObject *&)pTexCeilings[i], (uint)(TLF_FILTERING_NONE | TLF_GENERATE_MIPMAPS));
	}

	for (uint i = 0; i < MAX_WALL_CODE; ++i)
	{
		sprintf(buffer, RESOURCE_PATH"%s%d.tga", "wall_", i);
		p_res_man->Load(buffer, (IEngineBaseObject *&)pTexWalls[i], (uint)(TLF_FILTERING_NONE | TLF_GENERATE_MIPMAPS));
	}

	// Load map sprites

	p_res_man->Load(RESOURCE_PATH"shotgun_sprite.tga", (IEngineBaseObject *&)pTexSprites[SHOTGUN_ITEM - (MAX_WALL_CODE + 1)], TLF_FILTERING_NONE);
	p_res_man->Load(RESOURCE_PATH"medkit.tga", (IEngineBaseObject *&)pTexSprites[MEDKIT_ITEM - (MAX_WALL_CODE + 1)], TLF_FILTERING_NONE);

	for (uint i = 0; i < MAX_SPRITE_CODE - MAX_WALL_CODE; ++i)
	{
		sprintf(buffer, RESOURCE_PATH"%s%d.tga", "sprite_", i);
		p_res_man->Load(buffer, (IEngineBaseObject *&)pTexSprites[i], TLF_FILTERING_NONE);
	}

	// setup map sprites animation frames
	pTexSprites[1]->SetFrameSize(44, 128);
	pTexSprites[2]->SetFrameSize(56, 192);
	pTexSprites[3]->SetFrameSize(48, 96);
	pTexSprites[4]->SetFrameSize(35, 92);
	pTexSprites[5]->SetFrameSize(73, 110);
	pTexSprites[9]->SetFrameSize(74, 113);
}

void UpdateEnemy(TEnemy &enemy)
{
	enemy.sound = 0;

	if (enemy.state != TEnemy::DEAD && enemy.health <= 0)
	{
		enemy.state = TEnemy::DEAD;

		if (enemy.type == TEnemy::SOLDIER)
		{
			if (enemy.health <= ENEMY_HEALTH_TRESHOLD_TO_EXPLODE)
				enemy.anim = 42;
			else
				enemy.anim = 37;
		}
		else
			if (enemy.type == TEnemy::DEMON)
				enemy.anim = 7;

		return;
	}

	if (enemy.state == TEnemy::SHOOT)
	{
		enemy.anim += SPRITE_ANIM_STEP * 0.8f;

		if (((enemy.type == TEnemy::SOLDIER && (uint)enemy.anim == 35) || (enemy.type == TEnemy::DEMON && (uint)enemy.anim == 5)) && enemy.shooting >= ENEMY_SHOT_DELAY)
		{
			enemy.shooting = 0;
			
			if (enemy.type == TEnemy::DEMON)
			{
				pSndDemonShoot->Play();
				TraceBullet(enemy.pos, enemy.angle + RandomFloatInRange(-DEMON_SPREAD, DEMON_SPREAD));
			}
			else
				if (enemy.type == TEnemy::SOLDIER)
				{
					pSndSoldierShoot->Play();
					TraceBullet(enemy.pos, enemy.angle + RandomFloatInRange(-SOLDIER_SPREAD, SOLDIER_SPREAD));
				}
		}

		++enemy.shooting;

		if ((enemy.type == TEnemy::SOLDIER && (uint)enemy.anim == 36) || (enemy.type == TEnemy::DEMON && (uint)enemy.anim == 7))
		{
			enemy.anim = 1;
			enemy.state = TEnemy::ATTACK;
			return;
		}
	}
	else
		if (enemy.state == TEnemy::WALK || enemy.state == TEnemy::ATTACK)
		{
			const float viewing_distance = enemy.type == TEnemy::SOLDIER ? SOLDIER_VIEWING_DISTANCE : (uiCurrentLevelId == MAPS_COUNT ? DEMON_VIEWING_DISTANCE * 2.f : DEMON_VIEWING_DISTANCE);

			if (enemy.state == TEnemy::ATTACK || enemy.state == TEnemy::SHOOT)
			{
				enemy.angle = -ToDegrees(atan2(stPlayerPos.y - enemy.pos.y, stPlayerPos.x - enemy.pos.x)) + 
					// this will make enemies go away off the dead body
					(iPlayerHealth > 0 ? 90.f : -90.f);

				if ((enemy.pos - stPlayerPos).Length() < viewing_distance && TraceBullet(enemy.pos, enemy.angle, true))
				{
					enemy.state = TEnemy::SHOOT;

					if (enemy.type == TEnemy::SOLDIER)
						enemy.anim = 34;
					else
						if (enemy.type == TEnemy::DEMON)
							enemy.anim = 4;

					return;
				}
			}

			const bool attack_chance = rand() % 100 < ENEMY_CHANCE_TO_ATTACK;

			if (enemy.state == TEnemy::WALK && attack_chance && (stPlayerPos - enemy.pos).Length() < viewing_distance * 1.35f)
				enemy.state = TEnemy::ATTACK;
			
			const TPoint2 d = TPoint2(sin(ToRadians(enemy.angle)) * ENEMY_MOVING_SPEED, cos(ToRadians(enemy.angle)) * ENEMY_MOVING_SPEED);

			if (!TestMapCollisionBox(enemy.pos + TPoint2(d.x, 0.f)))
				enemy.pos.x += d.x;
			else
			{
				enemy.state = TEnemy::WALK;
				enemy.angle = rand() % 360;
			}

			if (!TestMapCollisionBox(enemy.pos + TPoint2(0.f, d.y)))
				enemy.pos.y += d.y;
			else
			{
				enemy.state = TEnemy::WALK;
				enemy.angle = rand() % 360;
			}

			enemy.anim += SPRITE_ANIM_STEP * 0.6f;;
			
			if ((uint)enemy.anim == 4)
				enemy.anim = 0.f;
		}
		else
			if (enemy.state == TEnemy::DEAD)
			{
				enemy.anim += SPRITE_ANIM_STEP * 0.6f;

				if (enemy.type == TEnemy::SOLDIER)
				{
					if ((uint)enemy.anim == 41)
						enemy.anim = 41;

					if ((uint)enemy.anim == 50)
						enemy.anim = 50;
				}
				else
					if (enemy.type == TEnemy::DEMON)
					{
						if ((uint)enemy.anim == 15)
							enemy.anim = 15;
					}
			}
}

void UpdatePlayerInput()
{
	bool key;

	if (iPlayerHealth <= 0)
	{
		pInput->GetKey(KEY_SPACE, key);
		
		if (key)
		{
			uiRedScreenCounter = 0;
			LoadMap(uiCurrentLevelId);
		}

		return;
	}

	const E_KEYBOARD_KEY_CODES move_keys[4] = {KEY_W, KEY_S, KEY_A, KEY_D};
	const float move_dirs[4] = {0.f, 180.f, 90.f, -90.f};

	for (uint i = 0; i < 4; ++i)
	{
		const float angle = fPlayerAngle + move_dirs[i];

		pInput->GetKey(move_keys[i], key);
	
		if (key)
		{
			const TPoint2 d = TPoint2(sin(ToRadians(angle)) * PLAYER_MOVING_SPEED, cos(ToRadians(angle)) * PLAYER_MOVING_SPEED);

			if (!TestMapCollisionBox(stPlayerPos + TPoint2(d.x, 0.f), true))
				stPlayerPos.x += d.x;
			
			if (!TestMapCollisionBox(stPlayerPos + TPoint2(0.f, d.y), true))
				stPlayerPos.y += d.y;
		}
	}

	TMouseStates ms;
	pInput->GetMouseStates(ms);

	fPlayerAngle -= ms.iDeltaX / MOUSE_SENSITIVITY;

	if (fPlayerAngle < 0.f) fPlayerAngle = 360.f + fPlayerAngle;
	if (fPlayerAngle > 360.f) fPlayerAngle = fPlayerAngle - 360.f;

	// weapon animation
	if ((uint)fShotAnimFrame != 0)
	{
		if (uiShotgunAmmo == 0)
		{
			fShotAnimFrame += SPRITE_ANIM_STEP;
			
			if ((uint)fShotAnimFrame == 3/*pistol animation last frame*/)
				fShotAnimFrame = -(PLAYER_AFTER_SHOT_DELAY * SPRITE_ANIM_STEP);
		}
		else
			{
				fShotAnimFrame += SPRITE_ANIM_STEP / 1.5f;
			
				if ((uint)fShotAnimFrame == 7/*shotgun animation last frame*/)
					fShotAnimFrame = -(PLAYER_AFTER_SHOT_DELAY * SPRITE_ANIM_STEP);
			}
	}

	if (ms.bLeftButton && (uint)fShotAnimFrame == 0)
	{
		if(uiShotgunAmmo == 0 && uiPistolAmmo != 0)
		{
			fShotAnimFrame = 1.f;
			--uiPistolAmmo;

			TraceBullet(stPlayerPos, fPlayerAngle + RandomFloatInRange(-PISTOL_SPREAD, PISTOL_SPREAD));

			pSndPistol->Play();
		}
		else
			if (uiShotgunAmmo != 0)
			{
				fShotAnimFrame = 1.f;
				--uiShotgunAmmo;

				for (uint i = 0; i < SHOTGUN_POWER; ++i)
					TraceBullet(stPlayerPos, fPlayerAngle + RandomFloatInRange(-SHOTGUN_SPREAD, SHOTGUN_SPREAD));

				pSndShotgun->Play();
			}
	}
}

void UpdateGame()
{
	if (uiRedScreenCounter != 0)
		uiRedScreenCounter--;

	uint k = 0;

	while (k < vecSparks.size())
	{
		vecSparks[k].anim += SPRITE_ANIM_STEP;
		
		if ((uint)vecSparks[k].anim == 3 /*spark animation last frame*/)
			vecSparks.erase(vecSparks.begin() + k);
		else
			++k;
	}

	for (k = 0; k < vecBlood.size(); ++k)
	{
		if (vecBlood[k].pos.z <= WORLD_UNIT * 1.5f)
			vecBlood[k].pos.z += WORLD_UNIT * 0.5f;
	}

	for (k = 0; k < vecEnemies.size(); ++k)
		UpdateEnemy(vecEnemies[k]);
	
	// give player some bullets if ran out of ammo
	uint64 tick;
	pEngineCore->GetTimer(tick);
	if (uiPistolAmmo < 10 && tick - ui64PrevAmmoTick > 2000 /*2 seconds*/)
	{
		ui64PrevAmmoTick = tick;
		++uiPistolAmmo;
	}

	UpdatePlayerInput();
}

void DGLE_API Update(void *pParameter)
{
	bool key;

	pInput->GetKey(KEY_ESCAPE, key);
	
	if (key)
		pEngineCore->QuitEngine();

	if (bIsInGame)
		UpdateGame();
	else
	{
		pInput->GetKey(KEY_SPACE, key);

		if(key)
		{
			bIsInGame = true;
			uiRedScreenCounter = 0;
			
			iSavePlayerHealth = PLAYER_HEALTH;
			uiSavePistolAmmo = PISTOL_AMMO * 6;
			uiSaveShotgunAmmo = 0;
			
			uiCurrentLevelId = 1;
			LoadMap(uiCurrentLevelId);
		}
	}

	++uiCounter;
}

void RenderGame()
{
	clTransformStack.Clear(MatrixIdentity());

	clTransformStack.MultLocal(MatrixRotate(90.f, TPoint3(1.f, 0.f, 0.f)));
	clTransformStack.MultLocal(MatrixRotate(fPlayerAngle, TPoint3(0.f, 0.f, 1.f)));
	clTransformStack.MultLocal(MatrixTranslate(TPoint3(stPlayerPos) + TPoint3(0.f, 0.f, iPlayerHealth > 0 ? CAMERA_Z : WORLD_UNIT * 0.3f)));
	
	DrawMap();

	pRender3D->ToggleAlphaTest(true);

	for (uint i = 0; i < vecSparks.size(); ++i)
	{
		clTransformStack.Push();
		
		clTransformStack.MultLocal(MatrixTranslate(TPoint3(-vecSparks[i].pos.x, -vecSparks[i].pos.y, -BULLETS_Z)));
		
		pRender3D->SetMatrix(MatrixScale(TPoint3(PARTICLE_SCALE, PARTICLE_SCALE, 1.f)) * MatrixBillboard(clTransformStack.Top()));
		
		pTexSparks->Draw3D(1 + (uint)vecSparks[i].anim);
		
		clTransformStack.Pop();
	}

	for (uint i = 0; i < vecBlood.size(); ++i)
	{
		clTransformStack.Push();

		if (vecBlood[i].pos.z > 1.5f * WORLD_UNIT)
		{
			clTransformStack.MultLocal(MatrixTranslate(TPoint3(-vecBlood[i].pos.x, -vecBlood[i].pos.y, -0.1f * WORLD_UNIT)));
			clTransformStack.MultLocal(MatrixScale(TPoint3(PARTICLE_SCALE, PARTICLE_SCALE, PARTICLE_SCALE)));
			clTransformStack.MultLocal(MatrixRotate(180.f, TVector3(1.f, 0.f, 0.f)));
			pRender3D->SetMatrix(clTransformStack.Top());
		}
		else
		{
			clTransformStack.MultLocal(MatrixTranslate(TPoint3(-vecBlood[i].pos.x, -vecBlood[i].pos.y, vecBlood[i].pos.z)));
			pRender3D->SetMatrix(MatrixScale(TPoint3(PARTICLE_SCALE, PARTICLE_SCALE, 1.f)) * MatrixBillboard(clTransformStack.Top()));
		}
			
		pTexBlood->Draw3D(0);

		clTransformStack.Pop();
	}

	for (uint i = 0; i < vecEnemies.size(); ++i)
	{
		TEnemy &enemy = vecEnemies[i];

		clTransformStack.Push();
			
		if (enemy.type == TEnemy::SOLDIER)
		{
			clTransformStack.MultLocal(MatrixTranslate(TPoint3(-enemy.pos.x, -enemy.pos.y, -(CELL_SIZE / 2.f))));
			pRender3D->SetMatrix(MatrixScale(TPoint3(CELL_SIZE, CELL_SIZE, 1.f)) * MatrixBillboard(clTransformStack.Top()));
				
			if (enemy.state == TEnemy::WALK || enemy.state == TEnemy::ATTACK)
			{
				float angle = enemy.angle - fPlayerAngle;
				
				if (angle > 360.f) angle -= 360.f;
				if (angle < 0.f) angle += 360.f;

				int state = 0;

				// 45 / 2 = 22.5
				if (angle >= 360.f - 22.5f || angle <= 0.f + 22.5f) state = 4;
				if (angle >= 45.f - 22.5f && angle <= 45.f + 22.5f) state = 3;
				if (angle >= 90.f - 22.5f && angle <= 90.f + 22.5f) state = 2;
				if (angle >= 135.f - 22.5f && angle <= 135.f + 22.5f) state = 1;
				if (angle >= 180.f - 22.5f && angle <= 180.f + 22.5f) state = 0;
				if (angle >= 225.f - 22.5f && angle <= 225.f + 22.5f) state = 7;
				if (angle >= 270.f - 22.5f && angle <= 270.f + 22.5f) state = 6;
				if (angle >= 315.f - 22.5f && angle <= 315.f + 22.5f) state = 5;

				pTexSoldier->Draw3D(1 + (uint)vecEnemies[i].anim * 8 + state);
			}
			else
				if(vecEnemies[i].state == TEnemy::DEAD || vecEnemies[i].state == TEnemy::SHOOT)
					pTexSoldier->Draw3D((uint)vecEnemies[i].anim);
		}
		else
			if (vecEnemies[i].type == TEnemy::DEMON)
			{
				if (uiCurrentLevelId == MAPS_COUNT) // draw boss
				{
					const float boss_scale = 1.5f;
					
					pRender3D->SetColor(ColorGreen());
					
					clTransformStack.MultLocal(MatrixTranslate(TPoint3(-vecEnemies[i].pos.x, -vecEnemies[i].pos.y, -(CELL_SIZE / 2.f) * boss_scale)));
					pRender3D->SetMatrix(MatrixScale(TPoint3(CELL_SIZE, CELL_SIZE * boss_scale, 1.f))*MatrixBillboard(clTransformStack.Top()));
					
					pTexDemon->Draw3D(1 + (uint)vecEnemies[i].anim);
					
					pRender3D->SetColor(ColorWhite());
				}
				else
				{
					clTransformStack.MultLocal(MatrixTranslate(TPoint3(-vecEnemies[i].pos.x, -vecEnemies[i].pos.y, -(CELL_SIZE / 2.f))));
					pRender3D->SetMatrix(MatrixScale(TPoint3(CELL_SIZE, CELL_SIZE, 1.f)) * MatrixBillboard(clTransformStack.Top()));
					
					pTexDemon->Draw3D(1 + (uint)vecEnemies[i].anim);
				}
			}

		clTransformStack.Pop();
	}

	pRender3D->ToggleAlphaTest(false);
}

void RenderHUD()
{
	pRender2D->Begin2D();
		
	if (uiRedScreenCounter != 0)
		pRender2D->DrawRectangle(TRectF(0, 0, GAME_VIEWPORT_WIDTH, GAME_VIEWPORT_HEIGHT), ColorRed(uiRedScreenCounter), PF_FILL);

	uint w, h;
	pTexHUD->GetDimensions(w, h);
	pTexHUD->Draw2DSimple(0, GAME_VIEWPORT_HEIGHT - h);

	char buffer[16];
		
	sprintf(buffer, "%d", iPlayerHealth);
	pMainFont->Draw2D(60, GAME_VIEWPORT_HEIGHT - 34, buffer, ColorRed());
		
	if (iPlayerHealth > 0)
	{
		const TPoint2 shaking = TPoint2(cos(stPlayerPos.x / 2.f) * 10.f, sin(stPlayerPos.y / 2.f) * 10.f);

		if (uiShotgunAmmo == 0)
		{
			pTexPistol->GetFrameSize(w, h);
			pTexPistol->Draw2D(350.f + shaking.x, 261.f + shaking.y, w * 2.f, h * 2.f, 0.f, fShotAnimFrame < 0.f ? 0 : (uint)fShotAnimFrame); 
			
			sprintf(buffer, "%d", uiPistolAmmo);
			pMainFont->Draw2D(10.f , GAME_VIEWPORT_HEIGHT - 34, buffer, ColorRed());
		}
		else
		{
			pTexShotgun->GetFrameSize(w, h);
			pTexShotgun->Draw2D(320.f + shaking.x, 101.f + shaking.y, w * 2.f, h * 2.f, 0.f, fShotAnimFrame < 0.f ? 0 : (uint)fShotAnimFrame); 
			
			sprintf(buffer, "%d", uiShotgunAmmo);
			pMainFont->Draw2D(10.f , GAME_VIEWPORT_HEIGHT - 34, buffer, ColorRed());
		}
	}
	else
	{
		const char txt[] = "Press \"Space\" to restart level!";
		pMainFont->GetTextDimensions(txt, w, h);
		pMainFont->Draw2D((GAME_VIEWPORT_WIDTH - w) / 2.f, GAME_VIEWPORT_HEIGHT - h * 4.f, txt, ColorRed());
	}

	pRender2D->End2D();
}

void RenderMenu()
{
	pRender2D->Begin2D();

	pTexBackdrop->Draw2D(0, 0, GAME_VIEWPORT_WIDTH, GAME_VIEWPORT_HEIGHT);
		
	if (uiCurrentLevelId == 0) // not at game ending screen
	{
		uint w, h;

		pTexSprites[2]->GetDimensions(w, h);	
		h *= 2;
		pTexSprites[2]->Draw2D(GAME_VIEWPORT_WIDTH - w, (GAME_VIEWPORT_HEIGHT - h) / 2.f, w, h, 0.f, (uiCounter / 5) % 3);
			
		pSystemFont->GetTextDimensions(APP_VERSION, w, h);
		pSystemFont->Draw2DSimple(GAME_VIEWPORT_WIDTH - w, 0, APP_VERSION);
	}
		
	pRender2D->End2D();
}

void DGLE_API Render(void *pParameter)
{
	if(bIsInGame)
	{
		RenderGame();
		RenderHUD();
	}
	else
		RenderMenu();
}

void DGLE_API OnFullScreenEvent(void *pParameter, IBaseEvent *pEvent)
{
	IEvGoFullScreen *p_event = (IEvGoFullScreen *)pEvent;

	uint res_width, res_height;
	bool go_fscreen;
	p_event->GetResolution(res_width, res_height, go_fscreen);

	if (go_fscreen)
	{
		pEngineCore->GetDesktopResolution(res_width, res_height);
		p_event->SetResolution(res_width, res_height);
	}
	else
		p_event->SetResolution(WINDOW_WIDTH, WINDOW_HEIGHT);
}

DGLE_DYNAMIC_FUNC

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if (GetEngine(DLL_PATH, pEngineCore))
	{
#ifdef _DEBUG
		const TEngineWindow wnd = TEngineWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, false);
#else
		const bool fscreen = MessageBox(NULL, "Run in fullscreen?", "Screen", MB_YESNO | MB_ICONQUESTION) == IDYES;
		
		uint res_width, res_height;
		pEngineCore->GetDesktopResolution(res_width, res_height);

		const TEngineWindow wnd = TEngineWindow(fscreen ? res_width : WINDOW_WIDTH, fscreen ? res_height : WINDOW_HEIGHT, fscreen, true);
#endif

		if (SUCCEEDED(pEngineCore->InitializeEngine(NULL, APP_CAPTION, wnd, GAME_UPDATE_INTERVAL, EIF_LOAD_ALL_PLUGINS)))
		{
#ifdef _DEBUG
			pEngineCore->ConsoleVisible(true);
			pEngineCore->ConsoleExecute("core_profiler 1");
			pEngineCore->ConsoleExecute("crgl_profiler 1");
			pEngineCore->ConsoleExecute("snd_profiler 1");
#endif
			pEngineCore->AddProcedure(EPT_INIT,	&Init);
			pEngineCore->AddProcedure(EPT_UPDATE, &Update);
			pEngineCore->AddProcedure(EPT_RENDER, &Render);
			pEngineCore->AddEventListener(ET_ON_FULLSCREEN, &OnFullScreenEvent);

			pEngineCore->StartEngine();
		}

		FreeEngine();
	}
	else
		MessageBox(NULL, "Couldn't load \""DLL_PATH"\"!", APP_CAPTION, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
	
	return 0;
}