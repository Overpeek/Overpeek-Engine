#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "audio/audioManager.h"

#include "logic/gameloop.h"
#include "logic/noise.h"
#include "logic/gameloop.h"
#include "logic/math.h"
#include "logic/FastNoise.h"
#include "logic/FastNoiseSIMD/FastNoiseSIMD.h"

#include "tools/clock.h"
#include "tools/random.h"
#include "tools/binaryIO.h"
#include "tools/logger.h"
#include "tools/debug.h"

#include "graphics/camera.h"
#include "graphics/shader.h"
#include "graphics/renderer.h"
#include "graphics/window.h"
#include "graphics/textureManager.h"
#include "graphics/buffers/buffer.h"
#include "graphics/buffers/vertexArray.h"
#include "graphics/buffers/indexBuffer.h"