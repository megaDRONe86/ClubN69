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

#include <DGLE.h>
#include <math.h>
#include <vector>

using namespace DGLE;

#define APP_CAPTION "ClubN69"
#define APP_VERSION "v1.1"

#define DLL_PATH "DGLE.dll"

#define WINDOW_WIDTH 1024u
#define WINDOW_HEIGHT 768u

#define GAME_VIEWPORT_WIDTH 640u
#define GAME_VIEWPORT_HEIGHT 480u

#define GAME_FOV 90.f

#define GAME_UPDATE_INTERVAL 30u

#define MOUSE_SENSITIVITY 5.f

#ifdef _DEBUG
#	define RESOURCE_PATH "..\\bin\\data\\"
#else
//#	define COMPILED_RESOURCES
#	ifdef COMPILED_RESOURCES
#		define RESOURCE_PATH "data.dcp|"
#	else
#		define RESOURCE_PATH "data\\"
#	endif
#endif
