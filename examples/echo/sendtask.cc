/*
 * gxmpp
 * Copyright 2013, Henry Lee <henglinli@gmail.com>.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "talk/xmpp/constants.h"
#include "talk/xmpp/xmppengine.h"
#include "sendtask.h"
namespace echo {
SendTask::SendTask(XmppTaskParentInterface* parent)
    : XmppTask(parent) {
}

SendTask::~SendTask() {
  Stop();
}

buzz::XmppReturnStatus SendTask::Send(const buzz::Jid& to,
                                const std::string& textmessage) {
  // Make sure we are actually connected.
  if (GetState() != STATE_INIT && GetState() != STATE_START) {
    return buzz::XMPP_RETURN_BADSTATE;
  }
  // Put together the chat stanza...
  buzz::XmlElement* message_stanza = new buzz::XmlElement(buzz::QN_MESSAGE);

  // ... and specify the required attributes...
  message_stanza->AddAttr(buzz::QN_TO, to.Str());
  message_stanza->AddAttr(buzz::QN_TYPE, "chat");
  message_stanza->AddAttr(buzz::QN_LANG, "en");

  // ... and fill out the body.
  buzz::XmlElement* message_body = new buzz::XmlElement(buzz::QN_BODY);
  message_body->AddText(textmessage);
  message_stanza->AddElement(message_body);

  // Now queue it up.
  QueueStanza(message_stanza);
  delete message_stanza;
  return buzz::XMPP_RETURN_OK;
}

int SendTask::ProcessStart() {
  const buzz::XmlElement* stanza = NextStanza();
  if (stanza == NULL) {
    return STATE_BLOCKED;
  }

  if (SendStanza(stanza) != buzz::XMPP_RETURN_OK) {
    return STATE_ERROR;
  }

  return STATE_START;
}
}
