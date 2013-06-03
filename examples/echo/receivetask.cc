#include "talk/base/logging.h"
#include "talk/xmpp/constants.h"
#include "receivetask.h"

namespace echo {
ReceiveTask::ReceiveTask(buzz::XmppTaskParentInterface* parent) :
    buzz::ReceiveTask(parent) {
}

ReceiveTask::~ReceiveTask() {
  Stop();
}
// Return true if the stanza should be received.
bool ReceiveTask::WantsStanza(const buzz::XmlElement* stanza) {
  //LOG(LS_SENSITIVE) << "handleRecieve";
  //LOG(LS_SENSITIVE) << stanza->Str();
  // Make sure that this stanza is a message
  if (stanza->Name() != buzz::QN_MESSAGE) {
    return false;
  }
  // see if there is any body
  const buzz::XmlElement* message_body = stanza->FirstNamed(buzz::QN_BODY);
  if (message_body == NULL) {
    return false;
  }  
  return true;
}
// Process the received stanza.
void ReceiveTask::ReceiveStanza(const buzz::XmlElement* stanza) {
  const buzz::XmlElement *message_body = stanza->FirstNamed(buzz::QN_BODY);
    // Looks good, so send the message text along.
  SignalReceived(buzz::Jid(stanza->Attr(buzz::QN_FROM)),
                 buzz::Jid(stanza->Attr(buzz::QN_TO)),
                 message_body->BodyText());
}
}
