#pragma once

cRGB HSItoRGBW(float hue, float saturation, float intensity) {
  cRGB result = {0,0,0,0};        // the RGBW result you'll return
  hue = constrain(hue, 0, 360);              // constrain hue to 0-360
  hue = hue * PI / 180;                      // Convert to radians.
  saturation = constrain(saturation / 100, 0, 1); // constrain to 0-1
  intensity = constrain(intensity / 100, 0, 1);   // constrain to 0-1

  // if hue is in the red/green sector:
  if (hue < 2 * PI / 3) {
	result.r = saturation * 255 * intensity / 3 * (1L + cos(hue) / cos((PI / 3) - hue));
	result.g = saturation * 255 * intensity / 3 * (1L + (1 - cos(hue) / cos((PI / 3) - hue)));
	result.b = 0;
	// if hue is in the green/blue sector:
  } else if (hue < 4 * PI / 3) {
	hue = hue - (2 * PI / 3);
	result.g = saturation * 255 * intensity / 3 * (1L + cos(hue) / cos((PI / 3) - hue));
	result.b = saturation * 255 * intensity / 3 * (1L + (1 - cos(hue) / cos((PI / 3) - hue)));
	result.r = 0;
	// if hue is in the red/blue sector:
  } else {
	hue = hue - (4 * PI / 3);
	result.b = saturation * 255 * intensity / 3 * (1L + cos(hue) / cos((PI / 3) - hue));
	result.r = saturation * 255 * intensity / 3 * (1L + (1 - cos(hue) / cos((PI / 3) - hue)));
	result.g = 0;
  }
  // white is a function of saturation and intensity regardless of hue:
  result.w = 255 * (1 - saturation) * intensity;

  // return result:
  return result;
}