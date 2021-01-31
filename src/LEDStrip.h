#include <Arduino.h>
#include <FastLED.h>
#include <string.h>

#pragma once

template <unsigned pin_num>
class LEDStrip
{
    private:
        int num_leds;
        int hue, sat, lum, dir;
        CRGB leds[30];
    public:
        LEDStrip():
            num_leds(0),
            hue(0), sat(0), lum(0), dir(1),
            leds({})
        {}
        explicit LEDStrip(int amount):
            num_leds(amount),
            hue(0), sat(0), lum(0), dir(1),
            leds({})
        {
            FastLED.addLeds<WS2812B, pin_num>(leds, num_leds);
        }
        ~LEDStrip() { delete[] leds; }
        void update_strip() 
        { 
            for(int i = 0; i < num_leds; i++) 
                leds[i] = CHSV(hue, sat, lum); 
        }
        void set_hue(int degree)
        {
            hue = degree;
            update_strip();
        }
        void set_saturation(int level)
        {
            sat = level;
            update_strip();
        }
        void set_brightness(int brightness)
        {
            lum = brightness;
            update_strip();
        }
        void tint_existing(int degree, int level, int brightness)
        {
            set_hue(qadd8(hue, degree));
            set_saturation(qadd8(sat, level));
            set_brightness(qadd8(lum, brightness));

            update_strip();
        }
        void solid_fill(int degree, int level, int brightness)
        {
            hue = degree;
            sat = level;
            lum = brightness;

            update_strip();
        }
        void rainbow(int hue_min, int hue_max, int step)
        {
            if(hue > hue_max)
            {
                hue = hue_max;
                dir = -1;
            }
            else if(hue < hue_min)
            {
                hue = hue_min;
                dir = 1;
            }

            hue += (step * dir);
            update_strip();
        }
        void cycle_rainbow(int step)
        {
            int offset = step;

            for(int i = 0; i < num_leds; i++)
            {
                hue += (offset * i);

                if(hue > 255) hue = (0 + (offset * i));

                leds[i] = CRGB(hue, sat, lum);
            }
        }
        void addGlitter(int degree, fract8 chance)
        {
            if(random8() < chance) leds[random16(num_leds)] += CRGB::White;
        }
        void addSparkle(int degree, fract8 chance)
        {
            if(random8() < chance)
            {
                leds[random16(num_leds)] += CHSV(degree, 0, 0);
                leds[random16(num_leds)] += CRGB::White;
            }
        }
};