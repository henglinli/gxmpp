/*
 * txmpp
 * Copyright 2010, Silas Sewell
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

#include <iostream>
#include <sstream>

#include "talk/base/logging.h"
#include "talk/xmpp/constants.h"
#include "talk/xmpp/xmppclient.h"
#include "xmpptasks.h"

namespace hello {

/**
 * Example message handler
 */

XmppTaskMessage::XmppTaskMessage(buzz::XmppTaskParentInterface *parent)
    : buzz::XmppTask(parent, buzz::XmppEngine::HL_TYPE) {}

XmppTaskMessage::~XmppTaskMessage() {}

int XmppTaskMessage::ProcessStart() {
  return STATE_RESPONSE;
}

int XmppTaskMessage::ProcessResponse() {

  const buzz::XmlElement* stanza = NextStanza();
  if (stanza == NULL) {
    return STATE_BLOCKED;
  }

  std::string from = "Someone";
  std::string text = "something too quiet to hear.";

  if (stanza->HasAttr(buzz::QN_FROM))
    from = stanza->Attr(buzz::QN_FROM);

  const buzz::XmlElement *body_stanza = stanza->FirstNamed(buzz::QN_BODY);
  if (body_stanza != NULL) {
    text = body_stanza->BodyText();
  }

  std::cout << "Message: " << from << " says " << text << std::endl;

  // Send response message
  if (from != "Someone") {

    std::stringstream response_text;
    response_text << "The thing you said was " << text.length()
                  << " characters long.";

    talk_base::scoped_ptr<buzz::XmlElement> message(
        new buzz::XmlElement(buzz::QN_MESSAGE));
    message->SetAttr(buzz::QN_TO, from);

    buzz::XmlElement *body = new buzz::XmlElement(buzz::QN_BODY);
    body->SetBodyText(response_text.str());

    message->AddElement(body);

    SendStanza(message.get());
  }

  return STATE_RESPONSE;
}

bool XmppTaskMessage::HandleStanza(const buzz::XmlElement *stanza) {

  if (stanza->Name() == buzz::QN_MESSAGE) {
    QueueStanza(stanza);
    return true;
  }

  return false;
}

/**
 * Example presence handler
 */

XmppTaskPresence::XmppTaskPresence(buzz::XmppTaskParentInterface *parent)
    : buzz::XmppTask(parent, buzz::XmppEngine::HL_TYPE) {}

XmppTaskPresence::~XmppTaskPresence() {}

int XmppTaskPresence::ProcessStart() {

  talk_base::scoped_ptr<buzz::XmlElement> presence(
      new buzz::XmlElement(buzz::QN_PRESENCE));
  SendStanza(presence.get());

  return STATE_RESPONSE;
}

int XmppTaskPresence::ProcessResponse() {

  const buzz::XmlElement* stanza = NextStanza();
  if (stanza == NULL) {
    return STATE_BLOCKED;
  }

  std::string from = "Someone";

  if (stanza->HasAttr(buzz::QN_FROM))
    from = stanza->Attr(buzz::QN_FROM);

  std::cout << "Presence: " << from << std::endl;

  return STATE_RESPONSE;
}

bool XmppTaskPresence::HandleStanza(const buzz::XmlElement *stanza) {

  if (stanza->Name() == buzz::QN_PRESENCE) {
    QueueStanza(stanza);
    return true;
  }

  return false;
}

/**
 * Example iq handler
 */

XmppTaskIq::XmppTaskIq(buzz::XmppTaskParentInterface *parent)
    : buzz::XmppTask(parent, buzz::XmppEngine::HL_SINGLE) {}

XmppTaskIq::~XmppTaskIq() {}

int XmppTaskIq::ProcessStart() {

  set_task_id(GetClient()->NextId());

  talk_base::scoped_ptr<buzz::XmlElement> privacy(
						  MakeIq("get", buzz::Jid(), task_id()));
  buzz::XmlElement *query = new buzz::XmlElement(buzz::QN_PRIVACY_QUERY);
  privacy->AddElement(query);

  SendStanza(privacy.get());

  return STATE_RESPONSE;
}

int XmppTaskIq::ProcessResponse() {

  const buzz::XmlElement* stanza = NextStanza();
  if (stanza == NULL) {
    return STATE_BLOCKED;
  }

  std::string from = "Someone";

  if (stanza->HasAttr(buzz::QN_FROM))
    from = stanza->Attr(buzz::QN_FROM);

  std::cout << "Iq: " << from << std::endl;

  return STATE_RESPONSE;
}

bool XmppTaskIq::HandleStanza(const buzz::XmlElement *stanza) {

  if (MatchResponseIq(stanza, buzz::Jid(), task_id())) {
    QueueStanza(stanza);
    return true;
  }

  return false;
}

}  // namespace hello
