
#include "SDLInput.h"
#include "Application/Model/Config.h"

int keyMapping[] = {SDLK_a,    SDLK_s,     SDLK_LEFT,  SDLK_RIGHT, SDLK_UP,
                    SDLK_DOWN, SDLK_RCTRL, SDLK_LCTRL, SDLK_SPACE};

int eventMapping[] = {SDLI_BUTTON_A,     SDLI_BUTTON_B,  SDLI_BUTTON_LEFT,
                      SDLI_BUTTON_RIGHT, SDLI_BUTTON_UP, SDLI_BUTTON_DOWN,
                      SDLI_BUTTON_L,     SDLI_BUTTON_R,  SDLI_BUTTON_START};

enum MappingType { MT_KEY, MT_JOYBUTTON, MT_JOYHAT, MT_JOYAXES };

int keyMappingType[9];

SDLInput::SDLInput() {

  // open first joystick if available

  if (SDL_NumJoysticks() > 0) {
    // Open joystick
    joystick_ = SDL_JoystickOpen(0);
    Trace::Debug("Number of axis:%d", SDL_JoystickNumAxes(joystick_));
    Trace::Debug("Number of buttons:%d", SDL_JoystickNumButtons(joystick_));
    Trace::Debug("Number of hats:%d", SDL_JoystickNumHats(joystick_));
  } else {
    joystick_ = 0;
  };
  Trace::Debug("Joystick: %s", joystick_ ? "opened" : "not found");

  // build translation table
  for (int i = 0; i < SDLK_LAST; i++) {
    keyname_[i] = SDL_GetKeyName((SDLKey)i);
  };
  // initialises default mapping table
  for (int i = 0; i < 9; i++) {
    keyMappingType[i] = MT_KEY;
  };
};

void SDLInput::ReadConfig() {

  mapKey(0, "KEY_A");
  mapKey(1, "KEY_B");
  mapKey(2, "KEY_LEFT");
  mapKey(3, "KEY_RIGHT");
  mapKey(4, "KEY_UP");
  mapKey(5, "KEY_DOWN");
  mapKey(6, "KEY_LSHOULDER");
  mapKey(7, "KEY_RSHOULDER");
  mapKey(8, "KEY_START");
}

void SDLInput::mapKey(int index, const char *keyname) {

  Config *config = Config::GetInstance();

  // Read the configuration file and look if we got a definition
  const char *key = config->GetValue(keyname);
  if (key) {
    // we found a key, let's find which type it is

    MappingType type = MT_KEY;
    if (key[3] == ':') {
      if (tolower(key[0]) == 'b') { // button
        type = MT_JOYBUTTON;
      }
      if (tolower(key[0]) == 'h') { // hat
        type = MT_JOYHAT;
      }
      if (tolower(key[0]) == 'j') { // joy
        type = MT_JOYAXES;
      }
    };

    keyMappingType[index] = type;
    switch (type) {
    case MT_KEY:
      for (int i = 0; i < SDLK_LAST; i++) {
        if (!strcmp(key, keyname_[i])) {
          keyMapping[index] = i;
          break;
        };
      };
      break;
    case MT_JOYBUTTON:
      keyMapping[index] = atoi(key + 4) - 1;
      break;
    case MT_JOYHAT: {
      int hat = atoi(key + 4) - 1;
      if (joystick_) {
        if (hat < SDL_JoystickNumHats(joystick_)) {
          switch (index) {
          case 2:
            keyMapping[index] = SDL_HAT_LEFT + hat * 100;
            break;
          case 3:
            keyMapping[index] = SDL_HAT_RIGHT + hat * 100;
            break;
          case 4:
            keyMapping[index] = SDL_HAT_UP + hat * 100;
            break;
          case 5:
            keyMapping[index] = SDL_HAT_DOWN + hat * 100;
            break;
          };
        };
      }
      break;
    }
    case MT_JOYAXES: {
      int axes = atoi(key + 4) - 1;
      if (joystick_) {
        if (axes < SDL_JoystickNumAxes(joystick_)) {
          switch (index) {
          case 2:
            keyMapping[index] = -20000 + axes * 100000;
            break;
          case 3:
            keyMapping[index] = 20000 + axes * 100000;
            break;
          case 4:
            keyMapping[index] = -20000 + axes * 100000;
            break;
          case 5:
            keyMapping[index] = 20000 + axes * 100000;
            break;
          };
        }
      }
    }
    };
  }
};

unsigned short SDLInput::GetButtonMask() {
  unsigned short mask = 0;
  Uint8 *keystate = SDL_GetKeyState(NULL);
  for (int i = 0; i < 9; i++) {
    switch (keyMappingType[i]) {
    case MT_KEY:
      mask |= (keystate[keyMapping[i]]) * eventMapping[i];
      break;
    case MT_JOYBUTTON:
      if (joystick_) {
        mask |=
            SDL_JoystickGetButton(joystick_, keyMapping[i]) * eventMapping[i];
      }
      break;
    case MT_JOYHAT:
      if (joystick_) {
        int hat = keyMapping[i] / 100;
        int mask = keyMapping[i] - 100 * hat;
        Uint8 state = SDL_JoystickGetHat(joystick_, hat);
        mask |= (state & mask) * eventMapping[i];
      }
      break;
    case MT_JOYAXES:
      if (joystick_) {
        int axes = (keyMapping[i] + 20000) / 100000;
        int trigger = keyMapping[i] - 100000 * axes;
        Sint16 position = SDL_JoystickGetAxis(joystick_, axes);
        if (trigger < 0) {
          if (position < trigger) {
            mask |= eventMapping[i];
          }
        } else {
          if (position > trigger) {
            mask |= eventMapping[i];
          }
        }
      }
      break;
    default:
      break;
    };
  }
  return mask;
};
