#ifndef Delay_h
#define Delay_h

#include <Arduino.h>

class MessageQueueNode {
public:
  MessageQueueNode(byte _messageType, byte _channel, byte _data1, byte _data2,
                   unsigned long _play_at);
  byte messageType;
  byte channel;
  byte data1;
  byte data2;
  unsigned long play_at;
  struct MessageQueueNode *next;
};

class MidiDelay {
public:
  MidiDelay();
  void enqueue(byte messageType, byte channel, byte data1, byte data2,
               unsigned long delay_ticks);
  void tick();
  void (*handleNoteOn)(byte channel, byte pitch, byte velocity);
  void (*handleNoteOff)(byte channel, byte pitch, byte velocity);
  // MidiManager *midi_manager;

private:
  void _send(MessageQueueNode *node);
  void _pop();
  unsigned long _ticks_elapsed;
  MessageQueueNode *_message_queue;
  MessageQueueNode *_message_queue_end;
};

#endif
