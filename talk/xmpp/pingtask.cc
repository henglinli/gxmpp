// Copyright 2011 Google Inc. All Rights Reserved.
#include "talk/xmpp/pingtask.h"

#include "talk/base/logging.h"
#include "talk/base/scoped_ptr.h"
#include "talk/xmpp/constants.h"

namespace buzz {

PingTask::PingTask(buzz::XmppTaskParentInterface* parent,
                   talk_base::MessageQueue* message_queue,
                   uint32 ping_period_millis,
                   uint32 ping_timeout_millis)
    : buzz::XmppTask(parent, buzz::XmppEngine::HL_SINGLE),
      message_queue_(message_queue),
      ping_period_millis_(ping_period_millis),
      ping_timeout_millis_(ping_timeout_millis),
      next_ping_time_(0),
      ping_response_deadline_(0) {
  ASSERT(ping_period_millis >= ping_timeout_millis);
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
}

PingTask::~PingTask() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
}

bool PingTask::HandleStanza(const buzz::XmlElement* stanza) {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  LOG(LS_SENSITIVE) << stanza->Str();
  #if 1
  if (!MatchResponseIq(stanza, Jid(STR_EMPTY), task_id())) {
    return false;
  }
  #else
  if (!MatchStanzaFrom(stanza, GetClient()->jid())) {
    return false;
  }
#endif
  if (stanza->Attr(buzz::QN_TYPE) != buzz::STR_RESULT &&
      stanza->Attr(buzz::QN_TYPE) != buzz::STR_ERROR) {
    return false;
  }

  QueueStanza(stanza);

  return true;
}

const XmlElement* PingTask::MakePingTo(const std::string& to)
{
  XmlElement* result = new XmlElement(QN_IQ);
  result->AddAttr(QN_TYPE, STR_GET);
  result->AddAttr(QN_TO, to);
  result->AddAttr(QN_ID, task_id());
  return result;
}

// This task runs indefinitely and remains in either the start or blocked
// states.
int PingTask::ProcessStart() {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  if (ping_period_millis_ < ping_timeout_millis_) {
    LOG(LS_ERROR) << "ping_period_millis should be >= ping_timeout_millis";
    return STATE_ERROR;
  }
  const buzz::XmlElement* stanza = NextStanza();
  if (stanza != NULL) {
    // Received a ping response of some sort (don't care what it is).
    ping_response_deadline_ = 0;
  }

  uint32 now = talk_base::Time();

  // If the ping timed out, signal.
  if (ping_response_deadline_ != 0 && now >= ping_response_deadline_) {
    SignalTimeout();
    LOG(LS_SENSITIVE) << "Ping Timeout";
    return STATE_ERROR;
  }
  LOG(LS_SENSITIVE) << "now:" << now << " next_ping_time_:" << next_ping_time_;
  // Send a ping if it's time.
  if (now >= next_ping_time_) {
    talk_base::scoped_ptr<buzz::XmlElement> stanza(
        #if 1
        MakeIq(buzz::STR_GET, Jid(STR_EMPTY), task_id()));
        #else
     // add by henglinli@gmail.com
     // fix ping without jid
        MakePingTo(GetClient()->jid().domain()));
        #endif
    stanza->AddElement(new buzz::XmlElement(QN_PING, true));
    SendStanza(stanza.get());
   
    LOG(LS_SENSITIVE) << now << " Send Ping ......";
   
    ping_response_deadline_ = now + ping_timeout_millis_;
    next_ping_time_ = now + ping_period_millis_;

    // Wake ourselves up when it's time to send another ping or when the ping
    // times out (so we can fire a signal).
    message_queue_->PostDelayed(ping_timeout_millis_, this);
    message_queue_->PostDelayed(ping_period_millis_, this);
  }
  return STATE_BLOCKED;
}

void PingTask::OnMessage(talk_base::Message* msg) {
  LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
  // Get the task manager to run this task so we can send a ping or signal or
  // process a ping response.
  Wake();
}

} // namespace buzz
