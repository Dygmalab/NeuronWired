#include "CRC.h"
#include <hardware/dma.h>

uint32_t crc32(const void *ptr, uint32_t len) {
  uint32_t dummy_dest, crc;

  int channel          = dma_claim_unused_channel(true);
  dma_channel_config c = dma_channel_get_default_config(channel);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
  channel_config_set_read_increment(&c, true);
  channel_config_set_write_increment(&c, false);
  channel_config_set_sniff_enable(&c, true);

  // Seed the CRC calculation
  dma_hw->sniff_data = 0xffffffff;

  // Mode 1, then bit-reverse the result gives the same result as
  dma_sniffer_enable(channel, 0x1, true);
  dma_hw->sniff_ctrl |= DMA_SNIFF_CTRL_OUT_REV_BITS;

  dma_channel_configure(channel, &c, &dummy_dest, ptr, len / 4, true);

  dma_channel_wait_for_finish_blocking(channel);

  // Read the result before resetting
  crc = dma_hw->sniff_data ^ 0xffffffff;

  dma_sniffer_disable();
  dma_channel_unclaim(channel);

  return crc;
}