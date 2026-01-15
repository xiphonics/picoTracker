#include "doctest/doctest.h"

#include "Foundation/Types/Types.h"

TEST_CASE("FourCC stable enum values") {
  CHECK(FourCC::InstrumentCommandArpeggiator == 0);
  CHECK(FourCC::VarTempo == 33);
  CHECK(FourCC::VarScaleRoot == 162);
}
