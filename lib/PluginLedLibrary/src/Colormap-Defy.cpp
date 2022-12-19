/* -*- mode: c++ -*-
 * Kaleidoscope-Colormap -- Per-layer colormap effect
 * Copyright (C) 2016, 2017, 2018  Keyboard.io, Inc
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


#include "Colormap-Defy.h"
#include <Kaleidoscope-EEPROM-Settings.h>
#include <Kaleidoscope-FocusSerial.h>
#include "kaleidoscope/layers.h"
#include "LED-Palette-Theme-Defy.h"

namespace kaleidoscope {
namespace plugin {

uint16_t ColormapEffectDefy::map_base_;
uint8_t ColormapEffectDefy::max_layers_;
uint8_t ColormapEffectDefy::top_layer_;

void ColormapEffectDefy::max_layers(uint8_t max_) {
  if (map_base_ != 0)
    return;

  max_layers_ = max_;
  map_base_ = ::LEDPaletteThemeDefy.reserveThemes(max_layers_);
}

void ColormapEffectDefy::TransientLEDMode::onActivate(void) {
  if (!Runtime.has_leds)
    return;
  parent_->led_mode.layer = parent_->top_layer_;
  Runtime.device().setLedMode(&(parent_->led_mode));

  parent_->top_layer_ = Layer.mostRecent();
  if (parent_->top_layer_ <= parent_->max_layers_)
    ::LEDPaletteThemeDefy.updateHandler(parent_->map_base_, parent_->top_layer_);
}


void ColormapEffectDefy::updateColorIndexAtPosition(uint8_t layer, uint16_t position, uint8_t palette_index) {
  if (layer >= max_layers_) return;

  uint16_t index = (Runtime.device().led_count) * layer + position;
  ::LEDPaletteThemeDefy.updateColorIndexAtPosition(map_base_, index, palette_index);
}

void ColormapEffectDefy::TransientLEDMode::refreshAt(KeyAddr key_addr) {
  if (parent_->top_layer_ <= parent_->max_layers_)
    ::LEDPaletteThemeDefy.refreshAt(parent_->map_base_, parent_->top_layer_, key_addr);
}

EventHandlerResult ColormapEffectDefy::onLayerChange() {
  if (::LEDControl.get_mode_index() == led_mode_id_)
    ::LEDControl.get_mode<TransientLEDMode>()->onActivate();
  return EventHandlerResult::OK;
}

EventHandlerResult ColormapEffectDefy::onFocusEvent(const char *command) {
  return ::LEDPaletteThemeDefy.themeFocusEvent(command, PSTR("colormap.map"),
                                           map_base_, max_layers_);
}
void ColormapEffectDefy::getColorPalette(uint8_t output_buf[16]) {
	::LEDPaletteThemeDefy.getColorPalette(output_buf);
  for (int i = 0; i < 16; ++i) {
	Serial.printf("%i ",output_buf[i]);
  }
  Serial.println("");
}

void ColormapEffectDefy::getLayer(uint8_t *output_buf) {

}

}
}

kaleidoscope::plugin::ColormapEffectDefy ColormapEffectDefy;
