/* -*- mode: c++ -*-
 * Kaleidoscope-Colormap -- Per-layer colormap effect
 * Copyright (C) 2022  Keyboard.io, Inc
 *
 * This program is free software: you can redistribute it and/or modify it under it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Colormap-Defy.h"  // for Colormap
#include "DefaultColormap.h"

#include <Arduino.h>                         // for F, PSTR, __FlashStringHelper
#include "Kaleidoscope-LEDControl.h"         // for LEDControl
#include "Kaleidoscope-LED-Palette-Theme.h"  // for LEDPaletteTheme
#include <stdint.h>                          // for uint8_t, uint16_t

#include "kaleidoscope/KeyAddr.h"  // for KeyAddr
#include "kaleidoscope/Runtime.h"  // for Runtime, Runtime_

namespace kaleidoscope {
namespace plugin {

const cRGB defaultcolormap::palette[16]{
	{255, 0, 0, 0},
	{0, 255, 0, 0},
	{0, 0, 255, 0},
	{0, 0, 0, 255},
	{0xff, 0xff, 0x55, 0},
	{0xff, 0x55, 0xff, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
};

enum {
  RED,
  GREEN,
  BLUE,
  WHITE,
  YELLOW,
  MAGENT
};
bool defaultcolormap::palette_defined = true;
const uint8_t defaultcolormap::colormap_layers = 4;
const uint8_t defaultcolormap::colormaps[4][kaleidoscope_internal::device.led_count+1] = {
	{
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN,
		GREEN, RED, GREEN, RED, GREEN, RED,

		GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED,
		//Keyboard2
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN,
		GREEN, RED, GREEN, RED, GREEN, RED,

		GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED,
		//Underblow left
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN,
		//Underblow right
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN,
		//Neuron
		WHITE
	},
	{
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE,

		BLUE, WHITE, BLUE, WHITE,
		BLUE, WHITE, BLUE, WHITE,
		//Keyboard2
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE,

		BLUE, WHITE, BLUE, WHITE,
		BLUE, WHITE, BLUE, WHITE,
		//Underblow left
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE,
		BLUE, WHITE, BLUE,
		//Underblow right
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE,
		BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE, BLUE, WHITE,
		BLUE, WHITE, BLUE,
		//Neuron
		WHITE
	},
	{
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT,

		YELLOW, MAGENT, YELLOW, MAGENT,
		YELLOW, MAGENT, YELLOW, MAGENT,
		//Keyboard2
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT,

		YELLOW, MAGENT, YELLOW, MAGENT,
		YELLOW, MAGENT, YELLOW, MAGENT,
		//Underblow left
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT,
		YELLOW, MAGENT, YELLOW,
		//Underblow right
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT,
		YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT, YELLOW, MAGENT,
		YELLOW, MAGENT, YELLOW,
		//Neuron
		MAGENT
	},
	{
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN,
		GREEN, RED, GREEN, RED, GREEN, RED,

		GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED,
		//Keyboard2
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN,
		GREEN, RED, GREEN, RED, GREEN, RED,

		GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED,
		//Underblow left
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN,
		//Underblow right
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED, GREEN, RED,
		GREEN, RED, GREEN,
		//Neuron
		RED
	}
};

void DefaultColormap::setup() {
  install();
}

void DefaultColormap::install() {
  if (!defaultcolormap::palette_defined)
	return;

  for (uint8_t i = 0; i < 16; i++) {
	cRGB color;

	color.r = defaultcolormap::palette[i].r;
	color.g = defaultcolormap::palette[i].g;
	color.b = defaultcolormap::palette[i].b;
	color.w = defaultcolormap::palette[i].w;

	::LEDPaletteTheme.updatePaletteColor(i, color);
  }

  if (defaultcolormap::colormap_layers==0)
	return;

  for (uint8_t layer = 0; layer < defaultcolormap::colormap_layers; layer++) {
	for (int16_t i = 0; i < Runtime.device().led_count+1; i++) {
	  const uint8_t idx = pgm_read_byte(&(defaultcolormap::colormaps[layer][i]));
	  ::ColormapEffectDefy.updateColorIndexAtPosition(layer, i, idx);
	}
  }
  Runtime.storage().commit();

  ::LEDControl.refreshAll();
}

}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::DefaultColormap DefaultColormap;