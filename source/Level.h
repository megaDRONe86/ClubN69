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

#pragma once

#include "Common.h"

// Game objects codes

/*
Obstacles

	1 - 10 : Different wall types based on corresponding textures "wall_N.bmp", where N = code - 1
	11 - 20 : Different decoration sprites based on corresponding image "sprite_N.png", where N = code - 11
	
Items

	21 : Shotgun
	22 : Medkit
	
Units
	
	23 : Soldier
	24 : Demon

Scripts

	90 - 94 : Player speach based on corresponding sound files "script_N.wav", where N = code - 90
	99 : Player respawn
	100: Next level location trigger

Warning: Be careful while changing defines below.
*/

#define MAX_WALL_CODE 10
#define MAX_SPRITE_CODE 20

#define SHOTGUN_ITEM 21
#define MEDKIT_ITEM 22

#define SOLDIER_UNIT 23
#define DEMON_UNIT 24

#define SCRIPTS_SECTION_BEGIN 90
#define PLAYER_RESPAWN 99
#define NEXT_LEVEL_TRIGGER 100

// Sprites obstacles setup

TMatrix4x4 GetSpriteLocalTransform(uint uiSpriteCode)
{
	switch (uiSpriteCode - MAX_WALL_CODE - 1)
	{
		case 0: return MatrixScale(TPoint3(0.75f, 1.5f, 1.f)) * MatrixTranslate(TPoint3(0.f, 0.f, -0.25f));
		case 1: return MatrixScale(TPoint3(0.75f, 1.5f, 1.f)) * MatrixTranslate(TPoint3(0.f, 0.f, -0.75f));
		case 2: return MatrixScale(TPoint3(1.f, 2.f, 1.f)) * MatrixTranslate(TPoint3(0.f, 0.f, -0.5f));
		case 3: return MatrixScale(TPoint3(0.5f, 1.f, 1.f)) * MatrixTranslate(TPoint3(0.f, 0.f, -0.5f));
		case 4: return MatrixScale(TPoint3(0.5f, 1.f, 1.f));
		case 5: return MatrixScale(TPoint3(0.5f, 1.f, 1.f));
		case 6: return MatrixScale(TPoint3(0.75f, 1.5f, 1.f)) * MatrixTranslate(TPoint3(0.f, 0.f, -0.75f));
		case 7: return MatrixScale(TPoint3(0.5f, 1.f, 1.f));
		case 8: return MatrixScale(TPoint3(0.75f, 0.75f, 1.f)) * MatrixTranslate(TPoint3(0.f, 0.f, 0.15f));
		case 9: return MatrixScale(TPoint3(0.75f, 1.5f, 1.f));
		case 10:return MatrixScale(TPoint3(1.f, 0.5f, 1.f)) * MatrixTranslate(TPoint3(0.f, 0.f, 0.2f));
		case 11:return MatrixScale(TPoint3(0.5f, 0.25f, 1.f)) * MatrixTranslate(TPoint3(0.f, 0.f, 0.38f));

		default: return MatrixIdentity();
	}
}

uint GetSpriteAnimationFrame(uint uiSpriteCode, uint uiCounter)
{
	switch (uiSpriteCode - MAX_WALL_CODE - 1)
	{
		case 1: return 1 + (uiCounter / 5) % 4;
		case 2: return 1 + (uiCounter / 5) % 3;
		case 3: return 1 + (uiCounter / 10) % 10;
		case 4: return 1 + (uiCounter / 10) % 2;
		case 5: return 1 + (uiCounter / 5) % 6;
		case 9: return 1 + (uiCounter / 20) % 3;

		default: return 0;
	}
}

bool SpriteHasBase(uint uiSpriteCode)
{
	switch (uiSpriteCode - MAX_WALL_CODE - 1)
	{
		case 1:
		case 3:
			return true;

		default: return false;
	}
}

bool IsSpriteAnObstacle(uint uiSpriteCode)
{
	switch (uiSpriteCode - MAX_WALL_CODE - 1)
	{
		case 6:
			return false;

		default: return true;
	}
}

// Walls setup

bool IsSingleBlockWall(uint uiWallCode)
{
	switch (uiWallCode)
	{
		case 4:
		case 5:
		case 6:
		case 7:
			return true;

		default: return false;
	}
}

// Maps description

#define MAPS_COUNT 6
#define MAP_SIZE 20

#define ENV_DEFAULT 0
#define ENV_BASEMENT 1
#define ENV_COUNT (ENV_BASEMENT + 1)

const uint8 ui8MapsData[MAPS_COUNT][MAP_SIZE + 1][MAP_SIZE] = {
	1 , 8 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,
	1 , 99, 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 8 , 100, 0, 23, 0 , 0 , 0 , 23, 0 , 1 ,
	1 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 1 ,
	1 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 1 ,
	3 , 91, 15, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 1 ,
	3 , 0 , 13, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 1 ,
	3 , 0 , 3 , 3 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 1 ,
	3 , 0 , 3 , 0 , 0 , 6 , 6 , 6 , 6 , 6 , 6 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 1 ,
	3 , 0 , 3 , 0 , 0 , 6 , 0 , 16, 0 , 16, 6 , 0 , 2 , 2 , 2 , 2 , 2 , 9 , 0 , 1 ,
	3 , 0 , 3 , 3 , 3 , 6 , 16, 0 , 16, 0 , 6 , 0 , 2 , 0 , 23, 0 , 2 , 0 , 0 , 1 ,
	3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 3 , 0 , 2 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	3 , 0 , 0 , 0 , 0 , 0 , 92, 0 , 0 , 91, 3 , 0 , 2 , 22, 9 , 0 , 2 , 0, 0 , 1 ,
	3 , 0 , 12, 0 , 0 , 0 , 12, 0 , 0 , 0 , 3 , 0 , 2 , 0 , 0 , 0 , 2 , 23, 0 , 1 ,
	3 , 0 , 0 , 0 , 90, 0 , 0 , 0 , 0 , 0 , 3 , 0 , 2 , 0 , 23, 0 , 2 , 0 , 0 , 1 ,
	3 , 0 , 0 , 90, 14, 90, 0 , 0 , 0 , 0 , 3 , 0 , 2 , 2 , 2 , 2 , 2 , 0 , 0 , 1 ,
	3 , 0 , 0 , 0 , 90, 0 , 0 , 0 , 0 , 0 , 3 , 0 , 2 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	3 , 0 , 12, 0 , 0 , 0 , 12, 0 , 0 , 0 , 3 , 7 , 7 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 92, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	3 , 13, 0 , 0 , 0 , 91, 0 , 0 , 0 , 15, 3 , 7 , 7 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 0 , 2 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,
	/* map additional data block below */
	45 /* player angle div 2 */, ENV_DEFAULT /* environment type */, 0 /* entering speech id */,
	0 /* fog */, 0 /* fog distance */,
	/* unused bytes below */
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,

	1 , 8 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 9 , 9 , 9 , 9 , 9 , 9 , 7 , 7 , 1 , 1 , 1 ,
	1 ,100, 1 , 0 , 0 , 0 , 0 , 0 , 0 , 9 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 99, 8 ,
	5 , 23, 5 , 0 , 0 , 0 , 0 , 0 , 0 , 9 , 0 , 0 , 0 , 0 , 0 , 7 , 7 , 1 , 1 , 1 ,
	5 , 0 , 5 , 0 , 0 , 0 , 0 , 0 , 0 , 9 , 0 , 0 , 0 , 0 , 0 , 9 , 0 , 0 , 0 , 0 ,
	5 , 0 , 5 , 0 , 0 , 0 , 0 , 0 , 0 , 9 , 23, 7 , 23, 0 , 0 , 9 , 9 , 9 , 9 , 9 ,
	5 , 0 , 5 , 0 , 0 , 0 , 0 , 0 , 0 , 9 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 23, 9 ,
	5 , 0 , 5 , 0 , 0 , 0 , 0 , 0 , 0 , 9 , 23, 0 , 0 , 0 , 7 , 22, 0 , 7 , 0 , 9 ,
	7 , 0 , 7 , 0 , 0 , 0 , 0 , 0 , 0 , 9 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 9 ,
	10, 0 , 10, 10, 10, 7 , 5 , 5 , 7 , 9 , 0 , 7 , 0 , 0 , 0 , 0 , 23, 0 , 0 , 9 ,
	10, 0 , 0 , 0 , 0 , 10, 16, 18, 10, 9 , 0 , 0 , 0 , 23, 0 , 0 , 0 , 0 , 0 , 9 ,
	10, 0 , 0 , 0 , 0 , 10, 0 , 0 , 10, 9 , 9 , 9 , 9 , 9 , 9 , 9 , 9 , 7 , 0 , 7 ,
	10, 0 , 0 , 23, 0 , 10, 0 , 0 , 10, 10, 10, 10, 0 , 0 , 0 , 0 , 0 , 10, 0 , 10,
	10, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 9 , 0 , 10, 0 , 0 , 0 , 0 , 0 , 10, 21, 10,
	10, 0 , 0 , 0 , 0 , 0 , 23, 23, 9 , 9 , 0 , 10, 0 , 0 , 0 , 0 , 0 , 10, 0 , 10,
	7 , 0 , 0 , 7 , 7 , 9 , 0 , 0 , 9 , 0 , 0 , 7 , 10, 10, 10, 10, 10, 10, 0 , 10,
	9 , 23, 0 , 9 , 18, 9 , 0 , 0 , 9 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 10,
	9 , 0 , 0 , 9 , 22, 9 , 0 , 0 , 9 , 0 , 9,  7 , 0 , 0 , 23, 0 , 0 , 0 , 0 , 10,
	9 , 16, 19, 9 , 0 , 0 , 0 , 0 , 9 , 23, 9 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 10,
	9 , 5 , 5 , 9 , 23, 0 , 0 , 0 , 0 , 0 , 9 , 0 , 0 , 0 , 0 , 0 , 23, 0 , 0 , 10,
	0 , 0 , 0 , 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 ,
	/* map additional data block below */
	90 /* player angle div 2 */, ENV_DEFAULT /* environment type */, 1 /* entering speech id */,
	1 /* fog */, 100 /* fog distance */,
	/* unused bytes below */
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,

	1 , 1 , 1 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 0 , 0 , 5 , 5 , 5 , 5 , 5 , 7 , 1 , 1 ,
	8 , 99, 0 , 23, 93, 20, 20, 22, 20, 7 , 4 , 4 , 5 , 22, 0 , 0 , 0 , 0 ,100, 8 ,
	1 , 1 , 1 , 5 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 4 , 5 , 0 , 0 , 0 , 0 , 7 , 1 , 1 ,
	0 , 0 , 0 , 5 , 0 , 0 , 0 , 93, 0 , 0 , 0 , 4 , 5 , 0 , 0 , 23, 0 , 0 , 0 , 5 ,
	0 , 0 , 0 , 5 , 20 ,0 , 20, 21, 20, 7 , 0 , 4 , 5 , 0 , 0 , 0 , 0 , 0 , 0 , 5 ,
	4 , 4 , 9 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 0 , 5 , 5 , 0 , 0 , 0 , 0 , 23, 0 , 5 ,
	4 , 0 , 0 , 0 , 0 , 19, 5 , 0 , 0 , 5 , 17, 5 , 5 , 23, 0 , 11, 24, 0 , 0 , 5 ,
	4 , 0 , 9 , 5 , 0 , 23, 5 , 0 , 0 , 9 , 23, 9 , 5 , 0 , 0 , 0 , 0 , 0 , 0 , 5 ,
	4 , 17, 4 , 5 , 0 , 0 , 9 , 4 , 4 , 4 , 0 , 4 , 5 , 0 , 0 , 0 , 0 , 0 , 0 , 5 ,
	4 , 0 , 4 , 5 , 0 , 0 , 0 , 0 , 17, 23, 0 , 4 , 5 , 11, 0 , 0 , 17, 0 , 0 , 5 ,
	9 , 23, 9 , 5 , 23, 0 , 9 , 4 , 4 , 4 , 4 , 4 , 5 , 0 , 23, 0 , 0 , 0 , 0 , 5 ,
	6 , 23, 6 , 5 , 0 , 23, 5 , 5 , 5 , 5 , 5 , 5 , 5 , 0 , 0 , 0 , 0 , 0 , 0 , 5 ,
	6 , 0 , 6 , 5 , 0 , 93, 5 , 22, 0 , 0 , 0 , 0 , 11, 0 , 0 , 0 , 0 , 11, 0 , 5 ,
	6 , 0 , 6 , 5 , 20, 20, 5 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 5 ,
	6 , 0 , 6 , 5 , 5 , 5 , 5 , 0 , 0 , 0 , 17, 0 , 23, 0 , 0 , 11, 0 , 0 , 0 , 5 ,
	6 , 0 , 6 , 5 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 5 ,
	6 , 0 , 6 , 7 , 0 , 0 , 0 , 0 , 21, 0 , 0 , 0 , 0 , 11, 0 , 0 , 0 , 0 , 0 , 5 ,
	6 , 23, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 23, 0 , 17, 0 , 5 ,
	6 , 16, 22, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 23, 0 , 0 , 0 , 0 , 0 , 22, 5 ,
	6 , 6 , 6 , 7 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 ,
	/* map additional data block below */
	0 /* player angle div 2 */, ENV_BASEMENT /* environment type */, 2 /* entering speech id */,
	1 /* fog */, 70 /* fog distance */,
	/* unused bytes below */
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,

	0 , 0 , 0 , 0 , 4 , 4 , 9 , 4 , 4 , 4 , 4 , 9 , 4 , 5 , 5 , 5 , 5 , 0 , 0 , 0 ,
	0 , 0 , 0 , 1 , 7 , 0 , 0 , 0 , 6 , 23, 0 , 6 , 20, 23, 6 , 22, 5 , 0 , 0 , 0 ,
	0 , 0 , 0 , 8 , 99, 21, 7 , 0 , 17, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 5 , 0 , 0 , 0 ,
	0 , 0 , 0 , 1 , 7 , 0 , 0 , 0 , 18, 6 , 23, 0 , 6 , 23, 0 , 0 , 5 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 4 , 4 , 9 , 4 , 4 , 4 , 4 , 9 , 4 , 5 , 11, 0 , 5 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 5 , 0 , 0 , 5 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 5 , 0 , 11, 5 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 5 , 0 , 23, 5 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 9 , 0 , 0 , 9 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 10, 11, 17, 10, 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 10, 23, 0 , 10, 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 10, 0 , 0 , 10, 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 10, 2 , 0 , 10, 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 10, 20, 0 , 10, 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 10, 0 , 24, 10, 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 10, 0 , 2 , 10, 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 9 , 10, 0 , 0 , 10, 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 13, 0 , 0 , 0 , 0 , 23, 23, 10, 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 8 ,100, 24, 0 , 0 , 0 , 0 , 0 , 19, 10, 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 1 , 9 , 10, 10, 10, 10, 0 , 0 , 0 ,
	/* map additional data block below */
	0 /* player angle div 2 */, ENV_BASEMENT /* environment type */, 2 /* entering speech id */,
	1 /* fog */, 50 /* fog distance */,
	/* unused bytes below */
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,

	1 , 1 , 1 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 ,
	8 , 99, 0 , 0 , 21, 92, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 23, 0 , 0 , 0 , 0 , 13, 3 ,
	1 , 1 , 1 , 3 , 3 , 0 , 0 , 0 , 0 , 0 , 23, 0 , 0 , 0 , 0 , 14, 0 , 0 , 0 , 3 ,
	0 , 0 , 0 , 0 , 3 , 0 , 0 , 0 , 90, 0 , 0 , 22, 0 , 23, 0 , 0 , 0 , 0 , 0 , 3 ,
	0 , 0 , 0 , 0 , 3 , 0 , 0 , 0 , 0 , 0 , 0 , 3 , 0 , 0 , 21, 0 , 22, 0 , 0 , 3 ,
	0 , 0 , 0 , 0 , 3 , 0 , 22, 3 , 23 , 12, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 12, 0 , 3 ,
	0 , 0 , 0 , 0 , 3 , 0 , 0 , 0 , 0 , 0 , 0 , 16, 0 , 0 , 0 , 0 , 0 , 0, 0 , 3 ,
	0 , 0 , 0 , 0 , 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 16, 0 , 23, 0 , 0 , 0 , 3 ,
	0 , 0 , 0 , 0 , 3 , 0 , 90, 14, 0 , 0 , 0 , 90, 16, 0 , 0 , 21, 0 , 3 , 0 , 3 ,
	0 , 0 , 0 , 0 , 3 , 0 , 0 , 0 , 0 , 0 , 15, 22, 16, 23, 0 , 15, 0 , 24, 0 , 3 ,
	0 , 0 , 0 , 0 , 3 , 0 , 0 , 0 , 21, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 3 ,
	0 , 0 , 0 , 0 , 3 , 0 , 0 , 3 , 0 , 0 , 0 , 16, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 3 ,
	0 , 0 , 0 , 0 , 3 , 22, 15, 0 , 23, 0 , 0 , 0 , 0 , 0 , 16, 23, 0 , 0 , 0 , 3 ,
	0 , 0 , 0 , 0 , 3 , 0 , 0 , 0 , 0 , 12, 0 , 23, 0 , 22, 0 , 0 , 23, 12, 23, 3 ,
	0 , 0 , 0 , 0 , 3 , 0 , 23, 0 , 0 , 0 , 0 , 91, 0 , 16, 0 , 0 , 0 , 0 , 0 , 3 ,
	0 , 0 , 0 , 0 , 3 , 13, 0 , 0 , 22, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 24, 3 ,
	0 , 0 , 0 , 0 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 7 , 0 , 7 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 1 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,100, 1 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 8 , 1 ,
	/* map additional data block below */
	0 /* player angle div 2 */, ENV_DEFAULT /* environment type */, 1 /* entering speech id */,
	0 /* fog */, 0 /* fog distance */,
	/* unused bytes below */
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,

	1 , 1 , 1 , 9 , 3 , 3 , 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	8 , 99, 0 , 0 , 0 , 13, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	1 , 1 , 1 , 9 , 0 , 3 , 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 3 , 0 , 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 3 , 0 , 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 3 , 0 , 3 , 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 16, 0 , 16, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 16, 0 , 16, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 16, 0 , 16, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 16, 0 , 16, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 16, 0 , 16, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 16, 0 , 16, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 16, 0 , 16, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 16, 24, 16, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 16, 92, 16, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 16, 0 , 16, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 16, 0 , 16, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 16,100, 16, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 13, 14, 13, 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 3 , 3 , 3 , 3 , 3 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	/* map additional data block below */
	0 /* player angle div 2 */, ENV_DEFAULT /* environment type */, 0 /* entering speech id */,
	0 /* fog */, 0 /* fog distance */,
	/* unused bytes below */
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
};
