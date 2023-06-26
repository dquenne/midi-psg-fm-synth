
#include "Delay.h"
#include <MIDI.h>
#include <midi_Defs.h>

MessageQueueNode::MessageQueueNode(byte _messageType, byte _channel,
                                   byte _data1, byte _data2,
                                   unsigned long _play_at) {
  messageType = _messageType;
  channel = _channel;
  data1 = _data1;
  data2 = _data2;
  play_at = _play_at;
  next = 0;
}

MidiDelay::MidiDelay() {
  _message_queue_end = 0;
  _message_queue = 0;
  _ticks_elapsed = 0;
}

void MidiDelay::enqueue(byte messageType, byte channel, byte data1, byte data2,
                        unsigned long delay_ticks) {
  MessageQueueNode *new_node = new MessageQueueNode(
      messageType, channel, data1, data2, _ticks_elapsed + delay_ticks);

  if (_message_queue_end != 0) {
    _message_queue_end->next = new_node;
  }
  if (_message_queue == 0) {
    _message_queue = new_node;
  }
  _message_queue_end = new_node;
}

void MidiDelay::_send(MessageQueueNode *node) {
  midi::MidiType messageType = midi::MidiType(node->messageType);
  byte channel = node->channel;
  byte data1 = node->data1;
  byte data2 = node->data2;

  switch (messageType) {
  case (midi::MidiType::NoteOn):
    handleNoteOn(channel + 1, data1, data2);
    return;
  case (midi::MidiType::NoteOff):
    handleNoteOff(channel + 1, data1, data2);
    return;
  }
}

void MidiDelay::_pop() {
  _send(_message_queue);
  MessageQueueNode *old_message = _message_queue;
  if (_message_queue->next == 0) {
    _message_queue = 0;
    _message_queue_end = 0;
  } else {
    _message_queue = _message_queue->next;
  }
  free(old_message);
}

void MidiDelay::tick() {
  _ticks_elapsed++;
  if (_message_queue == 0) {
    return;
  }

  if (_ticks_elapsed >= (_message_queue->play_at)) {
    _pop();
  }
}
