/*
 * Test stub for host-only builds.
 */
#ifndef _CONFIG_H_
#define _CONFIG_H_

class Config {
public:
  static Config *GetInstance() {
    static Config instance;
    return &instance;
  }

  int GetValue(const char *key) {
    (void)key;
    return importResampler_;
  }

  static void SetImportResampler(int value) {
    GetInstance()->importResampler_ = value;
  }

private:
  int importResampler_ = 0;
};

#endif
