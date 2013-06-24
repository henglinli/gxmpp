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
#include "talk/base/logging.h"
#include "rosterhandler.h" 

 namespace echo {
 	RosterHandler::RosterHandler(){};
 	RosterHandler::~RosterHandler(){};

  //! A request for a subscription has come in.
  //! Typically, the UI will ask the user if it is okay to let the requester
  //! get presence notifications for the user.  The response is send back
  //! by calling ApproveSubscriber or CancelSubscriber.
 	void RosterHandler::SubscriptionRequest(buzz::XmppRosterModule* roster,
 		const buzz::Jid& requesting_jid,
 		buzz::XmppSubscriptionRequestType type,
 		const buzz::XmlElement* raw_xml) {
 		LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
 	}
  //! Some type of presence error has occured
 	void RosterHandler::SubscriptionError(buzz::XmppRosterModule* roster,
 		const buzz::Jid& from,
 		const buzz::XmlElement* raw_xml) {
 		LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
 	}

 	void RosterHandler::RosterError(buzz::XmppRosterModule* roster,
 		const buzz::XmlElement* raw_xml) {
 		LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
 	}

  //! New presence information has come in
  //! The user is notified with the presence object directly.  This info is also
  //! added to the store accessable from the engine.
 	void RosterHandler::IncomingPresenceChanged(buzz::XmppRosterModule* roster,
 		const buzz::XmppPresence* presence) {
 		LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
 	}

  //! A contact has changed
  //! This indicates that the data for a contact may have changed.  No
  //! contacts have been added or removed.
 	void RosterHandler::ContactChanged(buzz::XmppRosterModule* roster,
 		const buzz::XmppRosterContact* old_contact,
 		size_t index) {
 		LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
 	}

  //! A set of contacts have been added
  //! These contacts may have been added in response to the original roster
  //! request or due to a "roster push" from the server.
 	void RosterHandler::ContactsAdded(buzz::XmppRosterModule* roster,
 		size_t index, size_t number) {
 		LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
 	}

  //! A contact has been removed
  //! This contact has been removed form the list.
 	void RosterHandler::ContactRemoved(buzz::XmppRosterModule* roster,
 		const buzz::XmppRosterContact* removed_contact,
 		size_t index) {
 		LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
 	}
 } 