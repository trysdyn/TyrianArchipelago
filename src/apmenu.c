
#include "font.h"
#include "fonthand.h"
#include "joystick.h"
#include "jukebox.h"
#include "keyboard.h"
#include "loudness.h"
#include "mouse.h"
#include "nortsong.h"
#include "palette.h"
#include "picload.h"
#include "sprite.h"
#include "tyrian2.h"
#include "varz.h"
#include "video.h"

#include "SDL_clipboard.h"

#include "archipelago/apconnect.h"

bool ap_connectScreen(void)
{
	JE_loadPic(VGAScreen2, 2, false);
	draw_font_hv_shadow(VGAScreen2, 10,  10, "01", small_font, centered,  1, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10,  20, "02", small_font, centered,  2, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10,  30, "03", small_font, centered,  3, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10,  40, "04", small_font, centered,  4, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10,  50, "05", small_font, centered,  5, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10,  60, "06", small_font, centered,  6, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10,  70, "07", small_font, centered,  7, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10,  80, "08", small_font, centered,  8, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10,  90, "09", small_font, centered,  9, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10, 100, "10", small_font, centered, 10, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10, 110, "11", small_font, centered, 11, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10, 120, "12", small_font, centered, 12, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10, 130, "13", small_font, centered, 13, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10, 140, "14", small_font, centered, 14, 6, false, 1);
	draw_font_hv_shadow(VGAScreen2, 10, 150, "15", small_font, centered, 15, 6, false, 1);
	memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);
	fade_palette(colors, 10, 0, 255);

	int error_time = 0;
	int visual_time = 0;
	int visual_pulse = 0;

// void draw_font_hv_shadow(SDL_Surface *surface, int x, int y, const char *text, Font font, FontAlignment alignment, Uint8 hue, Sint8 value, bool black, int shadow_dist)

	while (true)
	{
		service_SDL_events(true);
		push_joysticks_as_keyboard();

		++visual_time;
		visual_time &= 0x1F;

		if (visual_time & 0x10)
			visual_pulse = -2 + ((visual_time ^ 0x1F) >> 2);
		else
			visual_pulse = -2 + ( visual_time         >> 2);

		switch(Archipelago_ConnectionStatus())
		{
			case APCONN_NOT_CONNECTED:
				if (error_time)
				{
					--error_time;
					visual_pulse = 6;
					if (--error_time < 60)
						visual_pulse = -8 + (error_time >> 2);

					draw_font_hv_shadow(VGAScreen, 160, 100, "Failed to connect to the Archipelago server:", small_font, centered, 4, visual_pulse, false, 1);
					draw_font_hv_shadow(VGAScreen, 160, 110, Archipelago_GetConnectionError(), small_font, centered, 4, visual_pulse, false, 1);
				}

				if (newkey && lastkey_scan == SDL_SCANCODE_RETURN)
				{
					error_time = 300;
					visual_time = 0;
					Archipelago_Connect();
				}
				else if (newkey && lastkey_scan == SDL_SCANCODE_ESCAPE)
				{
					JE_playSampleNum(S_SPRING);
					fade_black(15);
					return false;
				}
				else if (newkey && lastkey_scan == SDL_SCANCODE_F)
				{
					// Force
					JE_playSampleNum(S_SELECT);
					fade_black(15);
					return newGame();
				}
				break;

			case APCONN_CONNECTING:
				draw_font_hv_shadow(VGAScreen, 160, 100, "Connecting...", small_font, centered, 12, visual_pulse, false, 1);
				break;

			case APCONN_TENTATIVE:
				draw_font_hv_shadow(VGAScreen, 160, 100, "Authenticating...", small_font, centered, 12, visual_pulse, false, 1);
				break;

			case APCONN_READY:
				// Ready to start the game
				JE_playSampleNum(S_SELECT);
				fade_black(15);
				return true;
		}

		JE_showVGA();
		SDL_Delay(16);
		memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);
	}
}

bool ap_titleScreen(void)
{
	const char *APMenuTexts[] =
	{
		"Start Game",
		"Options",
		"Jukebox",
		"Quit"
	};
	enum MenuItemIndex
	{
		MENU_ITEM_START_GAME = 0,
		MENU_ITEM_OPTIONS,
		MENU_ITEM_JUKEBOX,
		MENU_ITEM_QUIT,
		COUNT_MENU_ITEMS
	};

	if (shopSpriteSheet.data == NULL)
		JE_loadCompShapes(&shopSpriteSheet, '1');  // need mouse pointer sprites

	bool restart = true;

	size_t selectedIndex = MENU_ITEM_START_GAME;

	const int xCenter = VGAScreen->w / 2;
	const int yMenuItems = 140;
	const int hMenuItem = 13;
	int wMenuItem[COUNT_MENU_ITEMS] = { 0 };

	for (; ; )
	{
		if (restart)
		{
			play_song(SONG_TITLE);

			JE_loadPic(VGAScreen, 4, false);

			draw_font_hv_shadow(VGAScreen, 2, 192, opentyrian_version, small_font, left_aligned, 15, 0, false, 1);

			if (moveTyrianLogoUp)
			{
				memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

				blit_sprite(VGAScreenSeg, 11, 62, PLANET_SHAPES, 146); // tyrian logo

				fade_palette(colors, 10, 0, 255 - 16);

				for (int y = 60; y >= 4; y -= 2)
				{
					setDelay(2);

					memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);

					blit_sprite(VGAScreenSeg, 11, y, PLANET_SHAPES, 146); // tyrian logo

					JE_showVGA();

					service_wait_delay();
				}

				moveTyrianLogoUp = false;
			}
			else
			{
				blit_sprite(VGAScreenSeg, 11, 4, PLANET_SHAPES, 146); // tyrian logo

				fade_palette(colors, 10, 0, 255 - 16);
			}

			// Draw menu items.
			for (size_t i = 0; i < COUNT_MENU_ITEMS; ++i)
			{
				const char *const text = APMenuTexts[i];

				wMenuItem[i] = JE_textWidth(text, normal_font);
				const int x = xCenter - wMenuItem[i] / 2;
				const int y = yMenuItems + hMenuItem * i;

				draw_font_hv(VGAScreen, x - 1, y - 1, APMenuTexts[i], normal_font, left_aligned, 15, -10);
				draw_font_hv(VGAScreen, x + 1, y + 1, APMenuTexts[i], normal_font, left_aligned, 15, -10);
				draw_font_hv(VGAScreen, x + 1, y - 1, APMenuTexts[i], normal_font, left_aligned, 15, -10);
				draw_font_hv(VGAScreen, x - 1, y + 1, APMenuTexts[i], normal_font, left_aligned, 15, -10);
				draw_font_hv(VGAScreen, x,     y,     APMenuTexts[i], normal_font, left_aligned, 15, -3);
			}

			memcpy(VGAScreen2->pixels, VGAScreen->pixels, VGAScreen2->pitch * VGAScreen2->h);

			mouseCursor = MOUSE_POINTER_NORMAL;

			// Fade in menu items.
			fade_palette(colors, 20, 255 - 16 + 1, 255);

			restart = false;
		}

		memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);

		// Highlight selected menu item.
		draw_font_hv(VGAScreen, VGAScreen->w / 2, yMenuItems + hMenuItem * selectedIndex, APMenuTexts[selectedIndex], normal_font, centered, 15, -1);

		service_SDL_events(true);

		JE_mouseStartFilter(0xF0);
		JE_showVGA();
		JE_mouseReplace();

		bool mouseMoved = false;
		do
		{
			SDL_Delay(16);

			Uint16 oldMouseX = mouse_x;
			Uint16 oldMouseY = mouse_y;

			push_joysticks_as_keyboard();
			service_SDL_events(false);

			mouseMoved = mouse_x != oldMouseX || mouse_y != oldMouseY;
		} while (!(newkey || new_text || newmouse || mouseMoved));

		// Handle interaction.

		bool action = false;
		bool done = false;

		if (mouseMoved || newmouse)
		{
			// Find menu item that was hovered or clicked.
			for (size_t i = 0; i < COUNT_MENU_ITEMS; ++i)
			{
				const int xMenuItem = xCenter - wMenuItem[i] / 2;
				if (mouse_x >= xMenuItem && mouse_x < xMenuItem + wMenuItem[i])
				{
					const int yMenuItem = yMenuItems + hMenuItem * i;
					if (mouse_y >= yMenuItem && mouse_y < yMenuItem + hMenuItem)
					{
						if (selectedIndex != i)
						{
							JE_playSampleNum(S_CURSOR);

							selectedIndex = i;
						}

						if (newmouse && lastmouse_but == SDL_BUTTON_LEFT &&
						    lastmouse_x >= xMenuItem && lastmouse_x < xMenuItem + wMenuItem[i] &&
						    lastmouse_y >= yMenuItem && lastmouse_y < yMenuItem + hMenuItem)
						{
							action = true;
						}

						break;
					}
				}
			}
		}

		if (newmouse)
		{
			if (lastmouse_but == SDL_BUTTON_RIGHT)
			{
				JE_playSampleNum(S_SPRING);

				done = true;
			}
		}
		else if (newkey)
		{
			switch (lastkey_scan)
			{
			case SDL_SCANCODE_UP:
			{
				JE_playSampleNum(S_CURSOR);

				selectedIndex = selectedIndex == 0
					? COUNT_MENU_ITEMS - 1
					: selectedIndex - 1;
				break;
			}
			case SDL_SCANCODE_DOWN:
			{
				JE_playSampleNum(S_CURSOR);

				selectedIndex = selectedIndex == COUNT_MENU_ITEMS - 1
					? 0
					: selectedIndex + 1;
				break;
			}
			case SDL_SCANCODE_SPACE:
			case SDL_SCANCODE_RETURN:
			{
				action = true;
				break;
			}
			case SDL_SCANCODE_ESCAPE:
			{
				JE_playSampleNum(S_SPRING);

				done = true;
			}
			default:
				break;
			}
		}

		if (action && selectedIndex < COUNT_MENU_ITEMS)
		{
			JE_playSampleNum(S_SELECT);
			fade_black(15);

			switch (selectedIndex)
			{
				case MENU_ITEM_START_GAME:
					if (ap_connectScreen())
						return true;
					restart = true;
					break;
				case MENU_ITEM_OPTIONS:
					setupMenu();
					restart = true;
					break;
				case MENU_ITEM_JUKEBOX:
					jukebox();
					restart = true;
					break;
				case MENU_ITEM_QUIT:
					return false;
			}
		}

		if (done)
		{
			fade_black(15);

			return false;
		}
	}
}

// ----------------------------------------------------------------------------

void ap_itemScreen(void)
{
	if (shopSpriteSheet.data == NULL)
		JE_loadCompShapes(&shopSpriteSheet, '1');

	JE_loadPic(VGAScreen2, 13, false);
	play_song(DEFAULT_SONG_BUY);

	VGAScreen = VGAScreenSeg;

	while (true)
	{
		service_SDL_events(true);
		push_joysticks_as_keyboard();

		if (newkey && lastkey_scan == SDL_SCANCODE_ESCAPE)
		{
			JE_playSampleNum(S_SPRING);
			fade_black(15);
			return;
		}

		JE_showVGA();
		SDL_Delay(16);
		memcpy(VGAScreen->pixels, VGAScreen2->pixels, VGAScreen->pitch * VGAScreen->h);
	}
}
