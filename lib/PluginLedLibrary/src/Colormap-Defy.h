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
#include "LedModeCommunication.h"
#include "SpiComms.h"

namespace kaleidoscope {
namespace plugin {
class ColormapEffectDefy : public Plugin,
                           public LEDModeInterface,
                           public LedModeCommunication,
                           public AccessTransientLEDMode {
 public:
  ColormapEffectDefy(void) {
    spi_1.active_callback_.addListener([this](bool active) {
      if (active) {
        syncData(spi_1);
      }
    });
    spi_0.active_callback_.addListener([this](bool active) {
      if (active) {
        syncData(spi_0);
      }
    });
  }

  void syncData(SpiComms &spi) {
    Packet packet;
    packet.context.command = Side_communications_protocol::SET_PALETTE_COLORS;
    packet.context.size    = sizeof(cRGB) * 16;
    cRGB palette[16];
    getColorPalette(palette);
    memcpy(packet.data, palette, packet.context.size);
    spi.sendPacket(packet);
    uint8_t layerColors[Runtime.device().led_count];
    uint8_t baseKeymapIndex    = spi.sideCommunications == Side_communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT ? Runtime.device().ledDriver().key_matrix_leds : 0;
    uint8_t baseUnderGlowIndex = spi.sideCommunications == Side_communications_protocol::Devices::KEYSCANNER_DEFY_RIGHT ? (Runtime.device().ledDriver().key_matrix_leds) * 2 + Runtime.device().ledDriver().underglow_leds : Runtime.device().ledDriver().key_matrix_leds * 2;
    for (int i = 0; i < getMaxLayers(); ++i) {
      getLayer(i, layerColors);
      packet.context.command = SET_LAYER_KEYMAP_COLORS;
      packet.context.size    = Runtime.device().ledDriver().key_matrix_leds + 1;
      packet.data[0]         = i;
      memcpy(&packet.data[1], &layerColors[baseKeymapIndex], packet.context.size - 1);
      spi.sendPacket(packet);
      packet.context.command = SET_LAYER_UNDERGLOW_COLORS;
      packet.context.size    = Runtime.device().ledDriver().underglow_leds + 1;
      memcpy(&packet.data[1], &layerColors[baseUnderGlowIndex], packet.context.size - 1);
      spi.sendPacket(packet);
      ::LEDControl.set_mode(::LEDControl.get_mode_index());
    }
  }


  void max_layers(uint8_t max_);

  EventHandlerResult onLayerChange();
  EventHandlerResult onFocusEvent(const char *command);
  void updateColorIndexAtPosition(uint8_t layer, uint16_t position, uint8_t palette_index);
  uint8_t getColorIndexAtPosition(uint8_t layer, uint16_t position);
  LedModeSerializable_Layer &led_mode = ledModeSerializableLayer;
  void getColorPalette(cRGB output_palette[16]);
  void getLayer(uint8_t layer, uint8_t output_buf[Runtime.device().led_count]);

  // This class' instance has dynamic lifetime
  //
  class TransientLEDMode : public LEDMode {
   public:
    // Please note that storing the parent ptr is only required
    // for those LED modes that require access to
    // members of their parent class. Most LED modes can do without.
    //
    explicit TransientLEDMode(ColormapEffectDefy *parent)
      : parent_(parent) {}

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
}  // namespace plugin
}  // namespace kaleidoscope

extern kaleidoscope::plugin::ColormapEffectDefy ColormapEffectDefy;
