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

#pragma once

#include <Kaleidoscope-LEDControl.h>
#include <Kaleidoscope-LED-Palette-Theme.h>
#include "LedModeSerializable-Layer.h"

namespace kaleidoscope {
namespace plugin {
class ColormapEffectDefy : public Plugin,
  public LEDModeInterface,
  public AccessTransientLEDMode {
 public:
  ColormapEffectDefy(void) {}

  void max_layers(uint8_t max_);

  EventHandlerResult onLayerChange();
  EventHandlerResult onFocusEvent(const char *command);
  void updateColorIndexAtPosition(uint8_t layer, uint16_t position, uint8_t palette_index);
  uint8_t getColorIndexAtPosition(uint8_t layer, uint16_t position);
  LedModeSerializable_Layer &led_mode = ledModeSerializableLayer;
  void getColorPalette(cRGB output_palette[16]);
  void getLayer(uint8_t layer,uint8_t output_buf[Runtime.device().led_count]);

  // This class' instance has dynamic lifetime
  //
  class TransientLEDMode : public LEDMode {
   public:

    // Please note that storing the parent ptr is only required
    // for those LED modes that require access to
    // members of their parent class. Most LED modes can do without.
    //
    explicit TransientLEDMode(ColormapEffectDefy *parent) : parent_(parent) {}

   protected:

    friend class ColormapEffectDefy;

    void onActivate(void) final;
    void refreshAt(KeyAddr key_addr) final;
   private:

    ColormapEffectDefy *parent_;
  };

 private:
  static uint8_t top_layer_;
public:
  static uint8_t getMaxLayers();
private:
  static uint8_t max_layers_;
  static uint16_t map_base_;
};
}
}

extern kaleidoscope::plugin::ColormapEffectDefy ColormapEffectDefy;