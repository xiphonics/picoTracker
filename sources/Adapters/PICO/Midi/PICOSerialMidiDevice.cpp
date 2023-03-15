
#include "PICOSerialMidiDevice.h"
#include "System/Console/Trace.h"

#include "pico/stdlib.h"

#define UART_ID uart1
#define BAUD_RATE 31250
#define UART_TX_PIN 8
#define UART_RX_PIN 9

PICOSerialMidiOutDevice::PICOSerialMidiOutDevice(const char *name,
                                                 uart_inst_t *port)
    : MidiOutDevice(name) {
  port_ = port;
}

bool PICOSerialMidiOutDevice::Init() {

  // TODO: Move this to platform init
  // Set the TX and RX pins by using the function select on the GPIO
  // Set datasheet for more information on function select
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  // Set up our UART with the required speed.
  uint baudrate = uart_init(port_, BAUD_RATE);
  Trace::Log("MIDI", "Init MIDI device with %i baud rate", baudrate);

  return true;
}

void PICOSerialMidiOutDevice::Close(){};

bool PICOSerialMidiOutDevice::Start() { return true; };

void PICOSerialMidiOutDevice::Stop() {}

void PICOSerialMidiOutDevice::SendMessage(MidiMessage &msg) {
  uart_putc_raw(port_, msg.status_);

  if (msg.status_ < 0xF0) {
    uart_putc_raw(port_, msg.data1_);
    if (msg.data2_ != MidiMessage::UNUSED_BYTE) {
      uart_putc_raw(port_, msg.data2_);
    }
  }
}
