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
#ifndef _ECHO_ROSTERHANDLER_H_
#define _ECHO_ROSTERHANDLER_H_

#include "talk/xmllite/xmlelement.h"
#include "talk/xmpp/jid.h"
#include "talk/xmpp/xmppengine.h" 
#include "talk/xmpp/rostermodule.h"

namespace echo {
	class RosterHandler : public buzz::XmppRosterHandler
	{
	public:
		RosterHandler();
		virtual ~RosterHandler();

  //! A request for a subscription has come in.
  //! Typically, the UI will ask the user if it is okay to let the requester
  //! get presence notifications for the user.  The response is send back
  //! by calling ApproveSubscriber or CancelSubscriber.
  virtual void SubscriptionRequest(buzz::XmppRosterModule* roster,
                                   const buzz::Jid& requesting_jid,
                                   buzz::XmppSubscriptionRequestType type,
                                   const buzz::XmlElement* raw_xml);

  //! Some type of presence error has occured
  virtual void SubscriptionError(buzz::XmppRosterModule* roster,
                                 const buzz::Jid& from,
                                 const buzz::XmlElement* raw_xml);

  virtual void RosterError(buzz::XmppRosterModule* roster,
                           const buzz::XmlElement* raw_xml);

  //! New presence information has come in
  //! The user is notified with the presence object directly.  This info is also
  //! added to the store accessable from the engine.
  virtual void IncomingPresenceChanged(buzz::XmppRosterModule* roster,
                                       const buzz::XmppPresence* presence);

  //! A contact has changed
  //! This indicates that the data for a contact may have changed.  No
  //! contacts have been added or removed.
  virtual void ContactChanged(buzz::XmppRosterModule* roster,
                              const buzz::XmppRosterContact* old_contact,
                              size_t index);

  //! A set of contacts have been added
  //! These contacts may have been added in response to the original roster
  //! request or due to a "roster push" from the server.
  virtual void ContactsAdded(buzz::XmppRosterModule* roster,
                             size_t index, size_t number);

  //! A contact has been removed
  //! This contact has been removed form the list.
  virtual void ContactRemoved(buzz::XmppRosterModule* roster,
                              const buzz::XmppRosterContact* removed_contact,
                              size_t index); 
	};
} 
#endif // _ECHO_ROSTERHANDLER_H_